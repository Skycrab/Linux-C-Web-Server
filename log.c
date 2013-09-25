#include "parse.h"
#include "wrap.h"

FILE* logfp=NULL;

void initlog(const char* logp)
{
	logfp=Fopen(logp,"a+");
}

static int getmonth(struct tm* local)   // return month index ,eg. Oct->10
{
	char buf[8];
	int i;
	static char *months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};

	strftime(buf,127,"%b",local);
	for(i=0;i<12;++i)
	{
		if(!strcmp(buf,months[i]))
			return i+1;
	}
	return 0;
}

void writetime()
{
	time_t timeval;
	char other[24];
	char year[8];
	char together[64];
	int month;

	(void)time(&timeval);
	struct tm *local=localtime(&timeval);

/* get year */
	strftime(year,7,"%Y",local);
/*get month */
	month=getmonth(local);
/*get other */
	strftime(other,23,"%d %H:%M:%S",local);
/*together all */
	sprintf(together,"%s/%d/%s\r\n",year,month,other);
	fwrite(together,strlen(together),1,logfp);
}

char* timeModify(time_t timeval,char *time)
{
	char other[24];
	char year[8];
	int month;

	struct tm *local=localtime(&timeval);

/* get year */
	strftime(year,7,"%Y",local);
/*get month */
	month=getmonth(local);
/*get other */
	strftime(other,23,"%d %H:%M:%S",local);
/*together all */
	sprintf(time,"%s/%d/%s\r\n",year,month,other);
	return time;
}

void writelog(const char* buf)
{
	fwrite(buf,strlen(buf),1,logfp);
	fflush(logfp);
}



