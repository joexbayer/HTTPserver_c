#include "utils.h"
#include "syshead.h"

char* find_content_type(char* file_ext){
	if(strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "png") == 0 || strcmp(file_ext, "jpeg") == 0 || strcmp(file_ext, "ico") == 0){
        return "image/gif";
    } else if(strcmp(file_ext, "html") == 0){
        return "text/html";
    } else if(strcmp(file_ext, "js") == 0){
        return "text/javascript;charset=UTF-8";
    } else {
        return "text/plain";
    }
    return NULL;
}