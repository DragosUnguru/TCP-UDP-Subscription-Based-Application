#include "subscriber_utils.h"

void usage(int argc, char* argv[])
{
    if (argc < 4) {
		fprintf(stderr, "Usage: %s <subscriber_ID> <server_address> <server_port>\n", argv[0]);
	    exit(0);
	}
    if (strlen(argv[1]) > ID_LEN - 1) {
        fprintf(stderr, "Maximum ID length is 10. Please pick another name and retry.\n");
        exit(0);
    }
}

std::vector<std::string> tokenize(char* in) {
    std::vector<std::string> tokens;

    char* p = strtok(in, " ");
    while (p != NULL) {
        tokens.push_back(std::string(p));
        p = strtok(NULL, " ");
    }

    return tokens;
}

int check_input(char* in, std::vector<std::string>& sub_topics) {
    const char use_sub[37] = "Usage: subscribe <topic> <SF={0, 1}>";
    const char use_unsub[27] = "Usage: unsubscribe <topic>";
    const char invalid[30] = "Invalid number of arguments.";

    if (!strncmp(in, "exit", 4)) {
        return EXIT_SIG;
    }
    std::vector<std::string> tokens = tokenize(in);

    if (!strncmp(in, "subscribe", 9)) {
        if (tokens.size() != 3) {
            printf("%s. %s\n", invalid, use_sub);
            return ERR_IN;
        }

        int sf = tokens[2][0] - '0';
        std::string topic = tokens[1];

        if ((sf != 0 && sf != 1) || (tokens[2].size() > 2)) {
            printf("Invalid SF value. %s\n", use_sub);
            return ERR_IN;
        }

        for (std::string top : sub_topics) {
            if (top == topic) {
                std::cout << "You're already subscribed to topic \"" << topic << "\".\n";
                return ERR_IN;
            }
        }
        sub_topics.push_back(topic);
        return SUBSCRIBED;
    }

    if (!strncmp(in, "unsubscribe", 11)) {
        if (tokens.size() != 2) {
            printf("%s %s\n", invalid, use_unsub);
            return ERR_IN;
        }

        std::string topic = tokens[1];
        // Remove null terminator for correct comparasion
        topic = topic.substr(0, topic.size() - 1);
        std::vector<std::string>::iterator it = sub_topics.begin();

        while (it != sub_topics.end()) {
            if (*it == topic) {
                // Valid topic to unsubscribe from. Remove
                sub_topics.erase(it);
                return UNSUBSCRIBED;
            }
            ++it;
        }
        // If we're here, the topic wasn't found
        std::cout << "You're not subscribed to topic \"" << topic << "\". Subscribe to unsubscribe.\n";
        return ERR_IN;
    }
    
    printf("Invalid command! Valid commands:\n-> %s\n-> %s\n-> exit\n", use_sub + 7, use_unsub + 7);
    return ERR_IN;
}

void print_feedback(int ret, std::string topic) {
    switch (ret) {
    case EXIT_SIG:
        std::cout << "Received \"exit\" input. Terminating...\n";
        break;
    case SUBSCRIBED:
        std::cout << "Successfully subscribed to topic " << topic << ".\n";
        break;
    case UNSUBSCRIBED:
        std::cout << "Successfully unsubscribed to topic " << topic << ".\n";
        break;
    }
}

void parse_message(msg_to_tcp* msg) {
    char parsed_type[32];
    int parsed_payload;
    
    printf("%s:%d - %s", msg->udp_ip, msg->udp_port, msg->topic);
    switch (msg->type) {
    case UINT: {
        parsed_payload = ntohl(*(uint32_t*) (msg->payload + 1));
        printf(" - INT - ");
        printf((msg->payload[0]) ? "-%u\n" : "%u\n", parsed_payload);
    } break;
    case SHORT_FLOAT: {
        float parsed = ntohs(*(uint16_t*) (msg->payload)) / 100;
        printf(" - SHORT REAL - %.2f\n", parsed);
    } break;
    case FLOAT: {
        parsed_payload = ntohl(*(uint32_t*) (msg->payload + 1));
        int exp = pow(10, msg->payload[5]);

        printf(" - FLOAT - ");
        printf((msg->payload[0]) ? "-%u.%u\n" : "%u.%u\n", parsed_payload / exp, parsed_payload % exp);
    } break;
    case STRING:
        printf(" - STRING - %s\n", msg->payload);
        break;
    }
}