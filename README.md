
#  RUNNING THE PROGRAM

	!!! MIGTH NEED CMAKE TO COMPILE !!!
    	sudo apt-get install cmake
    
    The makefile automatically generates a random ID for
every other TCP subscriber.
    The IP is automatically set to LOCAL_HOST (127.0.0.1).

    - Compile with:        make
    - Run server with:     make run_server
    - Run TCP client with: make run_subscriber
    - Run UDP client with: make run_udp
            (set to manual mode)
    - Clean:               make clean


#  BRIEF INTRODUCTION

    A subscriber-based communication application between
multiple clients.
    A reliable implementation of the transport level
using a server which takes information from a UDP client
and sends it to a TCP subscriber.

#  CLIENT:

    The TCP client is implemented as simple as possible.
It's only job is to multiplex between the stdin and the 
socket through which it communiates with the server, 
check for user error and decode the received message from
the server.
    Accepted commands:
->  exit
->  subscribe <topic_name> <SF_FLAG>    <-implemented a store & forward feature
->  unsubscribe <topic_name>

->  SERVER:

    The server multiplexes between:
- stdin input
- UDP client message
- TCP client connection / request

    Its job is to parse every message received from
the UDP client, categorize them by topic / tcp clients
subscribed to, and forward them to the according subscribers
of the topic.
    It also implements a "Store & Forward" feature. This is the
reason it buffers the messages received from the UDP clients.
In case a subscriber sets the SF flag to 1, it demands all the
messages sent by the time of his request.


    This was implemented in a hurry and C++ was abused in some places.
Changes may come in the future.