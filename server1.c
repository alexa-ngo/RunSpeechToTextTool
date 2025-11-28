#include "server1.h"

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
    char* value = json_object_get_string(object);

    // Run the bash script
    //run_bash_script();
    printf("server.c run_bash_script >>> \n");

    return value;
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


char* build_http_ok_response(char* final_filename_output, char* results) {
    // Build the HTTP OK filename string to send to the client.
    // The server returns 200 OK if the content is good data
    char http_OK_filename_str_official[100000] = "\0";
    char* http_OK_filename_str = "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ";

    // Use strcat to build the JSON string first by finding  the length of the JSON string
    //	and send the filename to the client, so the client can request for that file
    char json_filename_str[100] = "\0";
    char* left_brace = "{";
    char* right_brace = "}";
    strcat(json_filename_str, final_filename_output);

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
    strcat(http_OK_filename_str_official, "\0");

    // Result outputs: {"filename" : "1234.mp4"}
    results = http_OK_filename_str_official;
    free(data_len_as_str);
    return results;
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

    // Malloc the size of the array to send
    for (int j = 0; j < uploaded_data_index; j++) {
        fputc(uploaded_data[j], output_file);
    }
    free(uploaded_data); 		// One hundred million bytes
    fclose(output_file);
}



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
    buffer = "1114";
    return buffer;
}

/* Build the wav final filename */
char* create_wav_filename() {

    //time_t now = time(NULL);         // Get current time
    //struct tm *t = localtime(&now);  // Convert to local time structure
    //char* date_time_buffer = malloc(200 * 5);

    // alexa-script.sh
    //system("bash run_bash_script.sh");
    printf("ran the bash script\n");

    int now = time(NULL);
    char* wav = ".wav";

    char* UNIX_time_str_filename = "\0";
    UNIX_time_str_filename = "1114";
    char* unix_str = num_2_key_str(now);
    printf("now: %s\n", unix_str);
    //char* UNIX_time_str_filename = num_2_key_str(now);
   // strcat(UNIX_time_str_filename, wav);
   // printf("This is the unix time: %s\n", UNIX_time_str_filename);
    return UNIX_time_str_filename;
}




