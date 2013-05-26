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

	newEntity

	Creates a new entity with empty settings and places it in the entity list

-------------------------------------------------------------------------------*/

entity_t *newEntity(void) {
	entity_t *entity;
	int c;
	
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
	entity->sizex=0;
	entity->sizey=0;
	entity->sprite=0;
	for( c=0; c<30; c++ ) {
		entity->skill[c]=0;
		entity->fskill[c]=0;
	}
	entity->behavior=NULL;
	return entity;
}