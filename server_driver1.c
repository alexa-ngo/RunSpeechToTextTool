#include "server.h"

// Global variable declared in main
int listener_d;

// Catch the signal handle
int catch_signal(int sig, void (*handler)(int)) {

    struct sigaction action;
    action.sa_handler = handler;
    // Use an empty mask
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig, &action, NULL);
}

// Handles the shutdown process
void handle_shutdown(int sig) {
    if (listener_d) {
        close(listener_d);
    }
    printf("Bye!\n");
    exit(0);
}


// Run the server
int main(int argc, char* argv[]) {

    if (argc < 2) {    // argv[0]     argv[1]
        printf("Usage: <program_name> <port>");
        exit(1);
    }

    const char* port = argv[1];
    int port_num = atoi(port);

    // Calls handle_shutdown() if CTRL-C is hit
    if (catch_signal(SIGINT, handle_shutdown) == DOES_NOT_EXIST) {
        fprintf(stderr, "Can't see the interrupt handler");
        exit(1);
    }

    // Listen and bind to a port
    int listener_d = open_listener_socket();
    bind_to_port(listener_d, port_num);
    if (listen(listener_d, MAX_5_NUM_OF_RUNNING_PROCESSES) == DOES_NOT_EXIST) {
        fprintf(stderr, "Can't listen");
        exit(1);
    }

    puts("Waiting for connection");
    struct sockaddr_storage client_addr;
    unsligned int address_size = sizeof(client_address);

    // Loop throught the data
    /* Loop to accept data */
    while (1) {
        int connect_d = accept(listener_d, (struct sockaddr *)&client_addr, &address_size);
        if (connect_d == -1) {
            fprintf(stderr, "Can't open secondary socket using the accept method\n");
            close(connect_d);
        }

        int child_pid = fork();
        if (child_pid == -1) {
            fprintf(stderr, "Could not fork the child\n");
            close(connect_d);
            exit(1);
        }
        if (child_pid  > 0) {
            fprintf(stderr, "Parent is closing child socket. Child PID: %d\n\n", h);

    }
}