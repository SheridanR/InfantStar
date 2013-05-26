/*-------------------------------------------------------------------------------

	INFANT STAR
	File: list.c
	Desc: contains list handling functions.

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

/*-------------------------------------------------------------------------------

	list_FreeAll

	Frees an entire list and all of its contents

-------------------------------------------------------------------------------*/

int list_FreeAll(list_t *list) {
	node_t *node;
	node_t *nextnode;
	
	if( list->first == NULL ) return 1;
	
	node=list->first;
	while(node!=NULL) {
		nextnode = node->next;
		(*node->deconstructor)(node->element);
		free(node);
		node=nextnode;
	}
	list->first = NULL;
	list->last = NULL;
	return 0;
}

/*-------------------------------------------------------------------------------

	list_RemoveNode

	Removes a specific node from a list

-------------------------------------------------------------------------------*/

int list_RemoveNode(node_t *node) {
	if( node->list->first == NULL || node->list->last == NULL ) return 1;
	
	// if this is the first node...
	if( node == node->list->first ) {
		// is it also the last node?
		if( node->list->last == node ) {
			node->list->first = NULL;
			node->list->last = NULL;
			return 0;
		}
		
		// otherwise, the "first" pointer needs to point to the next node
		else {
			node->next->prev = NULL;
			node->list->first = node->next;
		}
	}
	
	// if this is the last node, but not the first...
	else if( node == node->list->last ) {
		node->prev->next = NULL;
		node->list->last = node->prev; // the "last" pointer needs to point to the previous node
	}
	
	// if the node is neither first nor last, it is in the middle
	else {
		// bridge the previous node and the first node together
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	
	// once the node is removed from the list, delete it
	(*node->deconstructor)(node->element);
	return 0;
}

/*-------------------------------------------------------------------------------

	list_AddNode

	Inserts a new node at the end of a given list

-------------------------------------------------------------------------------*/

node_t *list_AddNode(list_t *list) {
	node_t *node;
	
	// allocate memory for node
	if( (node = (node_t *) malloc(sizeof(node_t))) == NULL ) {
		fprintf( stderr, "failed to allocate memory for new node!\n" );
		exit(1);
	}
	
	// integrate it into the list
	if( list->last != NULL ) {
		// there are prior nodes in the list
		list->last->next = node;
		node->prev = list->last;
	} else {
		// inserting into an empty list
		node->prev = NULL;
		list->first = node;
	}
	list->last = node;
	node->next = NULL;
	
	return node;
}