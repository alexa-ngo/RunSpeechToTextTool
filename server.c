#include "server.h"

// Input of the filename string: {1234.mp4}
char* api_transcribe_get_value(int connect_d, char* retrieved_file_in_vid_dir_str) {

    printf("File path: %s\n", retrieved_file_in_vid_dir_str);

    // Make the input_json_str as a JSON object with the JSON-C library
    char filename[] = "./videos/18.txt";

    char find_this_key[] = "filename";
    json_object *jdata, *object;

    jdata = json_object_from_file(filename);
    if (jdata == NULL) {
        fprintf(stderr, "Unable to process %s\n", find_this_key);
        // Returns the HTTP/1.1 400 Error Message
        char* api_transcribe_400 = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 401 ERROR.\n";
        int send_400_error_code = send(connect_d, api_transcribe_400, strlen(api_transcribe_400), 0);
        if (send_400_error_code == DOES_NOT_EXIST) {
            fprintf(stderr, "Error in sending\n");
            close(connect_d);
            exit(1);
        }
    }

    // The value output is 1234.mp4
    json_object_object_get_ex(jdata, find_this_key, &object);
    //char* value = json_object_get_string(object);

    // Run the bash script
    //run_bash_script();
    printf("server.c run_bash_script >>> \n");

    //return value;
}


// Bind to a port
void bind_to_port(int socket, int port) {
    struct sockaddr_in the_socket;
    the_socket.sin_family = PF_INET;
    the_socket.sin_port = (in_port_t)htons(port);
    the_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    // Reuse the socket to restart the server without a problem
    int reuse = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
        fprintf(stderr, "Can't set the reuse option on the socket");
        exit(1);
    }

    // Bind to a socket
    int c = bind(socket, (struct sockaddr *) &the_socket, sizeof(the_socket));
    if (c == -1) {
        fprintf(stderr, "Can't bind to socket");
        exit(1);
    }
}

char* build_http_ok_response(char* unix_filename, char* results) {
    // Build the HTTP OK filename string to send to the client.
    // The server returns 200 OK if the content is good data
    char http_OK_filename_str_official[100000] = "\0";
    char* http_OK_filename_str = "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ";

    // Use strcat to build the JSON string first by finding the length of the JSON string
    //	and send the filename to the client, so the client can request for that file
    char json_filename_str[100] = "\0";
    char* left_brace = "{";
    char* right_brace = "}";
    strcat(json_filename_str, unix_filename);

    // Build the official filename string
    char* file_label = "\"filename\" : ";
    char* null_char = "\0";
    char* quote = "\"";
    char* two_slash_n = "\n\n";

    // Build the data json; Ex. {"filename" : "12345.mp4"}
    char data_content_bytes[100] = "\0";
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
    strcat(http_OK_filename_str_official, null_char);

    // Result outputs: {"filename" : "1234.mp4"}
    results = http_OK_filename_str_official;
    free(data_len_as_str);
    return results;
}

// Transcription Data: Build the HTTP JSON string to send back to the client
char* build_http_data_json_ok_response(int connect_d, char* transcription_result, char* transcription_data_str, int len_of_data_bytes) {

    // Build the HTTP Transcript JSON Object to send back to the client
    // The server returns 200 OK if the content is good data
   // char http_data_json_OK_filename_s
    // Output: {"data" : "01234: I have a dream."}
    //printf(">>> this is the transcription data string: %s\n", transcription_data_str);
    printf(">>> len_of_data: %i\n", len_of_data_bytes);

    char http_transcription_data_str_official[1000000] = "\0";
    char* built_http_200_header = "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ";

    // Builds: {"data"} : {xyz}
    char json_data_label_str[100] = "\0";
    char* data_label = "{\"data\"} : {";
    char* null_char = "\0";
    char* right_brace = "}";
    char* two_slash_n = "\n\n";
    strcat(json_data_label_str, data_label);
    strcat(json_data_label_str, transcription_data_str);
    strcat(json_data_label_str, right_brace);
    strcat(json_data_label_str, two_slash_n);
    strcat(json_data_label_str, null_char);
//    printf("126: %s", json_data_label_str);

    strcat(http_transcription_data_str_official, built_http_200_header);
    char* data_len_as_str = num_2_key_str(len_of_data_bytes);
    strcat(http_transcription_data_str_official, data_len_as_str);
    strcat(http_transcription_data_str_official, two_slash_n);
    strcat(http_transcription_data_str_official, data_len_as_str);
    strcat(http_transcription_data_str_official, null_char);

    printf(" 138 >>> %s\n", http_transcription_data_str_official);
    transcription_result = http_transcription_data_str_official;
    return transcription_result;
}


/* Build the wav final filename */
char* create_wav_filename(char* bash_arg1, char* bash_arg2) {

    char bash_str_result[150] = "\0";
    char* space = " ";

	// Create the 171234.wav filename
    int now = time(NULL);
    char* now_str = num_2_key_str(now);
	char* slash_video = "/videos/";
    char* wav = ".wav";

	// Build the unix time filename
    char* UNIX_time_str_filename = malloc(100);
    strcat(UNIX_time_str_filename, now_str);
    strcat(UNIX_time_str_filename, wav);

	// Make the bash script command
	char* initial_bash_str = "bash run_bash_script.sh ";
    strcat(bash_str_result, initial_bash_str);
    strcat(bash_str_result, bash_arg1);
    strcat(bash_str_result, space);

	char cwd[PATH_MAX];
	char* curr_dir = getcwd(cwd, sizeof(cwd));
	strcat(bash_str_result, curr_dir);
	strcat(bash_str_result, slash_video);
	strcat(bash_str_result, UNIX_time_str_filename);
    system(bash_str_result);

	return UNIX_time_str_filename;
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

    char* post_result = malloc(10);
    strcpy(post_result, post_request_buffer);
    printf("54 >> %s", post_result);

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

// Create a streaming socket
int open_listener_socket(void) {
    int streaming_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (streaming_socket == -1){
        fprintf(stderr, "Can't open the socket");
        exit(1);
    }
    return streaming_socket;
}

/* Make a filename string with braces
    Creates the filename to send over the network
    Return: {1234.wav}
*/
char* make_filename_brace_str(char* final_filename_output, char* filename_str) {
    char* left_brace = "{";
    char* right_brace = "}";
    strcat(filename_str, left_brace);
    strcat(filename_str, final_filename_output);
    strcat(filename_str, right_brace);
    return filename_str;
}

char* make_final_filename(char* either_mp4_or_wav) {

    int now = time(NULL);
    char* wav = ".wav";

    char* UNIX_time_str_filename = num_2_key_str(now);
    strcat(UNIX_time_str_filename, wav);
    return UNIX_time_str_filename;
}

/* Converts an integer to a string */
char* num_2_key_str(int num) {
    int idx = 0;
    char* buffer = malloc(sizeof(char) * 100);		// just enough for 55 digits

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

// Stream each byte of the media file
void run_data_parser(int connect_d, char* final_filename_output) {
    /* Minimal Multipart Form Data Parser
       Parses out each byte of an audio/video file
       Credit: written by Bryan Khuu and can be found on GitHub */
    static MinimalMultipartParserContext state = {0};
    FILE *sockfile = (FILE*) fdopen(connect_d, "r");

    // Malloc the size of the array to send
    char* uploaded_data = malloc(sizeof(char) * ONE_HUNDRED_MILLION);

    // Read and parse each character
    int uploaded_data_index = 0;
    int each_char;
    while ((each_char = fgetc(sockfile)) != EOF) {
    // Processor handles incoming stream character by character
        MultipartParserEvent event = minimal_multipart_parser_process(&state, (char)each_char);

        // Handle Special Events
        if (event == MultipartParserEvent_DataBufferAvailable) {
            // Data to be received
            for (unsigned int j = 0; j < minimal_multipart_parser_get_data_size(&state); j++) {
                char rx = minimal_multipart_parser_get_data_buffer(&state)[j];
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
    /* ./videos/1763933938.mp4
        HTTP/1.1 200 OK
        Content-Type: application/json
        Content-Length: 33
    */
    char filename[50] = "\0";
    char* filepath = "./videos/";
    strcat(filename, filepath);
    strcat(filename, final_filename_output);
    FILE* output_file = fopen(filename, "w");

    printf("\n\nFilename: %s\n\n", filename);

    // Malloc the size of the array to send
    for (int j = 0; j < uploaded_data_index; j++) {
        //fputc(uploaded_data[j], output_file);

    }
    free(uploaded_data); 		// One hundred million bytes
    fclose(output_file);
}

// Check if the video (XYZ.mp4) video exists in the videos directory
// and transcribes the video if the video does exists
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
    }

    else {
        // Returns the HTTP/1.1 400 Error Message
        char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 888 ERROR.'";
        int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
        if (send_400_error_code == DOES_NOT_EXIST) {
            fprintf(stderr, "Error in sending\n");
            exit(1);
        }
    }
    return retrieved_file_in_vid_dir_str;
}


