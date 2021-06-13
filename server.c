#include "http_server.h"

void test(){
    char* age = http_getparameter("age");
    char* name = http_getparameter("name");

    printf("%s\n", name);
    printf("%s\n", age);



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
