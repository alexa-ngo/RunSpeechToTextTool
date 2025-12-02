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

// Check if the request is a POST request and the handle:
// summarize, transcribe, or upload
int is_post (int connect_d, char* api_buffer) {

    // Buffer for the data, but clear it first
    char buf[BYTES_OF_DATA_100000] = "\0";
    memset(buf, '\0', BYTES_OF_DATA_100000);

    // Read the client's message from the socket
    /// Ex: POST /api/upload HTTP/1.1 along with User-Agent: curl/8.15.0
    //        and other metadata suchs as Context-Length, Content-Type
    int chars_read = recv(connect_d, buf, BYTES_OF_DATA_100000, 0);
    if (chars_read == 0) {
        fprintf(stderr, "Error. Received 0 bytes.\n");
        close(connect_d);
        exit(1);
    }

    // Check if the request is a POST request
    int idx = 0;
    char post_request_buffer[BYTES_OF_DATA_30] = "\0";
    while (buf[idx] != ' ') {
        post_request_buffer[idx] = buf[idx];
        idx++;
    }

    // Freed the memory
    char* post_result = malloc(10);
    strcpy(post_result, post_request_buffer);

    // Send 400 Error if the request is not a POST request
    if (post_result == FALSE) {
        printf("This is not POST\n");
        char* not_post_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 44\n\nThis is a 400 ERROR and not a POST response.\n\n";
        int send_400_post = send(connect_d, not_post_response, strlen(not_post_response), 0);
        if (send_400_post == DOES_NOT_EXIST) {
            fprintf(stderr, "Error. Not a POST request.\n");
            exit(1);
        }
    }
    free(post_result);
    return 0;
}

// Build the final filename
char* make_mp4_final_filename(void) {

    int filename = (int)time(NULL);
    char* filename_str = num_2_key_str(filename);
    char* final_filename = strcat(filename_str, ".wav");
    printf("The final_filename: %s\n", final_filename);
    return final_filename;
}

// Execute the transcription process
// Convert the code from a .mp4 code to .wav
int run_transcribe_code(int connect_d, char* final_filename_output) {

    printf("Running transcribe code\n");

    // Parse the data using Minimal Multipark Form Data Parser
    // Minimal Multipart Form Data Parser
    // Parses out each byte of an audio/video file
    // Credit: written by Bryan Khuu and hte code can be found on GitHub
    static MinimalMultipartParserContext state = {0};
    FILE *sockfile = (FILE*) fdopen(connect_d, "r");

	// Malloc the size of the array to send
	char* uploaded_data = malloc(sizeof(char) * ONE_HUNDRED_MILLION);

	// Read and parse each character
	int uploaded_data_index = 0;
	int each_char;
    while ((each_char = fgetc(sockfile)) != EOF) {
        // Processor handles incoming stream character by character
         const MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)each_char);

         // Handle Special Events
         if (event == MultipartParserEvent_DataBufferAvailable) {
            // Data to be received
            for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++) {
                const char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
				uploaded_data[uploaded_data_index] = rx;
                uploaded_data_index++;
            }
         }
         else if (event == MultipartParserEvent_DataStreamCompleted) {
            // Data Stream Finished;
            break;
            }
         }

    	 // Create output file in the videos directory
		char filename[50];
        char* filepath = "./videos/";
        strcat(filename, filepath);
		strcat(filename, final_filename_output);
    	FILE* output_file = fopen(filename, "w");

    	// Malloc the size of the array to send
    	for (int j = 0; j < uploaded_data_index; j++) {
        	fputc(uploaded_data[j], output_file);
    	}
    	fclose(output_file);

}


// Run the server
int main(int argc, char* argv[]) {

    if (argc < 2) {          // argv[0]     argv[1]  argv[2]
        printf("Usage: <program_name> <port>  <optional_transcription_file_path>     ");
        exit(1);
    }

    const char* port = argv[1];
    int port_num = atoi(port);
    char buf[BYTES_OF_DATA_100000];

    // Benchmark time
    clock_t start;
    clock_t end;
    double upload_cpu_time_used;

    start = clock();

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

            // Make the filename to output the streamed data. Ex. 176447623.mp4
            char* final_filename_output = make_mp4_final_filename();

            if (summary_result == IS_TRUE){
                printf("Run summary code.\n");
                //int run_summary_code_result = run_summary_code();
            } else if (transcribe_result == IS_TRUE) {
                printf("Transcribing..\n\n");

                printf(">>>>>>>>>>>> GOAL: get the transcribed data, make a buffer and send the string back to the client as data in a value\n");
                printf("data : {and then transcribed text }" );

                // Run the transcription Bash script
                printf("Starting Whisper transcription...\n");
               // char buffer[100];
                system("bash ./whisper_wav_to_txt.sh z-audio-output1.wav z-audio-whisper1.txt");
                //system(buffer);
                printf("in progress\n\n.. done with transcribing!\n\n");

                char* transcribe_file_path_from_user = (char*)malloc(100);
                //char* retrieved_file_in_vid_dir_str = "/shanna";
                //transcribe_video_method(connect_d, transcribe_file_path_from_user, retrieved_file_in_vid_dir_str);

                //printf("The file you're transcribing: %s\n", retrieved_file_in_vid_dir_str_result);
                /*
                Method: /api/transcribe
                Purpose: Client sends a JSON string to the server to retrieve the transcribed data.
                            The server sends the transcribed data back to the client.
                Input: JSON string {"filename" : "1234.wav"}
                Output: Transcribed data sent from the server back to the client {"data" : "Hi there!"}

                Workflow:
                    1. Client sends request to server {"filename": "1234.mp4"} to the server
                    2. Server uses the JSON library to convert the JSON string into a JSON object
                    3. Retrieve the filename from the JSON object
                    4. Use the filename to transcribe
                    5. Make the transcription file
                    6. Send the transcription data back to the client {"data":"Hi there!"}
                 */

                // Get the file

                // Execute
                end = clock();
                double transcribe_cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Transcribe time: %d seconds\n", transcribe_cpu_time_used);



                free(final_filename_output);
                close(connect_d);
                exit(0);

            } else if (upload_result == IS_TRUE) {
                printf("Run upload code.\n");

                // Stream the data with connect_d
                run_data_parser(connect_d, final_filename_output);

                // Build a unique http ok response
                char* result;
                char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
                int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
                if (send_200_ok == DOES_NOT_EXIST) {
                    fprintf(stderr, "Error in 200 sending\n");
                    exit(1);
                }

                // Send a 200 message
                //char* built_http_200_response = "HTTP/1.1 200 OK\nContent-Type: video/mp4\nContent-Length: 17\n\nThis is a 200 OK.\n\n";
                //int send_200_ok1 = send(connect_d, built_http_200_response, strlen(built_http_200_response), 0);
		        //if (send_200_ok1 == DOES_NOT_EXIST) {
                  //  fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                    //close(connect_d);
                    //exit(0);
                // }i


                end = clock();
                upload_cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Upload time: %d seconds\n", upload_cpu_time_used);

                free(final_filename_output);
                close(connect_d);
                exit(0);
            } else {
                /*
                // Send an 404 Error if the file is not in the directory
			    char* built_http_404_response = "HTTP/1.1 404 Bad Request\nContent-Type: video/mp4\nContent-Length: 20\n\nThis is a 404 ERROR.\n'";
				int send_404_ok = send(connect_d, built_http_404_response, strlen(built_http_404_response), 0);
                fprintf(stderr, "Error 404 in transcribing\n");
                close(connect_d);
                exit(0);
                */
            }

            // Send a 200 message
            char* built_http_200_response = "HTTP/1.1 200 OK\nContent-Type: video/mp4\nContent-Length: 17\n\nThis is a 200 OK.\n\n";
            int send_200_ok = send(connect_d, built_http_200_response, strlen(built_http_200_response), 0);
		    if (send_200_ok == DOES_NOT_EXIST) {
                fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                close(connect_d);
                exit(0);
            }

            // Close the connection
            printf("Closing the connection now!\n");

            free(final_filename_output);
            free(input_buf);
            close(connect_d);
            exit(0);
           }
    }
}
/*
            // POST request with /api/upload or /api/transcribe or /api/upload
            if (int_api_upload_buff_result == TRUE) {
                printf("Uploading.. ");
                final_filename_output = create_wav_filename();
                printf("Make final_filename: %s\n", final_filename_output);

                // Stream the data with connect_d
                //run_data_parser(connect_d, final_filename_output); // there are about 600 bytes in her

                char filename_str[100] = "\0";
                //filename_str = "1114";
                char* filename_result = make_filename_brace_str(final_filename_output, filename_str);
                printf(">>  158: filename_result: %s\n\n\ns", filename_result);

				// Create filename. Ex unixtime.mp4 -> 1764352279.wav
				// Execute the api transcribe method
    			char retrieved_file_in_vid_dir_str[100] = "\0";
                char* retrieved_file_in_vid_dir_str_result = transcribe_video_method(connect_d, final_filename_output, retrieved_file_in_vid_dir_str);
               	char* api_results = api_transcribe_get_value(connect_d, retrieved_file_in_vid_dir_str);
                //printf("\n 161 >> send this data to client: %s\n", retrieved_file_in_vid_dir_str);
                //printf("\n 162 >> send this filename to the client: %s\n", final_filename_output);
				char* result;
                printf(">> 164\n", api_results);
				if (api_results != NULL) {
                     printf("\n 166 >> inside the api_results");

					char* built_http_ok_response = build_http_ok_response(final_filename_output, result);
				//	printf("169 >> %s\n", built_http_ok_response);
                    /*

*/
			//	} else {
                    //printf("181 this is to send the 400 error\n");

					// Send an 400 Error if the file is not in the directory
			    	//char* built_http_ok_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.\n'";
				    //int send_200_ok = send(connect_d, built_http_ok_response, strlen(built_http_ok_response), 0);
                	//int send_200_ok = send(connect_d, final_filename_output, strlen(final_filename_output), 0);
                  //  printf("189\n");
                    //if (send_200_ok == DOES_NOT_EXIST) {
                    //	fprintf(stderr, "Error in 200 sending in send 200 OK\n");
                    //	exit(1);
                	//}
/*
            else {

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
*/
            // Close the connection
            //printf("Closing the connection now!\n");
            //close(connect_d);
            //exit(0);
    //   }
  //  }
   // printf("314 >> this is the end \n");
   // exit(0);
    //close(listener_d);
//}

/*
            // Build the Unix filename to be 17123123.wav
            char* results;
            int unix_num = time(NULL);
            char unix_filename_buff[20];
            char* wav = ".wav";

            sprintf(unix_filename_buff, "%d", unix_num);
            strcat(unix_filename_buff, wav);
            printf("207 >> The unix time: %s\n", unix_filename_buff);
            char* built_http_ok_response = build_http_ok_response(unix_filename_buff, results;
*/

