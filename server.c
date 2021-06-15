#include "http_server.h"

void home(){

    http_sendhtml("index.html");
}

void text(){

    http_sendtext("text\ntext");
}

// login example
void login(){
    // 0 = query, 1 = fragment
    char* username = http_get_parameter("username", 0);
    char* password = http_get_parameter("password", 0);


    if(strcmp(username, "joe") == 0 && strcmp(password, "123") == 0){

        http_sendhtml("index.html");
        return;
    }
    http_404();
}

int main()
{

    http_addroute("GET", "/", &home);

    http_addroute("POST", "/login", &login);

    http_addroute("GET", "/text", &text);

    http_addfolder("/");

    http_add_responseheader("Content-Security-Policy: script-src 'unsafe-inline';");

    // http_start(PORT, DEBUG)
    http_start(8080, 1);

    return 0;
}
