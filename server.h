#ifndef SERVER
#define SERVER

/*
    This is the function declarations of the server.
*/

#include <arpa/inet.h>
#include "json-c/json.h"
#include <linux/limits.h>
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
#define IS_FALSE 1
#define IS_TRUE 0
#define LETTER_OF_A 65
#define MAX_5_NUM_OF_RUNNING_PROCESSES 5
#define BASH_PATH_MAX 100
#define ONE_HUNDRED_MILLION 100000000
#define SENDING_DATA_WAS_SUCCESSFUL 0
#define TOTAL_CHAR_OF_ALPHABET 26
#define TRUE 1

char* api_transcribe_get_value(int connect_d, char* retrieved_file_in_vid_dir_str);
void bind_to_port(int socket, int port);
char* build_http_ok_response(char* final_filename_output, char* results);
char* built_http_data_json_ok_response(char* transcription_str, char* results);
int catch_signal(int sig, void (*handle)(int));
char* create_UNIX_brace_filename(char* bash_arg1, char* bash_arg2);
int is_post(int connect_d, char* api_buffer);
char* itoa(int now  );
char* make_final_filename(char* wav);
char* make_filename_brace_str(char* final_filename_output, char* filename_str);
char* create_wav_filename(char* arg1, char* arg2);
char* num_2_key_str(int num);
int open_listener_socket(void);
void run_data_parser(int connect_d, char* file_filename_output);
char* transcribe_video_method(int conenct_d, char* final_filename_output, char* retrieved_file_in_vid_dir_str);

#endif