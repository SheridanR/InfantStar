/*-------------------------------------------------------------------------------

	INFANT STAR
	File: behaviors.c
	Desc: contains all code for entity behaviors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

/*-------------------------------------------------------------------------------

	drawBackground / drawForeground

	Draws the world tiles that are viewable at the given camera coordinates

-------------------------------------------------------------------------------*/

void drawBackground(long camx, long camy) {
	long x, y, z;
	long minx, miny, maxx, maxy;
	unsigned char index;
	SDL_Rect pos;
	
	minx = max(camx>>4,0);
	maxx = min((camx>>4)+xres/16+2,map.width);
	miny = max(camy>>4,0);
	maxy = min((camy>>4)+yres/16+2,map.height);
	for( z=0; z<OBSTACLELAYER; z++ ) {
		for( y=miny; y<maxy; y++ ) {
			for( x=minx; x<maxx; x++ ) {
				index = map.tiles[z+y*MAPLAYERS+x*MAPLAYERS*map.height];
				if( index > 0) {
					pos.x = (x<<4)-camx; pos.y = (y<<4)-camy;
					pos.w = 0; pos.h = 0;
					if( tiles[index] != NULL )
						SDL_BlitSurface(tiles[index], NULL, screen, &pos);
					else
						SDL_BlitSurface(sprites[0], NULL, screen, &pos);
				}
			}
		}
	}
}

void drawForeground(long camx, long camy) {
	long x, y, z;
	long minx, miny, maxx, maxy;
	unsigned char index;
	SDL_Rect pos;

	minx = max(camx>>4,0);
	maxx = min((camx>>4)+xres/16+2,map.width);
	miny = max(camy>>4,0);
	maxy = min((camy>>4)+yres/16+2,map.height);
	for( z=OBSTACLELAYER; z<MAPLAYERS; z++ ) {
		for( y=miny; y<maxy; y++ ) {
			for( x=minx; x<maxx; x++ ) {
				index = map.tiles[z+y*MAPLAYERS+x*MAPLAYERS*map.height];
				if( index > 0) {
					pos.x = (x<<4)-camx; pos.y = (y<<4)-camy;
					pos.w = 0; pos.h = 0;
					if( tiles[index] != NULL )
						SDL_BlitSurface(tiles[index], NULL, screen, &pos);
					else
						SDL_BlitSurface(sprites[0], NULL, screen, &pos);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	drawEntities
	
	Draws all entities in the level while accounting for the given camera
	coordinates

-------------------------------------------------------------------------------*/

void drawEntities(long camx, long camy) {
	node_t *node;
	entity_t *entity;
	SDL_Rect pos;
	
	// draw entities
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity = (entity_t *)node->element;
		pos.x = entity->x-camx-entity->focalx; pos.y = entity->y-camy-entity->focaly;
		pos.w = 0; pos.h = 0;
		if( entity->sprite >=0 && entity->sprite<numsprites ) {
			if( sprites[entity->sprite] != NULL )
				SDL_BlitSurface(sprites[entity->sprite], NULL, screen, &pos);
			else
				SDL_BlitSurface(sprites[0], NULL, screen, &pos);
		}
		else
			SDL_BlitSurface(sprites[0], NULL, screen, &pos);
	}
}

/*-------------------------------------------------------------------------------

	printText

	Prints formatted text to the screen using a font bitmap

-------------------------------------------------------------------------------*/

void printText( SDL_Surface *font_bmp, int x, int y, char *fmt, ... ) {
	int c;
	int numbytes;
	char str[100];
	va_list argptr;
	SDL_Rect src, dest, odest;
	
	// format the string
	va_start( argptr, fmt );
	numbytes = vsprintf( str, fmt, argptr );
	
	// define font dimensions
	dest.x = x;
	dest.y = y;
	dest.w = font_bmp->w/16; src.w = font_bmp->w/16;
	dest.h = font_bmp->h/16; src.h = font_bmp->h/16;
	
	// print the characters in the string
	for( c=0; c<numbytes; c++ ) {
		// edge of the screen prompts an automatic newline
		if( xres-dest.x < src.w ) {
			dest.x = x;
			dest.y += src.h;
		}
		
		src.x = (str[c]*src.w)%font_bmp->w;
		src.y = floor((str[c]*src.w)/font_bmp->w)*src.h;
		odest.x=dest.x; odest.y=dest.y;
		SDL_BlitSurface( font_bmp, &src, screen, &dest );
		dest.x=odest.x; dest.y=odest.y;
		switch( str[c] ) {
			case 10: // line feed
				dest.x = x;
				dest.y += src.h;
				break;
			default:
				dest.x += src.w; // move over one character
				break;
		}
	}
	va_end( argptr );
}