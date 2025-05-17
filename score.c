#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{
  char name[100];
  int score;
}Score;

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

int existsUserName(Score *list,int size, char *name)
{
  for(int i=0;i<size;i++)
    {
      if(strcmp(list[i].name,name)==0)
	{
	  return i;
	}
    }
  return -1;
}
int main(int argc,char *argv[])
{
  if(argc!=2)
    {
      printf("Invalid number of argumets!\n");
      exit(-1);
    }
  FILE *treasure_file=NULL;
  char path[512];
  sprintf(path,"%s/%s",argv[1],argv[1]);
  if((treasure_file=fopen(path,"r"))==NULL)
    {
      printf("Error when opening treasure file!\n");
      exit(-1);
    }
  Treasure buffer[100];
  int numberOfTreasuresRead;
  Score *scores=malloc(1*sizeof(Score));
  int index=0;
  int size=1;
  int position;
  while((numberOfTreasuresRead=fread(buffer,sizeof(Treasure),100,treasure_file))>0)
    {
      for(int i=0;i<numberOfTreasuresRead;i++)
	{
	  if((position=existsUserName(scores,index,buffer[i].name))==-1)
	    {
	      if(index==size)
		{
		  size=size*2;
		  if((scores=realloc(scores,size*sizeof(Score)))==NULL)
		    {
		      printf("Error when allocating memory!\n");
		      free(scores);
		      exit(-1);
		    }
		}
	      strcpy(scores[index].name,buffer[i].name);
	      scores[index].score=buffer[i].value;
	      index++;
	    }
	  else
	    {
	      scores[position].score=scores[position].score+buffer[i].value;
	    }
	}
    }
  fclose(treasure_file);
  for(int i=0;i<index;i++)
    {
      printf("In %s %s obtained %d points\n",argv[1],scores[i].name,scores[i].score);
    }
  free(scores);
  return 0;
}
