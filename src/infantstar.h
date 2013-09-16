/*-------------------------------------------------------------------------------

	INFANT STAR
	File: infantstar.h
	Desc: contains some prototypes as well as various type definitions

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <time.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "sprig.h"

// game world structure
typedef struct map_t {
	char name[32];   // name of the map
	char author[32]; // author of the map
	unsigned int width, height;  // size of the map
	int *tiles;
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
extern list_t button_l;

// entity structure
typedef struct entity_t {
	long x, y;   // world coordinates
	long focalx, focaly;  // entity focal point
	int sprite; // the entity's sprite index
	
	// entity attributes
	double fskill[30];
	int skill[30];
	
	// a pointer to the entity's location in a list
	node_t *node;
	
	// behavior function pointer
	void (*behavior)(struct entity_t *my);
} entity_t;

typedef struct button_t {
	char label[32];   // button label
	int x, y;         // onscreen position
	int sizex, sizey; // size of the button
	int visible;      // invisible buttons are ignored by the handler
	int focused;      // allows this button to function when a subwindow is open
	
	// a pointer to the button's location in a list
	node_t *node;
	
	void (*action)(struct button_t *my);
} button_t;

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
extern int fullscreen;
extern int xres;
extern int yres;
extern int mainloop;
extern unsigned long ticks;
extern int keystatus[323];
extern char keypressed;
extern int mousestatus[6];
extern int mousex, mousey;
extern int mousexrel, mouseyrel;
extern long camx, camy;
extern long newcamx, newcamy;
extern entity_t *selectedEntity;

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
extern char *animatedtiles;

// function prototypes for init.c:
int initApp(char *title, int fullscreen);

// function prototypes for list.c:
int list_FreeAll(list_t *list);
int list_RemoveNode(node_t *node);
node_t *list_AddNodeFirst(list_t *list);
node_t *list_AddNodeLast(list_t *list);

// function prototypes for objects.c:
void n_DefaultDeconstructor(void *data);
entity_t *newEntity(int sprite, int pos);
button_t *newButton(void);

// function prototypes for draw.c:
void drawLayer(long camx, long camy, int z);
void drawBackground(long camx, long camy);
void drawForeground(long camx, long camy);
void drawSky(SDL_Surface *srfc);
void drawEntities(long camx, long camy);
void drawGrid(long camx, long camy);
void drawMinimap(long camx, long camy);
void drawWindow(int x1, int y1, int x2, int y2);
void drawDepressed(int x1, int y1, int x2, int y2);
void printTextFormatted( SDL_Surface *font_bmp, int x, int y, char *fmt, ... );
void printText( SDL_Surface *font_bmp, int x, int y, char *str );

// function prototypes for files.c:
int loadMap(char *filename);
int saveMap(char *filename);

// function prototypes for cursors.c:
SDL_Cursor *newCursor(char *image[]);

// cursor bitmap definitions
extern char *cursor_pencil[];
extern char *cursor_brush[];
extern char *cursor_fill[];