#ifndef SERVER
#define SERVER

/*
    This is the declaration of the server.
*/

#include <arpa/inet.h>
#include "json-c/json.h"
#include "minimal_multipart_parser.h"
#include "server.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUF_LEN 256
#define BYTES_OF_DATA_30 30
#define BYTES_OF_DATA_100000 100000
#define DOES_NOT_EXIST -1
#define FALSE 0
#define LETTER_OF_A 65
#define MAX_5_NUM_OF_RUNNING_PROCESSES 5
#define ONE_HUNDRED_MILLION 100000000
#define SENDING_DATA_WAS_SUCCESSFUL 0
#define TOTAL_CHAR_OF_ALPHABET 26
#define TRUE 1

int api_transcribe(char* input_json_str);
void bind_to_port(int socket, int port);
char* build_http_ok_response(char* final_filename_output, char* results);
void child_process_handles_request(int listener_d, int connect_d, char* buf);
char* data_longer_than_or_equal_to_key(int key_length, char* key_str, int data_length, char* data_str);
int get_value_in_jstring(const char* jstring);
void kill_the_process(void);
char* make_final_filename(void);
char* num_2_key_str(int num);
int open_listener_socket(void);
int read_in(int socket, char* buf, int len);
void run_data_parser(int connect_d, char* file_filename_output);
int say(int socket, char* s);

#endif