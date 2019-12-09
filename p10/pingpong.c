#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
// operating system check
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

#define STACKSIZE 32768
#define NOVA 1
#define PRONTA 2
#define SUSPENSA 3
#define EXEC 4
#define TERMINADA 5

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

ucontext_t contextMain;
int idJoin;
task_t *taskAtual=NULL;
task_t *taskPtr=NULL;
task_t *taskMain=NULL;
task_t *pronta=NULL,*suspensa=NULL,*terminada=NULL,*exec=NULL,*soneca=NULL;
task_t dispatcher;
unsigned int tempo=0;
unsigned int somaAux=0;
int ptrExit;
int ctx =1;
int sTasks=0;

/*****************************************************/
unsigned int systime () ;
void imprimeValores(task_t* task);
int sem_create (semaphore_t *s, int value){
	if(s!=NULL&&value>=0){
		s->value=value;
		s->task=NULL;
		return 0;
	}
	printf("ERRO! Valor inválido!");
	return -1;
}

int sem_down (semaphore_t *s){

	//printf("Life ");
	if(s != NULL){
		ctx=0;
		s->value--;
		if(s->value<0){
			
			queue_remove((queue_t **)&pronta, (queue_t *)taskAtual);
			queue_append((queue_t **)&(s->task), (queue_t *)taskAtual);
			//task_suspend(taskAtual,&pronta);
			sTasks++;
			ctx=1;
			task_yield();
		}

		if(s==NULL)	
			return -1;
		
		ctx=1;
		return 0;
		
	}
	printf("ERRO! Semáforo inexistente ou destruído!");
	return -1;
}

int sem_up (semaphore_t *s){

	
	if(s != NULL){
		ctx=0;
		s->value++;
		if(s->task!=NULL){
			queue_t* ptr = (queue_t*)s->task; 
			queue_remove((queue_t **)&(s->task), (queue_t *)ptr);
			queue_append((queue_t **)&pronta, (queue_t *)ptr);
			sTasks--;
			//task_resume(s->task);
		}
		ctx=1;
		return 0;
	}

	printf("ERRO! Semáforo inexistente ou destruído!");
	return -1;
}

int sem_destroy (semaphore_t *s){
	
	queue_t* ptr;
	 //cara a vida é complicada, quando a gente esta animado para fazer, acha que tudo vai dar certo, algo que vc ja tinha 
	if(s == NULL){ //feito começa a dar errado e vc nao entende onde vc errou, pois aquilo estava no passado. As lembranças entao vem
		return -1; // e vc percebe o efeito da nostalgia, e tudo aquilo que parecia maravilhoso vc percebe que era so sua mente 
	}  				//alterando levemente as lembranças para elas parecerem melhores e tudo aquilo q vc viveu foi tao ruim quanto o q 
					// vc esta vivendo agora. Isso acontece para suportarmos o peso do mundo e parecer que o passado nao foi tao ruim
	while(s->task!=NULL){	 // nos dando a ilusão de que o futuro pode ser melhor, criando esperança. O problema é quando vc percebe
		ptr = (queue_t*)s->task;// tudo isso e ve que nao há esperança no futuro e q ele será so uma repetição do q ja aconteceu, so q pior
		queue_remove((queue_t **)&(s->task), (queue_t *)ptr);
		queue_append((queue_t **)&pronta, (queue_t *)ptr);
		sTasks--;
		//task_resume(ptr);	// :(
		if(s->task==NULL){
			return 0;
		}
	}
	return 0;
}

// tratador do sinal
void tratador (int signum)
{
  tempo++; 
  taskAtual->quantum--;
  if(taskAtual->flag==0 && ctx==1){
		if(taskAtual->quantum==0){
			task_yield();
		}
		
	}	
	else
	{
		return;
	}	
}

int task_join(task_t *task){

	taskPtr=taskAtual;
	task->tid=idJoin;

	if(task==NULL||task->state==TERMINADA){
		return -1;
	}
	
	task_suspend(taskAtual,&pronta);
	
	return ptrExit;
}
void task_sleep(int t){

	
	taskAtual->state=SUSPENSA;
	taskAtual->tsono= (unsigned int)(1000*t)+systime();
	queue_remove ((queue_t**) &pronta, (queue_t*) taskAtual) ;
	queue_append ((queue_t **) &soneca, (queue_t*) taskAtual);
	task_yield();
}

void task_resume_soneca(){


	if(soneca!=NULL){
		task_t* ptr= soneca;
		
		while(soneca!=NULL){
		//printf("Entro\n");
		if(ptr->tsono<=systime()){
					ptr->state=PRONTA;
					ptr->tsono=0;
					queue_remove ((queue_t**) &soneca, (queue_t*) ptr) ;
					queue_append ((queue_t **) &pronta, (queue_t*) ptr);
					if(soneca==NULL){
						return;
					}
					ptr=soneca;
				}
			ptr=ptr->next;
		}
		
	}
}


task_t * scheduler(){

	task_t *ptr = pronta;
	task_t *ptrPrio =pronta;
	int i;
	int tam = queue_size((queue_t*)pronta);
	int auxP = pronta->prioD;


	for(i=0;i<tam;i++){
		if((ptr->prioD) < auxP){
			auxP=ptr->prioD;
			ptrPrio=ptr;
		}
		ptr=ptr->next;
	}
	
	for(i=0;i<tam;i++){
		if(ptr!=ptrPrio && ptr->prioD>(-19))
			ptr->prioD=(ptr->prioD)-1;
		ptr=ptr->next;
	}

	ptrPrio->prioD=ptrPrio->prio;
	ptrPrio->quantum=20;
    return ptrPrio;
}

void task_yield(){
	
	task_switch(&dispatcher);

}

unsigned int systime () {
	return tempo;
}

void imprimeValores(task_t* task){
	printf("Task %d exit: ",task->tid);
	printf("execution time %d ms, ",task->execTime);
	printf("process time %d ms, ",task->processTime);
	printf("%d activiations ",task->activs);
}

void dispatcher_body (){ // dispatcher é uma tarefa

   //pronta=pronta->prev;
   task_t* next;
   
   while ( queue_size((queue_t*) pronta) > 0 
   ||queue_size((queue_t*) suspensa) > 0
   ||queue_size((queue_t*) soneca) > 0 
   ||sTasks> 0)
   {
		task_resume_soneca();
		somaAux=0;
      	next = scheduler() ; // scheduler é uma função
	  	somaAux= systime();
      	if (next)
      	{
			somaAux= systime();
			task_switch (next) ;
			somaAux = systime()-somaAux;
			next->processTime+=somaAux;
      	}
   }
 task_exit(0) ; // encerra a tarefa dispatcher
}

void pingpong_init () {

	setvbuf (stdout, 0, _IONBF, 0) ;
	taskMain = (task_t*)(malloc(sizeof(task_t)));
	taskMain->tid = 0;
	task_create(taskMain,NULL,"Main");
	taskMain->context = contextMain;
	taskAtual = taskMain;

	task_create(&dispatcher,dispatcher_body,"Dispatcher");

	// registra a a��o para o sinal de timer SIGALRM
	action.sa_handler = tratador ;
	sigemptyset (&action.sa_mask) ;
	action.sa_flags = 0 ;
	if (sigaction (SIGALRM, &action, 0) < 0)
	{
		perror ("Erro em sigaction: ") ;
		exit (1) ;
	}

	// ajusta valores do temporizador
	timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
	timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
	timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

	// arma o temporizador ITIMER_REAL (vide man setitimer)
	if (setitimer (ITIMER_REAL, &timer, 0) < 0)
	{
		perror ("Erro em setitimer: ") ;
		exit (1) ;
	}
	//while (1) ;
	task_yield();
}

int task_create (task_t *task, void (*start_routine)(void *), void *arg){

	static int id=0;
	char *stack ;

	id++;

	task->args = arg;
	task->tid = id;
	task->state= PRONTA;
	task->prio =0;
	task->prioD = 0;
	task->execTime=systime();
	task->processTime=0;
	task->activs=0;
	task->next=NULL;
	task->prev=NULL;
	getcontext (&task->context);
	

	stack = malloc (STACKSIZE) ;
	if (stack){
		task->context.uc_stack.ss_sp = stack ;
		task->context.uc_stack.ss_size = STACKSIZE;
		task->context.uc_stack.ss_flags = 0;
		task->context.uc_link = 0;
	}
	else{
		perror ("Erro na criação da pilha: ");
		exit (1);
	}
	
	makecontext (&task->context,(void *)(*start_routine), 1, arg);

	if(task!=&dispatcher)
		queue_append((queue_t**)&pronta,(queue_t*)task);
	
	

	return id;
	
}

int task_switch (task_t *task){

	if (task){
		ucontext_t *aux= &taskAtual->context;	
		taskAtual= task;
		task->activs++;
		swapcontext(aux, &task->context);
		
	}
	else
		return -1;
}

void task_exit (int exit_code){

	ucontext_t *aux= &taskAtual->context;
	taskAtual->state=TERMINADA;
	task_t* ptr =taskAtual;
	if(taskAtual==&dispatcher){
		taskAtual->execTime= systime()-taskAtual->execTime;
		imprimeValores(taskAtual);
		taskAtual=taskMain;
		
	}
	else{
		queue_remove((queue_t**)&pronta,(queue_t*)taskAtual);
		queue_append((queue_t**)&terminada,(queue_t*)taskAtual);
		taskAtual->execTime= systime()-taskAtual->execTime;
		somaAux = systime() -somaAux;
		taskAtual->processTime+=somaAux;
		imprimeValores(taskAtual);
		taskAtual=&dispatcher;
	}
	if(ptr->tid==idJoin){
		task_resume(taskPtr);
	}
	ptrExit=exit_code;
	swapcontext(aux, &taskAtual->context);
}

int task_id (){
	return taskAtual->tid;
}

void task_setprio (task_t *task, int prio){
	task_t * ptr = task;
	if(prio>-20&&prio<20){
		if(ptr==NULL){
			taskAtual->prio=prio;
			taskAtual->prioD=prio;
			return;
		}
		else{
			ptr->prio=prio;
			ptr->prioD=prio;
			return;
		}
	}
	else
	{
		printf("Prioridade invalida");
	}
}

int task_getprio (task_t *task){
	task_t * ptr = task;
	if(ptr==NULL){
		return taskAtual->prio;
	}
	else{
		return ptr->prio;
	}
	
}


void task_suspend(task_t *task, task_t **queue){

	if(task==NULL){
		queue_remove ((queue_t**) &pronta, (queue_t*) taskAtual) ;
		queue_append ((queue_t **) &suspensa, (queue_t*) taskAtual);
		
		taskAtual->state = SUSPENSA;
	}
	else{
		queue_remove ((queue_t**) &pronta, (queue_t*) task) ;
		queue_append ((queue_t **) &suspensa, (queue_t*) task);
		task->state = SUSPENSA;
	}
	task_yield();
	
}

void task_resume (task_t *task){

	queue_remove ((queue_t**) &suspensa, (queue_t*) task) ;
	queue_append ((queue_t **) &pronta, (queue_t*) task);
	task->state=PRONTA;
	
}


