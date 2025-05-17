#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<stdarg.h>
#include<stdbool.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<dirent.h>


pid_t monitorPID=-1;
bool monitor_shutdown=false;
int pfd[2];

void read_pipe(int file)
{
  char buffer[4096];
  int bytes_read;
  if((bytes_read=read(file,buffer,4095))>0)
    {
      buffer[bytes_read]='\0';
      printf("%s",buffer);
    }
}

void sigchld_handler(int sig)
{
  int status;
  pid_t pid;
  while((pid=waitpid(-1,&status,WNOHANG))>0)
    {
      if(pid==monitorPID)
	{
	  if(WIFEXITED(status))
	    {
	      printf("Monitor process ended normally\n");
	    }
	  else
	    {
	      printf("Monitor porcess ended abnormally\n");
	    }
	  monitor_shutdown=false;
	  monitorPID=-1;
	}
      else
	{
	  if(WIFEXITED(status)==0)
	    {
	      printf("Execution process terminated abnormally\n");
	      exit(EXIT_FAILURE);
	    }
	}
    }
}

void monitor_handler_usr1(int sig)
{
    FILE *file = NULL;
    char buffer[100];
    char arg[3][50];
    int index = 0;
    pid_t execution_processPID;

    if ((file=fopen("tempOP.txt","r")) == NULL)
      {
        fprintf(stderr,"Error when opening file\n");
        exit(EXIT_FAILURE);
      }

    if (fgets(buffer,100,file)==NULL)
      {
        fprintf(stderr,"Error when writing in buffer\n");
        fclose(file);
        exit(EXIT_FAILURE);
      }

    buffer[strcspn(buffer,"\n")]='\0';
    fclose(file);

    char *p=strtok(buffer," ");
    while((p!=NULL)&&(index<3))
      {
        strcpy(arg[index++],p);
        p=strtok(NULL," ");
      }
    if ((execution_processPID=fork())<0)
      {
        fprintf(stderr,"Failed to fork execution process");
        exit(EXIT_FAILURE);
      }
    if (execution_processPID==0)
      {
        if(strcmp(arg[0],"--list_hunts")==0)
	  {
            execlp("./treasure_manager","./treasure_manager",arg[0],NULL);
	  }
	else if(strcmp(arg[0],"--list")==0)
	  {
            execlp("./treasure_manager","./treasure_manager",arg[0],arg[1],NULL);
	  }
	else if(strcmp(arg[0],"--view")==0)
	  {
            execlp("./treasure_manager","./treasure_manager",arg[0],arg[1],arg[2],NULL);
	  }
        fprintf(stderr, "Failed to execute treasure_manager");
        exit(EXIT_FAILURE);
    }
}
void monitor_handler_sigterm(int sig)
{
  sleep(10);
  exit(EXIT_SUCCESS);
}
void child_process(void)
{
    struct sigaction monitor_action;
    memset(&monitor_action,0,sizeof(struct sigaction));
    monitor_action.sa_handler = monitor_handler_usr1;
    if (sigaction(SIGUSR1,&monitor_action,NULL)<0)
      {
        fprintf(stderr,"Failed to set SIGUSR1 handler\n");
        exit(EXIT_FAILURE);
      }
    monitor_action.sa_handler=monitor_handler_sigterm;
    if (sigaction(SIGTERM,&monitor_action,NULL)<0)
      {
        fprintf(stderr, "Failed to set SIGTERM handler\n");
        exit(EXIT_FAILURE);
      }
    while(1)
      {
        pause();
      }
}
void startMonitor(void)
{
   if(pipe(pfd)<0)
      {
        printf("Error when creating pipe!\n");
        exit(-1);
      }
    if ((monitorPID=fork())<0)
      {
        fprintf(stderr,"Failed to fork monitor process\n");
        exit(EXIT_FAILURE);
      }
    if (monitorPID==0)
      {
	close(pfd[0]);
	dup2(pfd[1],1);
        child_process();
        exit(EXIT_SUCCESS);
      }
    close(pfd[1]);
    printf("Monitor process started with PID %d\n", monitorPID);
}
char *getOP(const int n, ...)
{
  char *operation=NULL;
  int initial = 1;
  va_list list;
  va_start(list,n);
  for(int i=0;i<n;i++)
    {
      initial=initial+strlen(va_arg(list,char *))+1;
    }
  va_end(list);
  if((operation=malloc(sizeof(char)*initial))==NULL)
    {
      fprintf(stderr,"Error when allocating memory!\n");
      exit(-1);
    }
  operation[0]='\0';
  va_start(list,n);
  for(int i=0;i<n;i++)
    {
      strcat(operation,va_arg(list,char *));
      if(i<n-1)
	{
	  strcat(operation," ");
	}
    }
  va_end(list);
  strcat(operation,"\n");
  return operation;
}
void writeOP(char *command)
{
  int file;
  if((file=open("tempOP.txt",O_WRONLY|O_TRUNC))<0)
    {
      fprintf(stderr,"Error when opening file for writing!\n");
      free(command);
      exit(EXIT_FAILURE);
    }
  if(write(file,command,strlen(command))!=strlen(command))
    {
      fprintf(stderr,"Error when writing in file!\n");
      close(file);
      free(command);
      exit(EXIT_FAILURE);
    }
  close(file);
  free(command);
}

void calculate_score(void)
{
  int cs_pfd[2];
  DIR *folder=NULL;
  struct dirent *item=NULL;
  struct stat info;

  if(pipe(cs_pfd)<0)
    {
      printf("Error when creating pipe!\n");
      exit(-1);
    }
  if((folder=opendir("."))==NULL)
    {
      printf("Error when opening folder!\n");
      exit(-1);
    }
  while((item=readdir(folder))!=NULL)
    {
      char itemName[256];
      strcpy(itemName,item->d_name);
       if (itemName[0] == '.' || strcmp(itemName, "input") == 0 || strcmp(itemName, "build") == 0)
	 {
            continue;
	 }
       if(lstat(itemName,&info)<0)
	 {
	    printf("Error when  reading statistics!\n");
            exit(-1);
	 }
       if(S_ISDIR(info.st_mode))
	 {
	   pid_t pid;
	   if((pid=fork())<0)
	     {
	       fprintf(stderr, "Failed fork!\n");
	       exit(-1);
	     }
	   if(pid==0)
	     {
	       close(cs_pfd[0]);
	       dup2(cs_pfd[1],1);
	       execlp("./score", "./score",itemName,NULL);
	       fprintf(stderr,"Failed to execute program!\n");
	       exit(-1);
	     }
	   close(cs_pfd[1]);
	   read_pipe(cs_pfd[0]);
	 }
    }
  close(cs_pfd[0]);
  closedir(folder);
}

void parent_process(void)
{
    char command[50];
    while(1)
      {
        if (fgets(command,50,stdin)==NULL)
	  {
            fprintf(stderr, "Failed to read command\n");
            exit(EXIT_FAILURE);
	  }
        command[strcspn(command,"\n")]='\0';
        if (monitor_shutdown==true)
	  {
            printf("Monitor is currently shutting down. Please wait...\n");
            continue;
	  }
        if(strcmp(command,"exit")==0)
	  {
            if(monitorPID>0)
	      {
                printf("Monitor is already running with PID %d\n",monitorPID);
                continue;
	      }
	    close(pfd[0]);
            remove("tempOP.txt");
            break;
	  }
	else if(strcmp(command,"start_monitor")==0)
	  {
            if (monitorPID>0)
	      {
                printf("Monitor is already running with PID %d\n", monitorPID);
	      }
	    else
	      {
                startMonitor();
	      }
        }
	else if(strcmp(command,"list_hunts")==0)
	  {
            if (monitorPID==-1)
	      {
                printf("Monitor isn't running. Start it with 'start_monitor' first.\n");
	      }
	    else
	      {
                writeOP(getOP(1,"--list_hunts"));
                if (kill(monitorPID,SIGUSR1)<0)
		  {
                    fprintf(stderr,"Failed to send SIGUSR1 to monitor\n");
                    exit(EXIT_FAILURE);
		  }
		read_pipe(pfd[0]);
            }
        }
	else if(strcmp(command,"list_treasures")==0)
	  {
            if (monitorPID==-1)
	      {
                printf("Monitor isn't running. Start it with 'start_monitor' first.\n");
	      }
	    else
	      {
                char huntID[50];
                printf("Enter hunt ID: ");
                scanf("%49s", huntID);
                getchar();
                writeOP(getOP(2, "--list", huntID));
                if (kill(monitorPID, SIGUSR1)<0)
		  {
                    fprintf(stderr, "Failed to send SIGUSR1 to monitor\n");
                    exit(EXIT_FAILURE);
		  }
		read_pipe(pfd[0]);
            }
        }
	else if(strcmp(command,"view_treasure")==0)
	  {
            if (monitorPID==-1)
	      {
                printf("Monitor isn't running. Start it with 'start_monitor' first.\n");
	      }
	    else
	      {
                char huntID[50],treasureID[50];

                printf("Enter hunt ID: ");
                scanf("%49s", huntID);
                getchar();

                printf("Enter treasure ID: ");
                scanf("%49s", treasureID);
                getchar();

                writeOP(getOP(3, "--view", huntID, treasureID));
                if (kill(monitorPID,SIGUSR1)<0)
		  {
                    fprintf(stderr, "Failed to send SIGUSR1 to monitor\n");
                    exit(EXIT_FAILURE);
		  }
		read_pipe(pfd[0]);
	      }
        }
	else if (strcmp(command,"calculate_score")==0)
	  {
	    calculate_score();
	  }
	else if(strcmp(command,"stop_monitor")==0)
	  {
            if (monitorPID == -1)
	      {
                printf("Monitor isn't running.\n");
	      }
	    else
	      {
                printf("Sending termination signal to monitor process...\n");
                if (kill(monitorPID, SIGTERM) < 0)
		  {
                    fprintf(stderr, "Failed to send SIGTERM to monitor\n");
                    exit(EXIT_FAILURE);
		  }
                monitor_shutdown = true;
	      }
	  }
	else
	  {
            printf("Invalid command.\n");
	  }
      }
}
int main(void)
{
    int file;
    struct sigaction action;
    if ((file = open("tempOP.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))<0)
      {
        fprintf(stderr, "Failed to create temporary file\n");
        exit(EXIT_FAILURE);
      }
    close(file);
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler=SIG_IGN;
    if (sigaction(SIGUSR1, &action,NULL)<0)
      {
        fprintf(stderr, "Failed to set SIGUSR1 handler\n");
        exit(EXIT_FAILURE);
      }
    if (sigaction(SIGTERM, &action, NULL)<0)
      {
        fprintf(stderr, "Failed to set SIGTERM handler\n");
        exit(EXIT_FAILURE);
      }
    action.sa_handler = sigchld_handler;
    action.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &action, NULL)<0)
      {
        fprintf(stderr, "Failed to set SIGCHLD handler\n");
        exit(EXIT_FAILURE);
      }
    parent_process();
    return 0;
}
