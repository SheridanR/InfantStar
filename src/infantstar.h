/*-------------------------------------------------------------------------------

	INFANT STAR
	File: infantstar.h
	Desc: contains some prototypes as well as various type definitions

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "SDL.h"

// game world structure
typedef struct map_t {
	char name[32];   // name of the map
	char author[32]; // author of the map
	unsigned int width, height;  // size of the map
	char *tiles;
} map_t;

// node structure
typedef struct node_t {
	struct node_t *next;
	struct node_t *prev;
	struct list_t *list;
	void *element;
	void (*deconstructor)(void *data);
} node_t;

// list structure
typedef struct list_t {
	node_t *first;
	node_t *last;
} list_t;
extern list_t entity_l;

// entity structure
typedef struct entity_t {
	unsigned int x, y;   // world coordinates
	int sizex, sizey;    // bounding box size
	unsigned int sprite; // the entity's sprite index
	node_t *node;        // a pointer to the entity's location in a list
} entity_t;

extern SDL_Surface *screen;
extern SDL_Event event;
extern int xres;
extern int yres;
extern int mainloop;
extern int keystatus[323];
extern int mousestatus[5];
extern int mousex, mousey;

// various definitions
extern SDL_Surface *font8_bmp;
extern SDL_Surface *font16_bmp;
extern SDL_Surface **sprites;
extern SDL_Surface **tiles;
extern char *tile_flags;
extern Mix_Chunk **sounds;
int numsprites=0;
int numtiles=0;
int numsounds=0;
extern int audio_rate, audio_channels, audio_buffers;
extern Uint16 audio_format;

// function prototypes
int list_FreeList(list_t *list);
int list_RemoveNode(node_t *node);
node_t *list_AddNode(list_t *list);
void n_DefaultDeconstructor(void *data);
entity_t *e_CreateEntity(void);