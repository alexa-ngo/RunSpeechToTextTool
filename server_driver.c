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
    if (argc < 4) {    // argv[0]     argv[1] argv[2]                      argv[3]
        printf("Usage: <program_name> <port> <absolute_path_of_media_file> <output_txt_filename>");
        exit(1);
    }

    const char* port = argv[1];
	char* bash_arg1 = argv[2];
	char* bash_arg2 = argv[3];
    int port_num = atoi(port);
    char buf[BYTES_OF_DATA_100000];

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
    unsigned int address_size = sizeof(client_addr);

    // Loop through the request of the client
    while (1) {
        int connect_d = accept(listener_d, (struct sockaddr *)&client_addr, &address_size);
        if (connect_d == -1) {
            fprintf(stderr, "Can't open secondary socket using the accept method\n");
            close(connect_d);
        }
        // Fork the child process
        int child_pid = fork();
        if (child_pid == -1) {
            fprintf(stderr, "Could not fork the child\n");
            close(connect_d);
            exit(1);
        }
        if (child_pid  > 0) {
            fprintf(stderr, "Parent is closing child socket. Child PID: %d\n\n", child_pid);
            //Parent is closing the CLIENT SOCKET
            close(connect_d);
        } else {
            // This is the child process, so close the listener.
            close(listener_d);

            // Clear the buffer first
            memset(buf, '\0', BYTES_OF_DATA_100000);

            /* Read the client's message from the socket */
            int chars_read = recv(connect_d, buf, BYTES_OF_DATA_100000, 0);
            if (chars_read == 0) {
                fprintf(stderr, "Error. Received 0 bytes.");
                close(connect_d);
                exit(1);
            }

            // API Buffer. Check if the request is /api/upload
            //char* my_string = "";
            char api_buffer[BYTES_OF_DATA_30] = ""; // Initialize with double quotes
            int api_buffer_idx = 0;
            int idx = 0;

            // POST Buffer. Check if the request is a POST request
            char post_request_buffer[BYTES_OF_DATA_30];
            while (buf[idx] != ' ') {
                post_request_buffer[idx] = buf[idx];
                idx++;
            }
            // Add the null terminating character to end the string
            post_request_buffer[idx] = '\0';

            // Stop immediately after if a request is not a POST request
            char* post_char_str = malloc(10);
            char* post_str = "POST";
            strcpy(post_char_str, post_request_buffer);
            int post_result = (strcmp(post_request_buffer, post_str) == IS_TRUE);
            free(post_char_str);

            // Returns the HTTP/1.1 400 Error Message
            if (post_result == FALSE) {
                char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 25\n\nThis is a 400 POST ERROR.'";
                int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
                if (send_400_error_code == DOES_NOT_EXIST) {
                    fprintf(stderr, "Error in sending\n");
                    exit(1);
                }
            }

            /* Reset the index */
            idx = 0;

            // Iterate through a buf[BYTES_OF_DATA_100000] to extract the /api/XYZ
            int first_idx = 0;
            while (buf[idx] != '\0') {
                idx++;
                if (buf[idx] == ' ') {
                    idx++;
                    if (first_idx == 0) {
                        first_idx = idx;
                    }
                }
            }

            // Copies into the api_buffer
            // Check if /api/summarize, /api/transcribe, or /api/upload
            // api/transcribe has 15 characters
            while (buf[first_idx] != ' ') {
                api_buffer[api_buffer_idx] = buf[first_idx];
                api_buffer_idx++;
                first_idx++;
            }

            // Post has already been handled
            // Check which resource to use and run script
            char* summarize_str = "/api/summarize";
            char* transcribe_str = "/api/transcribe";
            char* upload_str = "/api/upload";

            char* input_buf = (char*)malloc(BYTES_OF_DATA_30 * sizeof(char));

            strcpy(input_buf, api_buffer);
            int summary_result = strcmp(summarize_str, input_buf);
            int transcribe_result = strcmp(transcribe_str, input_buf);
            int upload_result = strcmp(upload_str, input_buf);

			char* result;
            char* arg1;
            char* arg2;
			char* final_filename_output;

			printf("Summarize_str: %s\n", summarize_str);
			printf("Transcribe_str: %s\n", transcribe_str);
			printf("Upload_str: %s\n", upload_str);
			printf("Summarize_result: %i\n", summary_result);
			printf("Transcribe_result: %i\n", transcribe_result);
			printf("Upload_result: %i\n", upload_result);

			//char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
            if (upload_result == IS_TRUE) {

			    printf("Run upload code.\n");

				// Create the wav file in the video directory
				final_filename_output = create_wav_filename(bash_arg1, bash_arg2);

                // Stream the data with connect_d
                run_data_parser(connect_d, final_filename_output);
            } else if (transcribe_result == IS_TRUE) {
                printf("Run transcribe code.\n");

                FILE* fptr;
                char ch;
                char* transcription_data_str = (char*)malloc(100000);
                int data_idx = 0;

                printf("1\n");
                fptr = fopen("transcriptions/17.txt", "r");
                if (fptr == NULL) {
                    printf("File does not exist or cannot be opened.\n");
                    return 1;
                } else {
                    printf("Opening the file\n");
                     while((ch = fgetc(fptr)) != EOF) {
                         putchar(ch);
                         *(transcription_data_str + data_idx) = ch;
                         data_idx++;
                    }
                    fclose(fptr);
                }

                // Build the Data JSON string to send to the client
                char* transcription_result;
                int len_of_data = strlen(transcription_data_str);

                printf(">>>>\n\n208\n");
                char* built_http_translation_response = build_http_data_json_ok_response(connect_d, transcription_result, transcription_data_str, len_of_data);
                printf(">> 209 server_driver.c >> %s\n\n", built_http_translation_response);

////char* built_http_translation_response = "HTTP/1.1 200 OK\nContent-Type: video/mp4\nContent-Length: 20\n\nThis is a ZZZ ERROR.\n";
                int send_200_ok = send(connect_d, built_http_translation_response, strlen(built_http_translation_response), 0);
		        if (send_200_ok == DOES_NOT_EXIST) {
                    fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                    close(connect_d);
                    exit(0);
                }
                printf(">> 219 server_driver.c\n\n");
			} else if (summary_result == IS_TRUE) {
				printf("Run summary code.\n");

            } else {
                // Send an 400 Error if the file is not in the directory
			    char* built_http_400_response = "HTTP/1.1 400 Bad Request\nContent-Type: video/mp4\nContent-Length: 20\n\nThis is a 400 ERROR.\n'";
				int send_408_ok = send(connect_d, built_http_400_response, strlen(built_http_400_response), 0);
                fprintf(stderr, "Error 400 in transcribing\n");
                close(connect_d);
                exit(0);
            }
            // Send a 200 message
			char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
            char* built_http_200_response = "HTTP/1.1 200 OK\nContent-Type: video/mp4\nContent-Length: 17\n\nThis is a 200 OK.\n";
            int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
		    if (send_200_ok == DOES_NOT_EXIST) {
                fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                close(connect_d);
                exit(0);
            }

            // Close the connection
            printf("Closing the connection now!\n");
            free(input_buf);
            close(connect_d);
            exit(0);
           }
    }
}
