#ifndef __HTTP_SERVER_H
#define __HTTP_SERVER_H

#include "syshead.h"
#include "http_status.h"

#define NUMBER_OF_ROUTES 50
#define NUMBER_OF_FOLDERS 50
#define NUMBER_OF_HEADERS 50

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


void http_redirect(char* location);
int http_addfolder(char* folder);
int http_add_responseheader(char* header);
int http_addroute(char* method, char* path, void (*f)());
void http_sendfile(char* file);
void http_sendtext(char* text);
void http_start(int port, int debugmode);
char* http_get_request_header(char* header_name);
char* http_get_parameter(char* variable, int mode);

#endif