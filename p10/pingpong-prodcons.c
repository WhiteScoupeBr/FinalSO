#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include <time.h>
#include <stdlib.h>

int item;
int buffer[5];
int prod =3;
task_t p1,p2,p3,c1,c2;
semaphore_t s_buffer, s_item, s_vaga ;
 

 int bufferCheio(){
		if(buffer[5]!=-1){
			return 1;
		}
		else
		{
			return 0;
		}
	}

 int bufferVazio(){
		if(buffer[0]==-1){
			return 1;
		}
		else
		{
			return 0;
		}
	}

void produtor(void * arg)
{


for(int aux =0;aux<8;aux++)
	{
	task_sleep (1);
	
	item = random()%100;
	sem_down (&s_vaga);
	sem_down (&s_buffer);
	for(int i=0;i<5;i++){
		if(buffer[i]==-1){
			buffer[i]=item;
			break;
		}
	}
	
	printf (" produtor item: %d\n", item) ;
	sem_up (&s_buffer);
	sem_up (&s_item);
	}
	prod--;
	task_exit(0);
}


void consumidor(void * arg)
{

  while(0<prod)
	{
	sem_down (&s_item);
	sem_down (&s_buffer);
	if(buffer[0]!=-1){
		item = buffer[0];
		for(int i=0;i<4;i++){
			buffer[i]=buffer[i+1];
		}
		buffer[5]=-1;
	}
	printf (" consumidor item: %d\n", item) ;
	sem_up (&s_buffer);
	sem_up (&s_vaga);
	task_sleep (1);
	}

	task_exit(0);
}

int main(int argc, char *argv[]){


	printf ("Main INICIO\n") ;
	for(int i=0;i<5;i++){
		buffer[i]=-1;
		//printf("%d", buffer[i]);
	}
	pingpong_init();

	sem_create (&s_item, 0) ;
	sem_create (&s_vaga, 5) ;
	sem_create (&s_buffer, 5) ;
	task_create (&p1, produtor, "p1") ;
  	task_create (&p2, produtor, "p2") ;
    task_create (&p3, produtor, "p3") ;
    task_create (&c1, consumidor, "c1") ;
	task_create (&c2, consumidor, "c2") ;
	task_join(&p1);
	task_join(&p2);
	task_join(&p3);
	task_join(&c1);
	task_join(&c2);
	
	sem_destroy (&s_buffer);
    sem_destroy (&s_vaga);
    sem_destroy (&s_item);
	
	printf ("Main FIM\n") ;
   	task_exit (0) ;

  	 exit (0) ;

}

