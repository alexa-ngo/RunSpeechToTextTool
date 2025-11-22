#include "server.h"

/*
 *  This is a server that reads in data from a client.
 *          Usage: ./server <port_num>
 *
 *  To compile the program:
 *         gcc -ljson-c server_driver.c server.c minimal_multi_parser.c -o the_program
           ./the_program 1234
 */

/* Global variable declared in main */
int listener_d;

/* Catch the signal */
int catch_signal(int sig, void (*handler)(int)) {

    struct sigaction action;
    action.sa_handler = handler;
    /* Use an empty mask */
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction (sig, &action, NULL);
}

/* Handles the shutdown process */
void handle_shutdown(int sig) {
    if (listener_d) {
        close(listener_d);
    }
    printf("Bye!\n");
    exit(0);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {     // argv[0]    argv[1]
        printf("Usage: <program_name> <port>\n");
        exit(1);
    }

    const char* port = argv[1];
    int port_num = atoi(port);
    char buf[BYTES_OF_DATA_100000];
	int count_of_num_chars = 0;

    /* Calls handle_shutdown() if CTRL-C is hit */
    if (catch_signal(SIGINT, handle_shutdown) == DOES_NOT_EXIST) {
        fprintf(stderr, "Can't set the interrupt handler");
        exit(1);
    }

    /* Listen and bind to a port */
    int listener_d = open_listener_socket();
    bind_to_port(listener_d, port_num);
    if (listen(listener_d, MAX_5_NUM_OF_RUNNING_PROCESSES) == DOES_NOT_EXIST) {
        fprintf(stderr, "Can't listen");
        exit(1);
    }

    puts("Waiting for connection");
    struct sockaddr_storage client_addr;
    unsigned int address_size = sizeof(client_addr);

    /* Loop to accept data */
    while (1) {
        int connect_d = accept(listener_d, (struct sockaddr *)&client_addr, &address_size);
        if (connect_d == -1) {
            fprintf(stderr, "Can't open secondary socket using the accept method\n");
            continue;
        }

        int child_pid = fork();
        if (child_pid == -1) {
            fprintf(stderr, "Could not fork the child\n");
            exit(1);
        }
        if (child_pid > 0) {
            fprintf(stderr, "Parent is closing child socket. Child PID: %d\n\n", child_pid);
            /* Parent is closing the CLIENT SOCKET */
            close(connect_d);
        } else {

            void child_process_handles_request(listener_d, connect_d, buf);

            } else {
                // Returns the HTTP/1.1 400 Error Message
                char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.'";
                int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
                if (send_400_error_code == DOES_NOT_EXIST) {
                    fprintf(stderr, "Error in sending\n");
                    exit(1);
                }
            }
            // Close the connection
			printf("Closing the connection now!\n");
            close(connect_d);
            exit(0);
        }
    }
}