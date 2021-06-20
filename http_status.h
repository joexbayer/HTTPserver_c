#ifndef __HTTP_ERRORS_H
#define __HTTP_ERRORS_H

int http_400(int client);
int http_404(int client);
int http_301(int client, char* location);

#endif
