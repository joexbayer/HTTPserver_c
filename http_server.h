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

#define NUMBER_OF_ROUTES 50
#define NUMBER_OF_FOLDERS 50
#define HTTP_BUFFER_SIZE 2048

struct http_header
{
	char* method;
	char* route;
	char* parameters;
};

struct http_route
{
	char* route;
	char* method;
	void (*http_routefunction)();
};

void http_addfolder(char* folder);
void http_addroute(char* path, void (*f)(), char* method);
void http_routehandler();
void http_sendfile(char* file);
void http_start(int port, int debugmode);
char* http_getparameter(char* variable);

#endif