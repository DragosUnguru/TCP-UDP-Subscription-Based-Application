#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <unordered_map>

#define MAX(a, b) ((a < b) ? b : a)

/*
	Maximum lengths
*/
#define BUFLEN 1571
#define PAYLOAD_LEN 1500
#define ID_LEN 11
#define TOPIC_NAME 50
#define MAX_CLIENTS 0x7FFFFFFF

/*
	Personal error codes
*/
#define INPUT_OK 7
#define ERR_IN -1
#define EXIT_SIG -2
#define SUBSCRIBED -3
#define UNSUBSCRIBED -4

/*
	Topic types
*/
#define UINT 0
#define SHORT_FLOAT 1
#define FLOAT 2
#define STRING 3

struct subscriber {
	std::string id;
	std::vector<std::string> topics;

	subscriber(char* _id): id(std::string(_id)) {}
	subscriber(std::string _id): id(_id) {}
};

struct msg_udp {
	char topic[TOPIC_NAME];
	char type;
	char payload[PAYLOAD_LEN];
};

struct __attribute__((packed)) msg_to_tcp {
	char payload[PAYLOAD_LEN];
	char topic[TOPIC_NAME];
	char udp_ip[16];
	char type;
	int udp_port;
};


/*
 * Error checking macro
 * Eg:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)
