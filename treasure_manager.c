#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<dirent.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
typedef struct
{
  float latitude,longitude;
}Coordinates;

typedef struct
{
  int ID;
  char name[100];
  Coordinates coordinates;
  char clue[100];
  int value;
}Treasure;

void print_treasure(Treasure t)
{
  printf("ID:%d\n",t.ID);
  printf("Name:%s\n",t.name);
  printf("Coordinates:%f %f\n",t.coordinates.latitude,t.coordinates.longitude);
  printf("Clue:%s\n",t.clue);
  printf("Value:%d\n",t.value);
}

int search_file(char *directory,char *name)
{
  DIR *folder=NULL;
  if((folder=opendir(directory))==NULL)
    {
      printf("Error when opening folder:%s\n",directory);
      exit(-1);
    }
  struct dirent *item=NULL;
  while((item=readdir(folder))!=NULL)
    {
      if(strcmp(name,item->d_name)==0)
	{
	  closedir(folder);
	  return 1;
	}
    }
  closedir(folder);
  return 0;
}
void log_op(char *huntID,char *op)
{
  char path[500];
  int file;
  sprintf(path,"%s/logged_hunt",huntID);
  if((file=open(path, O_WRONLY|O_APPEND))<0)
    {
      printf("Error when opening file logged_hunt\n");
      exit(2);
    }
  if(write(file,op,strlen(op))!=strlen(op))
    {
      printf("Error when writing in logged_hunt\n");
      exit(2);
    }
  close(file);
}
void add(char *huntID)
{
  int existsHunt=search_file(".",huntID);
  char path1[500],path2[500],link[500];
  int file1,file2;
  sprintf(path1,"%s/logged_hunt",huntID);
  sprintf(path2,"%s/%s",huntID,huntID);
  if(existsHunt==0)
    {
      if(mkdir(huntID,S_IRUSR|S_IWUSR|S_IXUSR)==-1)
	{
	  printf("Error when creating %s hunt\n",huntID);
	  exit(-2);
	}
      if((file1=open(path1,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR))==-1)
	{
	  printf("Error when creating logged_hunt\n");
	  exit(-2);
	}
      close(file1);
       if((file2=open(path2,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR))==-1)
	{
	  printf("Error when creating treasure file\n");
	  exit(-2);
	}
       close(file2);
       sprintf(link,"logged_hunt-%s",huntID);
       if(symlink(path1,link)==-1)
	 {
	   printf("Error when creating link\n");
	   exit(-2);
	 }
    }
  Treasure treasure;
  scanf("%d", &treasure.ID);
  getchar();
  fgets(treasure.name, 100, stdin);
  treasure.name[strcspn(treasure.name,"\n")]='\0';
  scanf("%f", &treasure.coordinates.latitude);
  scanf("%f", &treasure.coordinates.longitude);
  getchar();
  fgets(treasure.clue, 100, stdin);
  treasure.clue[strcspn(treasure.clue,"\n")]='\0';
  scanf("%d", &treasure.value);
  if((file2=open(path2,O_WRONLY|O_APPEND))==-1)
    {
        printf("Error when opening  treasure file for writing\n");
	exit(-2);
    }
  if(write(file2,&treasure,sizeof(treasure))!=sizeof(treasure))
    {
        printf("Error when writing in treasure file\n");
	exit(-2);
    }
  close(file2);
}
void list(char *huntID)
{
  char path[500];
  int file;
  int existsFile=search_file(".",huntID);
  sprintf(path,"%s/%s",huntID,huntID);
  if(existsFile==0)
    {
      printf("Hunt folder doesn't exists\n");
      exit(-2);
    }
  struct stat statistics;
  if(stat(path,&statistics)==-1)
    {
      printf("Error when reading statistics\n");
      exit(-2);
    }
  if((file=open(path,O_RDONLY))==-1)
    {
        printf("Error when opening treasure file\n");
        exit(-2);
    }
  printf("Hunt:%s\n", huntID);
  printf("File size:%ld\n", statistics.st_size);
  printf("Last modification:%s\n", ctime(&statistics.st_mtime));
  Treasure treasure;
  while(read(file,&treasure,sizeof(Treasure))==sizeof(Treasure))
    {
      print_treasure(treasure);
    }
  close(file);
}
void view(char *huntID,int treasureID)
{
  char path[500];
  int file;
  int found=0;
  int existsFile=search_file(".",huntID);
  sprintf(path,"%s/%s",huntID,huntID);
  if(existsFile==0)
    {
      printf("Hunt folder doesn't exists\n");
      exit(-2);
    }
   if((file=open(path,O_RDONLY))==-1)
    {
        printf("Error when opening treasure file\n");
        exit(-2);
    }
   Treasure treasure;
    while (read(file, &treasure, sizeof(Treasure)) == sizeof(Treasure))
    {
        if (treasure.ID==treasureID)
        {
            print_treasure(treasure);
	    found=1;
            break;
        }
    }
    if(found==0)
      printf("There is no treasure with ID: %d\n", treasureID);
    close(file);
}
void remove_treasure(char *huntID,int treasureID)
{
  char path[500];
  int found=0;
  int index=0;
  int size=1;
  Treasure *treasures=malloc(sizeof(Treasure));
  Treasure treasure;
  if(treasures==NULL)
    {
      printf("Error when allocating memory\n");
      exit(-1);
    }
  int file;
  int existsFile=search_file(".",huntID);
  sprintf(path,"%s/%s",huntID,huntID);
  if(existsFile==0)
    {
      printf("Hunt folder doesn't exists\n");
      exit(-2);
    }
   if((file=open(path,O_RDONLY))==-1)
    {
        printf("Error when opening treasure file\n");
        exit(-2);
    }
   while(read(file,&treasure,sizeof(Treasure))==sizeof(Treasure))
     {
       if(treasure.ID!=treasureID)
	 {
	   if(size==index)
	     {
	       size=size*2;
	       if((treasures=realloc(treasures,size*sizeof(Treasure)))==NULL)
		 {
		   printf("Error when allocating memory\n");
		   free(treasures);
		   exit(-1);
		 }
	     }
	   treasures[index++]=treasure;
	 }
       else
	 {
	   found=1;
	 }
     }
   if(found==0)
     {
       printf("Treasure not found\n");
       free(treasures);
       exit(-2);
     }
   close(file);
    if ((file = open(path, O_WRONLY | O_TRUNC)) < 0)
    {
        printf("Error when opening treasure file for writing\n");
        exit(-2);
    }
    if (write(file, treasures,index * sizeof(Treasure)) != index * sizeof(Treasure))
    {
        printf("Error at writing in treasure file\n");
        free(treasures);
        exit(-2);
    }
    free(treasures);
    close(file);
}
void remove_hunt(char *huntID)
{
  DIR *folder=NULL;
  int found=0;
  struct dirent *item=NULL;
  char path1[500];
  char path2[500];
  char path3[500];
  sprintf(path1, "%s/%s", huntID, huntID);
  sprintf(path2, "%s/logged_hunt", huntID);
  sprintf(path3, "logged_hunt-%s", huntID);
  if((folder=opendir("."))==NULL)
    {
      printf("Error when  opening folder\n");
      exit(-2);
    }
  while((item=readdir(folder))!=NULL)
    {
      if(strcmp(item->d_name,huntID)==0)
	{
	   remove(path1);
	   remove(path2);
	   remove(path3);
	   rmdir(huntID);
	   found=1;
	   break;
	}
    }
  if(found==0)
    printf("There is no hunt to delete\n");
  closedir(folder);
}
void list_hunts(char *op)
{
    DIR *folder = NULL;
    struct dirent *item = NULL;
    struct stat info;
    int ok = 0;

    if((folder=opendir("."))==NULL)
      {
        printf("Error when opening folder\n");
        exit(-2);
      }

    while((item=readdir(folder))!=NULL)
      {
        char itemName[256];
        strcpy(itemName,item->d_name);
        if(itemName[0]=='.'||strcmp(itemName,"input")== 0||strcmp(itemName,"build")==0)
	  {
            continue;
	  }
        if(lstat(itemName,&info)<0)
	  {
            printf("Error reading statistics from file\n");
            exit(-2);
	  }

        if(S_ISDIR(info.st_mode))
	  {
            int number = 0;
            char path[512];
            sprintf(path,"%s/%s",itemName,itemName);
            int file;
            if((file=open(path,O_RDONLY))<0)
	      {
                printf("Error opening treasure file for reading\n");
                exit(-2);
	      }
            Treasure buffer[100];
            int numberOfTreasureRead;
            while((numberOfTreasureRead=read(file,&buffer,100*sizeof(Treasure)))>0)
	      {
                number=number+(numberOfTreasureRead/sizeof(Treasure));
	      }
            close(file);
            printf("%s has %d treasures\n", itemName, number);
            log_op(itemName, op);
            ok = 1;
        }
    }
    if(ok==0)
      {
        printf("There are no hunts\n");
      }
    closedir(folder);
}
void printUsage()
{
    printf("Usage:\n");
    printf("  --add <hunt_id>\n");
    printf("  --list <hunt_id>\n");
    printf("  --view <hunt_id> <treasure_id>\n");
    printf("  --remove_treasure <hunt_id> <treasure_id>\n");
    printf("  --remove_hunt <hunt_id>\n");
    printf("  --list_hunts\n");
}
char *getOP(int argc, char *argv[])
{
    int initial_size = 1;
    for (int i = 0; i < argc; i++)
    {
        initial_size += strlen(argv[i]) + 1;
    }
    char *operation = malloc(initial_size * sizeof(char));
    if (operation == NULL)
    {
        printf("Error when allocating memory\n");
        exit(-2);
    }
    operation[0] = '\0';
    for (int i = 0; i < argc; i++)
    {
        strcat(operation, argv[i]);
        if (i < argc - 1)
        {
            strcat(operation, " ");
        }
    }
    strcat(operation, "\n");
    return operation;
}

 int main(int argc, char *argv[])
{
    if (argc < 2 || argc>4)
    {
        printUsage();
        exit(-1);
    }
    char *operation = getOP(argc, argv);
    if (strcmp(argv[1], "--add") == 0)
    {
        add(argv[2]);
	log_op(argv[2],operation);
	free(operation);
    }
    else if (strcmp(argv[1], "--list") == 0)
    {
        list(argv[2]);
	log_op(argv[2],operation);
	free(operation);
    }
    else if (strcmp(argv[1], "--view") == 0)
    {
      view(argv[2], atoi(argv[3]));
      log_op(argv[2],operation);
      free(operation);
    }
    else if (strcmp(argv[1], "--remove_treasure") == 0)
    {
      remove_treasure(argv[2], atoi(argv[3]));
      log_op(argv[2],operation);
      free(operation);
    }
    else if (strcmp(argv[1], "--remove_hunt") == 0)
    {
        remove_hunt(argv[2]);
	free(operation);
    }
    else if(strcmp(argv[1], "--list_hunts") == 0)
    {
      list_hunts(operation);
      free(operation);
    }
    else
    {
        printf("Invalid command!\n");
        printUsage();
        exit(-1);
    }
    return 0;
}
