#include "server.h"

/*
 *  This is an server takes in a data stream passed from an client.
 *          Usage: ./server <port_num>
 */


// Listener_d is a global variable declared in main
int listener_d;

// Handles the shutdown process
void handle_shutdown(int sig) {
    if (listener_d) {
        close(listener_d);
    }
    printf("Bye!\n");
    exit(0);
}

int main(int argc, char* argv[]) {

    char buf[BYTES_OF_DATA_100000];
    int first_idx = 0;

    if (argc < 2) {     // argv[0]    argv[1]
        printf("Usage: <program_name> <port>\n");
        exit(1);
    }

    const char* port = argv[1];
    int port_num = atoi(port);

    // Calls handle_shutdown() if CTRL-C is hit
    if (catch_signal(SIGINT, handle_shutdown) == DOES_NOT_EXIST) {
        fprintf(stderr, "Can't set the interrupt handler");
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
    unsigned int address_size = sizeof(client_addr);

    while (1) {
        // Keep accepting data being sent in
        int connect_d = accept(listener_d, (struct sockaddr *)&client_addr, &address_size);
        if (connect_d == DOES_NOT_EXIST) {
            fprintf(stderr, "Can't open secondary socket using the accept method");
            continue;
        }

        int child_pid = fork();
        if (child_pid == DOES_NOT_EXIST) {
            fprintf(stderr, "Could not fork the child\n");
            exit(1);
        }
        if (child_pid > 0) {
            fprintf(stderr, "Parent is closing child socket. Child PID: %d\n\n", child_pid);
            // Parent is closing the CLIENT SOCKET
            close(connect_d);
        } else {
            // The process is a child process and should handle the request
            // so close the listener
            close(listener_d);

            // Clear the buffer first
            memset(buf, '\0', BYTES_OF_DATA_100000);

            // Read the client's message from the socket
            int chars_read = recv(connect_d, buf, BYTES_OF_DATA_100000, 0);
            printf("%s\n", buf);

            char api_buffer[BYTES_OF_DATA_30];
            int api_buffer_idx = 0;
            int idx = 0;
            char post_request_buffer[BYTES_OF_DATA_30];
            int post_request_idx = 0;

            // See if this is a POST request
            while (buf[idx] != ' ') {
                post_request_buffer[idx] = buf[idx];
                idx++;
            }
            printf("The post request buffer: %s\n", post_request_buffer);

            // Reset the index
            idx = 0;

            // Iterate through a buf[BYTES_OF_DATA_100000];
            // extract the /api/transcribe
            while (buf[idx] != '\0') {
                printf("%c\n", buf[idx]);
                idx++;
                if (buf[idx] == ' ') {
                    printf("%c\n");
                    idx++;
                    if (first_idx == 0) {
                        first_idx = idx;
                    }
                }
            }

            // Places /api/transcribe into the API buffer
            while ((buf[first_idx] != ' ')) {
                api_buffer[api_buffer_idx] = buf[first_idx];
                api_buffer_idx++;
                first_idx++;
            }

            // Check if the request from the client is a POST request
            // and if there is the /api/transcribe
            // Send HTTP 400 if can't use /api/transcribe
            //    Content-Type: header text/plains
            printf("122 > the api_buffer is: %s\n", api_buffer);
            //printf("");
            if ((strcmp(api_buffer, "/api/transcribe") == 0) && strcmp(post_request_buffer, "POST") == 0) {

                printf("yes, the buffer and the post request matches\n");
            } else {
                fprintf(stderr, "Error in the /api/transcribe");
            }

            if (chars_read == DOES_NOT_EXIST) {
                fprintf(stderr, "Error in receiving\n");
                close(connect_d);
                exit(1);
            }
            if (chars_read == 0) {
                fprintf(stderr, "Error. Received 0 bytes.");
                close(connect_d);
                exit(1);
            }

            char* result_str = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 5\n\nWOAAH'";

            int send_result = send(connect_d, result_str, strlen(result_str), 0);
            if (send_result == DOES_NOT_EXIST) {
                fprintf(stderr, "Error in sending\n");
                exit(1);
            }

            // Close the connection
            close(connect_d);
            exit(0);
        }
    }

}
