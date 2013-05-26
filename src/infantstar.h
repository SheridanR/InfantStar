/*-------------------------------------------------------------------------------

	INFANT STAR
	File: infantstar.h
	Desc: contains some prototypes as well as various type definitions

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <math.h>
#include "SDL.h"
#include "SDL_mixer.h"

// game world structure
typedef struct map_t {
	char name[32];   // name of the map
	char author[32]; // author of the map
	unsigned int width, height;  // size of the map
	unsigned char *tiles;
} map_t;

#define MAPLAYERS 5 // number of layers contained in a single map
#define OBSTACLELAYER 2 // obstacle layer in map

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
	long x, y;   // world coordinates
	unsigned int sizex, sizey;    // bounding box size
	int focalx, focaly;  // entity focal point
	unsigned int sprite; // the entity's sprite index
	
	// entity attributes
	double fskill[30];
	int skill[30];
	
	// a pointer to the entity's location in a list
	node_t *node;
	
	// behavior function pointer
	void (*behavior)(struct entity_t *my);
} entity_t;

#define max(a,b) \
		({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a > _b ? _a : _b; })
#define min(a,b) \
		({ typeof (a) _a = (a); \
		typeof (b) _b = (b); \
		_a < _b ? _a : _b; })

extern SDL_TimerID timer;
extern SDL_Surface *screen;
extern SDL_Event event;
extern int xres;
extern int yres;
extern int mainloop;
extern unsigned long ticks;
extern int keystatus[323];
extern int mousestatus[5];
extern int mousex, mousey;
extern long camx, camy;
extern long newcamx, newcamy;

// various definitions
extern map_t map;
extern SDL_Surface *font8_bmp;
extern SDL_Surface *font16_bmp;
extern SDL_Surface **sprites;
extern SDL_Surface **tiles;
extern Mix_Chunk **sounds;
extern int numsprites;
extern int numtiles;
extern int numsounds;
extern int audio_rate, audio_channels, audio_buffers;
extern Uint16 audio_format;

// function prototypes for list.c:
int list_FreeAll(list_t *list);
int list_RemoveNode(node_t *node);
node_t *list_AddNode(list_t *list);

// function prototypes for objects.c:
void n_DefaultDeconstructor(void *data);
entity_t *newEntity(void);

// function prototypes for game.c:
Uint32 timerCallback(Uint32 interval, void *param);
void handleEvents(void);
void gameLogic(void);

// function prototypes for behaviors.c:
void actPlayer(entity_t *my);

// function prototypes for draw.c:
void drawBackground(long camx, long camy);
void drawForeground(long camx, long camy);
void drawEntities(long camx, long camy);
void printText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... );