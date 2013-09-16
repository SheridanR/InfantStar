/*-------------------------------------------------------------------------------

	INFANT STAR
	File: game.c
	Desc: contains main game code for Infant Star project

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"
#include "game.h"

// definitions
SDL_TimerID timer=NULL;
SDL_Surface *screen=NULL;
SDL_Event event;
int fullscreen = 1;
int xres = 320;
int yres = 200;
int mainloop = 1;
unsigned long ticks;
int keystatus[323];
char keypressed=0;
int mousestatus[6];
int mousex=0, mousey=0;
int mousexrel=0, mouseyrel=0;
long camx=0, camy=0;
long newcamx, newcamy;
entity_t *selectedEntity = NULL;

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

// game resources
SDL_Surface *sky_bmp;

// function prototypes
Uint32 timerCallback(Uint32 interval, void *param);
void handleEvents(void);
void gameLogic(void);

/*-------------------------------------------------------------------------------

	gameLogic

	Updates the gamestate; moves actors, primarily

-------------------------------------------------------------------------------*/

void gameLogic(void) {
	node_t *node;
	node_t *nextnode;
	entity_t *entity;
	
	// move entities
	for( node=entity_l.first; node!=NULL; node=nextnode ) {
		nextnode = node->next;
		entity = (entity_t *)node->element;
		if( entity->behavior != NULL )
			(*entity->behavior)(entity); // execute the entity's behavior function
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
				mainloop = 0;
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				keystatus[event.key.keysym.sym] = 1; // set this key's index to 1
				if( (event.key.keysym.unicode & 0xFF80) == 0 )
					keypressed=event.key.keysym.unicode&0x7F; // record the ascii character that was pressed
				break;
			case SDL_KEYUP: // if a key is unpressed...
				keystatus[event.key.keysym.sym] = 0; // set this key's index to 0
				break;
			case SDL_MOUSEBUTTONDOWN: // if a mouse button is pressed...
				mousestatus[event.button.button] = 1; // set this mouse button to 1
				break;
			case SDL_MOUSEBUTTONUP: // if a mouse button is released...
				mousestatus[event.button.button] = 0; // set this mouse button to 0
				break;
			case SDL_MOUSEMOTION: // if the mouse is moved...
				mousex = event.motion.x;
				mousey = event.motion.y;
				mousexrel = event.motion.xrel;
				mouseyrel = event.motion.yrel;
				break;
			case SDL_USEREVENT: // if the game timer elapses
				gameLogic();
				break;
			case SDL_VIDEORESIZE: // if the window is resized
				if(fullscreen)
					break;
				xres = max(event.resize.w,100);
				yres = max(event.resize.h,75);
				if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_RESIZABLE )) == NULL) {
					fprintf(stderr, "failed to set video mode.\n");
					mainloop=0;
					break;
				}
		}
	}
	
	if( keystatus[SDLK_ESCAPE] )
		mainloop=0;
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

	main
	
	Initializes game resources, harbors main game loop, and cleans up
	afterwords

-------------------------------------------------------------------------------*/

int main(int argc, char **argv ) {
	int x, y, z, c;
	node_t *node;
	entity_t *entity;
	Mix_Music *music;
	
	// initialize
	if( (x=initApp("Infant Star",fullscreen)) )
		exit(x);
	
	// instatiate a timer
	timer = SDL_AddTimer(50, timerCallback, NULL);
	srand(time(NULL));
	
	// load sky image
	sky_bmp = SDL_LoadBMP("images/sky.bmp");
	
	// load music
	music = Mix_LoadMUS("music/music.ogg");
	Mix_VolumeMusic(64);
	
	// load a map
	map.tiles = NULL;
	if( argc >= 1 && argv[1] != NULL )
		x=loadMap(argv[1]);
	else
		x=loadMap("testmap.imp");
	
	if(!x) {
		// create tile animator entities where necessary
		for(x=0;x<map.width;x++)
			for(y=0;y<map.height;y++)
				for(z=0;z<MAPLAYERS;z++) {
					if( animatedtiles[map.tiles[z+y*MAPLAYERS+x*MAPLAYERS*map.height]] ) {
						entity = newEntity(-1,0);
						entity->x = x;
						entity->y = y;
						entity->skill[0] = z; // remember tile layer
						entity->skill[1] = animatedtiles[map.tiles[z+y*MAPLAYERS+x*MAPLAYERS*map.height]];
						map.tiles[z+y*MAPLAYERS+x*MAPLAYERS*map.height]-=entity->skill[1]-1;
					}
				}
		
		// assign entity behaviors
		for( node=entity_l.first; node!=NULL; node=node->next ) {
			entity = (entity_t *)node->element;
			switch( entity->sprite ) {
				case -1:	entity->behavior = &actAnimator; break;
				case 1:	entity->behavior = &actPlayer;
						camx = entity->x-xres/2;
						camy = entity->y-yres/2;
						camx = min(max(0,camx),(map.width-20)<<4);
						camy = min(max(0,camy),(map.height-13)<<4);
						break;
				case 53:	entity->behavior = &actPGem; break;
				case 57:	entity->behavior = &actRGem; break;
				case 75:	entity->skill[0]=1; // turn left
				case 74:	entity->behavior = &actTroll;
						entity->HEALTH=4;
						break;
				default:	entity->behavior = NULL;
			}
		}
		
		// main loop
		fprintf(stderr, "running main loop.\n");
		while(mainloop) {
			// handle music
			if(!Mix_PlayingMusic())
				Mix_PlayMusic(music, 1);
				
			// game logic
			handleEvents();
			
			// drawing
			drawSky(sky_bmp);
			drawBackground(camx,camy);
			drawEntities(camx,camy);
			drawForeground(camx,camy);
			SDL_Flip( screen );
		}
	}
	
	// deinit
	fprintf(stderr, "freeing lists...\n");
	list_FreeAll(&entity_l);
	fprintf(stderr, "removing engine timer...\n");
	SDL_RemoveTimer(timer);
	fprintf(stderr, "freeing engine resources...\n");
	SDL_FreeSurface(font8_bmp);
	SDL_FreeSurface(font16_bmp);
	SDL_FreeSurface(sky_bmp);
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
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	fprintf(stderr, "closing SDL and SDL_Mixer...\n");
	Mix_CloseAudio();
	SDL_Quit();
	fprintf(stderr, "success\n");
	return 0;
}