#include "parse.h"
#include "wrap.h"
extern char *cwd; 

FILE *configfp=NULL;
static FILE *getfp(char *path)
{
	configfp=Fopen(path,"r");
	return configfp;
}


static char* getconfig(char* name)
{
/*
pointer meaning:

...port...=...8000...
   |  |   |   |  |
  *fs |   |   |  *be    f->forward  b-> back
      *fe |   *bs       s->start    e-> end
          *equal
*/
	static char info[64];
	int find=0;
	char tmp[256],fore[64],back[64],tmpcwd[MAXLINE];
	char *fs,*fe,*equal,*bs,*be,*start;

	strcpy(tmpcwd,cwd);
	strcat(tmpcwd,"/");
	FILE *fp=getfp(strcat(tmpcwd,"config.ini"));
	while(fgets(tmp,255,fp)!=NULL)
	{
		start=tmp;
		equal=strchr(tmp,'=');

		while(isblank(*start))
			++start;
		fs=start;

		if(*fs=='#')
			continue;
		while(isalpha(*start))
			++start;
		fe=start-1;

		strncpy(fore,fs,fe-fs+1);
		fore[fe-fs+1]='\0';
		if(strcmp(fore,name)!=0)
			continue;
		find=1;

		start=equal+1;
		while(isblank(*start))
			++start;
		bs=start;

		while(!isblank(*start)&&*start!='\n')
			++start;
		be=start-1;

		strncpy(back,bs,be-bs+1);
		back[be-bs+1]='\0';
		strcpy(info,back);
		break;
	}
	if(find)
		return info;
	else
		return NULL;
}

char* Getconfig(char* name)
{
	char *p=getconfig(name);
	if(p==NULL)
	{
		syslog(LOG_ERR,"there is no %s in the configure",name);
		exit(0);
	}
	else
		return p;
}

/* parse_config test

int main()
{
	char *s=getconfig("port");
	printf("%s\n",s);

	s=getconfig("root");
	printf("%s\n",s);
}

*/


