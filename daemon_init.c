#include "parse.h"

void init_daemon(void)
{
	int i;
	pid_t pid;
	struct sigaction sa;
	umask(0);
	if(pid=fork())
		exit(0);
	else if(pid<0)
		exit(1);
	setsid();

	sa.sa_handler=SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags=0;
	sigaction(SIGHUP,&sa,NULL);


	if(pid=fork())
		exit(0);
	else if(pid<0)
		exit(1);

	for(i=0;i<NOFILE;++i)
		close(i);

	chdir("/");
}
