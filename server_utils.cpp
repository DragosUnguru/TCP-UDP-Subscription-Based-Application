#include "server_utils.h"

void usage_server(char *file)
{
	fprintf(stderr, "Usage: %s <server_port>\n", file);
	exit(0);
}

void check_topic(std::unordered_map<std::string, std::vector<int>>& topics, const std::string& topic) {
	try {
		topics.at(topic);
	} catch (std::out_of_range e) {
		topics.insert(std::make_pair(topic, std::vector<int>()));
	}
}

void subscribe(int newsockfd, std::unordered_map<std::string, std::vector<int>>& topics, char* buffer,
				std::unordered_map<std::string, std::vector<msg_to_tcp>>& buffered_msg) {
	std::string topic(buffer + 10);
	std::string tmp(buffer + 11 + topic.size());
	int sf = tmp[0] - '0';

	try {
		topics.at(topic).push_back(newsockfd);
		if (sf == 1) {
			std::vector<msg_to_tcp> tmp = buffered_msg.at(topic);
			for (msg_to_tcp msg : tmp) {
				int ret = send(newsockfd, (char *) &msg, BUFLEN, 0);
				DIE(ret < 0, "send");
			}
		}

	} catch (std::out_of_range e) {
		topics.insert(std::make_pair(topic, std::vector<int>()));
        topics.at(topic).push_back(newsockfd);
	}
}

int check_duplicate_ID(std::unordered_map<int, subscriber>& subs, const std::string& id,
						int newsockfd, struct sockaddr_in sockaddr_TCP) {
	for (auto& it : subs) {
    	if (it.second.id == id) {
			std::cout << "There already exists a subscriber with id: " << id << "\n";

			// Terminate subscriber:
			int ret = send(newsockfd, "TERMINATE", 10, 0);
			DIE(ret < 0, "send");
			return ERR_IN;
		}
	}
	// Unique subscriber ID. Add it to our map as it is valid
    std::cout << "New client [" << id << "] connected from " << inet_ntoa(sockaddr_TCP.sin_addr)
            << ":" << ntohs(sockaddr_TCP.sin_port) << ".\n";

	subs.insert(std::make_pair(newsockfd, subscriber(id)));
	return INPUT_OK;
}

void buffer_msg(std::unordered_map<std::string, std::vector<msg_to_tcp>>& buffered_msg,
				std::string topic, const msg_to_tcp& msg) {
	try {
		buffered_msg.at(topic).push_back(msg);
	} catch (std::out_of_range e) {
		buffered_msg.insert(std::make_pair(topic, std::vector<msg_to_tcp>()));
        buffered_msg.at(topic).push_back(msg);
	}
}

void update_fdmax(int& fdmax, const fd_set& fds) {
	for (int i = fdmax; i >= 0; --i) {
        if(FD_ISSET(i, &fds)) {
            fdmax = i;
            break;
        }
    }
}

void terminate_all(fd_set *fds, int fdmax, const std::unordered_map<int, subscriber>& subscribers) {
    for (auto& it : subscribers) {
        int ret = send(it.first, "TERMINATE", 10, 0);
        DIE(ret < 0, "send");
    }

    for (int i = fdmax; i > 1; --i) {
        if (FD_ISSET(i, fds)) {
            close(i);
        }
    }
}