#include "parse.h"

/*
*  Function: can accesser access this web or not
*	     e.g. the seg ip address: 192.168.1.0/255.255.255.0
*                 accesser ip: 192.168.1.2 & 255.255.255.0 ?= 192.168.1.0
*/


/*
*  Function:  a ip address effective or not
*  return value :  -1 error
*/


/*
*   Function: convert ip to long long
*/
static long long ipadd_to_longlong(const char *ip)
{
	const char *p=ip;
	int ge,shi,bai,qian;
	qian=atoi(p);

	p=strchr(p,'.')+1;
	bai=atoi(p);

	p=strchr(p,'.')+1;
	shi=atoi(p);

	p=strchr(p,'.')+1;
	ge=atoi(p);

	return (qian<<24)+(bai<<16)+(shi<<8)+ge;
}


int access_ornot(const char *destip) // 0 -> not 1 -> ok
{
	//192.168.1/255.255.255.0

	char ipinfo[16],maskinfo[16];
	char *p,*ip=ipinfo,*mask=maskinfo;
	char count=0;
	char *maskget=Getconfig("mask");
	const char *destipconst,*ipinfoconst,*maskinfoconst;
	if(maskget=="")
	{
		printf("ok:%s\n",maskget);
		return 1;
	}	
	p=maskget;

/* get ipinfo[] start */

	while(*p!='/')
	{
		if(*p=='.')
			++count;
		*ip++=*p++;
	}

	while(count<3)
	{
		*ip++='.';
		*ip++='0';
		++count;
	}
	*ip='\0';

/* get ipinfo[] end */

/* get maskinfo[] start */

	++p;
	while(*p!='\0')
	{
		if(*p=='.')
			++count;
		*mask++=*p++;
	}

	while(count<3)
	{
		*mask++='.';
		*mask++='0';
		++count;
	}
	*mask='\0';

/* get maskinfo[] end */
	destipconst=destip;
	ipinfoconst=ipinfo;
	maskinfoconst=maskinfo;

	return ipadd_to_longlong(ipinfoconst)==(ipadd_to_longlong(maskinfoconst)&ipadd_to_longlong(destipconst));
}

/* access_ornot test

int main()
{
	printf("%d\n",access_ornot("127.0.0.1"));
}

*/
