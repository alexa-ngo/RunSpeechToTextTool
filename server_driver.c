#include "server.h"
#include <string.h>
#include <time.h>
#include "minimal_multipart_parser.h"

/*
 *  This is a server that reads in data from a client.
 *          Usage: ./server <port_num>
 */

#define BUF_LEN 256

/* Global variable declared in main */
int listener_d;

/* Converts an integer to a string */
char* num_2_key_str(int num) {
    int idx = 0;
    char* buffer = malloc(sizeof(char) * 7);		// just enough for 7 digits

    int quotient = num;
    while (quotient > 0) {
        int digit = quotient % 10;

        char v = '0' + digit;
        buffer[idx] = v;
        idx++;
        quotient = quotient / 10;
    }
    buffer[idx] = '\0';		// don't forget the null terminating character

    // Reverse the string because the "number" is now backwards.
    int buffer_length = idx;
    for (int i = 0, j = buffer_length -1; i < j; i++, j--) {
        char t = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = t;
    }
    return buffer;
}


/* StrCatStr concatenates two strings */
char *strCatStr(char *s1, char *s2) {

  // Get the length of s1[] and s2[]
  int lengthOfS1 = strlen(s1);
  int lengthOfS2 = strlen(s2);
  printf("The lengthOfS1: %i\n", lengthOfS1);
  printf("The lengthOfS2: %i\n", lengthOfS2);

  // Malloc a new buffer with the lengthOfs1 and lengthOfs2
  char *result = malloc(lengthOfS1 + lengthOfS2 + 1);

  // Copy s1 and s2 into the result pointer
  for (int i = 0; i < lengthOfS1; i++) {
    result[i] = s1[i];
  }

  for (int j = 0; j < lengthOfS2; j++) {
    result[lengthOfS1 + j] = s2[j];
  }
	return result;
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

			// Used to check if the request is /api/upload
            char api_buffer[BYTES_OF_DATA_30];
            int api_buffer_idx = 0;
            int idx = 0;

			// Used to check if the request is POST
            char post_request_buffer[BYTES_OF_DATA_30];
            int post_request_idx = 0;

            /* See if this is a POST request */
            while (buf[idx] != ' ') {
                post_request_buffer[idx] = buf[idx];
                idx++;
            }
            /* Reset the index */
            idx = 0;

            /* Iterate through a buf[BYTES_OF_DATA_100000];
             extract the /api/upload
            */
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

            /* Places /api/upload into the API buffer */
            while (buf[first_idx] != ' ') {
                api_buffer[api_buffer_idx] = buf[first_idx];
                api_buffer_idx++;
                first_idx++;
            }

            /* Check if the request from the client is a POST request with /api/upload */
            if ((strcmp(post_request_buffer, "POST") == 0) && (strcmp(api_buffer, "/api/upload")) == 0) {

				int each_char;

				/* Parses out each byte of an audio/video file */
				// Minimal Multipart Form Data Parser
                static MinimalMultipartParserContext state = {0};
                FILE *sockfile = (FILE*) fdopen(connect_d, "r");

				// Malloc the size of the array to send
				printf("will allocate the buffer for uploaded data now.\n");
				int one_hundred_million = 100000000;
				char* uploaded_data = malloc(sizeof(char) * one_hundred_million);

				int uploaded_data_index = 0;
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
                // Check if file has been received
                if (minimal_multipart_parser_is_file_received(&state)) {
                    printf("File Received Successfully\n");
                } else {
                    printf("File Reception Failed\n");
                }

				// Make the timestamp for the output file
    			char buf[BUF_LEN] = {0};
    			time_t rawtime = time(NULL);
    			if (rawtime == -1) {
       				puts("The time() function failed");
        			return 1;
    			}

				// Make the output file name
				int file_name = (int)time(NULL);
				char* filename_str = num_2_key_str(file_name);
				char* final_filename = strCatStr(filename_str, ".mp4");

				// Making the output file
				FILE* output_file = fopen(final_filename, "w");

				// Malloc the size of the array to send
				for (int j = 0; j < uploaded_data_index; j++) {
					fputc(uploaded_data[j], output_file);
				}
				fclose(output_file);

				// Build the HTTP OK filename string to send to the client.
				// The server returns 200 OK if the content is good data
                char* http_OK_filename_str = "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ";

				// Use strcat to build the JSON string first by finding  the length of the JSON string
				//	and send the filename to the client, so the client can request for that file
				char json_filename_str[100];
				char* left_brace = "{";
				char* right_brace = "}";
				strcat(json_filename_str, final_filename);

				//printf("file_str_len >> %s\n", file_str_len);

				// Build the official filename string
				char http_OK_filename_str_official[100000];
				char* file_label = "\"filename\" : ";
				char* null_char = "\0";
				char* quote = "\"";
				char* two_slash_n = "\n\n"
;
				char data_content_bytes[100];

				strcat(data_content_bytes, left_brace);
				strcat(data_content_bytes, file_label);
				strcat(data_content_bytes, quote);
				strcat(data_content_bytes, json_filename_str);
				strcat(data_content_bytes, quote);
				strcat(data_content_bytes, right_brace);
				strcat(data_content_bytes, two_slash_n);
				strcat(data_content_bytes, null_char);

				strcat(http_OK_filename_str_official, http_OK_filename_str);
				int data_len = strlen(data_content_bytes);
				char* data_len_as_str = num_2_key_str(data_len);
				strcat(http_OK_filename_str_official, data_len_as_str);
				strcat(http_OK_filename_str_official, two_slash_n);
				strcat(http_OK_filename_str_official, data_content_bytes);

				printf("Here is the http: \n\n\n%s\n", http_OK_filename_str_official);

               	int send_200_ok = send(connect_d, http_OK_filename_str_official, strlen(http_OK_filename_str_official), 0);
				//int send_200_ok = send(connect_d, result_str, strlen(result_str), 0);
                if (send_200_ok == DOES_NOT_EXIST) {
                    fprintf(stderr, "Error in 200 sending\n");
                    exit(1);
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
			printf("Closing the connection now!\n");
            close(connect_d);
            exit(0);
        }
    }
}