#include "utils.h"
#include "syshead.h"


// HTTP pre defined status replies


/**************************************************************
    Summery: 

    http_400 returns the 400 bad request. It is called when headers are incorrect.

    @PARAMS:client fd
    @returns: VOID
**************************************************************/
int http_400(int client){
    char *header = "HTTP/1.1 400 Bad Request \nContent-Type: text/html\nContent-Length: 16\n\n 400 Bad Request";
    int w = write(client, header, strlen(header)+2);
    if(w == 0){
         printf(KRED "%s\n" KWHT, "[ERROR] 400 Response could not be sent!");
    } else {
        printf("%s\n", "[LOG] 400 Reponse was sent");
    }
    return w;
}

/**************************************************************
    Summery: 

    http_404 returns the 404 status code. It is called if a file or route is not found.

    @PARAMS: client fd
    @returns: VOID
**************************************************************/
int http_404(int client){
    char *header = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: 13\n\n 404 Not found";
    int w = write(client, header, strlen(header)+2);
    if(w == 0){
         printf(KRED "%s\n" KWHT, "[ERROR] 404 Response could not be sent!");
    }
    printf("%s\n", "[LOG] 404 Response has been sent.");
    return w;
}


/**************************************************************
    Summery: 

    http_301 returns the 301 status code. Redirects to given location

    @PARAMS: client fd, location string
    @returns: VOID
**************************************************************/
int http_301(int client, char* location, char* extra_headers){

    char *pre_header = "HTTP/1.1 301 Moved Permanently\nConnection: Close\nLocation: ";
    int header_length = strlen(pre_header)+strlen(location)+strlen(extra_headers)+3;

    char header[header_length];

    strcpy(header, pre_header);
    strcat(header, location);
    strcat(header, extra_headers);
    strcat(header, "\n\n");
    header[header_length] = 0;

    int w = write(client, header, strlen(header)+1);
    if(w == 0){
         printf(KRED "%s\n" KWHT, "[ERROR] 301 Response could not be sent!");
    }
    printf("%s %s\n", "[LOG] 301 Response. Client has been redirected too ", location);
    return w;
}