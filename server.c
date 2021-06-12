#include "http_server.h"

void test(){
    http_sendfile("index.html");
}

int main()
{

    http_addroute("/", &test, "GET");

    http_addfolder("/");

    // http_start(PORT, DEBUG)
    http_start(8080, 1);

    return 0;
}
