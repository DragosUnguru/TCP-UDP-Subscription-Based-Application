
CFLAGS = -Wall -Wextra -g

# Listening server port
PORT = 3010

# Server IP address: LOCAL HOST
IP_SERVER = 127.0.0.1

# For cliend ID
i := $(shell bash -c 'echo $$RANDOM')

all: server subscriber

# Compile server.cpp
server: server.cpp server_utils.cpp

# Compile subscriber.cpp
subscriber: subscriber.cpp subscriber_utils.cpp

.PHONY: clean run_server run_subscriber

# Run server
run_server:
	./server ${PORT}

# Run TCP client
run_subscriber:
	./subscriber clnt_$i ${ID_SUBSCRIBER} ${IP_SERVER} ${PORT}

# Run UDP client
run_udp:
	python3 udp_client.py ${IP_SERVER} ${PORT} --mode manual

clean:
	rm -f server subscriber
