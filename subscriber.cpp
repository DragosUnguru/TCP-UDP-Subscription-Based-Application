#include "subscriber_utils.h"

int main(int argc, char* argv[]) {
    const int serv_port = atoi(argv[3]);
    const char* serv_IP = argv[2];
    const char* my_ID = argv[1];

    int sockfd, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
    std::vector<std::string> sub_topics;

    // Check command line input
    usage(argc, argv);

    // Init socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket subscriber");

    // Init server details
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	ret = inet_aton(serv_IP, &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

    /*
        Connect to server
    */
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

    /*
        Initialize fd set and temporary set
    */
	fd_set fds, tmp_fds;
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
    FD_SET(0, &fds);

    /*
        Send server details (ID).
    */
    sprintf(buffer, "%s", my_ID);
    ret = send(sockfd, buffer, BUFLEN, 0);
    DIE(ret < 0, "send ID");

	while (1) {
		memset(buffer, 0, BUFLEN);
        tmp_fds = fds;

		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

		if (FD_ISSET(sockfd, &tmp_fds)) {
            // Received message from server
			ret = recv(sockfd, buffer, BUFLEN, 0);
			DIE(ret < 0, "recv");
            
            // Check if server sent a terminating signal
            if (!strncmp(buffer, "TERMINATE", 9)) {
                printf("Received TERMINATE_SIG!\nProbable cause:\n-> Duplicate ID.\n-> Server closed.\nTerminating...\n");
                break;
            }
            msg_to_tcp* msg = (msg_to_tcp *) buffer;
            parse_message(msg);
		} else {
            // Read from stdin until we get a valid command
            do {
			    fgets(buffer, BUFLEN - 1, stdin);
                ret = check_input(buffer, sub_topics);
            } while (ret == ERR_IN);

            if (ret == EXIT_SIG) {
                break;
            }

            // Send request to server
            ret = send(sockfd, buffer, BUFLEN, 0);
            DIE(ret < 0, "send");

            // print_feedback(ret, tokenize(buffer)[1]);
		}
	}

	close(sockfd);
    return 0;
}