#include "server_utils.h"

int main(int argc, char* argv[]) {

	const int portno = atoi(argv[1]);
    int sockfd_UDP, sockfd_TCP, newsockfd, flag = 1;
	char buffer[BUFLEN];
	struct sockaddr_in sockaddr_TCP, sockaddr_UDP, cli_addr;
	int ret;
	socklen_t sock_len = sizeof(struct sockaddr);
	std::unordered_map<int, subscriber> subscribers;
	std::unordered_map<std::string, std::vector<int>> topics;
	std::unordered_map<std::string, std::vector<msg_to_tcp>> buffered_msg;

	fd_set read_fds;	// read set of fds
	fd_set tmp_fds;		// temporary set. select() erases all fds
	int fdmax;			// maximum value of current fds

	if (argc < 2) {
		usage_server(argv[0]);
	}

	// Init fds and sockets
    FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd_TCP = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd_TCP < 0, "socket TCP");

	sockfd_UDP = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(sockfd_TCP < 0, "socket UDP");

	// Bind UDP socket
	memset((char *) &sockaddr_UDP, 0, sizeof(sockaddr_UDP));
	sockaddr_UDP.sin_family = AF_INET;
	sockaddr_UDP.sin_port = htons(portno);
	sockaddr_UDP.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd_UDP, (struct sockaddr *) &sockaddr_UDP, sizeof(struct sockaddr_in));
	DIE(ret < 0, "bind UDP");

	// Bind TCP socket
	memset((char *) &sockaddr_TCP, 0, sizeof(sockaddr_TCP));
	sockaddr_TCP.sin_family = AF_INET;
	sockaddr_TCP.sin_port = htons(portno);
	sockaddr_TCP.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd_TCP, (struct sockaddr *) &sockaddr_TCP, sizeof(struct sockaddr_in));
	DIE(ret < 0, "bind TCP");

	// Wait for TCP clients
	ret = listen(sockfd_TCP, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// Set sockets
	FD_ZERO(&read_fds);
    FD_SET(sockfd_TCP, &read_fds);
    FD_SET(sockfd_UDP, &read_fds);
    FD_SET(0, &read_fds);
    fdmax = MAX(sockfd_TCP, sockfd_UDP);

	while (1) {
		tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (int i = 0; i <= fdmax; i++) {
			memset(buffer, 0, BUFLEN);

			if (FD_ISSET(i, &tmp_fds)) {
				if (i == 0) {
					// Receiveing command from stdin:
					fgets(buffer, BUFLEN - 1, stdin);
					if (!strncmp(buffer, "exit", 4)) {
						terminate_all(&read_fds, fdmax, subscribers);
						goto terminate;
					} else {
						std::cout << "Only available command is \"exit\"\n";
					}
				} else if (i == sockfd_TCP) {
					/*
						Got a new TCP connection from a subscriber
						on the listening socket. Accept it
					*/
					newsockfd = accept(sockfd_TCP, (struct sockaddr *) &sockaddr_TCP, &sock_len);
					DIE(newsockfd < 0, "accept");

					// Deactivate Neagle's algorithm
                    setsockopt(newsockfd, IPPROTO_TCP, 1, (char *) &flag, sizeof(int));

					//	Fetch subscriber ID and check eligibility
					ret = recv(newsockfd, buffer, sizeof(buffer), 0);
					DIE(ret < 0, "recv");

					if (check_duplicate_ID(subscribers, std::string(buffer), newsockfd, sockaddr_TCP) == INPUT_OK) {
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}
					}
				} else if (i == sockfd_UDP) {
					// Received a new message from a UDP client
					ret = recvfrom(sockfd_UDP, buffer, BUFLEN, 0, (struct sockaddr *) &sockaddr_UDP, &sock_len);
					DIE(ret < 0, "recvfrom");

					msg_udp* recv_msg = (msg_udp *) buffer;
					
					// Update maps
					std::string topic(recv_msg->topic);
					check_topic(topics, topic);

					// Compose the message to be sent to the TCP subscriber
					msg_to_tcp msg;
					msg.type = recv_msg->type;
					msg.udp_port = ntohs(sockaddr_UDP.sin_port);
					strcpy(msg.udp_ip, inet_ntoa(sockaddr_UDP.sin_addr));
					strcpy(msg.topic, recv_msg->topic);
					memcpy(msg.payload, recv_msg->payload, PAYLOAD_LEN);

					buffer_msg(buffered_msg, topic, msg);

					// Send the new message to all subscribed clients
					for (int sub : topics.at(topic)) {
						ret = send(sub, (char *) &msg, BUFLEN, 0);
						DIE(ret < 0, "send");
					}
				} else {
					/*
						Received a new request from a subscriber
						via the TCP listening port
					*/
					ret = recv(i, buffer, sizeof(buffer), 0);
					DIE(ret < 0, "recv");

					// If the subscriber wants to disconnect
					if (ret == 0) {
						std::cout << "Client [" << subscribers.at(i).id << "] disconnected.\n";

						// Clear maps
						subscribers.erase(i);
						for (auto& it : topics) {
							it.second.erase(std::remove(it.second.begin(), it.second.end(), i), it.second.end());
						}

						FD_CLR(i, &read_fds);
						update_fdmax(fdmax, read_fds);
						close(i);
					} else {
						// Received a "subscribe" or "unsubscribe" request
						if (buffer[0] == 's') {
							// If received a subscribe request
							subscribe(i, topics, buffer, buffered_msg);
						} else {
							// If received a unsubscribe request
							std::string topic(buffer + 12);
							topic = topic.substr(0, topic.size() - 1);
							topics.at(topic).erase(std::remove(topics.at(topic).begin(), topics.at(topic).end(), i), topics.at(topic).end());
						}
					}
				}
			}
		}
	}

terminate:
	close(sockfd_TCP);
	close(sockfd_UDP);
    return 0;
}