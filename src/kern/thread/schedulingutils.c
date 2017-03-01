/*
 * SORTING UTILTIIES
 *
 * Used for sorting threadlists, aging, and working with multi-level queues
 *
 */


#include <types.h>
#include <lib.h>
#include <thread.h>
#include <threadlist.h>
#include <schedulingutils.h>

void threadlist_sort(struct threadlist* tl){

	if(tl->tl_count  < 2){
		// kprintf("\nThreadlist contains less than two nodes. No need to sort \n");
		return;
	}

	// putch('B');putch('e');putch('f');putch('o');putch('r');putch('e');putch(':');putch('\n'); printfromnode(tl->tl_head.tln_next);
	// struct threadlistnode** firstRef = &tl->tl_head.tln_next; //This will refer to the very first node of the resulting linked list
	// threadlist_bubblesort(firstRef, tl->tl_count); //Merge sort operates as a linkedlist. The prev links still need to be fixed
	// putch('A');putch('f');putch('t');putch('e');putch('r');putch(':');putch('\n'); printfromnode(tl->tl_head.tln_next);
}

void threadlist_bubblesort(struct threadlistnode** firstRef, unsigned int size){
	struct threadlistnode* firstNode = *firstRef;
	struct threadlistnode* current;
	unsigned int i,j;
	for(i=0; i<size; i++){
		current = firstNode;
		for(j=1; j<size; j++){
			if(current->tln_self->t_priority >= current->tln_next->tln_self->t_priority){
				threadlist_swap(current, current->tln_next);
				if(current == firstNode) firstNode = current->tln_prev;
			}
			else current = current->tln_next;
		}
	}
}

void threadlist_swap(struct threadlistnode* a, struct threadlistnode* b){
	a->tln_next = b->tln_next;
	b->tln_prev = a->tln_prev;
	a->tln_prev->tln_next = b;
	b->tln_next->tln_prev = a;
	b->tln_next = a;
	a->tln_prev = b;
}



void threadlist_mergesort(struct threadlistnode** firstRef){ //Merge sort function takes the first node of the list

	struct threadlistnode* firstNode = *firstRef;
	struct threadlistnode* a;
	struct threadlistnode* b;

	if((firstNode->tln_self == NULL) || (firstNode->tln_next->tln_self == NULL)){
		return;
	}

	/* RECURSIVE CASE */

	/*
	 *  (1) The list is split into two lists: one with head "a" and another with head "b"
	 *  To achieve this, we  iterate through the list with a fast pointer and slow pointer
	 */

		 /* When the fast pointer reaches null, slow will be one node behind the middle  */
		struct threadlistnode* slow = firstNode;
 		struct threadlistnode* fast = firstNode->tln_next;
		while(fast->tln_self != NULL){ //When this loop ends b will be at the middle
			if(fast->tln_self != NULL) fast = fast->tln_next;
			if(fast->tln_self != NULL){
				slow = slow->tln_next;
				fast = fast->tln_next;
			}
		}

		//Slow pointer should now point to the last node of the first half of the list

		 a = firstNode;
		 b = slow->tln_next;

		 struct threadlistnode nullNode; //Assigns to slow to prevent unitialized compiler errors
		 nullNode.tln_self = NULL;
		 nullNode.tln_next = NULL;
		 nullNode.tln_prev = slow;
		 slow->tln_next = &nullNode;

		 /* a should point to the first node of the first list and b points to the first node of the second list */

			//Lets try to iterate through the two lists pointed by a & b, just for debugging:

			printfromnode(a);
 			printfromnode(b);

	/*
	 *  (2) Recursively split into sublists
	 */
		threadlist_mergesort(&a);
		threadlist_mergesort(&b);

	/*
	 *  (3) Merge the sorted sublists into one
	 */

		//Lets try to iterate through the two lists pointed by a & b, just for debugging:

		printfromnode(a);
		printfromnode(b);

		*firstRef = threadlist_merger(a,b);
		printfromnode(*firstRef);
}



/* Helper functions for merge sort. Merges two sorted lists */
struct threadlistnode* threadlist_merger(struct threadlistnode* a, struct threadlistnode* b){

	/* This function merges two sorted linked lists recurisively in O(n) time */
	/* Base case */
	if (a==NULL || a->tln_self == NULL) {return(b);}
	else if (b==NULL || b->tln_self == NULL) {return(a);}
	/* Recursive step: We merge whatever node's has highest priority with this node's next and the other node */
	if (a->tln_self->t_priority <= b->tln_self->t_priority)
	{
				a->tln_next = threadlist_merger(a->tln_next, b);
        return a;
	}
	else
	{
				b->tln_next = threadlist_merger(a, b->tln_next);
        return b;
	}
}


/* Function to print lists useful for debugging */
void printthreadlist(struct threadlist* tl){

	kprintf("\n\n*************************** Printing threadlist *************************** \n");
	kprintf("Number of nodes in this list = %u \n" , tl->tl_count);

	if(threadlist_isempty(tl)){
		kprintf("\n Threadlist is empty. No nodes to show \n");
	}
	else{
		unsigned int count = 1;
		struct threadlistnode *temp = tl->tl_head.tln_next;

		kprintf("Printing threads in threadlist...\n");
		while(temp->tln_self != NULL){
			kprintf("Node#%u: Priority = %u \n", count++, temp->tln_self->t_priority);
			temp = temp->tln_next;
		}
	}

	kprintf("\n****************************************************************** \n\n");
}

void printfromnode(struct threadlistnode* tln){

	struct threadlistnode* temp = tln;
	while(temp->tln_self != NULL){
	 char priority = temp->tln_self->t_priority + '0';
	 putch(priority);
	 putch(' '); putch('-'); putch('>'); putch(' ');
	 temp = temp->tln_next;
	}
	putch('\n');
}


/* Function to decrease a thread's priority by one */

void decreasePriority(struct threadlistnode* node){
	if(node->tln_self->t_priority > 0)
        node->tln_self->t_priority--;
}

void increasePriority(struct threadlistnode* node){
	if(node->tln_self->t_priority < 0xFFFF)
        node->tln_self->t_priority++;
}

void setage(struct threadlistnode *curthread, struct threadlist *tl){
	if(threadlist_isempty(tl) || (tl->tl_count <= 1)){
		// list is empty or only has 1 element
		return;
	}
	// putch('b'); putch('e');
	// printfromnode(tl->tl_head.tln_next);

	// increment the thread in the cpu
	increasePriority(curthread);

	// iterate through the list and modify as needed
	
	
	struct threadlistnode *firstRef = tl->tl_head.tln_next;
	
	putch('\n');
	putch(firstRef->tln_self->t_state+'0');
	putch('\n');

	do{	
		if (firstRef->tln_self->t_state == S_RUN){
			decreasePriority(firstRef);
			putch('\n');
			putch('D');
			putch('\n');

		}
		firstRef = firstRef->tln_next;
	}
	while(firstRef->tln_next != NULL);
	// putch('a');putch('f');
	// printfromnode(tl->tl_head.tln_next);

}