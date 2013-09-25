#include "wrap.h"
#include "parse.h"

int main(void) {
    char *buf, *p;
    char name[MAXLINE], passwd[MAXLINE],content[MAXLINE];

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
	p = strchr(buf, '&');
	*p = '\0';
	strcpy(name, buf);
	strcpy(passwd, p+1);
    }


    /* Make the response body */
    sprintf(content, "Welcome to auth.com:%s and %s\r\n<p>",name,passwd);
    sprintf(content, "%s\r\n", content);

    sprintf(content, "%sThanks for visiting!\r\n", content);
  
    /* Generate the HTTP response */
    printf("Content-length: %d\r\n", strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);
    exit(0);
}
