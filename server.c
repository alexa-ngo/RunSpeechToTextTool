#include "server.h"

/*
*      This is the code for the microservice for the Speech-To-Text Tool.
*/

/*
    Method: /api/transcribe
    Purpose: Client sends a JSON string to the server to retrieve the transcribed data.
                The server sends the transcribed data back to the client.
    Input: JSON string
        {
            "filename" : "1234.mp4"
        }

    Output: Transcribed data sent from the server back to the client
        {
            "data" : "Hi there!"
        }

    Workflow:
        1. Client sends a request {"filename":"1234.mp4"} to the server
        2. Server uses the JSON library to convert the JSON string into a JSON object
        3. Retrieve the filename from the JSN object
        4. Use the filename to transcribe
        5. Make the transcription file
        6. Send the transcription data back to the client {"data":"Hi there!"}
*/

int run_bash_script() {
    system("./run_bash_script.sh");
    return 0;
}

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
		char* result_str = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 20\n\nThis is a 400 ERROR.\n";
		int send_400_error_code = send(connect_d, result_str, strlen(result_str), 0);
		printf(">> Result String: %s\n", result_str);
		if (send_400_error_code == DOES_NOT_EXIST) {
			fprintf(stderr, "Error in sending\n");
			exit(1);
		}
    }

    // The value output is 1234.mp4
    json_object_object_get_ex(jdata, find_this_key, &object);
    char* value = json_object_get_string(object);

	// Run the bash script
	run_bash_script();

    return value;
}

/* Create a streaming socket */
int open_listener_socket(void) {
    int streaming_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (streaming_socket == -1        ) {
        fprintf(stderr, "Can't open the socket");
        exit(1);
    }
    return streaming_socket;
}

/* Bind to a port */
void bind_to_port(int socket, int port) {
    struct sockaddr_in the_socket;
    the_socket.sin_family = PF_INET;
    the_socket.sin_port = (in_port_t)htons(port);
    the_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Reuse the socket to restart the server without a problem */
    int reuse = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(int)) == -1) {
        fprintf(stderr, "Can't set the reuse option on the socket");
        exit(1);
    }

    /* Bind to a socket */
    int c = bind(socket, (struct sockaddr *) &the_socket, sizeof(the_socket));
    if (c == -1) {
        fprintf(stderr, "Can't bind to socket");
        exit(1);
    }
}

char* build_http_ok_response(char* final_filename_output, char* results) {
    // Build the HTTP OK filename string to send to the client.
    // The server returns 200 OK if the content is good data
    char http_OK_filename_str_official[100000];
    char* http_OK_filename_str = "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ";

    // Use strcat to build the JSON string first by finding  the length of the JSON string
    //	and send the filename to the client, so the client can request for that file
    char json_filename_str[100];
    char* left_brace = "{";
    char* right_brace = "}";
    strcat(json_filename_str, final_filename_output);

    // Build the official filename string
    char* file_label = "\"filename\" : ";
    char* null_char = "\0";
    char* quote = "\"";
    char* two_slash_n = "\n\n";

    // Build the data json; Ex. {"filename" : "12345.mp4"}
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

    // Result outputs: {"filename" : "1234.mp4"}
    results = http_OK_filename_str_official;
    return results;
}


/* Kill the process */
void kill_the_process(void) {
    fprintf(stderr, "Goodbye, I am ending this process now...\n");
    exit(1);
}

/* Build the final filename */
char* make_final_filename(char* either_mp4_or_wav) {

	// Make the filename using Year-Month-Date.wav
    //int file_name = (int)time(NULL);
    //char* filename_str = num_2_key_str(file_name);
	// The transcription code will get the ./video_file.mp4 file and then make a timestamp with
	// the .wav
  	time_t now = time(NULL);         // Get current time
  	struct tm *t = localtime(&now);  // Convert to local time structure
	char* date_time_buffer = malloc(200 * 5);

	strftime(date_time_buffer, 100, "%Y-%m-%d.wav", t);
	printf("The date time: %s\n", date_time_buffer);

	return date_time_buffer;
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
    return buffer;
}

/* Receive the data from a socket */
int read_in(int socket, char* buf, int len) {

    char* buffer = buf;
    int slen = len;

    /* Must receive at least once to start the while loop */
    int received_data = recv(socket, buffer, slen, 0);
    while ((received_data > 0) && (buffer[received_data-1] != '\n')) {
        buffer += received_data;
        slen -= received_data;
        received_data = recv(socket, buffer, slen, 0);
    }

    if (received_data < 0) {
        return received_data;
        // Send back an empty string if there is an empty string
    } else if (received_data == 0) {
        buf[0] = '\0';
    } else {
        buffer[received_data - 1] = '\0';
    }
    return len - slen;
}

/* Stream each byte */
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
    fclose(output_file);
}