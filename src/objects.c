/*-------------------------------------------------------------------------------

	INFANT STAR
	File: objects.c
	Desc: contains object constructors and deconstructors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

/*-------------------------------------------------------------------------------

	n_DefaultDeconstructor

	Frees the memory occupied by a typical node's data. Do not use for more
	complex nodes that malloc extraneous data to themselves!

-------------------------------------------------------------------------------*/

void n_DefaultDeconstructor(void *data) {
	free(data);
}

/*-------------------------------------------------------------------------------

	e_CreateEntity

	Creates a new entity with empty settings and places it in the entity list

-------------------------------------------------------------------------------*/

entity_t *e_CreateEntity(void) {
	entity_t *entity;
	
	// allocate memory for entity
	if( (entity = (entity_t *) malloc(sizeof(entity_t))) == NULL ) {
		fprintf( stderr, "failed to allocate memory for new entity!\n" );
		exit(1);
	}
	
	// add the entity to the entity list
	entity->node = list_AddNode(&entity_l);
	entity->node->element = entity;
	entity->node->deconstructor = &n_DefaultDeconstructor;
	
	// now set all of my data elements to ZERO or NULL
	entity->x=0;
	entity->y=0;
	entity->z=0;
	entity->ang=0;
	entity->sizex=0;
	entity->sizey=0;
	entity->sizez=0;
	entity->texture=0;
	return entity;
}