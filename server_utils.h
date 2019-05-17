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
#include "utils.h"

/**
 * Checks the command line input validity
*/
void usage_server(char *file);

/**
 * Checks the existence of "topic" in "topics". If it
 * is not found, then it's added.
*/
void check_topic(std::unordered_map<std::string, std::vector<int>>& topics, const std::string& topic);

/**
 * If received a new subscribe request through "newsockfd" socket file descriptor,
 * check the SF flag and send buffered messages accordingly. If the topic he subscribed
 * to is inexistent, initialize it and wait for UDP clients to populate it
*/
void subscribe(int newsockfd, std::unordered_map<std::string, std::vector<int>>& topics, char* buffer,
				std::unordered_map<std::string, std::vector<msg_to_tcp>>& buffered_msg);
/**
 * Checks if the newly connected subscriber has a invalid (duplicate) ID.
 * If so, terminate connection
*/
int check_duplicate_ID(std::unordered_map<int, subscriber>& subs, const std::string& id,
						int newsockfd, struct sockaddr_in sockaddr_TCP);

/**
 * When a new message from a UDP client is received, buffer it (store&forward).
*/
void buffer_msg(std::unordered_map<std::string, std::vector<msg_to_tcp>>& buffered_msg,
				std::string topic, const msg_to_tcp& msg);

/**
 * If a subscriber disconnected, update fdmax
*/
void update_fdmax(int& fdmax, const fd_set& fds);

/**
 * Closes all processes. Sends terminating signals to all subscribers
 * and closes all file descriptors
*/
void terminate_all(fd_set *fds, int fdmax, const std::unordered_map<int, subscriber>& subscribers);