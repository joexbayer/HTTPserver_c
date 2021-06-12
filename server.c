#include "http_server.h"

void test(){
    http_sendfile("index.html");
}

int main()
{

    http_addroute("/joe", &test);

    http_addfolder("testdir");

    // http_start(PORT, DEBUG)
    http_start(8080, 1);

    return 0;
}
