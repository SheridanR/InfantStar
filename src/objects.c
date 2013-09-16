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

entity_t *newEntity(int sprite, int pos) {
	entity_t *entity;
	int c;
	
	// allocate memory for entity
	if( (entity = (entity_t *) malloc(sizeof(entity_t))) == NULL ) {
		fprintf( stderr, "failed to allocate memory for new entity!\n" );
		exit(1);
	}
	
	// add the entity to the entity list
	if(!pos)
		entity->node = list_AddNodeFirst(&entity_l);
	else
		entity->node = list_AddNodeLast(&entity_l);
	entity->node->element = entity;
	entity->node->deconstructor = &n_DefaultDeconstructor;
	
	// now set all of my data elements to ZERO or NULL
	entity->x=0;
	entity->y=0;
	entity->sprite=sprite;
	switch( sprite ) {
		// player
		case 1:	entity->focalx=8;
				entity->focaly=32;
				break;
		// purple gem
		case 53:	entity->focalx=8;
				entity->focaly=8;
				break;
		// red gem
		case 57:	entity->focalx=8;
				entity->focaly=8;
				break;
		// item glean
		case 66:	entity->focalx=8;
				entity->focaly=8;
				break;
		// particles
		case 70:	
		case 71:	
		case 72:	
		case 73:	entity->focalx=3;
				entity->focaly=3;
				break;
		// troll
		case 74:	
		case 75:	entity->focalx=16;
				entity->focaly=32;
				break;
		// everything else
		default:	entity->focalx=0;
				entity->focalx=0;
				break;
	}
	for( c=0; c<30; c++ ) {
		entity->skill[c]=0;
		entity->fskill[c]=0;
	}
	entity->behavior=NULL;
	return entity;
}

/*-------------------------------------------------------------------------------

	newButton

	Creates a new button and places it in the button list

-------------------------------------------------------------------------------*/

button_t *newButton(void) {
	button_t *button;
	
	// allocate memory for button
	if( (button = (button_t *) malloc(sizeof(button_t))) == NULL ) {
		fprintf( stderr, "failed to allocate memory for new button!\n" );
		exit(1);
	}
	
	// add the button to the button list
	button->node = list_AddNodeLast(&button_l);
	button->node->element = button;
	button->node->deconstructor = &n_DefaultDeconstructor;
	
	// now set all of my data elements to ZERO or NULL
	button->x=0;
	button->y=0;
	button->sizex=0;
	button->sizey=0;
	button->visible=1;
	button->focused=0;
	strcpy(button->label,"nodef");
	return button;
}

/*-------------------------------------------------------------------------------

	newUndo

	Creates a copy of the current map and entity list and stores them in a list

-------------------------------------------------------------------------------*/

void newUndo(void) {
	button_t *button;
	
	// allocate memory for button
	if( (button = (button_t *) malloc(sizeof(button_t))) == NULL ) {
		fprintf( stderr, "failed to allocate memory for new button!\n" );
		exit(1);
	}
	
	// add the button to the button list
	button->node = list_AddNodeLast(&button_l);
	button->node->element = button;
	button->node->deconstructor = &n_DefaultDeconstructor;
	
	// now set all of my data elements to ZERO or NULL
	button->x=0;
	button->y=0;
	button->sizex=0;
	button->sizey=0;
	button->visible=1;
	button->focused=0;
	strcpy(button->label,"nodef");
}