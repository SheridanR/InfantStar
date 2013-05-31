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
	int c;
	FILE *fp;
	char name[128];
	node_t *node;
	entity_t *entity;
	
	// initialize
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) == -1 ) {
		fprintf(stderr, "could not initialize SDL. aborting...\n");
		exit(1);
	}
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		fprintf(stderr, "unable to open audio! rate: %d format: %d channels: %d buffers: %d\n", audio_rate, audio_format, audio_channels, audio_buffers);
		exit(1);
	}
	screen = SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_FULLSCREEN );
	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption( "Infant Star", 0 );
	SDL_ShowCursor(SDL_DISABLE);
	entity_l.first=NULL; entity_l.last=NULL;
	button_l.first=NULL; button_l.last=NULL;
	
	// load resources
	font8_bmp = SDL_LoadBMP("images/8font.bmp");
	SDL_SetColorKey( font8_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font8_bmp->format, 255, 0, 255 ) );
	font16_bmp = SDL_LoadBMP("images/16font.bmp");
	SDL_SetColorKey( font16_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font16_bmp->format, 255, 0, 255 ) );
	sky_bmp = SDL_LoadBMP("images/sky.bmp");
	
	// load sprites
	fp = fopen("images/sprites.txt","r");
	for( numsprites=0; !feof(fp); numsprites++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	sprites = (SDL_Surface **) malloc(sizeof(SDL_Surface *)*numsprites);
	fp = fopen("images/sprites.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		sprites[c] = SDL_LoadBMP(name);
		if( sprites[c] != NULL )
			SDL_SetColorKey( sprites[c], SDL_SRCCOLORKEY, SDL_MapRGB( sprites[c]->format, 0, 0, 255 ) );
	}
	
	// load tiles
	fp = fopen("images/tiles.txt","r");
	for( numtiles=0; !feof(fp); numtiles++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	tiles = (SDL_Surface **) malloc(sizeof(SDL_Surface *)*numtiles);
	fp = fopen("images/tiles.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		tiles[c] = SDL_LoadBMP(name);
		if( tiles[c] != NULL )
			SDL_SetColorKey( tiles[c], SDL_SRCCOLORKEY, SDL_MapRGB( tiles[c]->format, 0, 0, 0 ) );
	}
	
	// load sound effects
	fp = fopen("sound/sounds.txt","r");
	for( numsounds=0; !feof(fp); numsounds++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	sounds = (Mix_Chunk **) malloc(sizeof(Mix_Chunk *)*numsounds);
	fp = fopen("sound/sounds.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		sounds[c] = Mix_LoadWAV(name);
	}
	fclose(fp);
	
	// initialize some vars to zero
	entity_l.first = NULL; entity_l.last = NULL;
	button_l.first = NULL; button_l.last = NULL;
	
	// instatiate a timer
	timer = SDL_AddTimer(50, timerCallback, NULL);
	
	// create a simple test map
	map.tiles = NULL;
	if( argc >= 1 && argv[1] != NULL )
		loadMap(argv[1]);
	else
		loadMap("testmap.imp");
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity = (entity_t *)node->element;
		if( entity->sprite == 1 ) {
			entity->behavior = &actPlayer;
			camx = entity->x-xres/2;
			camy = entity->y-yres/2;
			camx = min(max(0,camx),(map.width-20)<<4);
			camy = min(max(0,camy),(map.height-13)<<4);
		}
	}
	
	// main loop
	while(mainloop) {
		// game logic
		handleEvents();
		
		// drawing
		drawSky(sky_bmp);
		drawBackground(camx,camy);
		drawEntities(camx,camy);
		drawForeground(camx,camy);
		SDL_Flip( screen );
	}
	
	// deinit
	list_FreeAll(&entity_l);
	SDL_RemoveTimer(timer);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(font8_bmp);
	SDL_FreeSurface(font16_bmp);
	SDL_FreeSurface(sky_bmp);
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
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}