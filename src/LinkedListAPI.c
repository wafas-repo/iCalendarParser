/*
 * CIS2750 F2017
 * Assignment 0
 * Wafa Qazi: 0932477
 * This file contains the implementation of the linked List API
 * This implementation is based on the List API that I implemented in my Assignment 2 for CIS2520, whcih I took
 * in the fall of 2016 with professor Fangju Wang.  The permission to use my CIS2520 code in CIS2750 was obtained on my behalf by
 * my current CIS2750 professor, Denis Nikitenko.
 */

#include <stdio.h>
#include <stdlib.h>
#include "LinkedListAPI.h"
//#include <assert.h>

/** Function to initialize the list metadata head with the appropriate function pointers.
 *@return the list struct
 *@param printFunction function pointer to print a single node of the list
 *@param deleteFunction function pointer to delete a single piece of data from the list
 *@param compareFunction function pointer to compare two nodes of the list in order to test for equality or order
 **/
List initializeList(char* (*printFunction)(void *toBePrinted),void (*deleteFunction)(void *toBeDeleted),int (*compareFunction)(const void *first,const void *second))
{
	//assert(printFunction != NULL);
	//assert(deleteFunction != NULL);
	//assert(compareFunction != NULL);

	List list;

	list.deleteData = deleteFunction;
	list.printData = printFunction;
	list.compare = compareFunction;
	list.head = NULL;
	list.tail = NULL;
	list.length = 0;

	return list;

}

/**Function for creating a node for the linked list. 
 * This node contains abstracted (void *) data as well as previous and next
 * pointers to connect to other nodes in the list
 *@pre data should be of same size of void pointer on the users machine to avoid size conflicts. data must be valid.
 *data must be cast to void pointer before being added.
 *@post data is valid to be added to a linked list
 *@return On success returns a node that can be added to a linked list. On failure, returns NULL.
 *@param data - is a void * pointer to any data type.  Data must be allocated on the heap.
 **/
Node *initializeNode(void *data)
{


	Node * newNode = malloc(sizeof(Node));

	newNode->data = (void*) data;
	newNode->previous = NULL;
	newNode->next = NULL;

	return newNode;
}

/**Inserts a Node at the front of a linked list.  List metadata is updated
 * so that head and tail pointers are correct.
 *@pre 'List' type must exist and be used in order to keep track of the linked list.
 *@param list pointer to the dummy head of the list
 *@param toBeAdded a pointer to data that is to be added to the linked list
 **/
void insertFront(List *list, void *toBeAdded)
{


	Node * tmpNode = initializeNode((void*)toBeAdded);
	if(list == NULL){
		return;
	}
	else if(list->head == NULL)
	{
		//list->head = list->tail = tmpNode;
		list->head = tmpNode;
		list->tail = list->head;
		list->length++;
		return;
	}

	tmpNode->next = list->head;

	if(list->head !=  NULL)
	{
		list->head->previous = tmpNode;
	}

	list->head = tmpNode;
	list->length++;
}

/**Inserts a Node at the back of a linked list. 
 *List metadata is updated so that head and tail pointers are correct.
 *@pre 'List' type must exist and be used in order to keep track of the linked list.
 *@param list pointer to the dummy head of the list
 *@param toBeAdded a pointer to data that is to be added to the linked list
 **/
void insertBack(List *list, void *toBeAdded)
{

	Node * tmpNode = initializeNode((void*)toBeAdded);

	if(list == NULL)
	{
		return;
	}

	else if(list->head == NULL)
	{
		list->head = list->tail = tmpNode;
		list->length++;
		return;
	}

	Node * current = list->head;

	while(current->next != NULL)
	{
		current = current->next;
	}

	current->next = tmpNode;
	tmpNode->previous = current;

	list->tail = tmpNode;

	list->length++;
}

/** Clears the contents linked list, freeing all memory asspociated with these contents.
 * uses the supplied function pointer to release allocated memory for the data
 *@pre 'List' type must exist and be used in order to keep track of the linked list.
 *@param list pointer to the List-type dummy node
 **/
void clearList(List *list)
{

	if(list == NULL)
	{
		return;
	}

	if(list->head == NULL)
	{
		return;
	}

	Node * tmp = list->head;

	while(list->head->next != NULL)
	{
		tmp = list->head;
		list->head = list->head->next;
		list->deleteData(tmp->data);
		free(tmp);
	}

	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
}

/** Uses the comparison function pointer to place the element in the 
 * appropriate position in the list.
 * should be used as the only insert function if a sorted list is required.  
 *@pre List exists and has memory allocated to it. Node to be added is valid.
 *@post The node to be added will be placed immediately before or after the first occurrence of a related node
 *@param list a pointer to the dummy head of the list containing function pointers for delete and compare, as well 
 as a pointer to the first and last element of the list.
 *@param toBeAdded a pointer to data that is to be added to the linked list
 **/
void insertSorted(List *list, void *toBeAdded)
{

	if(list == NULL)
	{
		return;
	}
	// Recycled code from 2520 assignment with fangju wang 2016

	Node * newNode = initializeNode((void*)toBeAdded);
	Node* current = list->head;


	if(list->head == NULL)
	{
		list->head = list->tail = newNode;

	} else if((list->compare(list->head->data, newNode->data)) > 0) {


		newNode->next = list->head;

		if(list->head !=  NULL)
		{
			list->head->previous = newNode;
		}

		list->head = newNode;

	} else if((list->compare(list->tail->data, newNode->data)) < 0)	{

		list->tail->next = newNode;
		newNode->previous = list->tail;
		list->tail = newNode;

	} else{

		current = list->head;

		while(current->next!= NULL && (list->compare(current->next->data, newNode->data)) > 0)
		{
			current = current->next;
		}

		current->previous->next = newNode;
		newNode->previous = current->previous;
		current->previous = newNode;
		newNode->next = current;

	}
	list->length++;
}

/** Removes data from from the list, deletes the node and frees the memory,
 * changes pointer values of surrounding nodes to maintain list structure.
 * returns the data 
 *@pre List must exist and have memory allocated to it
 *@post toBeDeleted will have its memory freed if it exists in the list.
 *@param list pointer to the dummy head of the list containing deleteFunction function pointer
 *@param toBeDeleted pointer to data that is to be removed from the list
 *@return on success: void * pointer to data  on failure: NULL
 **/
void* deleteDataFromList(List *list, void *toBeDeleted)
{
	//Recycled code from my 2520 assignment with fangju wang 2016

	Node * toRemove = NULL;
	Node * temp;

	if(toBeDeleted == NULL)
	{
		return NULL;
	}

	if(list->compare(list->head->data, toBeDeleted) == 0)
	{
		temp = list->head;
		toRemove = list->head->data;
		list->head = list->head->next;
		list->head->previous = NULL;
		return (void*)toRemove;
		free(temp);
		list->length--;
	}

	else if(list->head->next != NULL)
	{
		temp = list->head;

		while(temp->next->next != NULL)
		{
			if(list->compare(temp->next->data, toBeDeleted) == 0)
			{
				Node * current = temp->next;
				toRemove = temp->next->data;
				temp->next = current->next;
				current->next->previous = current->previous;
				return (void*)toRemove;
				free(current);
				list->length--;
			}

			temp = temp->next;
		}

		if(list->compare(temp->next->data, toBeDeleted) == 0)
		{
			toRemove = temp->next->data;
			temp = temp->next;
			list->tail = list->tail->previous;
			list->tail->next = NULL;
			return (void*)toRemove;
			free(temp);
			list->length--;
		}	

	}

	return NULL;

}

/**Returns a pointer to the data at the front of the list. Does not alter list structure.
 *@pre The list exists and has memory allocated to it
 *@param list pointer to the dummy head of the list containing the head of the list
 *@return pointer to the data located at the head of the list
 **/
void* getFromFront(List list)
{
	if(list.head != NULL)
	{
		return list.head->data;

	} else {

		return NULL;
	}
}

/**Returns a pointer to the data at the back of the list. Does not alter list structure.
 *@pre The list exists and has memory allocated to it
 *@param list pointer to the dummy head of the list containing the tail of the list
 *@return pointer to the data located at the tail of the list
 **/
void* getFromBack(List list)
{

	if(list.tail != NULL)
	{
		return list.tail->data;

	} else {

		return NULL;
	}
}

/**Returns a string that contains a string representation of
  the list traversed from  head to tail. Utilize the list's printData function pointer to create the string.
  returned string must be freed by the calling function.
 *@pre List must exist, but does not have to have elements.
 *@param list Pointer to linked list dummy head.
 *@return on success: char * to string representation of list (must be freed after use).  on failure: NULL
 **/
char* toString(List list)
{
	Node * temp = list.head;

	int size = 0;

	while(list.head != NULL)
	{
		size += strlen(list.printData(list.head->data));
		list.head = list.head->next;
	}

	char* str = malloc(sizeof(char)*size);

	while(temp != NULL)
	{
		strcat(str, list.printData(temp->data));
		temp = temp->next;

	}

	return str;

}

/** Function for creating an iterator for the linked list. 
 * This node contains abstracted (void *) data as well as previous and next
 * pointers to connect to other nodes in the list
 *@pre List exists and is valid
 *@post List remains unchanged.  The iterator has been allocated and points to the head of the list.
 *@return The newly created iterator object.
 *@param list - a pointer to the list to iterate over.
 **/
ListIterator createIterator(List list)
{

	ListIterator iter;

	iter.current = list.head;

	return iter;

}

/** Function that returns the next element of the list through the iterator. 
 * This function returns the head of the list the first time it is called after.
 * the iterator was created. Every subsequent call returns the next element.
 *@pre List exists and is valid.  Iterator exists and is valid.
 *@post List remains unchanged.  The iterator points to the next element on the list.
 *@return The data associated with the list element that the iterator pointed to when the function was called.
 *@param iter - an iterator to a list.
 **/
void* nextElement(ListIterator* iter)
{

	if(iter->current == NULL)
	{
		return NULL;
	}

	void * ptr;

	ptr = iter->current->data;

	iter->current = iter->current->next;

	return ptr;
}

/**Returns the number of elements in the list.
 *@pre List must exist, but does not have to have elements.
 *@param list - the list struct.
 *@return on success: number of eleemnts in the list (0 or more).  on failure: -1 (e.g. list not initlized correctly)
 **/
int getLength(List list)
{
	/*int size = 0;

	  if(list.head == NULL)
	  {
	  return 0;
	  } 

	  Node * iter = list.head;

	  while(iter != NULL)
	  {
	  iter = iter->next;
	  size++;
	  }

	  return size;*/
	return (list.head ? list.length: -1);
}

/** Function that searches for an element in the list using a comparator function.
 * If an element is found, a pointer to the data of that element is returned
 * Returns NULL if the element is not found.
 *@pre List exists and is valid.  Comparator function has been provided.
 *@post List remains unchanged.
 *@return The data associated with the list element that matches the search criteria.  If element is not found, return NULL.
 *@param list - a list sruct
 *@param customCompare - a pointer to comparator fuction for customizing the search
 *@param searchRecord - a pointer to search data, which contains seach criteria
 *Note: while the arguments of compare() and searchRecord are all void, it is assumed that records they point to are
 *      all of the same type - just like arguments to the compare() function in the List struct
 **/
void* findElement(List list, bool (*customCompare)(const void* first,const void* second), const void* searchRecord)
{
	void* elem;

	ListIterator iter = createIterator(list);

	while ((elem = nextElement(&iter)) != NULL)
	{
		const void* tmp = (const void*)elem;

		if(customCompare((const void*)tmp, (const void*)searchRecord))
		{
			return (void*)tmp;
		}
	}
	return NULL;
}
