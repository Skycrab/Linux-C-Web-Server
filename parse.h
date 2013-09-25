#ifndef _PARSE_CONFIG_H
#define _PARSE_CONFIG_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
#include <dirent.h>

#ifdef HTTPS 
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define _GNU_SOURCE   //enable the getopt_long
#include <getopt.h>

/* daemon_init.c */
void init_daemon(void);

/* parse_config.c */
char* Getconfig(char*);
extern FILE *configfp;

/*parse_option.c */
#ifdef HTTPS 
void parse_option(int argc,char **argv,char* d,char** portp,char** logp,char** sslp,char* dossl);
#else
void parse_option(int argc,char **argv,char* d,char** portp,char** logp);
#endif

/* log.c */
#define MAXLINELEN 8192
extern FILE *logfp;
void initlog(const char* logp);
void writetime();
void writelog(const char* buf);
char* timeModify(time_t timeval,char *time);

/* secure_access.c */
int access_ornot(const char *destip); // 0 -> not 1 -> ok

/* main.c */


#endif

