#include "http_server.h"

void home(){

    http_sendfile("index.html");
}

void login(){
    char* username = http_getparameter("username");
    char* password = http_getparameter("password");


    if(strcmp(username, "joe") == 0 && strcmp(password, "123") == 0){

        http_sendfile("index.html");
    }
    http_404();
}

int main()
{

    http_addroute("GET", "/", &home);

    http_addroute("POST", "/login", &login);

    http_addfolder("/");

    // http_start(PORT, DEBUG)
    http_start(8080, 1);

    return 0;
}
