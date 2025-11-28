#include "server1.h"

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
    char buf[BYTES_OF_DATA_100000] = "\0";

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

    // Loop throught the data
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

            // Read the client's message from the socket
            // Ex: POST /api/upload HTTP/1.1 along with User-Agent: curl/8.15.0
            //        and other metadata such as Context-Length, Content-Type
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

            // Builds the API buffer of either /api/upload or /api/transcribe
            char api_buffer[BYTES_OF_DATA_30] = "\0";
            int api_buffer_idx = 0;
            int idx = 0;

			// Check if POST request
            char post_request_buffer[BYTES_OF_DATA_30] = "\0";
            int post_request_idx = 0;
            while (buf[idx] != ' ') {
                post_request_buffer[idx] = buf[idx];
                idx++;
            }
            // Reset the index
            idx = 0;

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
            // Places /api/XYZ into the API buffer which can be either upload or transcribe
            while (buf[first_idx] != ' ') {
                api_buffer[api_buffer_idx] = buf[first_idx];
                api_buffer_idx++;
                first_idx++;
            }

            // Check if the request is a POST, and if the api is /api/upload or /api/transcribe
			int str_cmp_post_result = (strcmp(post_request_buffer, "POST") == IS_TRUE);
			int int_api_upload_buff_result = (strcmp(api_buffer, "/api/upload") == IS_TRUE);
			int int_api_transcribe_buff_result = (strcmp(api_buffer, "/api/transcribe") == IS_TRUE);

            // POST request with /api/upload or /api/transcribe or /api/upload
            if ((str_cmp_post_result == TRUE && int_api_upload_buff_result == TRUE) ||
                 (str_cmp_post_result == TRUE && int_api_transcribe_buff_result == TRUE)) {
				// Create filename. Ex unixtime.mp4
                //char* wav = "wav";
                //char* final_filename_output;
                //final_filename_output = "1114";
                char* final_filename_output = create_wav_filename();
                printf("Make final_filename: %s\n", final_filename_output);

				// Stream the data with connect_d
				//run_data_parser(connect_d, final_filename_output); // there are about 600 bytes in her


                char filename_str[100] = "\0";
                //filename_str = "1114";
                char* filename_result = make_filename_brace_str(final_filename_output, filename_str);
                printf(">>  158: filename_result: %s\n", filename_result);

				// Execute the api transcribe method
    			char retrieved_file_in_vid_dir_str[100] = "\0";
                char* retrieved_file_in_vid_dir_str_result = transcribe_video_method(connect_d, final_filename_output, retrieved_file_in_vid_dir_str);
               	char* api_results = api_transcribe_get_value(connect_d, retrieved_file_in_vid_dir_str);
                printf("\n 161 >> send this data to client: %s\n", retrieved_file_in_vid_dir_str);
                printf("\n 162 >> send this filename to the client: %s\n", final_filename_output);
				char* result;
                printf(">> 164\n", api_results);
				if (api_results != NULL) {
                     printf("\n 166 >> inside the api_results");

					char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
					printf("169 >> %s\n", built_http_ok_response);
                    /*
                    int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
                    printf("\n 166 >> send this data over: %s\n", built_http_ok_response);
					if (send_200_ok == DOES_NOT_EXIST) {
                    	fprintf(stderr, "Error in 200 sending in send 200 OK\n");
						//free(final_filename_output);
                    	exit(1);
                	}
					//free(final_filename_output);
*/
                    printf("172\n");

				} else {
                    printf("181 this is to send the 400 error\n");

					// Send an 400 Error if the file file is not in the directory
			    	//char* built_http_ok_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.\n'";
				    //int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
                	//int send_200_ok = send(connect_d, final_filename_output, strlen(final_filename_output), 0);
                    printf("189\n");
                    //if (send_200_ok == DOES_NOT_EXIST) {
                    //	fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                    //	exit(1);
                	//}

				}

            } else {

                // Returns the HTTP/1.1 400 Error Message
                //char* result_str = "HI";
                //printf("200\n");
                char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 888 ERROR.'";
                int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
                if (send_400_error_code == DOES_NOT_EXIST) {
                    printf("204\n");
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
    close(listener_d);
}