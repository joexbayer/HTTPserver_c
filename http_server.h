#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <signal.h>
#include <sys/select.h>

#define NUMBER_OF_ROUTES 50
#define NUMBER_OF_FOLDERS 50
#define NUMBER_OF_HEADERS 50

// colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define HTTP_BUFFER_SIZE 8192 // 8KB


struct http_header
{
	char* method;

	char* route;

	char* query;

	char* content_type;

	char* content;

	char* fragment;

	char* headers[NUMBER_OF_HEADERS];
	int total_headers;

	int keep_alive;
};

struct http_route
{
	char* route;
	char* method;
	void (*http_routefunction)();
};

int http_addfolder(char* folder);
int http_add_responseheader(char* header);
int http_addroute(char* method, char* path, void (*f)());
void http_sendfile(char* file);
void http_sendtext(char* text);
void http_start(int port, int debugmode);
char* http_get_request_header(char* header_name);
char* http_get_parameter(char* variable, int mode);
void http_404();

#endif