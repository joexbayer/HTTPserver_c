#include "http_server.h"

// todo: POST / GET, enable global indexing with addfoler("/"), ? delimiter as function paramters.

/**************************************************************
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

    http_routes is a list of added routes, it is mirrored with http_routefunctions which
    is a array that counts a pointer to function that is called when a route is accessed.
    http_routecounter is a counter for how many routes has been added.

    To add routes use http_addroute(char* route, void (*f)());

    Example: http_addroute("/dog", &dog);
**************************************************************/
int http_client = -1;
struct http_route* http_routes[NUMBER_OF_ROUTES];
int http_routecounter = 0;

int server_fd;

/**************************************************************
    http_folders contains names of folders that are able to be accessed throuhg indexing.
    If the folders name is inside the indexed path, the file is returned, if present.

    To add folders use http_addfolder(char* folder); 

    Example: http_addfolder("dogs");
**************************************************************/
char* http_folders[NUMBER_OF_FOLDERS];
int http_foldercount = 0;

/*
    Global variables to enable debug mode or allow indexing.
*/
int debug = 0;


struct http_header header;


/**************************************************************
    Summery: 

    http_404 returns the 404 status code. It is called if a file or route is not found.

    PARAMS: NONE
    Returns: VOID
**************************************************************/
void http_404(){
    char *header = "HTTP/1.1 404 Not Found\nContent-Type: text/html\nContent-Length: 13\n\n 404 Not found";
    write(http_client, header, strlen(header)+2);
    if(debug)
        printf("%s\n", "[DEBUG] 404 Response has been sent.");
    close(http_client);
}


/**************************************************************
    Summery: 

    Makes a route accessible and calls user defined function. 
    The user defined functions must have return type of void, and have no parameters.

    PARAMS: name of route, function pointer.
    Returns: VOID
**************************************************************/
void http_addroute(char* method, char* path, void (*f)()){

    struct http_route* route = malloc(sizeof(struct http_route));
    route->method = method;
    route->route = path;
    route->http_routefunction = f;

    http_routes[http_routecounter] = route;
    http_routecounter++;

}

/**************************************************************
    Summery: 

    Adds a folder name to a list of indexable folders.
    If folder name is included in the route, all files inside the folder will be indexable.

    PARAMS: name of folder
    Returns: VOID
**************************************************************/
void http_addfolder(char* folder){
    http_folders[http_foldercount] = folder;
    http_foldercount++;
}


/**************************************************************
    Summery: 

    Handles parsing of given route and returning files or run given functions.
    If alle files are indexable it tries to send indexed file.
    If files are not indexable it will go through all routes and check if it has been added.
    Then run according function. At last it will go through all folders and see if route
    includes the folder name.

    If none of the above occur 404 will be returned.




    PARAMS: 
    Returns: VOID
**************************************************************/
void http_route_handler(){

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
    http_404();
}
/**************************************************************
    Summery: 

    Parses parameters and returns the value of the selected variable from the request.

    PARAMS: name of variable
    Returns: value of variable
**************************************************************/
char* http_getparameter(char* variable){
    printf("%s\n", header.parameters);
    char* variable_name;
    char* parameter = malloc(strlen(header.parameters)+1);
    strcpy(parameter, header.parameters);
    if(strstr(header.parameters, "&") != NULL){
        char* parameter_tok = strtok(parameter, "&");
        while(parameter_tok != NULL && (strstr(parameter_tok, variable) == NULL)){
            parameter_tok = strtok(NULL, "&");
        }
        variable_name = strtok(parameter_tok, "=");
    } else {
        variable_name = strtok(header.parameters, "=");
    }
    if(variable_name != NULL && strcmp(variable, variable_name) == 0){
        char* variable_value = strtok(NULL, "=");
        free(parameter);
        return variable_value;
    }
    free(parameter);
    return NULL;
}


void intHandler(){
    printf("\n%s\n", "[SHUTDOWN] Server is shutting down...");
    for (int i = 0; i < http_routecounter; ++i)
    {
        free(http_routes[http_routecounter]);
    }
    close(server_fd);
    printf("%s\n", "[SHUTDOWN] Goodbye.");
    exit(0);
}

/**************************************************************
    Summery: 

    Will return file with given filename. First it checks if file exsists.
    If not 404 will be returned. If it does exist the file will be loaded into
    memory then added to the repose header and send to client.


    PARAMS: name of file
    Returns: VOID
**************************************************************/
void http_sendfile(char* file){

    if(access(file, F_OK)) {
        http_404();
        return;
    }
     // open reponse file
    FILE *fp = fopen(file, "rb");
    if ( NULL == fp ) {
        perror("FILE");
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

    // allocate response buffer
    char buff[content_size+100];

    //server response header HTTP format
    char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

    // add content length and content
    strcpy(buff, header);
    strcat(buff, filesizestr);
    strcat(buff, "\n\n");
    strcat(buff, content);

    write(http_client, buff, content_size+100);
    if(debug)
        printf("%s\n", "[DEBUG] File has been sent.");
    close(http_client);
}

/**************************************************************
    Summery: 

    Parses the given http request, and fills http_header.
    
    PARAMS: request buffer
    Returns: VOID
**************************************************************/
void http_parser(char* buffer){
    // get method
    char* get = strtok(buffer, " ");
    header.method = get;
    get = strtok(NULL, " ");

    // check for url parameters
    if(strstr(get, "?") != NULL){
        char* parameters = strtok(get, "?");
        parameters = strtok(NULL, "?");
        header.parameters = parameters;
    }
    header.route = get;

    // check all lines
    char* line = strtok(NULL, "\n");
    char* content_type_raw;
    char* content;
    while(line != NULL){

        // content type
        if(strstr(line, "Content-Type") != NULL){
            content_type_raw = line;
        }
        content = line;
        line = strtok(NULL, "\n");
    }

    // get http content type
    char* content_type = strtok(content_type_raw, " ");
    content_type = strtok(NULL, " ");
    header.content_type = content_type;
    header.content_type[33] = 0;

    // if content type is from form, set content has parameters
    if(strcmp(header.content_type, "application/x-www-form-urlencoded") == 0){
        header.parameters = content;
    }
    header.content = content;

    http_route_handler();
}

/**************************************************************
    Summery: 

    Creates tcp socket with given port and will set global variables.
    After tcp socket is created it will accept clients then create a new chil process to handle request.
    While parent process keeps accepting new clients.

    PARAMS: PORT,  set debugmode
    Returns: VOID
**************************************************************/
void http_start(int PORT, int debugmode){
    printf("%s\n", "[STARTUP] Starting HTTP server on port 8080.");
    debug = debugmode;

    if(debug)
        printf("%s\n", "[STARTUP] Debug mode is active");

    
    //Define sockets
    long valread;
    /*
    The htons() function makes sure that numbers are stored in memory in network byte order, which is with the most significant byte first.
    */
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    /*server socket
    AF_INET = IP address family
    SOCK_STREAM = virtual circuit service.
    */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        //error handling
        perror("FD socket");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", "[STARTUP] TCP socket succesfully created.");

    //define sockaddr_in struct variables
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    /*
    The C library function void *memset(void *str, int c, size_t n)
    copies the character c (an unsigned char) to the first n
    characters of the string pointed to, by the argument str.
    */

    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        exit(1);

    //bind server socket to sockaddr
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        //error handling
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    //set socket to listen
    if (listen(server_fd, 10) < 0)
    {
        //error handling
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", "[STARTUP] Server now accepting requests...");

    signal(SIGINT, intHandler);

    //listen loop
    while(1)
    {
        //accpet new socket connection
        //int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        if ((http_client = accept(server_fd, (struct sockaddr *)&addrlen, (socklen_t*)&addrlen)) < 0)
        {
            //error handling
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if(debug)
            printf("%s", "[DEBUG] Incomming request for ");

        if(fork() == 0){
            //child ->

            //prepare buffer
            char buffer[HTTP_BUFFER_SIZE] = {0};
            valread = read(http_client, buffer, HTTP_BUFFER_SIZE);

            printf("%s\n", buffer);

            http_parser(buffer);
            close(server_fd);
            exit(1);
        }

        close(http_client);
    }
}