/*
 * Andrei's Persistent Doubly Linked List
 */
#include <alloca.h>
#include <assert.h>
#include <complex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Atlas includes
#include "atlas_alloc.h"
#include "atlas_api.h"

typedef struct Node {
    char *data;
    struct Node *next;
    struct Node *prev;
} Node;

Node *head = NULL;
Node *tail = NULL;
int size = 0;

#ifdef _FORCE_FAIL
int randval;
#endif

// ID of Atlas persistent region
uint32_t sll_rgn_id;

Node *createNode() {
    Node *node = (Node *)malloc(sizeof(Node));
    //no need synch yet cause empty node
    return node;
}

void insert(char* d){
	Node *n = createNode();
	char *data = (char *)malloc(sizeof(char)*100);
	strcpy(data, d);
	n->data = data;
	n->next = NULL;
	n->prev = NULL;
	NVM_FLUSH(&n->data);
	NVM_FLUSH(&n);
	if(size != 0){
		n->prev = tail;
		NVM_FLUSH(&n->prev);
		//NVM_BEGIN_DURABLE();
		tail->next = n;
		NVM_FLUSH(&tail->next);
		tail = n;
		NVM_FLUSH(&tail);
		size++;
		//NVM_END_DURABLE();
	}else{
		head = n;
		tail = n;
		size++;
	}
}
	
void printList(){
	fprintf(stdout, "CURRENT LIST:\n");
	Node *n = head;
	for(int i =0; i < size; i++){
		fprintf(stdout, "Node %i: %s", i + 1, n->data);
		n = n->next;
	}
	fprintf(stdout, "END LIST\n");
}


int main(int argc, char *argv[]) {
    struct timeval tv_start;
    struct timeval tv_end;
    gettimeofday(&tv_start, NULL);

    // Initialize Atlas
    NVM_Initialize();
    // Create an Atlas persistent region
    sll_rgn_id = NVM_FindOrCreateRegion("andrei", O_RDWR, NULL);

    //do the stuff
    char inp[100];
    for(;;){
        fprintf(stdout,"Type i <data> to insert, r <index> to remove, p to print, q to quit \n");
        if(fgets(inp, 100, stdin) == NULL){
	    exit(1);
	}
	if(inp[0] == 'i'){
		insert(inp + 2);
		//fprintf(stdout, "INSERT\n");
		//printList();
	}
	if(inp[0] == 'p'){
		printList();
	}
	if(inp[0] == 'q'){
		break;
	}
    }



    // Close the Atlas persistent region
    NVM_CloseRegion(sll_rgn_id);
    // Optionally print Atlas stats
#ifdef NVM_STATS
    NVM_PrintStats();
#endif
    // Atlas bookkeeping
    NVM_Finalize();

    gettimeofday(&tv_end, NULL);

    fprintf(stderr, "time elapsed %ld us\n",
            tv_end.tv_usec - tv_start.tv_usec +
                (tv_end.tv_sec - tv_start.tv_sec) * 1000000);

    return 0;
}
