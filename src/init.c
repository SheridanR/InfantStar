/*-------------------------------------------------------------------------------

	INFANT STAR
	File: init.c
	Desc: contains program initialization code

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

int initApp(char *title, int fullscreen) {
	char name[128];
	FILE *fp;
	int x, c;
	
	fprintf(stderr, "initializing SDL...\n");
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER ) == -1 ) {
		fprintf(stderr, "failed to initialize SDL.\n");
		return 1;
	}
	fprintf(stderr, "opening audio mixer. rate: %d format: %d channels: %d buffers: %d\n", audio_rate, audio_format, audio_channels, audio_buffers);
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		SDL_Quit();
		fprintf(stderr, "failed to open audio.\n");
		return 2;
	}
	fprintf(stderr, "setting display mode to %dx%d...\n",xres,yres);
	if(!fullscreen) {
		SDL_ShowCursor(SDL_ENABLE);
		if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_RESIZABLE )) == NULL) {
			SDL_Quit();
			Mix_CloseAudio();
			fprintf(stderr, "failed to set video mode.\n");
			return 3;
		}
	}
	else {
		SDL_ShowCursor(SDL_DISABLE);
		if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_FULLSCREEN )) == NULL) {
			SDL_Quit();
			Mix_CloseAudio();
			fprintf(stderr, "failed to set video mode.\n");
			return 3;
		}
	}
	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption(title, 0);
	SDL_EnableKeyRepeat(400,50);
	entity_l.first=NULL; entity_l.last=NULL;
	button_l.first=NULL; button_l.last=NULL;
	
	// load resources
	fprintf(stderr, "loading engine resources...\n");
	if((font8_bmp=SDL_LoadBMP("images/8font.bmp")) == NULL) {
		SDL_Quit();
		Mix_CloseAudio();
		fprintf(stderr, "failed to load 8font.bmp\n");
		return 4;
	}
	SDL_SetColorKey( font8_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font8_bmp->format, 255, 0, 255 ) );
	if((font16_bmp=SDL_LoadBMP("images/16font.bmp")) == NULL) {
		SDL_Quit();
		Mix_CloseAudio();
		SDL_FreeSurface(font8_bmp);
		fprintf(stderr, "failed to load 16font.bmp\n");
		return 5;
	}
	SDL_SetColorKey( font16_bmp, SDL_SRCCOLORKEY, SDL_MapRGB( font16_bmp->format, 255, 0, 255 ) );
	
	// load sprites
	fprintf(stderr, "loading sprites...\n");
	fp = fopen("images/sprites.txt","r");
	for( numsprites=0; !feof(fp); numsprites++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	if( numsprites==0 ) {
		SDL_Quit();
		Mix_CloseAudio();
		SDL_FreeSurface(font8_bmp);
		SDL_FreeSurface(font16_bmp);
		fprintf(stderr, "failed to identify any sprites in sprites.txt\n");
		return 6;
	}
	sprites = (SDL_Surface **) malloc(sizeof(SDL_Surface *)*numsprites);
	fp = fopen("images/sprites.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		sprites[c] = SDL_LoadBMP(name);
		if( sprites[c] != NULL )
			SDL_SetColorKey( sprites[c], SDL_SRCCOLORKEY, SDL_MapRGB( sprites[c]->format, 0, 0, 255 ) );
		else {
			fprintf(stderr, "warning: failed to load '%s' listed at line %d in sprites.txt\n",name,c+1);
			if( c==0 ) {
				SDL_Quit();
				Mix_CloseAudio();
				SDL_FreeSurface(font8_bmp);
				SDL_FreeSurface(font16_bmp);
				for( x=0; x<c; x++ )
					SDL_FreeSurface(sprites[x]);
				free(sprites);
				fprintf(stderr, "sprite 0 cannot be NULL!\n");
				return 7;
			}
		}
	}
	
	// load tiles
	fprintf(stderr, "loading tiles...\n");
	fp = fopen("images/tiles.txt","r");
	for( numtiles=0; !feof(fp); numtiles++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	if( numtiles==0 ) {
		SDL_Quit();
		Mix_CloseAudio();
		SDL_FreeSurface(font8_bmp);
		SDL_FreeSurface(font16_bmp);
		for( c=0; c<numsprites; c++ )
			SDL_FreeSurface(sprites[c]);
		free(sprites);
		fprintf(stderr, "failed to identify any tiles in tiles.txt\n");
		return 8;
	}
	tiles = (SDL_Surface **) malloc(sizeof(SDL_Surface *)*numtiles);
	animatedtiles = (char *) malloc(sizeof(char)*numtiles);
	fp = fopen("images/tiles.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		tiles[c] = SDL_LoadBMP(name);
		animatedtiles[c] = 0;
		if( tiles[c] != NULL ) {
			SDL_SetColorKey( tiles[c], SDL_SRCCOLORKEY, SDL_MapRGB( tiles[c]->format, 0, 0, 0 ) );
			for(x=0;x<strlen(name);x++) {
				if( name[x]>=50 && name[x]<58 ) {
					animatedtiles[c]=name[x]-48;
					break;
				}
			}
			if( tiles[c]->w != 16 || tiles[c]->h != 16 )
				fprintf(stderr, "warning: improperly sized tile '%s' listed at line %d in tiles.txt\n",name,c+1);
		}
		else {
			fprintf(stderr, "warning: failed to load '%s' listed at line %d in tiles.txt\n",name,c+1);
			if( c==0 ) {
				SDL_Quit();
				Mix_CloseAudio();
				SDL_FreeSurface(font8_bmp);
				SDL_FreeSurface(font16_bmp);
				for( x=0; x<numsprites; x++ )
					SDL_FreeSurface(sprites[x]);
				free(sprites);
				for( x=0; x<c; x++ )
					SDL_FreeSurface(tiles[x]);
				free(tiles);
				fprintf(stderr, "tile 0 cannot be NULL!\n");
				return 9;
			}
		}
	}
	
	// load sound effects
	fprintf(stderr, "loading sounds...\n");
	fp = fopen("sound/sounds.txt","r");
	for( numsounds=0; !feof(fp); numsounds++ ) {
		while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
	}
	fclose(fp);
	if( numsounds==0 ) {
		SDL_Quit();
		Mix_CloseAudio();
		SDL_FreeSurface(font8_bmp);
		SDL_FreeSurface(font16_bmp);
		for( c=0; c<numsprites; c++ )
			SDL_FreeSurface(sprites[c]);
		free(sprites);
		for( c=0; c<numtiles; c++ )
			SDL_FreeSurface(tiles[c]);
		free(tiles);
		fprintf(stderr, "failed to identify any sounds in sounds.txt\n");
		return 10;
	}
	sounds = (Mix_Chunk **) malloc(sizeof(Mix_Chunk *)*numsounds);
	fp = fopen("sound/sounds.txt","r");
	for( c=0; !feof(fp); c++ ) {
		fscanf(fp,"%s",name); while( fgetc(fp) != '\n' ) if( feof(fp) ) break;
		sounds[c] = Mix_LoadWAV(name);
		if( sounds[c] == NULL )
			fprintf(stderr, "warning: failed to load '%s' listed at line %d in sounds.txt\n",name,c+1);
		else
			Mix_VolumeChunk(sounds[c],MIX_MAX_VOLUME/2);
	}
	fclose(fp);
	return 0;
}