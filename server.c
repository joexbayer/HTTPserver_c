#include "http_server.h"

void home(){

    printf("%s\n", http_get_request_header("Host:"));

    http_sendfile("www/index.html");
}

void text(){

    http_sendtext("text\ntext");
}

void favicon(){
    http_sendfile("www/favicon.ico");
}

// login example
void login(){
    // 0 = query, 1 = fragment
    char* username = http_get_parameter("username", 0);
    char* password = http_get_parameter("password", 0);

    if(strcmp(username, "joe") == 0 && strcmp(password, "123") == 0){


        http_add_cookie("login", "true");

        http_redirect("/?success=1");
        return;
    }

    http_redirect("/?success=0");
}

int main()
{

    http_addroute("GET", "/", &home);
    http_addroute("POST", "/login", &login);
    http_addroute("GET", "/text", &text);
    http_addroute("GET", "/favicon.ico", &favicon);
    http_addfolder("/");

    // http_start(PORT, DEBUG)
    http_start(8081, 1);

    return 0;
}
