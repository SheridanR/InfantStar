/*-------------------------------------------------------------------------------

	INFANT STAR
	File: editor.c
	Desc: main code for the level editor

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"
#include "editor.h"

// definitions
SDL_TimerID timer;
SDL_Surface *screen=NULL;
SDL_Event event;
int fullscreen = 0;
int xres = 800;
int yres = 600;
int mainloop = 1;
unsigned long ticks;
int keystatus[323];
char keypressed=0;
int mousestatus[6];
int mousex=0, mousey=0;
int omousex=0, omousey=0;
int mousexrel=0, mouseyrel=0;
long camx=0, camy=-16;
long newcamx, newcamy;

// various definitions
map_t map;
SDL_Surface *font8_bmp;
SDL_Surface *font16_bmp;
SDL_Surface **sprites;
SDL_Surface **tiles;
Mix_Chunk **sounds;
list_t entity_l;
list_t button_l;
int numsprites, numtiles, numsounds;
char *animatedtiles;

// audio definitions
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16;
int audio_channels = 2;
int audio_buffers = 512;

// function prototypes
Uint32 timerCallback(Uint32 interval, void *param);
void handleEvents(void);
void mainLogic(void);

// editor variables
int drawlayer=0, drawx=0, drawy=0;
int alllayers=1;
int scroll=0;
char layerstatus[20];
button_t *pressed = NULL;
int menuVisible=0;
int subwindow=0;
int subx1, subx2, suby1, suby2;
char subtext[1024];
int toolbox=1;
int statusbar=1;
int viewsprites=1;
int showgrid=0;
int selectedTile=0;
int tilepalette=0;
int spritepalette=0;
int mclick=0;
int selectedTool=0; // 0: point draw 1: brush 2: select 3: fill
int allowediting=0; // only turned on when the mouse is over paintable screen region
int openwindow=0, savewindow=0, newwindow=0;
int slidery=0, slidersize=16;
int menuDisappear=0;
int selectedFile=0;
char **d_names=NULL;
unsigned long d_names_length=0;
entity_t *selectedEntity=NULL;
char filename[32];
char message[48];
int messagetime=0;
int cursorflash=0;
char widthtext[4], heighttext[4], nametext[32], authortext[32];
int editproperty=0;
SDL_Cursor *cursorArrow, *cursorPencil, *cursorBrush, *cursorSelect, *cursorFill;
int *palette;

// buttons
button_t *butX;
button_t *but_;
button_t *butTilePalette;
button_t *butSprite;
button_t *butPoint;
button_t *butBrush;
button_t *butSelect;
button_t *butFill;
button_t *butFile;
button_t *butNew;
button_t *butOpen;
button_t *butSave;
button_t *butSaveAs;
button_t *butExit;
button_t *butEdit;
button_t *butCut;
button_t *butCopy;
button_t *butPaste;
button_t *butDelete;
button_t *butSelectAll;
button_t *butView;
button_t *butToolbox;
button_t *butStatusBar;
button_t *butAllLayers;
button_t *butViewSprites;
button_t *butGrid;
button_t *butFullscreen;
button_t *butMap;
button_t *butAttributes;
button_t *butClearMap;
button_t *butHelp;
button_t *butAbout;

/*-------------------------------------------------------------------------------

	mainLogic

	handles time dependent procedures

-------------------------------------------------------------------------------*/

void mainLogic(void) {
	// messages
	if( messagetime > 0 )
		messagetime--;
	cursorflash++;
	if( cursorflash >= 40 )
		cursorflash = 0;
	
	// basic editing functions are not available under these cases
	if( subwindow || tilepalette || spritepalette )
		return;
	
	// scroll camera on minimap
	if( mousestatus[SDL_BUTTON_LEFT] && toolbox ) {
		if( omousex >= xres-120 && omousex < xres-8 ) {
			if( omousey >= 24 && omousey < 136 ) {
				camx = ((long)map.width<<4)*(double)(mousex-xres+120)/112 - xres/2;
				camy = ((long)map.height<<4)*(double)(mousey-24)/112 - yres/2;
			}
		}
	}

	// basic editor functions
	camx += (keystatus[SDLK_RIGHT]-keystatus[SDLK_LEFT])*16;
	camy += (keystatus[SDLK_DOWN]-keystatus[SDLK_UP])*16;
	if( camx < -xres/2 )
		camx = -xres/2;
	if( camx > ((long)map.width<<4)-((long)xres/2) )
		camx = ((long)map.width<<4)-((long)xres/2);
	if( camy < -yres/2 )
		camy = -yres/2;
	if( camy > ((long)map.height<<4)-((long)yres/2) )
		camy = ((long)map.height<<4)-((long)yres/2);
	
	if(scroll<0) { // mousewheel up
		drawlayer = min(drawlayer+1,MAPLAYERS-1);
		scroll=0;
	}
	if(scroll>0) { // mousewheel down
		drawlayer = max(drawlayer-1,0);
		scroll=0;
	}
		
	switch( drawlayer ) {
		case 0: strcpy(layerstatus,"FAR BACKGROUND"); break;
		case 1: strcpy(layerstatus,"BACKGROUND"); break;
		case 2: strcpy(layerstatus,"OBSTACLES"); break;
		case 3: strcpy(layerstatus,"FOREGROUND"); break;
		case 4: strcpy(layerstatus,"CLOSE FOREGROUND"); break;
		default: strcpy(layerstatus,"UNKNOWN"); break;
	}
}

/*-------------------------------------------------------------------------------

	handleButtons

	Draws buttons and processes clicks

-------------------------------------------------------------------------------*/

void handleButtons(void) {
	node_t *node;
	node_t *nextnode;
	button_t *button;
	
	// handle buttons
	for( node=button_l.first; node!=NULL; node=nextnode ) {
		nextnode = node->next;
		button = (button_t *)node->element;
		if( !subwindow && button->focused ) {
			list_RemoveNode(button->node);
			continue;
		}
		if( button->visible == 0 )
			continue; // invisible buttons are not processed
		if( subwindow && !button->focused ) {
			// unfocused buttons do not work when a subwindow is active
			drawWindow(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
			printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
		} else {
			if( omousex >= button->x && omousex < button->x+button->sizex )
				if( omousey >= button->y && omousey < button->y+button->sizey ) {
					if( button == butFile && menuVisible )
						menuVisible = 1;
					if( button == butEdit && menuVisible )
						menuVisible = 2;
					if( button == butView && menuVisible )
						menuVisible = 3;
					if( button == butMap && menuVisible )
						menuVisible = 4;
					if( button == butHelp && menuVisible )
						menuVisible = 5;
					if( mousestatus[SDL_BUTTON_LEFT] )
						pressed = button;
				}
			if( button == pressed ) {
				if( omousex >= button->x && omousex < button->x+button->sizex && mousex >= button->x && mousex < button->x+button->sizex ) {
					if( omousey >= button->y && omousey < button->y+button->sizey && mousey >= button->y && mousey < button->y+button->sizey ) {
						drawDepressed(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
						printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
						if( !mousestatus[SDL_BUTTON_LEFT] ) { // releasing the mouse over the button
							pressed = NULL;
							if( button->action != NULL ) {
								(*button->action)(button); // run the button's assigned action
								if( !subwindow && button->focused )
									list_RemoveNode(button->node);
							}
						}
					} else {
						drawWindow(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
						printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
						if( !mousestatus[SDL_BUTTON_LEFT] ) // releasing the mouse over nothing
							pressed = NULL;
					}
				} else {
					drawWindow(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
					printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
					if( !mousestatus[SDL_BUTTON_LEFT] ) // releasing the mouse over nothing
						pressed = NULL;
				}
			} else {
				if( (button!=butFile||menuVisible!=1) && (button!=butEdit||menuVisible!=2) && (button!=butView||menuVisible!=3) && (button!=butMap||menuVisible!=4) && (button!=butHelp||menuVisible!=5) ) {
					drawWindow(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
					printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
				} else {
					drawDepressed(button->x,button->y,button->x+button->sizex,button->y+button->sizey);
					printTextFormatted(font8_bmp,button->x+4,button->y+4,button->label,drawx,drawy,drawlayer,layerstatus);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	handleEvents

	Handles all SDL events; receives input, updates gamestate, etc.

-------------------------------------------------------------------------------*/

void handleEvents(void) {
	mousexrel=0; mouseyrel=0;
	keypressed=0;
	while( SDL_PollEvent(&event) ) { // poll SDL events
		// Global events
		switch( event.type ) {
			case SDL_QUIT: // if SDL receives the shutdown signal
				buttonExit(NULL);
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				keystatus[event.key.keysym.sym] = 1; // set this key's index to 1
				if( (event.key.keysym.unicode & 0xFF80) == 0 )
					keypressed=event.key.keysym.unicode&0x7F; // record the ascii character that was pressed
				break;
			case SDL_KEYUP: // if a key is unpressed...
				keystatus[event.key.keysym.sym] = 0; // set this key's index to 0
				break;
			case SDL_MOUSEMOTION: // if the mouse is moved...
				mousex = event.motion.x;
				mousey = event.motion.y;
				mousexrel = event.motion.xrel;
				mouseyrel = event.motion.yrel;
				break;
			case SDL_MOUSEBUTTONDOWN: // if a mouse button is pressed...
				mousestatus[event.button.button] = 1; // set this mouse button to 1
				if(mousestatus[4])
					scroll=1;
				else if(mousestatus[5])
					scroll=-1;
				break;
			case SDL_MOUSEBUTTONUP: // if a mouse button is released...
				mousestatus[event.button.button] = 0; // set this mouse button to 0
				break;
			case SDL_USEREVENT: // if the game timer elapses
				mainLogic();
				break;
			case SDL_VIDEORESIZE: // if the window is resized
				if(fullscreen)
					break;
				xres = max(event.resize.w,100);
				yres = max(event.resize.h,75);
				free(palette);
				palette = (int *) malloc(sizeof(unsigned int)*xres*yres);
				if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_RESIZABLE )) == NULL) {
					fprintf(stderr, "failed to set video mode.\n");
					mainloop=0;
				}
				break;
		}
	}
	if(!mousestatus[SDL_BUTTON_LEFT]) {
		omousex=mousex;
		omousey=mousey;
	}
}

/*-------------------------------------------------------------------------------

	timerCallback

	A callback function for the game timer which pushes an SDL event

-------------------------------------------------------------------------------*/

Uint32 timerCallback(Uint32 interval, void *param) {
	SDL_Event event;
	SDL_UserEvent userevent;
	
	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;
	
	event.type = SDL_USEREVENT;
	event.user = userevent;
	
	ticks++;
	SDL_PushEvent(&event);
	return(interval);
}

/*-------------------------------------------------------------------------------

	editFill

	Fills a region of the map with a certain tile

-------------------------------------------------------------------------------*/

void editFill(int x, int y, int layer, int type) {
	int repeat = 1;
	int fillspot;

	if( type == map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height] )
		return;
	
	fillspot = map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height];
	map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height] = type+numtiles;
	
	if( map.tiles[layer+y*MAPLAYERS+(x+1)*MAPLAYERS*map.height] == fillspot && x < map.width-1 )
		map.tiles[layer+y*MAPLAYERS+(x+1)*MAPLAYERS*map.height] = type+numtiles;
	if( map.tiles[layer+y*MAPLAYERS+(x-1)*MAPLAYERS*map.height] == fillspot && x > 0 )
		map.tiles[layer+y*MAPLAYERS+(x-1)*MAPLAYERS*map.height] = type+numtiles;
	if( map.tiles[layer+(y+1)*MAPLAYERS+x*MAPLAYERS*map.height] == fillspot && y < map.height-1 )
		map.tiles[layer+(y+1)*MAPLAYERS+x*MAPLAYERS*map.height] = type+numtiles;
	if( map.tiles[layer+(y-1)*MAPLAYERS+x*MAPLAYERS*map.height] == fillspot && y > 0 )
		map.tiles[layer+(y-1)*MAPLAYERS+x*MAPLAYERS*map.height] = type+numtiles;

	while( repeat ) {
		repeat = 0;
		for( x=0; x<map.width; x++ )
			for( y=0; y<map.height; y++ ) {
				if( map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height] == type+numtiles ) {
					if( map.tiles[layer+y*MAPLAYERS+(x+1)*MAPLAYERS*map.height] == fillspot && x < map.width-1 ) {
						map.tiles[layer+y*MAPLAYERS+(x+1)*MAPLAYERS*map.height] = type+numtiles;
						repeat = 1;
					}
					if( map.tiles[layer+y*MAPLAYERS+(x-1)*MAPLAYERS*map.height] == fillspot && x > 0 ) {
						map.tiles[layer+y*MAPLAYERS+(x-1)*MAPLAYERS*map.height] = type+numtiles;
						repeat = 1;
					}
					if( map.tiles[layer+(y+1)*MAPLAYERS+x*MAPLAYERS*map.height] == fillspot && y < map.height-1 ) {
						map.tiles[layer+(y+1)*MAPLAYERS+x*MAPLAYERS*map.height] = type+numtiles;
						repeat = 1;
					}
					if( map.tiles[layer+(y-1)*MAPLAYERS+x*MAPLAYERS*map.height] == fillspot && y > 0 ) {
						map.tiles[layer+(y-1)*MAPLAYERS+x*MAPLAYERS*map.height] = type+numtiles;
						repeat = 1;
					}
				}
			}
	}
	
	for( x=0; x<map.width; x++ )
		for( y=0; y<map.height; y++ ) {
			if( map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height] == type+numtiles )
				map.tiles[layer+y*MAPLAYERS+x*MAPLAYERS*map.height] = type;
		}
}

/*-------------------------------------------------------------------------------

	main
	
	Initializes program resources, harbors main loop, and cleans up
	afterwords

-------------------------------------------------------------------------------*/

int main(int argc, char **argv ) {
	button_t *button;
	node_t *node;
	node_t *nextnode;
	entity_t *entity;
	SDL_Rect pos;
	int c;
	int x, y, z;
	int x2, y2;
	char action[32];
	
	// initialize
	if( (x=initApp("Infant Star Editor",fullscreen)) )
		exit(x);
	
	// load cursors
	cursorArrow = SDL_GetCursor();
	cursorPencil = newCursor(cursor_pencil);
	cursorBrush = newCursor(cursor_brush);
	cursorSelect = cursorArrow;
	cursorFill = newCursor(cursor_fill);
	
	// instatiate a timer
	timer = SDL_AddTimer(50, timerCallback, NULL);
	srand(time(NULL));
	
	// create an empty map
	map.width = 32;
	map.height = 23;
	map.tiles = (int *) malloc(sizeof(int)*map.width*map.height*MAPLAYERS);
	for( z=0; z<MAPLAYERS; z++ ) {
		for( y=0; y<map.height; y++ ) {
			for( x=0; x<map.width; x++ ) {
				if(z==0)
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 1;
				else if(z==2) {
					if(x==0||y==0||x==map.width-1||y==map.height-1)
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 2;
					else
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
				}
				else
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
			}
		}
	}
	
	// initialize editor settings
	drawx=0; drawy=0; drawlayer=0;
	strcpy(layerstatus,"BACKGROUND");
	palette = (int *) malloc(sizeof(unsigned int)*xres*yres);
	
	// main interface
	button = butFile = newButton();
	strcpy(button->label,"File");
	button->x=0; button->y=0;
	button->sizex=40; button->sizey=16;
	button->action=&buttonFile;
	
	button = butEdit = newButton();
	strcpy(button->label,"Edit");
	button->x=40; button->y=0;
	button->sizex=40; button->sizey=16;
	button->action=&buttonEdit;
	
	button = butView = newButton();
	strcpy(button->label,"View");
	button->x=80; button->y=0;
	button->sizex=40; button->sizey=16;
	button->action=&buttonView;
	
	button = butMap = newButton();
	strcpy(button->label,"Map");
	button->x=120; button->y=0;
	button->sizex=32; button->sizey=16;
	button->action=&buttonMap;
	
	button = butHelp = newButton();
	strcpy(button->label,"Help");
	button->x=152; button->y=0;
	button->sizex=40; button->sizey=16;
	button->action=&buttonHelp;
	
	button = butX = newButton();
	strcpy(button->label,"X");
	button->x=xres-16; button->y=0;
	button->sizex=16; button->sizey=16;
	button->action=&buttonExit;
	
	button = but_ = newButton();
	strcpy(button->label,"_");
	button->x=xres-32; button->y=0;
	button->sizex=16; button->sizey=16;
	button->action=&buttonIconify;
	
	// toolbox
	button = butTilePalette = newButton();
	strcpy(button->label,"Palette ...");
	button->x=xres-112; button->y=152;
	button->sizex=96; button->sizey=16;
	button->action=&buttonTilePalette;
	
	button = butSprite = newButton();
	strcpy(button->label,"Sprite  ...");
	button->x=xres-112; button->y=168;
	button->sizex=96; button->sizey=16;
	button->action=&buttonSprite;
	
	button = butPoint = newButton();
	strcpy(button->label,"Point");
	button->x=xres-96; button->y=204;
	button->sizex=64; button->sizey=16;
	button->action=&buttonPoint;
	
	button = butBrush = newButton();
	strcpy(button->label,"Brush");
	button->x=xres-96; button->y=220;
	button->sizex=64; button->sizey=16;
	button->action=&buttonBrush;
	
	button = butSelect = newButton();
	strcpy(button->label,"Select");
	button->x=xres-96; button->y=236;
	button->sizex=64; button->sizey=16;
	button->action=&buttonSelect;
	
	button = butFill = newButton();
	strcpy(button->label,"Fill");
	button->x=xres-96; button->y=252;
	button->sizex=64; button->sizey=16;
	button->action=&buttonFill;
	
	// file menu
	butNew = button = newButton();
	strcpy(button->label,"New          Ctrl+N");
	button->x=16; button->y=16;
	button->sizex=160; button->sizey=16;
	button->action=&buttonNew;
	button->visible=0;
	
	butOpen = button = newButton();
	strcpy(button->label,"Open ...     Ctrl+O");
	button->x=16; button->y=32;
	button->sizex=160; button->sizey=16;
	button->action=&buttonOpen;
	button->visible=0;
	
	butSave = button = newButton();
	strcpy(button->label,"Save         Ctrl+S");
	button->x=16; button->y=48;
	button->sizex=160; button->sizey=16;
	button->action=&buttonSave;
	button->visible=0;
	
	butSaveAs = button = newButton();
	strcpy(button->label,"Save As ...");
	button->x=16; button->y=64;
	button->sizex=160; button->sizey=16;
	button->action=&buttonSaveAs;
	button->visible=0;
	
	butExit = button = newButton();
	strcpy(button->label,"Exit         Alt+F4");
	button->x=16; button->y=80;
	button->sizex=160; button->sizey=16;
	button->action=&buttonExit;
	button->visible=0;
	
	// edit menu
	butCut = button = newButton();
	strcpy(button->label,"Cut         Ctrl+X");
	button->x=56; button->y=16;
	button->sizex=152; button->sizey=16;
	button->action=&buttonCut;
	button->visible=0;
	
	butCopy = button = newButton();
	strcpy(button->label,"Copy        Ctrl+C");
	button->x=56; button->y=32;
	button->sizex=152; button->sizey=16;
	button->action=&buttonCopy;
	button->visible=0;
	
	butPaste = button = newButton();
	strcpy(button->label,"Paste       Ctrl+V");
	button->x=56; button->y=48;
	button->sizex=152; button->sizey=16;
	button->action=&buttonPaste;
	button->visible=0;
	
	butDelete = button = newButton();
	strcpy(button->label,"Delete      Del");
	button->x=56; button->y=64;
	button->sizex=152; button->sizey=16;
	button->action=&buttonDelete;
	button->visible=0;
	
	butSelectAll = button = newButton();
	strcpy(button->label,"Select All  Ctrl+A");
	button->x=56; button->y=80;
	button->sizex=152; button->sizey=16;
	button->action=&buttonSelectAll;
	button->visible=0;
	
	// view menu
	butStatusBar = button = newButton();
	strcpy(button->label,"Statusbar   Ctrl+I");
	button->x=96; button->y=16;
	button->sizex=152; button->sizey=16;
	button->action=&buttonStatusBar;
	button->visible=0;
	
	butToolbox = button = newButton();
	strcpy(button->label,"Toolbox     Ctrl+T");
	button->x=96; button->y=32;
	button->sizex=152; button->sizey=16;
	button->action=&buttonToolbox;
	button->visible=0;
	
	butAllLayers = button = newButton();
	strcpy(button->label,"All Layers  Ctrl+L");
	button->x=96; button->y=48;
	button->sizex=152; button->sizey=16;
	button->action=&buttonAllLayers;
	button->visible=0;
	
	butViewSprites = button = newButton();
	strcpy(button->label,"Sprites     Ctrl+E");
	button->x=96; button->y=64;
	button->sizex=152; button->sizey=16;
	button->action=&buttonViewSprites;
	button->visible=0;
	
	butGrid = button = newButton();
	strcpy(button->label,"Grid        Ctrl+G");
	button->x=96; button->y=80;
	button->sizex=152; button->sizey=16;
	button->action=&buttonGrid;
	button->visible=0;
	
	butFullscreen = button = newButton();
	strcpy(button->label,"Fullscreen  Ctrl+F");
	button->x=96; button->y=96;
	button->sizex=152; button->sizey=16;
	button->action=&buttonFullscreen;
	button->visible=0;
	
	// map menu
	butAttributes = button = newButton();
	strcpy(button->label,"Attributes ...  Ctrl+M");
	button->x=136; button->y=16;
	button->sizex=232; button->sizey=16;
	button->action=&buttonAttributes;
	button->visible=0;
	
	butClearMap = button = newButton();
	strcpy(button->label,"Clear Map       Ctrl+Shift+N");
	button->x=136; button->y=32;
	button->sizex=232; button->sizey=16;
	button->action=&buttonClearMap;
	button->visible=0;
	
	// help menu
	butAbout = button = newButton();
	strcpy(button->label,"About  F1");
	button->x=168; button->y=16;
	button->sizex=80; button->sizey=16;
	button->action=&buttonAbout;
	button->visible=0;
	
	// main loop
	fprintf(stderr, "running main loop.\n");
	while(mainloop) {
		// game logic
		handleEvents();
		
		// move buttons
		if( !fullscreen ) {
			butX->visible = 0;
			but_->visible = 0;
		} else {
			butX->visible = 1;
			but_->visible = 1;
			butX->x = xres-16;
			but_->x = xres-32;
		}
		butTilePalette->x = xres-112;
		butSprite->x = xres-112;
		butPoint->x = xres-96;
		butBrush->x = xres-96;
		butSelect->x = xres-96;
		butFill->x = xres-96;
		
		if( !spritepalette && !tilepalette ) {
			allowediting=1;
			if( (omousex>=xres-128 && toolbox) || omousey<16 || (omousey>=yres-16 && statusbar) || subwindow || menuVisible )
				allowediting=0;
			if( menuVisible == 1 ) {
				if((omousex>16+butNew->sizex||omousey>96||(omousey<16&&omousex>192))&&mousestatus[SDL_BUTTON_LEFT]) {
					menuVisible=0;
					menuDisappear=1;
				}
			} else if( menuVisible == 2 ) {
				if((omousex>56+butCut->sizex||omousex<40||omousey>96||(omousey<16&&omousex>192))&&mousestatus[SDL_BUTTON_LEFT]) {
					menuVisible=0;
					menuDisappear=1;
				}
			} else if( menuVisible == 3 ) {
				if((omousex>96+butToolbox->sizex||omousex<80||omousey>112||(omousey<16&&omousex>192))&&mousestatus[SDL_BUTTON_LEFT]) {
					menuVisible=0;
					menuDisappear=1;
				}
			} else if( menuVisible == 4 ) {
				if((omousex>136+butClearMap->sizex||omousex<120||omousey>48||(omousey<16&&omousex>192))&&mousestatus[SDL_BUTTON_LEFT]) {
					menuVisible=0;
					menuDisappear=1;
				}
			} else if( menuVisible == 5 ) {
				if((omousex>168+butAbout->sizex||omousex<152||omousey>32||(omousey<16&&omousex>192))&&mousestatus[SDL_BUTTON_LEFT]) {
					menuVisible=0;
					menuDisappear=1;
				}
			}
			if( !mousestatus[SDL_BUTTON_LEFT] )
				menuDisappear=0;
			
			if( allowediting && !menuDisappear ) {
				// MAIN LEVEL EDITING
				drawx = (mousex+camx)>>4;
				drawy = (mousey+camy)>>4;
				
				// set the cursor
				switch( selectedTool ) {
					case 0:	SDL_SetCursor(cursorPencil);
							break;
					case 1:	SDL_SetCursor(cursorBrush);
							break;
					case 2:	SDL_SetCursor(cursorSelect);
							break;
					case 3:	SDL_SetCursor(cursorFill);
							break;
					default:	SDL_SetCursor(cursorArrow);
							break;
				}
				
				// move entities
				if( entity_l.first != NULL && viewsprites && allowediting ) {
					for( node=entity_l.first; node!=NULL; node=nextnode ) {
						nextnode = node->next;
						entity = (entity_t *)node->element;
						if( entity == selectedEntity ) {
							if( mousestatus[SDL_BUTTON_LEFT] ) {
								mousestatus[SDL_BUTTON_LEFT] = 0;
								selectedEntity = NULL;
								break;
							}
							else if( mousestatus[SDL_BUTTON_RIGHT] ) {
								selectedEntity=newEntity(entity->sprite,0);
								selectedEntity->x=entity->x;
								selectedEntity->y=entity->y;
								mousestatus[SDL_BUTTON_RIGHT]=0;
								break;
							}
							entity->x = (long)(drawx<<4)+(entity->focalx);
							entity->y = (long)(drawy<<4)+(entity->focaly);
						}
						else {
							if( entity->sprite > 0 && entity->sprite < numsprites ) {
								if( sprites[entity->sprite] != NULL ) {
									if( omousex+camx >= entity->x-entity->focalx && omousey+camy >= entity->y-entity->focaly && omousex+camx < entity->x+sprites[entity->sprite]->w-entity->focalx && omousey+camy < entity->y+sprites[entity->sprite]->h-entity->focaly ) {
										if( mousestatus[SDL_BUTTON_LEFT] ) {
											// select sprite
											selectedEntity=entity;
											mousestatus[SDL_BUTTON_LEFT]=0;
										}
										else if( mousestatus[SDL_BUTTON_RIGHT] ) {
											// duplicate sprite
											selectedEntity=newEntity(entity->sprite,0);
											selectedEntity->x=entity->x;
											selectedEntity->y=entity->y;
											mousestatus[SDL_BUTTON_RIGHT]=0;
										}
									}
								}
								else {
									if( (omousex+camx)>>4 == (entity->x-entity->focalx)>>4 && (omousey+camy)>>4 == (entity->y-entity->focaly)>>4 ) {
										if( mousestatus[SDL_BUTTON_LEFT] ) {
											// select sprite
											selectedEntity=entity;
											mousestatus[SDL_BUTTON_LEFT]=0;
										}
										else if( mousestatus[SDL_BUTTON_RIGHT] ) {
											// duplicate sprite
											selectedEntity=newEntity(entity->sprite,0);
											selectedEntity->x=entity->x;
											selectedEntity->y=entity->y;
											mousestatus[SDL_BUTTON_RIGHT]=0;
										}
									}
								}
							}
							else {
								if( (omousex+camx)>>4 == (entity->x-entity->focalx)>>4 && (omousey+camy)>>4 == (entity->y-entity->focaly)>>4 ) {
									if( mousestatus[SDL_BUTTON_LEFT] ) {
										// select sprite
										selectedEntity=entity;
										mousestatus[SDL_BUTTON_LEFT]=0;
									}
									else if( mousestatus[SDL_BUTTON_RIGHT] ) {
										// duplicate sprite
										selectedEntity=newEntity(entity->sprite,0);
										selectedEntity->x=entity->x;
										selectedEntity->y=entity->y;
										mousestatus[SDL_BUTTON_RIGHT]=0;
									}
								}
							}
						}
					}
				}
				
				// modify world
				if( mousestatus[SDL_BUTTON_LEFT] && selectedEntity == NULL ) {
					if( allowediting ) {
						if( selectedTool == 0 ) { // point draw
							if( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
								map.tiles[drawlayer+drawy*MAPLAYERS+drawx*MAPLAYERS*map.height] = selectedTile;
						}
						if( selectedTool == 1 ) { // brush tool
							for(x=drawx-1; x<=drawx+1;x++)
								for(y=drawy-1; y<=drawy+1;y++)
									if( (x != drawx-1 || y != drawy-1) && (x != drawx+1 || y != drawy-1) && (x != drawx-1 || y != drawy+1) && (x != drawx+1 || y != drawy+1) )
										if( x >= 0 && x < map.width && y >= 0 && y < map.height )
											map.tiles[drawlayer+y*MAPLAYERS+x*MAPLAYERS*map.height] = selectedTile;
						}
						if( selectedTool == 3 ) { // fill tool
							if( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
								editFill(drawx,drawy,drawlayer,selectedTile);
						}
					}
				}
				if( mousestatus[SDL_BUTTON_RIGHT] && selectedEntity == NULL ) {
					if( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
						selectedTile = map.tiles[drawlayer+drawy*MAPLAYERS+drawx*MAPLAYERS*map.height];
				}
			}
			else
				SDL_SetCursor(cursorArrow);
			
			// main drawing
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0)); // wipe screen
			
			if( alllayers )
				drawBackground(camx,camy);
			else
				drawLayer(camx,camy,drawlayer);
			if( viewsprites )
				drawEntities(camx,camy);
			if( alllayers )
				drawForeground(camx,camy);
			if( showgrid )
				drawGrid(camx,camy);
			
			// primary interface
			drawWindow(0,0,xres,16);
			if( toolbox ) {
				if( statusbar )
					drawWindow(xres-128,16,xres,yres-16);
				else
					drawWindow(xres-128,16,xres,yres);
				drawMinimap(camx,camy);
				
				// draw selected tile / hovering tile
				pos.x = xres-32; pos.y = 328;
				pos.w = 0; pos.h = 0;
				SDL_BlitSurface(tiles[selectedTile], NULL, screen, &pos);
				pos.x = xres-32; pos.y = 352;
				pos.w = 0; pos.h = 0;
				if( drawx >= 0 && drawx < map.width && drawy >= 0 && drawy < map.height )
					SDL_BlitSurface(tiles[map.tiles[drawlayer+drawy*MAPLAYERS+drawx*MAPLAYERS*map.height]], NULL, screen, &pos);
				else
					SDL_BlitSurface(sprites[0], NULL, screen, &pos);
				printText(font8_bmp,xres-120,332,"Selected:");
				printText(font8_bmp,xres-120,356,"   Above:");
				
				// print selected tool
				switch( selectedTool ) {
					case 0:	printText(font8_bmp,xres-84,276,"POINT"); break;
					case 1:	printText(font8_bmp,xres-84,276,"BRUSH"); break;
					case 2:	printText(font8_bmp,xres-88,276,"SELECT");
							printText(font8_bmp,xres-128,288,"(NON-FUNCTIONAL)"); break;
					case 3:	printText(font8_bmp,xres-80,276,"FILL"); break;
				}
			}
			if( statusbar ) {
				drawWindow(0,yres-16,xres,yres);
				printTextFormatted(font8_bmp,4,yres-12,"X: %4d Y: %4d Z: %d %s",drawx,drawy,drawlayer+1,layerstatus);
				if( messagetime )
					printTextFormatted(font8_bmp,xres-384,yres-12,"%s",message);
			}
			
			// handle main menus
			if( menuVisible == 1 ) {
				drawWindow(0,16,16,96);
				butNew->visible = 1;
				butOpen->visible = 1;
				butSave->visible = 1;
				butSaveAs->visible = 1;
				butExit->visible = 1;
			}
			else {
				butNew->visible = 0;
				butOpen->visible = 0;
				butSave->visible = 0;
				butSaveAs->visible = 0;
				butExit->visible = 0;
			}
			if( menuVisible == 2 ) {
				drawWindow(40,16,56,96);
				butPaste->visible = 1;
				butCut->visible = 1;
				butCopy->visible = 1;
				butDelete->visible = 1;
				butSelectAll->visible = 1;
			}
			else {
				butPaste->visible = 0;
				butCut->visible = 0;
				butCopy->visible = 0;
				butDelete->visible = 0;
				butSelectAll->visible = 0;
			}
			if( menuVisible == 3 ) {
				drawWindow(80,16,96,112);
				butToolbox->visible = 1;
				butStatusBar->visible = 1;
				butAllLayers->visible = 1;
				butViewSprites->visible = 1;
				butGrid->visible = 1;
				butFullscreen->visible = 1;
				if( statusbar )
					printText(font8_bmp,84,20,"]");
				if( toolbox )
					printText(font8_bmp,84,36,"]");
				if( alllayers )
					printText(font8_bmp,84,52,"]");
				if( viewsprites )
					printText(font8_bmp,84,68,"]");
				if( showgrid )
					printText(font8_bmp,84,84,"]");
				if( fullscreen )
					printText(font8_bmp,84,100,"]");
			}
			else {
				butToolbox->visible = 0;
				butStatusBar->visible = 0;
				butAllLayers->visible = 0;
				butViewSprites->visible = 0;
				butGrid->visible = 0;
				butFullscreen->visible = 0;
			}
			if( menuVisible == 4 ) {
				drawWindow(120,16,136,48);
				butAttributes->visible = 1;
				butClearMap->visible = 1;
			}
			else {
				butAttributes->visible = 0;
				butClearMap->visible = 0;
			}
			if( menuVisible == 5 ) {
				drawWindow(152,16,168,32);
				butAbout->visible = 1;
			}
			else {
				butAbout->visible = 0;
			}
			
			// subwindows
			if( subwindow ) {
				drawWindow(subx1,suby1,subx2,suby2);
				if( subtext != NULL )
					printTextFormatted(font8_bmp,subx1+8,suby1+8,subtext);
					
				// open and save windows
				if( (openwindow || savewindow) && d_names != NULL ) {
					drawDepressed(subx1+4,suby1+20,subx2-20,suby2-52);
					drawDepressed(subx2-20,suby1+20,subx2-4,suby2-52);
					slidersize=min(((suby2-52)-(suby1+20)),((suby2-52)-(suby1+20)) / ((double)d_names_length/20));
					slidery=min(max(suby1+20,slidery),suby2-52-slidersize);
					drawWindow(subx2-20,slidery,subx2-4,slidery+slidersize);
					
					// directory list offset from slider
					y2 = ((double)(slidery-suby1-20) / ((suby2-52)-(suby1+20)))*d_names_length;
					if( mousestatus[SDL_BUTTON_LEFT] && omousex >= subx2-20 && omousex < subx2-4 && omousey >= suby1+20 && omousey < suby2-52 ) {
						slidery += mouseyrel;
						mclick=1;
						selectedFile = min(max(y2,selectedFile),min(d_names_length-1,y2+19));
						strcpy(filename,d_names[selectedFile]);
					}
					
					// select a file
					if( mousestatus[SDL_BUTTON_LEFT] )
						if( omousex >= subx1+8 && omousex < subx2-24 && omousey >= suby1+24 && omousey < suby2-56 ) {
							selectedFile = y2+((omousey-suby1-24)>>3);
							selectedFile = min(max(y2,selectedFile),min(d_names_length-1,y2+19));
							strcpy(filename,d_names[selectedFile]);
						}
					pos.x = subx1+8; pos.y = suby1+24+(selectedFile-y2)*8;
					pos.w = subx2-subx1-32; pos.h = 8;
					SDL_FillRect(screen,&pos,SDL_MapRGB(screen->format,64,64,64));
					
					// print all the files within the directory
					x=subx1+8; y=suby1+24;
					c=min(d_names_length,20+y2);
					for(z=y2;z<c;z++) {
						printTextFormatted(font8_bmp,x,y,"%s",d_names[z]);
						y+=8;
					}
					
					// text box to enter file
					drawDepressed(subx1+4,suby2-48,subx2-68,suby2-32);
					printText(font8_bmp,subx1+8,suby2-44,filename);
					
					// enter filename
					x=29;
					for( c=0; c<29; c++ ) {
						if( filename[c]==0 ) {
							x=c;
							break;
						}
					}
					if( keypressed >= 32 && keypressed < 127 && x != 29 ) // standard character
						if( keypressed != 34 && keypressed != 42 && keypressed != 92 && keypressed != 58 && keypressed != 60 && keypressed != 62 && keypressed != 63 && keypressed != 124 ) {
							strcat(filename,&keypressed);
							cursorflash=0;
						}
					if( keypressed == 8 && x != 0 ) { // backspace
						filename[x-1] = 0;
						cursorflash=0;
					}
					if( cursorflash < 20 )
						printText(font8_bmp,subx1+8+x*8,suby2-44,"_");
				}
				
				// new map and attributes windows
				if( newwindow ) {
					printText(font8_bmp,subx1+8,suby1+28,"Map name:");
					drawDepressed(subx1+4,suby1+40,subx2-4,suby1+56);
					printText(font8_bmp,subx1+8,suby1+44,nametext);
					printText(font8_bmp,subx1+8,suby1+64,"Author name:");
					drawDepressed(subx1+4,suby1+76,subx2-4,suby1+92);
					printText(font8_bmp,subx1+8,suby1+80,authortext);
					printText(font8_bmp,subx1+8,suby2-44,"Map width:");
					drawDepressed(subx1+104,suby2-48,subx1+168,suby2-32);
					printText(font8_bmp,subx1+108,suby2-44,widthtext);
					printText(font8_bmp,subx1+8,suby2-20,"Map height:");
					drawDepressed(subx1+104,suby2-24,subx1+168,suby2-8);
					printText(font8_bmp,subx1+108,suby2-20,heighttext);
					
					if( keystatus[SDLK_TAB] ) {
						keystatus[SDLK_TAB] = 0;
						cursorflash=0;
						editproperty++;
						if( editproperty == 4 )
							editproperty = 0;
					}
					
					// select a textbox
					if( mousestatus[SDL_BUTTON_LEFT] ) {
						if( omousex >= subx1+4 && omousey >= suby1+40 && omousex < subx2-4 && omousey < suby1+56 ) {
							editproperty=0;
							cursorflash=0;
						}
						if( omousex >= subx1+4 && omousey >= suby1+76 && omousex < subx2-4 && omousey < suby1+92 ) {
							editproperty=1;
							cursorflash=0;
						}
						if( omousex >= subx1+104 && omousey >= suby2-48 && omousex < subx1+168 && omousey < suby2-32 ) {
							editproperty=2;
							cursorflash=0;
						}
						if( omousex >= subx1+104 && omousey >= suby2-24 && omousex < subx1+168 && omousey < suby2-8 ) {
							editproperty=3;
							cursorflash=0;
						}
					}
					
					if( editproperty == 0 ) { // edit map name
						x=32;
						for( c=0; c<32; c++ ) {
							if( nametext[c]==0 ) {
								x=c;
								break;
							}
						}
						if( keypressed >= 32 && keypressed < 127 && x != 32 ) // standard character
							if( keypressed != 34 && keypressed != 42 && keypressed != 92 && keypressed != 58 && keypressed != 60 && keypressed != 62 && keypressed != 63 && keypressed != 124 ) {
								strcat(nametext,&keypressed);
								cursorflash=0;
							}
						if( keypressed == 8 && x != 0 ) { // backspace
							nametext[x-1] = 0;
							cursorflash=0;
						}
						if( cursorflash < 20 )
							printText(font8_bmp,subx1+8+x*8,suby1+44,"_");
					}
					if( editproperty == 1 ) { // edit author name
						x=32;
						for( c=0; c<32; c++ ) {
							if( authortext[c]==0 ) {
								x=c;
								break;
							}
						}
						if( keypressed >= 32 && keypressed < 127 && x != 32 ) // standard character
							if( keypressed != 34 && keypressed != 42 && keypressed != 92 && keypressed != 58 && keypressed != 60 && keypressed != 62 && keypressed != 63 && keypressed != 124 ) {
								strcat(authortext,&keypressed);
								cursorflash=0;
							}
						if( keypressed == 8 && x != 0 ) { // backspace
							authortext[x-1] = 0;
							cursorflash=0;
						}
						if( cursorflash < 20 )
							printText(font8_bmp,subx1+8+x*8,suby1+80,"_");
					}
					if( editproperty == 2 ) { // edit map width
						x=4;
						for( c=0; c<4; c++ ) {
							if( widthtext[c]==0 ) {
								x=c;
								break;
							}
						}
						if( keypressed >= 48 && keypressed < 58 && x != 4 ) { // number character
							strcat(widthtext,&keypressed);
							cursorflash=0;
						}
						if( keypressed == 8 && x != 0 ) { // backspace
							widthtext[x-1] = 0;
							cursorflash=0;
						}
						if( cursorflash < 20 )
							printText(font8_bmp,subx1+108+x*8,suby2-44,"_");
					}
					if( editproperty == 3 ) { // edit map height
						x=4;
						for( c=0; c<4; c++ ) {
							if( heighttext[c]==0 ) {
								x=c;
								break;
							}
						}
						if( keypressed >= 48 && keypressed < 58 && x != 4 ) { // number character
							strcat(heighttext,&keypressed);
							cursorflash=0;
						}
						if( keypressed == 8 && x != 0 ) { // backspace
							heighttext[x-1] = 0;
							cursorflash=0;
						}
						if( cursorflash < 20 )
							printText(font8_bmp,subx1+108+x*8,suby2-20,"_");
					}
				}
			} else {
				// handle hotkeys
				if( keystatus[SDLK_LCTRL]||keystatus[SDLK_RCTRL] ) {
					if( keystatus[SDLK_n] && !keystatus[SDLK_LSHIFT] && !keystatus[SDLK_RSHIFT] ) {
						keystatus[SDLK_n]=0;
						buttonNew(NULL);
					}
					if( keystatus[SDLK_s] ) {
						keystatus[SDLK_s]=0;
						buttonSave(NULL);
					}
					if( keystatus[SDLK_o] ) {
						keystatus[SDLK_o]=0;
						buttonOpen(NULL);
					}
					if( keystatus[SDLK_x] ) {
						keystatus[SDLK_x]=0;
						buttonCut(NULL);
					}
					if( keystatus[SDLK_c] ) {
						keystatus[SDLK_c]=0;
						buttonCopy(NULL);
					}
					if( keystatus[SDLK_v] ) {
						keystatus[SDLK_v]=0;
						buttonPaste(NULL);
					}
					if( keystatus[SDLK_a] ) {
						keystatus[SDLK_a]=0;
						buttonSelectAll(NULL);
					}
					if( keystatus[SDLK_g] ) {
						keystatus[SDLK_g]=0;
						buttonGrid(NULL);
					}
					if( keystatus[SDLK_t] ) {
						keystatus[SDLK_t]=0;
						buttonToolbox(NULL);
					}
					if( keystatus[SDLK_e] ) {
						keystatus[SDLK_e]=0;
						buttonViewSprites(NULL);
					}
					if( keystatus[SDLK_l] ) {
						keystatus[SDLK_l]=0;
						buttonAllLayers(NULL);
					}
					if( keystatus[SDLK_i] ) {
						keystatus[SDLK_i]=0;
						buttonStatusBar(NULL);
					}
					if( keystatus[SDLK_f] ) {
						keystatus[SDLK_f]=0;
						buttonFullscreen(NULL);
					}
					if( keystatus[SDLK_m] ) {
						keystatus[SDLK_m]=0;
						buttonAttributes(NULL);
					}
					if( keystatus[SDLK_LSHIFT]||keystatus[SDLK_RSHIFT] )
						if( keystatus[SDLK_n] ) {
							keystatus[SDLK_n]=0;
							buttonClearMap(NULL);
						}
				} else {
					if( keystatus[SDLK_s] ) {
						keystatus[SDLK_s]=0;
						spritepalette=1;
					}
					if( keystatus[SDLK_t] ) {
						keystatus[SDLK_t]=0;
						tilepalette=1;
					}
				}
				if( keystatus[SDLK_LALT]||keystatus[SDLK_RALT] ) {
					if( keystatus[SDLK_f] ) {
						keystatus[SDLK_f]=0;
						menuVisible=1;
					}
					if( keystatus[SDLK_e] ) {
						keystatus[SDLK_e]=0;
						menuVisible=2;
					}
					if( keystatus[SDLK_v] ) {
						keystatus[SDLK_v]=0;
						menuVisible=3;
					}
					if( keystatus[SDLK_m] ) {
						keystatus[SDLK_m]=0;
						menuVisible=4;
					}
					if( keystatus[SDLK_h] ) {
						keystatus[SDLK_h]=0;
						menuVisible=5;
					}
					if( keystatus[SDLK_F4] ) {
						keystatus[SDLK_F4]=0;
						buttonExit(NULL);
					}
				}
				if( keystatus[SDLK_DELETE] ) {
					keystatus[SDLK_DELETE]=0;
					buttonDelete(NULL);
				}
				if( keystatus[SDLK_F1] ) {
					keystatus[SDLK_F1]=0;
					buttonAbout(NULL);
				}
				if( keystatus[SDLK_1] ) {
					keystatus[SDLK_1]=0;
					selectedTool=0;
				}
				if( keystatus[SDLK_2] ) {
					keystatus[SDLK_2]=0;
					selectedTool=1;
				}
				if( keystatus[SDLK_3] ) {
					keystatus[SDLK_3]=0;
					selectedTool=2;
				}
				if( keystatus[SDLK_4] ) {
					keystatus[SDLK_4]=0;
					selectedTool=3;
				}
			}
			// process and draw buttons
			handleButtons();
		}
		if( spritepalette ) {
			x=0; y=0; z=0;
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0)); // wipe screen
			for( c=0; c<xres*yres; c++ )
				palette[c] = -1;
			for( c=0; c<numsprites; c++ ) {
				pos.w=0; pos.h=0;
				pos.x=x; pos.y=y;
				if( sprites[c] != NULL ) {
					SDL_BlitSurface(sprites[c], NULL, screen, &pos);
					for( x2=x; x2<x+sprites[c]->w; x2++ )
						for( y2=y; y2<y+sprites[c]->h; y2++ ) {
							if( x2<xres && y2<yres )
								palette[y2+x2*yres]=c;
						}
					x += sprites[c]->w;
					z = max(z,sprites[c]->h);
					if( c<numsprites-1 ) {
						if( sprites[c+1] != NULL ) {
							if( x+sprites[c+1]->w > xres ) {
								x=0;
								y+=z;
							}
						}
						else {
							if( x+sprites[0]->w > xres ) {
								x=0;
								y+=z;
							}
						}
					}
				}
				else {
					SDL_BlitSurface(sprites[0], NULL, screen, &pos);
					x += sprites[0]->w;
					z = max(z,sprites[0]->h);
					if( c<numsprites-1 ) {
						if( sprites[c+1] != NULL ) {
							if( x+sprites[c+1]->w > xres ) {
								x=0;
								y+=z;
							}
						}
						else {
							if( x+sprites[0]->w > xres ) {
								x=0;
								y+=z;
							}
						}
					}
				}
			}
			if(mousestatus[SDL_BUTTON_LEFT])
				mclick=1;
			if(!mousestatus[SDL_BUTTON_LEFT]&&mclick) {
				// create a new object
				if(palette[mousey+mousex*yres] >= 0) {
					entity=newEntity(palette[mousey+mousex*yres],0);
					selectedEntity=entity;
				}
				
				mclick=0;
				spritepalette=0;
			}
			if(keystatus[SDLK_ESCAPE]) {
				mclick=0;
				spritepalette=0;
			}
			switch( palette[mousey+mousex*yres] ) {
				case 1:	strcpy(action,"PLAYER"); break;
				case 53:	strcpy(action,"PURPLEGEM"); break;
				case 37:	strcpy(action,"REDGEM"); break;
				case 74:
				case 75:	strcpy(action,"TROLL"); break;
				default:	strcpy(action,"STATIC"); break;
			}
			if( palette[mousey+mousex*yres] >= 0 )
				printTextFormatted(font8_bmp,0,yres-8,"Sprite index:%5d Action: %s",palette[mousey+mousex*yres],action);
			else
				printText(font8_bmp,0,yres-8,"Click to cancel");
		}
		if( tilepalette ) {
			x=0; y=0;
			SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0)); // wipe screen
			for( c=0; c<xres*yres; c++ )
				palette[c] = -1;
			for( c=0; c<numtiles; c++ ) {
				pos.w=0; pos.h=0;
				pos.x=x; pos.y=y;
				if( tiles[c] != NULL ) {
					SDL_BlitSurface(tiles[c], NULL, screen, &pos);
					for( x2=x; x2<x+tiles[c]->w; x2++ )
						for( y2=y; y2<y+tiles[c]->h; y2++ ) {
							if( x2<xres && y2<yres )
								palette[y2+x2*yres]=c;
						}
					x += tiles[c]->w;
					if( c<numtiles-1 ) {
						if( tiles[c+1] != NULL ) {
							if( x+tiles[c+1]->w > xres ) {
								x=0;
								y+=16;
							}
						}
						else {
							if( x+sprites[0]->w > xres ) {
								x=0;
								y+=16;
							}
						}
					}
				}
				else {
					SDL_BlitSurface(sprites[0], NULL, screen, &pos);
					x += sprites[0]->w;
					if( c<numtiles-1 ) {
						if( tiles[c+1] != NULL ) {
							if( x+tiles[c+1]->w > xres ) {
								x=0;
								y+=16;
							}
						}
						else {
							if( x+sprites[0]->w > xres ) {
								x=0;
								y+=16;
							}
						}
					}
				}
			}
			if(mousestatus[SDL_BUTTON_LEFT])
				mclick=1;
			if(!mousestatus[SDL_BUTTON_LEFT]&&mclick) {
				// select the tile under the mouse
				if(palette[mousey+mousex*yres] >= 0)
					selectedTile=palette[mousey+mousex*yres];
				mclick=0;
				tilepalette=0;
			}
			if(keystatus[SDLK_ESCAPE]) {
				mclick=0;
				tilepalette=0;
			}
			if( palette[mousey+mousex*yres] >= 0 )
				printTextFormatted(font8_bmp,0,yres-8,"Tile index:%5d",palette[mousey+mousex*yres]);
			else
				printText(font8_bmp,0,yres-8,"Click to cancel");
		}
		
		SDL_Flip( screen );
	}
	
	// deinit
	fprintf(stderr, "freeing lists...\n");
	list_FreeAll(&entity_l);
	list_FreeAll(&button_l);
	fprintf(stderr, "removing engine timer...\n");
	SDL_RemoveTimer(timer);
	fprintf(stderr, "freeing engine resources...\n");
	SDL_SetCursor(cursorArrow);
	SDL_FreeCursor(cursorPencil);
	SDL_FreeCursor(cursorBrush);
	SDL_FreeCursor(cursorFill);
	SDL_FreeSurface(font8_bmp);
	SDL_FreeSurface(font16_bmp);
	fprintf(stderr, "freeing game data...\n");
	for( c=0; c<numsprites; c++ )
		SDL_FreeSurface(sprites[c]);
	free(sprites);
	for( c=0; c<numtiles; c++ )
		SDL_FreeSurface(tiles[c]);
	free(tiles);
	for( c=0; c<numsounds; c++ )
		Mix_FreeChunk(sounds[c]);
	free(sounds);
	free(map.tiles);
	free(palette);
	fprintf(stderr, "closing SDL and SDL_Mixer...\n");
	Mix_CloseAudio();
	SDL_Quit();
	fprintf(stderr, "success\n");
	return 0;
}