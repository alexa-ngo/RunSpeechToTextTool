#include "server.h"

// The client saves the filename in a JSON object, and the server checks to see
// if the .wav file exists.
// The server returns the value of the filename key.
//
// Input:
//      filename_and_path_in_vid_dir: ./videos/XYZ.wav
char* api_transcribe_get_value(int connect_d, char* filename_and_path_in_vid_dir) {

    printf("File path: %s\n", filename_and_path_in_vid_dir);

    // Program returns the value of the filename key
    char find_this_key[] = "filename";
    json_object *jdata, *object;

    jdata = json_object_from_file(filename_and_path_in_vid_dir);
    if (jdata == NULL) {
        fprintf(stderr, "Unable to process %s\n", find_this_key);
        // Returns the HTTP/1.1 400 Error Message
        char* api_transcribe_400 = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.\n";
        int send_400_error_code = send(connect_d, api_transcribe_400, strlen(api_transcribe_400), 0);
        if (send_400_error_code == DOES_NOT_EXIST) {
            fprintf(stderr, "Error in sending\n");
            close(connect_d);
            exit(1);
        }
    }

    // Execute a Bash script to transcribe the .wav file with the Whisper arguments
    // The Bash script will ask for user input of the file they want to transcribe and the output name.

    // The value output is 1234.mp4
    json_object_object_get_ex(jdata, find_this_key, &object);
    char* value = json_object_get_string(object);
    printf("32: the value: %s\n", value);

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
    printf("90 >> %s\n", data_content_bytes);

    strcat(http_OK_filename_str_official, http_OK_filename_str);
    int data_len = strlen(data_content_bytes);
    char* data_len_as_str = num_2_key_str(data_len);
    strcat(http_OK_filename_str_official, data_len_as_str);
    strcat(http_OK_filename_str_official, two_slash_n);
    strcat(http_OK_filename_str_official, data_content_bytes);
   // strcat(http_OK_filename_str_official, '\0');

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
void transcribe_video_method(int connect_d, char* transcribe_file_path_from_user, char* retrieved_file_in_vid_dir_str) {

    printf("126 Retrieved_file_in_vid_dir_str: %s\n", retrieved_file_in_vid_dir_str);
    // Make the video in ./videos/XYZ.wav
    char* video_path_str = "./videos/";
    strcat(retrieved_file_in_vid_dir_str, video_path_str);
    strcat(retrieved_file_in_vid_dir_str, transcribe_file_path_from_user);

    // Check file exists
    if (retrieved_file_in_vid_dir_str) {
        // Get the value of the key called "filename"
        char* data_val_of_file = api_transcribe_get_value(connect_d, retrieved_file_in_vid_dir_str);
        printf("File exists returned value: %s\n\n", data_val_of_file);
    } else {
        // If file doesn't exist, return a 400 Error message
        char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 888 ERROR.'";
        int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
        if (send_400_error_code == DOES_NOT_EXIST) {
            fprintf(stderr, "Error in sending\n");
            exit(1);
        }
    }
 //   return retrieved_file_in_vid_dir_str;
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

/* Build the wav final filename */
char* create_wav_filename(void) {

    // alexa-script.sh
    //system("bash run_bash_script.sh");
    printf("ran the bash script\n");

    int now = time(NULL);
    char* now_str = num_2_key_str(now);
    char* wav = ".wav";

    char* UNIX_time_str_filename = malloc(100);
    strcat(UNIX_time_str_filename, now_str);
    strcat(UNIX_time_str_filename, wav);
    return UNIX_time_str_filename;
}




