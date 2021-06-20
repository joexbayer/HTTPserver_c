#include "http_server.h"
#include "utils.h"

/*
    Standards and Syntax:

    Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
    https://datatracker.ietf.org/doc/html/rfc7230

    Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
    https://datatracker.ietf.org/doc/html/rfc7231

    Hypertext Transfer Protocol (HTTP/1.1): Conditional Requests
    https://datatracker.ietf.org/doc/html/rfc7232

    Hypertext Transfer Protocol (HTTP/1.1): Range Requests
    https://datatracker.ietf.org/doc/html/rfc7233

    Hypertext Transfer Protocol (HTTP/1.1): Caching
    https://datatracker.ietf.org/doc/html/rfc7234

    Hypertext Transfer Protocol (HTTP/1.1): Authentication
    https://datatracker.ietf.org/doc/html/rfc7235

    Secruity: 9.x RFC 7230

    Recipients ought to carefully limit the extent to which they process
    other protocol elements, including (but not limited to) request
    methods, response status phrases, header field-names, numeric values,
    and body chunks.  Failure to limit such processing can result in
    buffer overflows, arithmetic overflows, or increased vulnerability to
    denial-of-service attacks.

*/

/*************************************************************
    http_routes is a list of added routes.
    To add routes use http_addroute(char* route, void (*f)());

    Example: http_addroute("/dog", &dog);
**************************************************************/
int http_client = -1;
struct http_route* http_routes[NUMBER_OF_ROUTES];
int http_routecounter = 0;


int http_server_fd;

struct sockaddr_in address, client_addr;

int http_request_counter = 0;

/**************************************************************
    http_folders contains names of folders that are able to be accessed throuhg indexing.
    If the folders name is inside the indexed path, the file is returned, if present.

    To add folders use http_addfolder(char* folder); 

    Example: http_addfolder("dogs");
**************************************************************/
char* http_folders[NUMBER_OF_FOLDERS];
int http_foldercount = 0;

// custom headers to be added after server start.
char* http_custom_headers[10];
int http_total_custom_header = 0;

int debug = 0; // Global variable to enable debug mode.

struct http_header header;// request header, will be filled by http_parser

char* http_default_header = "Server: UniqueHttpd (Unix)\n";

char* http_response_header;



/**************************************************************
    Loops through all routes and frees the allocted memory
**************************************************************/
void http_free_routes(){

    for (int i = 0; i < http_routecounter; ++i)
    {
        free(http_routes[i]);
    }
    free(http_response_header);
}


/**************************************************************
    Redirects request to given location
**************************************************************/
void http_redirect(char* location){
    http_301(http_client, location);
}


/**************************************************************
    Loops through all routes and frees the allocted memory
**************************************************************/
void http_setup_header(){
    http_response_header = malloc(strlen(http_default_header)+2);
    strcpy(http_response_header, http_default_header);
    http_response_header[strlen(http_default_header)+1] = 0;
}

/**************************************************************
    Summery: 

    Adds given header to http_response_header!

    !header MUST not include newline (\n)

    @PARAMS: header to be added.
    @returns: length of header.
**************************************************************/
int http_add_responseheader(char* header){

    // if server has not been started, add to waitlist
    if(http_response_header == NULL){
        if(http_total_custom_header == 10){
            return -1;
        }

        http_custom_headers[http_total_custom_header] = header;
        http_total_custom_header++;
        
        return strlen(header);
    }

    // normal procedure
    int total_length = strlen(http_response_header)+strlen(header)+2;

    char* result = malloc(total_length);

    strcpy(result, http_response_header);
    strcat(result, header);
    strcat(result, "\n");
    result[total_length-1] = 0;
    free(http_response_header);
    http_response_header = result;
    return strlen(header);
}

/**************************************************************
    Summery: 

    Abstraction to add content type header.

    @PARAMS: content type value
    @returns: length of header.
**************************************************************/
int http_add_content_type(char* content_type_value){


    char* content_type = "Content-Type: ";

    int total_length = strlen(content_type)+strlen(content_type_value)+2;

    char* result = malloc(total_length);

    strcpy(result, content_type);
    strcat(result, content_type_value);
    result[total_length-1] = 0;

    http_add_responseheader(result);

    free(result);

    return strlen(content_type_value);
}




/**************************************************************
    Summery: 

    Makes a route accessible and calls user defined function. 
    The user defined functions must have return type of void, and have no parameters.

    All general-purpose servers MUST support the methods GET and HEAD.

    @PARAMS: name of route, function pointer.
    @returns: number of total routes, -1 on error
**************************************************************/
int http_addroute(char* method, char* path, void (*f)()){

    if(http_routecounter == NUMBER_OF_ROUTES){
        return -1;
    }

    struct http_route* route = malloc(sizeof(struct http_route));
    route->method = method;
    route->route = path;
    route->http_routefunction = f;

    http_routes[http_routecounter] = route;
    http_routecounter++;

    return http_routecounter;

}

/**************************************************************
    Summery: 

    Adds a folder name to a list of indexable folders.
    If folder name is included in the route, all files inside 
    the folder will be indexable.

    @PARAMS: name of folder
    @returns: number of folder, -1 on error.
**************************************************************/
int http_addfolder(char* folder){

    if(http_foldercount == NUMBER_OF_FOLDERS){
        return -1;
    }

    http_folders[http_foldercount] = folder;
    http_foldercount++;

    return http_foldercount;
}


/**************************************************************
    Summery: 

    Handles parsing of given route and returning files or run given functions.
    Routes are always prioritized before folder indexing.
    If no routes have been found, the server will check for folders

    If none of the above occur 404 will be returned.

    @PARAMS: void
    @returns: void
**************************************************************/
void http_route_handler(){


    header.route = strtok(header.route, "?");
    printf("%s\n", header.query);

    for (int i = 0; i < http_routecounter; ++i)
    {
        // checks if both route and method is correct.
        if((strcmp(header.route, http_routes[i]->route) == 0) && (strcmp(header.method, http_routes[i]->method) == 0)){
            (*(http_routes[i]->http_routefunction))();
            return;
        }
    }

    for (int i = 0; i < http_foldercount; ++i)
    {
        if(strstr(header.route, http_folders[i]) != NULL && strcmp(header.method, "POST") != 0){
            // add . inforont of path
            char file[strlen(header.route)+2];
            char* dot = ".";
            strcpy(file, dot);
            strcat(file, header.route);
            file[strlen(header.route)+2] = 0;

            http_sendfile(file);
            return;
        }
    }
    http_404(http_client);
}
/**************************************************************
    Summery: 

    Parses query / fragment parameters and returns the value of
    the selected variable from the request.
    
    Modes:
        0 = query
        1 = fragment

    @PARAMS: name of variable, int as selected mode
    @returns: value of variable, NULL on error.
**************************************************************/
char* http_get_parameter(char* variable, int mode){

    char* parameter;
    char* variable_name;

    if(!mode){
        parameter = malloc(strlen(header.query)+1);
        strcpy(parameter, header.query);
    } else {
        parameter = malloc(strlen(header.fragment)+1);
        strcpy(parameter, header.fragment);
    }
    char* delimiter = "&";

    if(strstr(parameter, delimiter) != NULL){
        char* parameter_tok = strtok(parameter, delimiter);
        while(parameter_tok != NULL && (strstr(parameter_tok, variable) == NULL)){
            parameter_tok = strtok(NULL, delimiter);
        }
        variable_name = strtok(parameter_tok, "=");
    } else {
        variable_name = strtok(header.query, "=");
    }
    if(variable_name != NULL && strcmp(variable, variable_name) == 0){
        char* variable_value = strtok(NULL, "=");
        free(parameter);
        return variable_value;
    }
    free(parameter);
    return NULL;
}


/**************************************************************
    Summery: 

    Searches request-headers for given header name and returns
    the value if found.

    @PARAMS: name of header
    @returns: value of header, NULL on error.
**************************************************************/
char* http_get_request_header(char* header_name){

    char* current_header;

    for (int i = 0; i < header.total_headers; ++i)
    {
        // possible malloc nullbyte error
        current_header = malloc(strlen(header.headers[i]));
        memcpy(current_header, header.headers[i], strlen(header.headers[i]));
        current_header[strlen(header.headers[i])] = 0;
        char* current_header_name = strtok(current_header, " ");

        if(current_header != NULL && strcmp(header_name, current_header_name) == 0){
            char* header_value = strtok(NULL, " ");
            free(current_header);
            return header_value;
        }

        free(current_header);
    }

    return NULL;
}

/**************************************************************
    Interrupt handler on CTRL-C
**************************************************************/
void intHandler(){
    printf("%s\n", "[CLOSING] Closing connection...");
    http_free_routes();
    close(http_server_fd);
    printf(KRED "%s PID: %ld, PORT: %d!.\n" KWHT, "[CLOSING] Goodbye ", (long)getpid(), client_addr.sin_port);
    exit(0);
}

/**************************************************************
    Summery: 

    Will return file with given filename. First it checks if file exists.
    If not 404 will be returned. If it does exist the file will be loaded into
    memory then added to the response header and send to client.


    @PARAMS: name of file
    @returns: void
**************************************************************/
void http_sendfile(char* file){

    if(access(file, F_OK)) {
        http_404(http_client);
        return;
    }

    // get file extension
    char file_copy[strlen(file)+1];
    strcpy(file_copy, file);
    file_copy[strlen(file)+1] = 0;

    char* file_ext = strtok(file_copy, ".");
    file_ext = strtok(NULL, ".");

    if(strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "png") == 0 || strcmp(file_ext, "jpeg") == 0 || strcmp(file_ext, "ico") == 0){
        http_add_content_type("image/gif");
    } else if(strcmp(file_ext, "html") == 0){
        http_add_content_type("text/html");
    } else if(strcmp(file_ext, "js") == 0){
        http_add_content_type("text/javascript;charset=UTF-8");
    } else {
        http_add_content_type("text/plain");
    }

     // open reponse file
    FILE *fp = fopen(file, "rb");
    if ( NULL == fp ) {
        perror("FILE");
        http_free_routes();
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);

    // get file info
    struct stat finfo;
    int fs = fstat(fd, &finfo);
    if(fs == -1){
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    // get content size
    int content_size = finfo.st_size;

    // file length int to string hack
    char filesizestr[(int)((ceil(log10(content_size))+1)*sizeof(char))];
    sprintf(filesizestr, "%d", content_size);

    // content buffer
    char content[content_size];

    // read file into content buffer
    int readf = fread(content, content_size, 1, fp);
    if(readf != 1){
        exit(EXIT_FAILURE);
    }

    fclose(fp);

    // allocate response buffer for content and reponse header
    char buff[content_size+100+strlen(http_response_header)];

    //server response header HTTP format
    char *header = "HTTP/1.1 200 OK\n";
    // add content length and content
    strcpy(buff, header);
    // add custom http_response_header
    strcat(buff, http_response_header);
    // add content length header
    strcat(buff, "Content-Length: ");
    strcat(buff, filesizestr);
    strcat(buff, "\n\n");

    // write header
    write(http_client, buff, strlen(buff));

    // write content
    write(http_client, content, content_size);

    write(http_client, buff, strlen(buff));

}


/**************************************************************
    Summery: 

    Will send given text.

    @PARAMS: char* text to send
    @returns: void
**************************************************************/
void http_sendtext(char* text){
    char buff[strlen(text)+100+strlen(http_response_header)];

    //server response header HTTP format
    char *header = "HTTP/1.1 200 OK\n";

    http_add_content_type("text/plain");
    // add content length and content
    strcpy(buff, header);
    // add custom http_response_header
    strcat(buff, http_response_header);
    // add content length header
    strcat(buff, "Content-Length: ");


    int text_size = strlen(text);

    // int to string hack
    char size[(int)((ceil(log10(text_size))+1)*sizeof(char))];
    sprintf(size, "%d", text_size);

    strcat(buff, size);
    strcat(buff, "\n\n");

    // add content
    strcat(buff, text); // use memcpy instead

    write(http_client, buff, strlen(buff));
    if(debug)
        printf("%s\n", "[DEBUG] File has been sent.");
}

/**************************************************************
    Summery: 

    Parses the given http request, and fills http_header.

    2.1.  Client/Server Messaging /rfc7230
    A client sends an HTTP request to a server in the form of a request
    message, beginning with a request-line that includes a method, URI,
    and protocol version (Section 3.1.1), followed by header fields
    containing request modifiers, client information, and representation
    metadata (Section 3.2), an empty line to indicate the end of the
    header section, and finally a message body containing the payload
    body (if any, Section 3.3).

    http-URI = "http:" "//" authority path-abempty [ "?" query ] [ "#" fragment ]

    A server MUST respond with a 400 (Bad Request) status code to any
    HTTP/1.1 request message that lacks a Host header field and to any
    request message that contains more than one Host header field or a
    Host header field with an invalid field-value.

    An origin server that receives a Content-Location field in a request
    message MUST treat the information as transitory request context
    rather than as metadata to be saved verbatim as part of the
    representation.
    
    @PARAMS: request buffer
    @returns: VOID
**************************************************************/
void http_parser(char* buffer, char* content){
    // get method
    char* get = strtok(buffer, " ");
    header.method = get;
    header.total_headers = 0;
    get = strtok(NULL, " ");

    char* uri = get;

    char* host = NULL;
    char* connection = NULL;
    char* line = strtok(NULL, "\n");

    char* content_type_raw = NULL;
    while(line != NULL){
        // break on end of header
        if(strcmp(line, " ") < 0){
            break;
        }
        // content type
        if(strstr(line, "Content-Type") != NULL){
            content_type_raw = line;
        }

        // check for host header
        if(strstr(line, "Host:") != NULL){
            host = line;
        }

        if(strstr(line, "Connection:") != NULL){
            connection = line;
        }

        header.headers[header.total_headers] = line;
        header.total_headers++;

        line = strtok(NULL, "\n");

    }

    /*
        5.4 - RFC 7230
        A server MUST respond with a 400 (Bad Request) status code to any
        HTTP/1.1 request message that lacks a Host header field
    */
    if(host == NULL){
        http_400(http_client);
        exit(1);
    }

    // handle potential keep alive header
    if(connection != NULL){
        char* connection_type = strtok(connection, " ");
        connection_type = strtok(NULL, " ");
        if(strstr(connection_type, "keep-alive") != NULL){
            http_add_responseheader("Connection: keep-alive");
            header.keep_alive = 1;
        }
    }

    // get http content type
    if(content_type_raw != NULL){
        char* content_type = strtok(content_type_raw, " ");
        content_type = strtok(NULL, " ");
        header.content_type = content_type;

            // if content type is from form, set content has parameters
        if(strstr(header.content_type, "application/x-www-form-urlencoded") != NULL){
            header.query = content;
        }

        header.content = content;
    }

    // check for uri query
    if(strstr(uri, "?") != NULL){
        char* query = strtok(uri, "?");
        query = strtok(NULL, "?");
        header.query = query;
    }
        // check for uri fragment
    if(strstr(uri, "#") != NULL){
        char* fragment = strtok(uri, "#");
        fragment = strtok(NULL, "#");
        header.fragment = fragment;
    }

    header.route = uri;

}

/**************************************************************
    Summery: 

    Handles if a socket closes

    @PARAMS: VOID
    @returns: VOID
**************************************************************/
void sigpipe_handler()
{
    printf(KRED "[ERROR] HTTP client socket has closed unexpectedly!\n" KWHT);
    // close connection
    http_free_routes();
    close(http_server_fd);
    close(http_client);
    if(debug)
        printf(KMAG "%s PID: %ld, PORT %d!.\n" KWHT, "[DEBUG] Child process ended! - ", (long)getpid(), client_addr.sin_port);
    exit(0);
}

/**************************************************************
    Summery: 

    Handles a request. With connection keep-alive;

    @PARAMS: PORT,  set debugmode
    @returns: VOID
**************************************************************/
void http_handle_request(char* buffer){
    //child ->
    const char *delim = "\r\n\r\n";

    signal(SIGPIPE,sigpipe_handler);

    if(debug){
        printf(KYEL "%s\n" KWHT, buffer);
    }

    char* buffer_header = strstr(buffer, delim);
    http_parser(buffer, buffer_header+strlen(delim));

    if(debug)
        printf("%s\n", "--------- Running user defined functions --------");

    http_route_handler();

    if(debug)
        printf("%s\n", "-------- Finished user defined functions --------");

    // if keep alive
    if(header.keep_alive == 1){
        // a 32 empty request limit for keep alive
        int empty_request_limit = 32;
        while(empty_request_limit > 0){

            fd_set readSockSet;
            struct timeval timeout;
            FD_ZERO(&readSockSet);
            FD_SET(http_client, &readSockSet);

            // wait for new connection for 8 seconds.
            timeout.tv_sec = 8;
            timeout.tv_usec = 0;

            int retval = select(FD_SETSIZE, &readSockSet, NULL, NULL, &timeout);
            if(retval > 0){
                //printf("%s %d %d\n", "[CHILD] New request from keep-alive", client_addr.sin_port, client_addr.sin_family);
                char buffer2[HTTP_BUFFER_SIZE-1] = {0};
                int valread = recv(http_client, buffer2, HTTP_BUFFER_SIZE, 0);
                buffer2[HTTP_BUFFER_SIZE-2] = 0;

                if(valread == 0 || buffer2[0] == 0){
                    usleep(250);
                    empty_request_limit--;
                    continue;
                } else {
                    free(http_response_header);
                    http_setup_header();
                    printf("%s\n", "[CHILD] Handling new request!");
                    http_handle_request(buffer2);
                    return;
                }
            } else {
                break;
            }
        }
    }
    // close connection
    http_free_routes();
    close(http_server_fd);
    close(http_client);
    if(debug)
        printf(KMAG "%s PID: %ld, PORT: %d!.\n" KWHT, "[DEBUG] Child process ended! - ", (long)getpid(), client_addr.sin_port);
    exit(0);
}


/**************************************************************
    Summery: 

    Creates tcp socket with given port and will set global variables.
    After tcp socket is created it will accept clients then create a new chil process to handle request.
    While parent process keeps accepting new clients.
    
    2.1.  Client/Server Messaging - rfc7230
    An HTTP "server" is a program
    that accepts connections in order to service HTTP requests by sending
    HTTP responses.

        request   >
    UA ======================================= O
                               <   response


    @PARAMS: PORT,  set debugmode
    @returns: VOID
**************************************************************/
void http_start(int PORT, int debugmode){
    printf(KBLU "%s %d\n" KWHT, "[STARTUP] Starting HTTP server on port", PORT);
    debug = debugmode;

    if(debug)
        printf(KBLU "%s\n" KWHT, "[STARTUP] Debug mode is active");

    
    long valread;
    int addrlen = sizeof(address);

    /*server socket
    AF_INET = IP address family
    SOCK_STREAM = virtual circuit service.
    */
    if ((http_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        //error handling
        perror("FD socket");
        exit(EXIT_FAILURE);
    }

    printf(KBLU "%s\n" KWHT, "[STARTUP] TCP socket succesfully created.");

    //define sockaddr_in struct variables
    /*
    The htons() function makes sure that numbers are stored in memory in network byte order, which is with the most significant byte first.
    */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    /*
    The C library function void *memset(void *str, int c, size_t n)
    copies the character c (an unsigned char) to the first n
    characters of the string pointed to, by the argument str.
    */

    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (setsockopt(http_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        exit(1);

    //bind server socket to sockaddr
    if (bind(http_server_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        //error handling
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    //set socket to listen
    if (listen(http_server_fd, 10) < 0)
    {
        //error handling
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf(KBLU "%s\n" KWHT, "[STARTUP] Server now accepting requests...");

    signal(SIGINT, intHandler);

    // setup for response header;

    // check for headers in waitlist
    if(http_total_custom_header > 0){
        for (int i = 0; i < http_total_custom_header; ++i)
        {
            // add headers in waitlist
            http_add_responseheader(http_custom_headers[i]);
        }
    }

    client_addr.sin_port = 0;

    http_setup_header();

    //listen loop
    while(1)
    {
        /*
            Main to accept new clients
        */
        //accpet new socket connection
        //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        if ((http_client = accept(http_server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) < 0)
        {
            //error handling
            perror("accept");
            intHandler();
            exit(EXIT_FAILURE);
        }

        http_request_counter++;
        if(fork() == 0){

            if(debug)
                printf(KMAG "%s PID: %ld, PORT: %d!.\n" KWHT, "[DEBUG] Child process started! - ", (long)getpid(), client_addr.sin_port);

            /*
                Setup FD set with select to timeout socket that doesnt send anything.
            */

            if(debug)
                printf(KGRN "%s PID: %ld, PORT: %d\n" KWHT, "[DEBUG] Accepted new connection, waiting for request...",(long)getpid(), client_addr.sin_port);

            // set FD sets
            fd_set readSockSet;
            struct timeval timeout;
            FD_ZERO(&readSockSet);
            FD_SET(http_client, &readSockSet);

            // wait for request for 3 seconds.
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
            // select for http_client
            int retval = select(FD_SETSIZE, &readSockSet, NULL, NULL, &timeout);
            if(retval > 0){

                //prepare buffer and read from client
                char buffer[HTTP_BUFFER_SIZE] = {0};
                valread = recv(http_client, buffer, HTTP_BUFFER_SIZE-1, 0);
                buffer[HTTP_BUFFER_SIZE-2] = 0;

                // if empty request is recived close.
                if(valread == 0){
                    printf("%s\n", "[ERROR] Empty request!");
                    close(http_client);
                    intHandler();
                }

                // send request to handle function
                http_handle_request(buffer);

            } else {
                // if select timed out
                printf("%s PID: %ld, PORT: %d\n", "[DEBUG] Incomming connection timed out!", (long)getpid(), client_addr.sin_port);
                close(http_client);
                intHandler();
            }
        } else {
            close(http_client);
            client_addr.sin_port = 0;
        }

    }
}