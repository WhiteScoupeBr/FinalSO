#include "queue.h"

#include <stdio.h>

void queue_append (queue_t **queue, queue_t *elem) {

	if(queue==NULL){
		printf("Lista Inexistente!\n");
		return ;
	}

	if(elem==NULL){
		printf("Elemento Nulo!\n");
		return ;
	}

	if(elem->prev!=NULL||elem->next!=NULL){
		printf("Elemento não está em nenhuma outra fila!\n");
		return ;
	}

	if(*queue==NULL){ //caso a lista esteja vazia, ela é iniciada
		(*queue)=elem;
		elem->prev=elem;
		elem->next=elem;
		return;
	}

	else{
        queue_t *ptr = (*queue)->prev; //ponteiro para a ultima posição da lista
        (*queue)->prev = elem; //o primeiro recebe o elemento
        ptr->next=elem; // o que era o ultimo anteriormente recebe o novo elemento como proximo
        elem->prev=ptr; // o novo elemento recebe o seu devido anterior
        elem->next= (*queue); // o novo elemento recebe o primeiro como seu proximo
        }

}



queue_t *queue_remove (queue_t **queue, queue_t *elem){

	if(queue==NULL){
		printf("Lista Inexistente!\n");
		return NULL;
	}

	if((*queue)->next==NULL||(*queue)->prev==NULL){
		printf("Lista Vazia!\n");
		return NULL;
    }

    if(*queue==NULL){
        printf("Lista Vazia!\n");
        return NULL;
    }

	if(elem==NULL){
		printf("Elemento Nulo!\n");
		return NULL;
	}

	if(queue_size(*queue)==1){
        if((*queue)==elem){
            (*queue)->prev=NULL;
            (*queue)->next=NULL;
            (*queue)=NULL;
            return elem;
        }
	}

	queue_t *ptr = (*queue); //ponteiro para rodar a lista
	int i,tam;
	tam=queue_size((*queue));

	for (i=0; i<=queue_size((*queue));i++){

		if(i==tam){
			printf("Elemento nao esta na lista\n");
			return NULL;
		}
		if(ptr==elem){
			break;
		}

		ptr=ptr->next;
	}

    if((*queue)==elem){
        (*queue)=(*queue)->next;
    }

	elem->prev->next=elem->next;
	elem->next->prev=elem->prev;

	elem->prev=NULL;
	elem->next=NULL;


	return elem;

}

int queue_size (queue_t *queue){

	if(queue==NULL){
		return 0;
	}

	queue_t *ptr = queue;
	int aux =1;

	while(ptr->next!=queue){
		aux++;
		ptr=ptr->next;
	}

	return aux;
}



void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {

    int tam=queue_size(queue);
    queue_t *ptr=queue;
    int i;
    for(i=0;i<tam;i++){
        print_elem(ptr);
        ptr=ptr->next;
    }

}
