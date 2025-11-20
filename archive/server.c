#include "server.h"

/*
*      This is the code for the microservice for the Speech-To-Text Tool.
*/

/* Create a streaming socket */
int open_listener_socket(void) {
    int streaming_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (streaming_socket == -1) {
        fprintf(stderr, "Can't open the socket");
        exit(1);
    }
    return streaming_socket;
}

/* Bind to a port */
void bind_to_port(int socket, int port) {
    struct sockaddr_in the_socket;
    the_socket.sin_family = PF_INET;
    the_socket.sin_port = (in_port_t)htons(port);
    the_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Reuse the socket to restart the server without a problem */
    int reuse = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
        fprintf(stderr, "Can't set the reuse option on the socket");
        exit(1);
    }

    /* Bind to a socket */
    int c = bind(socket, (struct sockaddr *) &the_socket, sizeof(the_socket));
    if (c == -1) {
        fprintf(stderr, "Can't bind to socket");
        exit(1);
    }
}

/* Catch the signal */
int catch_signal(int sig, void (*handler)(int)) {

    struct sigaction action;
    action.sa_handler = handler;
    /* Use an empty mask */
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction (sig, &action, NULL);
}

/* Kill the process */
void kill_the_process(void) {
    fprintf(stderr, "Goodbye, I am ending this process now...\n");
    exit(1);
}

/* Receive the data from a socket */
int read_in(int socket, char* buf, int len) {

    char* buffer = buf;
    int slen = len;

    /* Must receive at least once to start the while loop */
    int received_data = recv(socket, buffer, slen, 0);
    while ((received_data > 0) && (buffer[received_data-1] != '\n')) {
        buffer += received_data;
        slen -= received_data;
        received_data = recv(socket, buffer, slen, 0);
    }

    if (received_data < 0) {
        return received_data;
        // Send back an empty string if there is an empty string
    } else if (received_data == 0) {
        buf[0] = '\0';
    } else {
        buffer[received_data - 1] = '\0';
    }
    return len - slen;
}
