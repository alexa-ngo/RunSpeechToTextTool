#ifndef SERVER
#define SERVER

/*
    This is the declaration of the server.
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define BYTES_OF_DATA_30 30
#define BYTES_OF_DATA_100000 100000
#define DOES_NOT_EXIST -1
#define FALSE 0
#define LETTER_OF_A 65
#define MAX_5_NUM_OF_RUNNING_PROCESSES 5
#define SENDING_DATA_WAS_SUCCESSFUL 0
#define TOTAL_CHAR_OF_ALPHABET 26
#define TRUE 1

void bind_to_port(int socket, int port);
int catch_signal(int sig, void (*handler)(int));
char* data_longer_than_or_equal_to_key(int key_length, char* key_str, int data_length, char* data_str);
void kill_the_process(void);
int open_listener_socket(void);
int read_in(int socket, char* buf, int len);
int say(int socket, char* s);

#endif