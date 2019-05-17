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
void usage(int argc, char* argv[]);

/**
 * Tokenizes a string by spaces (" ") and returns
 * a vector of strings containing every word
*/
std::vector<std::string> tokenize(char* in);

/**
 * Checks if the input is valid and returns an
 * error code accordingly.
*/
int check_input(char* in, std::vector<std::string>& sub_topics);

/**
 * After receiveing a correct input, print to
 * stdout command feedback.
*/
void print_feedback(int ret, std::string topic);

/**
 * After receiveing a message from the server (from
 * the UDP client), parse it to human-readable text.
*/
void parse_message(msg_to_tcp* msg);