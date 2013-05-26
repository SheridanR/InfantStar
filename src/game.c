/*-------------------------------------------------------------------------------

	INFANT STAR
	File: game.c
	Desc: contains main game code for Infant Star project

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

// definitions
SDL_TimerID timer;
SDL_Surface *screen;
SDL_Event event;
int xres = 320;
int yres = 200;
int mainloop = 1;
unsigned long ticks;
int keystatus[323];
int mousestatus[5];
int mousex, mousey;
long camx=0, camy=0;
long newcamx, newcamy;

// various definitions
map_t map;
SDL_Surface *font8_bmp;
SDL_Surface *font16_bmp;
SDL_Surface **sprites;
SDL_Surface **tiles;
Mix_Chunk **sounds;
list_t entity_l;
int numsprites, numtiles, numsounds;

// audio definitions
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16;
int audio_channels = 2;
int audio_buffers = 512;

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
	mousex = 0;
	mousey = 0;
	while( SDL_PollEvent(&event) ) { // poll SDL events
		// Global events
		switch( event.type ) {
			case SDL_QUIT: // if SDL receives the shutdown signal
				mainloop = 0;
				break;
			case SDL_KEYDOWN: // if a key is pressed...
				keystatus[event.key.keysym.sym] = 1; // set this key's index to 1
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
				mousex = event.motion.xrel;
				mousey = event.motion.yrel;
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
	int x, y, z;
	FILE *fp;
	char name[128];
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
	SDL_WM_SetCaption( "Infant Star", 0 );
	SDL_ShowCursor(SDL_DISABLE);
	
	// load resources
	font8_bmp = SDL_LoadBMP("images/8font.bmp");
	SDL_SetColorKey( font8_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font8_bmp->format, 255, 0, 255 ) );
	font16_bmp = SDL_LoadBMP("images/16font.bmp");
	SDL_SetColorKey( font16_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font16_bmp->format, 255, 0, 255 ) );
	
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
	
	// instatiate a timer
	timer = SDL_AddTimer(50, timerCallback, NULL);
	
	// create a player object
	entity=newEntity();
	entity->behavior=&actPlayer;
	entity->sprite=4;
	entity->x=5<<4; entity->y=8<<4;
	entity->focalx=8; entity->focaly=32;
	
	// create a simple test map
	map.width = 40;
	map.height = 13;
	map.tiles = (unsigned char *) malloc(sizeof(unsigned char)*map.width*map.height*MAPLAYERS);
	for( z=0; z<MAPLAYERS; z++ ) {
		for( y=0; y<map.height; y++ ) {
			for( x=0; x<map.width; x++ ) {
				if( (x==0 || x==map.width-1 || y==0 || y==map.height-1) && z==OBSTACLELAYER )
					map.tiles[OBSTACLELAYER + y*MAPLAYERS + x*MAPLAYERS*map.height] = 2;
				else if( z==0 )
					map.tiles[0 + y*MAPLAYERS + x*MAPLAYERS*map.height] = 22;
				else
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
			}
		}
	}
	map.tiles[OBSTACLELAYER + 9*MAPLAYERS + 3*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 9*MAPLAYERS + 4*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 4*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 5*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 6*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 7*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 9*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 10*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 11*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 8*MAPLAYERS + 12*MAPLAYERS*map.height] = 2;
	
	map.tiles[OBSTACLELAYER + 7*MAPLAYERS + 11*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 6*MAPLAYERS + 11*MAPLAYERS*map.height] = 2;
	map.tiles[OBSTACLELAYER + 5*MAPLAYERS + 11*MAPLAYERS*map.height] = 2;
	
	// main loop
	while(mainloop) {
		// game logic
		handleEvents();
		
		// drawing
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,0,0,0)); // wipe screen
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