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

// Check if the video (XYZ.mp4) video exists in the videos directory
char* transcribe_video_method(int connect_d, char* final_filename_output, char* retrieved_file_in_vid_dir_str) {

	// Make the video in ./videos/XYZ.mp4
    char* video_path_str = "./videos/";
    strcat(retrieved_file_in_vid_dir_str, video_path_str);
    strcat(retrieved_file_in_vid_dir_str, final_filename_output);

	// Check file exists
    if (retrieved_file_in_vid_dir_str != NULL) {

		// Get the value of the key caled "filename"
		char* data_val_of_file = api_transcribe_get_value(connect_d, retrieved_file_in_vid_dir_str);
    	printf("File exists returned value: %s\n", data_val_of_file);
	} else {
		// Returns the HTTP/1.1 400 Error Message
		char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.'";
		int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
		if (send_400_error_code == DOES_NOT_EXIST) {
			fprintf(stderr, "Error in sending\n");
			exit(1);
		}
    }
    return retrieved_file_in_vid_dir_str;
}

// Runs the server
int main(int argc, char* argv[]) {

    if (argc < 2) {     // argv[0]    argv[1]
        printf("Usage: <program_name> <port>\n");
        exit(1);
    }

    const char* port = argv[1];
    int port_num = atoi(port);
    char buf[BYTES_OF_DATA_100000] = "\0";
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
            close(connect_d);
			//continue;
        }

        int child_pid = fork();
        if (child_pid == -1) {
            fprintf(stderr, "Could not fork the child\n");
			close(connect_d);
            exit(1);
        }
        if (child_pid > 0) {
            fprintf(stderr, "Parent is closing child socket. Child PID: %d\n\n", child_pid);
            /* Parent is closing the CLIENT SOCKET */
            close(connect_d);
			exit(1);
        } else {
			/* This is the child Process */
            /* Close the listener.
            The process is a child process and should handle the request */
            close(listener_d);

            /* Clear the buffer first */
            memset(buf, '\0', BYTES_OF_DATA_100000);

            /* Read the client's message from the socket */
            int chars_read = recv(connect_d, buf, BYTES_OF_DATA_100000, 0);
            if (chars_read == 0) {
                fprintf(stderr, "Error in receiving\n");
                close(connect_d);
                exit(1);
            }
            if (chars_read == 0) {
                fprintf(stderr, "Error. Received 0 bytes.");
                close(connect_d);
                exit(1);
            }

			// Buffer to check if the request is /api/upload or /api/transcribe
            char api_buffer[BYTES_OF_DATA_30] = "\0";
            int api_buffer_idx = 0;
            int idx = 0;

			// BUFFER to check if the request is POST
            char post_request_buffer[BYTES_OF_DATA_30] = "\0";
            int post_request_idx = 0;

            // See if this is a POST request
            while (buf[idx] != ' ') {
                post_request_buffer[idx] = buf[idx];
                idx++;
            }
            // Reset the index
            idx = 0;

            // Iterate through a buf[BYTES_OF_DATA_100000];
            // extract the /api/XYZ
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

            // Places /api/XYZ into the API buffer;
            // XYZ can be either to upload or transcribe
            while (buf[first_idx] != ' ') {
                api_buffer[api_buffer_idx] = buf[first_idx];
                api_buffer_idx++;
                first_idx++;
            }

			int int_buff_result = FALSE;
			int is_POST = FALSE;

			int str_cmp_post_result = (strcmp(post_request_buffer, "POST") == IS_TRUE);
			printf("This is str cmp post: %i\n", str_cmp_post_result);
			int int_api_upload_buff_result = (strcmp(api_buffer, "/api/upload") == IS_TRUE);
			int int_api_transcribe_buff_result = (strcmp(api_buffer, "/api/transcribe") == IS_TRUE);
			int is_post_and_api_upload = TRUE;
			int is_post_and_api_transcribe = TRUE;

			//printf("This is the post request: %i\n", str_cmp_post_result);
			//printf("This it the buff result: %i\n", api_buffer);
			//printf("The int buff result: %i\n", int_buff_result);

			//printf("server_driver.c 169 Here it is\n");
            // Check if the request from the client is a POST request with /api/upload or /api/transcribe

            if ((str_cmp_post_result == TRUE && int_api_upload_buff_result == TRUE) ||
                 (str_cmp_post_result == TRUE && int_api_transcribe_buff_result == TRUE)) {
				printf("Yes, POST and either buff and transcribe is true\n");
				// Builds the HTTP string
				// Make the filename. Ex. 181892.mp4
				char* mp4 = "mp4";
				char* final_filename_output = make_final_filename(mp4);

				// Stream the data with connect_d
				run_data_parser(connect_d, final_filename_output);

                // Create the filename to send over the network
                // filename_str returns {1234.mp4}
            	char filename_str[100] = "\0";
            	char* left_brace = "{";
				char* right_brace = "}";
            	strcat(filename_str, left_brace);
				strcat(filename_str, final_filename_output);
				strcat(filename_str, right_brace);

				// Execute the api transcribe method
    			char retrieved_file_in_vid_dir_str[100] = "\0";
                char* retrieved_file_in_vid_dir_str_result = transcribe_video_method(connect_d, final_filename_output, retrieved_file_in_vid_dir_str);

               	char* api_results = api_transcribe_get_value(connect_d, retrieved_file_in_vid_dir_str);

				char* result;
				printf("Debug server_driver.c 198\n");
				if (api_results != NULL) {
					printf("Debug server_driver.c 198\n");
					char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
					int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);

					if (send_200_ok == DOES_NOT_EXIST) {
                    	fprintf(stderr, "Error in 200 sending in send 200 OK\n");
						free(final_filename_output);
                    	exit(1);
                	}
					printf("Run the transcription code\n");
					printf("Debug server_driver.c 207 \n");
					free(final_filename_output);
				} else {
//NOT HERE
					printf("Debug server_driver.c 211\n");
					// Send an 400 Error if the file file is not in the directory
			    	char* built_http_ok_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.\n'";
				    int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
                	if (send_200_ok == DOES_NOT_EXIST) {
                    	fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                    	exit(1);
                	}
					printf("Debug server_driver.c 219\n");
				}

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
		//	free(send_200_ok);
			printf("Closing the connection now!\n");
            close(connect_d);
            exit(0);
        }
    }
	close(listener_d);
}