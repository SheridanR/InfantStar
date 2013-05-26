/*-------------------------------------------------------------------------------

	INFANT STAR
	File: infantstar.c
	Desc: contains main code for Infant Star project

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

// definitions
SDL_Surface *screen;
SDL_Event event;
int xres = 1280;
int yres = 720;
int mainloop = 1;
int keystatus[323];
int mousestatus[5];
int mousex, mousey;

// various definitions
SDL_Surface *font8_bmp;
SDL_Surface *font16_bmp;
SDL_Surface **sprites;
SDL_Surface **tiles;
char *tile_flags;
Mix_Chunk **sounds;
list_t entity_l;

// audio definitions
int audio_rate = 22050;
Uint16 audio_format = AUDIO_S16;
int audio_channels = 2;
int audio_buffers = 512;

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
		}
	}
	
	if( keystatus[SDLK_ESCAPE] )
		mainloop=0;
}

int main(int argc, char **argv ) {
	int c;
	char name[128];
	
	// initialize
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) == -1 ) {
		fprintf("Could not initialize SDL. Aborting...\n\n");
		exit(1);
	}
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		fprintf("unable to open audio! rate: format: channels: buffers: \n");
		exit(1);
	}
	screen = SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_FULLSCREEN );
	SDL_WM_SetCaption( "Infant Star", 0 );
	
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
	}
	
	// load tiles
	fp = fopen("images/tiles.txt","r");
	for( numtiles=0; !feof(fp); numtiles++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	tiles = (SDL_Surface **) malloc(sizeof(SDL_Surface *)*numtiles);
	tiles_flags = (char *) malloc(sizeof(char)*numtiles);
	fp = fopen("images/tiles.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != ' ' ) if( feof(fp) ) break;
		tiles[c] = SDL_LoadBMP(name);
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		tiles_flags[c] = atoi(name);
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
	
	// main loop
	while(mainloop) {
		// game logic
		handleEvents();
		
		// drawing
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format,50,50,150)); // wipe screen
		SDL_Flip( screen );
	}
	
	// deinit
	SDL_FreeSurface(screen);
	SDL_FreeSurface(font8_bmp);
	SDL_FreeSurface(font16_bmp);
	for( c=0; c<numsprites; c++ )
		SDL_FreeSurface(sprites[c]);
	free(sprites);
	for( c=0; c<numtiles; c++ )
		SDL_FreeSurface(tiles[c]);
	free(tiles);
	free(tile_flags);
	for( c=0; c<numsounds; c++ )
		Mix_FreeChunk(sounds[c]);
	free(sounds);
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}