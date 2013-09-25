#include "wrap.h" 
#include "parse.h"
#include "daemon_init.h"

void doit(int fd);
void get_requesthdrs(rio_t *rp);
void post_requesthdrs(rio_t *rp,int *length);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void serve_dir(int fd,char *filename);
void get_filetype(char *filename, char *filetype);
void get_dynamic(int fd, char *filename, char *cgiargs);
void post_dynamic(int fd, char *filename, int contentLength,rio_t *rp); 
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);

static int isShowdir=1;
char *cwd;

int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    pid_t pid;
    struct sockaddr_in clientaddr;
    char daemon=0,*portp=NULL,*logp=NULL,tmpcwd[MAXLINE];

    cwd=(char*)get_current_dir_name();	
    strcpy(tmpcwd,cwd);
    strcat(tmpcwd,"/");
    /* parse argv */
    parse_option(argc,argv,&daemon,&portp,&logp);
    portp==NULL ?(port=atoi(getconfig("port"))) : (port=atoi(portp));

    /* init log */
    if(logp==NULL) 
    	logp=getconfig("log");
    initlog(strcat(tmpcwd,logp));

    /* where show dir */
    if(strcmp(getconfig("dir"),"no")==0)
    	isShowdir=0;	
    	
    if(daemon==1||strcmp(getconfig("daemon"),"yes")==0)
	init_daemon();

    listenfd = Open_listenfd(port);

    while (1)
    {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
	if(access_ornot(inet_ntoa(clientaddr.sin_addr))==0)
	{
	    clienterror(connfd,"maybe this web server not open to you!" , "403", "Forbidden", "Tiny couldn't read the file");
	    continue;
	}

	if((pid=Fork())>0)
	{
		close(connfd);
		continue;
	}
	else if(pid==0)
	{
		doit(connfd);
	}

    }
}
/* $end main */


/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static,contentLength=0,isGet=1;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
  
    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcasecmp(method, "GET")!=0&&strcasecmp(method,"POST")!=0) 
    { 
       clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
        return;
    }

    
    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    

    if (lstat(filename, &sbuf) < 0) 
    {
	clienterror(fd, filename, "404", "Not found",
		    "Tiny couldn't find this file");
	return;
    }

    if(S_ISDIR(sbuf.st_mode)&&isShowdir)
    	serve_dir(fd,filename);


    if (strcasecmp(method, "POST")==0) 
    	isGet=0;


    if (is_static) 
    { /* Serve static content */
    	get_requesthdrs(&rio);

	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) 
	{
	    clienterror(fd, filename, "403", "Forbidden",
			"Tiny couldn't read the file");
	    return;
	}
	serve_static(fd, filename, sbuf.st_size);
    }
    else 
    { /* Serve dynamic content */
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) 
	{
	    clienterror(fd, filename, "403", "Forbidden",
			"Tiny couldn't run the CGI program");
	    return;
	}

	if(isGet)
	{
		get_requesthdrs(&rio);
		get_dynamic(fd, filename, cgiargs);
	}
	else
	{
		post_requesthdrs(&rio,&contentLength);
		post_dynamic(fd, filename,contentLength,&rio);
	}
    }
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin get_requesthdrs */
void get_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    writetime();  // write access time in log file
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) 
    {
	Rio_readlineb(rp, buf, MAXLINE);
	writelog(buf);
	printf("%s", buf);
    }
    return;
}
/* $end get_requesthdrs */

/* $begin post_requesthdrs */
void post_requesthdrs(rio_t *rp,int *length) 
{
    char buf[MAXLINE];
    char *p;

    Rio_readlineb(rp, buf, MAXLINE);
    writetime();  // write access time in log file
    printf("%s", buf);
    while(strcmp(buf, "\r\n")) 
    {
	Rio_readlineb(rp, buf, MAXLINE);
	if(strncasecmp(buf,"Content-Length:",15)==0)
	{
		p=&buf[15];	
		p+=strspn(p," \t");
		*length=atol(p);
	}
	writelog(buf);
	printf("%s", buf);
    }
    return;
}
/* $end post_requesthdrs */


/* $begin post_dynamic */
void serve_dir(int fd,char *filename)
{
	DIR *dp;
	struct dirent *dirp;
    	struct stat sbuf;
	int num=1;
	char files[MAXLINE],buf[MAXLINE],name[MAXLINE],img[MAXLINE];

	if((dp=opendir(filename))==NULL)
		syslog(LOG_ERR,"cannot open dir:%s",filename);

    	sprintf(files, "<html><title>Dir Browser</title>");
	sprintf(files, "%s<body bgcolor=""ffffff"" font-family=Arial color=#fff font-size=14px>\r\n", files);

	while((dirp=readdir(dp))!=NULL)
	{
		if(strcmp(dirp->d_name,".")==0||strcmp(dirp->d_name,"..")==0)
			continue;
		sprintf(name,"%s/%s",filename,dirp->d_name);
		lstat(name,&sbuf);

		if(S_ISDIR(sbuf.st_mode))
		{
			sprintf(img,"<img src=""dir.png"" width=""24px"" height=""24px"">");
		}
		else if(S_ISFIFO(sbuf.st_mode))
		{
			sprintf(img,"<img src=""fifo.png"">");
		}
		else if(S_ISLNK(sbuf.st_mode))
		{
			sprintf(img,"<img src=""link.png"">");
		}
		else if(S_ISSOCK(sbuf.st_mode))
		{
			sprintf(img,"<img src=""sock.png"">");
		}
		else
			sprintf(img,"<img src=""file.png"" width=""24px"" height=""24px"">");


		sprintf(files,"%s<p><pre>%-3d %s %-20s%20d</pre></p>\r\n",files,num++,img,dirp->d_name,(int)sbuf.st_size);
		//sprintf(files,"%s<p><pre>%-3d %-20s%20d</pre></p>\r\n",files,num++,dirp->d_name,(int)sbuf.st_size);
	}
	closedir(dp);
	sprintf(files,"%s</body></html>",files);
	printf("%s\n",files);

	/* Send response headers to client */
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, strlen(files));
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, "text/html");
	Rio_writen(fd, buf, strlen(buf));

	/* Send data to client */
	Rio_writen(fd, files, strlen(files));
	exit(0);

}
/* $end serve_dir */


/* $begin post_dynamic */
void post_dynamic(int fd, char *filename, int contentLength,rio_t *rp)
{
    char buf[MAXLINE],length[32], *emptylist[] = { NULL },data[MAXLINE];
    int p[2];

    sprintf(length,"%d",contentLength);

    if(pipe(p)==-1)
	syslog(LOG_ERR,"cannot create pipe");

    /*       The post data is sended by client,we need to redirct the data to cgi stdin.
    *  	 so, child read contentLength bytes data from fp,and write to p[1];
    *    parent should redirct p[0] to stdin. As a result, the cgi script can
    *    read the post data from the stdin. 
    */
    if (Fork() == 0)
    {  /* child  */ 
	close(p[0]);
	Rio_readnb(rp,data,contentLength);
	Rio_writen(p[1],data,contentLength);
	exit(0);
    }
    

    /* Send response headers to client */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    Dup2(p[0],STDIN_FILENO);  /* Redirct p[0] to stdin */
    close(p[0]);

    close(p[1]);
    setenv("CONTENT-LENGTH",length , 1); 
    Dup2(fd,STDOUT_FILENO);        /* Redirct stdout to client */ 
    Execve(filename, emptylist, environ); 
}
/* $end post_dynamic */





/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;
    char tmpcwd[MAXLINE];
    strcpy(tmpcwd,cwd);
    strcat(tmpcwd,"/");

    if (!strstr(uri, "cgi-bin")) 
    {  /* Static content */
	strcpy(cgiargs, "");
	strcpy(filename, strcat(tmpcwd,getconfig("root")));
	strcat(filename, uri);
	if (uri[strlen(uri)-1] == '/')
	    strcat(filename, "home.html");
	return 1;
    }
    else 
    {  /* Dynamic content */
	ptr = index(uri, '?');
	if (ptr) 
	{
	    strcpy(cgiargs, ptr+1);
	    *ptr = '\0';
	}
	else 
	    strcpy(cgiargs, "");
	strcpy(filename, cwd);
	strcat(filename, uri);
	return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else
	strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin get_dynamic */
void get_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));
  
    if (Fork() == 0) 
    { /* child */
	/* Real server would set all CGI vars here */
	setenv("QUERY_STRING", cgiargs, 1); 
	Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */
	Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}
/* $end get_dynamic */

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
/* $end clienterror */
