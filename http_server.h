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

#define NUMBER_OF_ROUTES 50
#define NUMBER_OF_FOLDERS 50
#define HTTP_BUFFER_SIZE 2048

struct http_header
{
	char* method;
	char* route;
	char* query;
	char* content_type;
	char* content;
	char* fragment;
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
void http_sendhtml(char* file);
void http_sendtext(char* text);
void http_start(int port, int debugmode);
char* http_get_parameter(char* variable, int mode);
void http_404();

#endif