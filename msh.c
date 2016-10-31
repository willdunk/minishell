/*******************************************************************************
 * Name        : msh.c
 * Author      : William Dunkerley
 * Date        : 3-3-16
 * Description : usage: ./msh <pathname>
 ******************************************************************************/

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <stdbool.h>

/* Print usage information */ 
static void usage(const char *bin_name) 
{
	printf("Usage: %s", bin_name);
}

static int mypwd()
{
	char cwd[1024];
	struct passwd *p = getpwuid(getuid());
   	char *username = p->pw_name;
	if(getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s:%s$ ", username, cwd);
		fflush(stdout);
	}
	else
	{return 1;}
	return 0;
}

static int printbackgroundstart(pid_t _pid, char* _name)
{
	printf("[%i] %s\n", _pid, _name);
	return 0;
}

static int printbackgroundend(pid_t _pid)
{
	printf("Done [%i]\n", _pid);
	return 0;
}

static void handler(int sig)
{
	int status;
  	pid_t pid;
  	pid = wait(NULL);
  	if (pid != -1)
  	{
  		printbackgroundend(pid);
  		mypwd();
  	}
}

int main(int argc, char **argv) 
{
	pid_t parent;
	pid_t pid;

	char buffer[256];
	memset(buffer, 0, sizeof(int)*256);
	char *argvtemp[256];
	char *progname;
	char *tok1;

	int num;
	int i;

	mypwd();

	while(read(STDIN_FILENO, buffer, sizeof(buffer)) > 0)
	{
		bool background = false;

		tok1 = malloc(sizeof(buffer));
		tok1 = strtok(buffer, " \n");

		progname = malloc(sizeof(tok1));
		strcat(progname, tok1);

		argvtemp[0] = progname;
		num = 1;

		tok1 = strtok(NULL, " \n");
  		while (tok1 != NULL)
  		{
  			argvtemp[num] = malloc(sizeof(tok1));
  			strcpy(argvtemp[num],tok1);
  			num++;
    		tok1 = strtok(NULL, " \n");
  		}

  		//see if we need to run in background
  		if(strcmp(argvtemp[num-1],"&")==0)
  		{
  			num--; //1 less argument, will get overidden next
  			//do some stuff here for background
  			background = true;
  		}

  		for(i = num; i < 256; i++)
  		{
  			argvtemp[num] = malloc(1);
  			argvtemp[num] = NULL;
  		}

   		memset(buffer, 0, sizeof(int)*256);

   		if(strcmp(argvtemp[0],"cd")==0)
   		{
   			//command was cd, no need to fork
   			if (argvtemp[1]!=NULL)
   			{
   				chdir(argvtemp[1]);
   			}
   			mypwd();
   		}
   		else if(strcmp(argvtemp[0],"exit")==0)
   		{
   			//command was exit, no need to fork
   			exit(0);
   		}
   		else
   		{
   			//FORK AND EXECUTE
			parent = getpid();
			pid = fork();
			if(pid == -1)
			{}//failed fork
			else if(pid > 0)
			{
				int status;
				if(background)
				{
					printbackgroundstart(pid,argvtemp[0]);
					mypwd();
					signal(SIGCHLD, handler);
				}
				else
				{
					waitpid(pid, &status, 0);
					mypwd();
				}
			}
			else
			{
				if(execvp(argvtemp[0],argvtemp))
				{
					//if I got here, it means it borked
					printf("Invalid Command\n");
					_exit(EXIT_FAILURE);
				}
				return 0;
			}
		}
	}
	printf("\n----------------\nEOF DETECTED\n----------------");
	fflush(stdout);
	killpg(getpid(), SIGINT);
	_exit(0);
	return 0;
}
