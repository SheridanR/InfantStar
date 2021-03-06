/*-------------------------------------------------------------------------------

	INFANT STAR
	File: behaviors.c
	Desc: contains all code for entity behaviors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

/*-------------------------------------------------------------------------------

	drawLayer / drawBackground / drawForeground

	Draws the world tiles that are viewable at the given camera coordinates

-------------------------------------------------------------------------------*/

void drawLayer(long camx, long camy, int z) {
	long x, y;
	long minx, miny, maxx, maxy;
	int index;
	SDL_Rect pos;
	
	minx = max(camx>>4,0);
	maxx = min((camx>>4)+xres/16+2,map.width);
	miny = max(camy>>4,0);
	maxy = min((camy>>4)+yres/16+2,map.height);
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

void drawBackground(long camx, long camy) {
	long z;
	for( z=0; z<OBSTACLELAYER; z++ )
		drawLayer(camx,camy,z);
}

void drawForeground(long camx, long camy) {
	long z;
	for( z=OBSTACLELAYER; z<MAPLAYERS; z++ )
		drawLayer(camx,camy,z);
}

/*-------------------------------------------------------------------------------

	drawSky
	
	Draws a sky backdrop using the given SDL_Surface

-------------------------------------------------------------------------------*/

void drawSky(SDL_Surface *srfc) {
	SDL_Rect pos;
	pos.x=0; pos.y=0;
	pos.w=0; pos.h=0;
	SDL_BlitSurface(srfc, NULL, screen, &pos);
}

/*-------------------------------------------------------------------------------

	drawEntities
	
	Draws all entities in the level while accounting for the given camera
	coordinates

-------------------------------------------------------------------------------*/

void drawEntities(long camx, long camy) {
	node_t *node;
	entity_t *entity;
	SDL_Rect pos, box;
	
	if( entity_l.first == NULL )
		return;
	
	// draw entities
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity = (entity_t *)node->element;
		pos.x = entity->x-camx-entity->focalx; pos.y = entity->y-camy-entity->focaly;
		pos.w = 0; pos.h = 0;
		if( entity->sprite >= 0 && entity->sprite<numsprites ) {
			if( sprites[entity->sprite] != NULL ) {
				if( entity == selectedEntity ) {
					box.w = sprites[entity->sprite]->w; box.h = sprites[entity->sprite]->h;
					box.x = pos.x; box.y = pos.y;
					SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,255,0,0));
					box.w = sprites[entity->sprite]->w-2; box.h = sprites[entity->sprite]->h-2;
					box.x = pos.x+1; box.y = pos.y+1;
					SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,0,0,255));
				}
				SDL_BlitSurface(sprites[entity->sprite], NULL, screen, &pos);
			}
			else {
				if( entity == selectedEntity ) {
					box.w = sprites[0]->w; box.h = sprites[0]->h;
					box.x = pos.x; box.y = pos.y;
					SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,255,0,0));
					box.w = sprites[0]->w-2; box.h = sprites[0]->h-2;
					box.x = pos.x+1; box.y = pos.y+1;
					SDL_FillRect(screen,&box,SDL_MapRGB(screen->format,0,0,255));
				}
				SDL_BlitSurface(sprites[0], NULL, screen, &pos);
			}
		}
	}
}

/*-------------------------------------------------------------------------------

	drawGrid
	
	Draws a white line grid for the tile map

-------------------------------------------------------------------------------*/

void drawGrid(long camx, long camy) {
	long x, y;
	Uint32 color;
	color = SDL_MapRGB(screen->format,127,127,127);
	
	SPG_LineH(screen,-camx,(map.height<<4)-camy-1,(map.width<<4)-camx-1,color);
	SPG_LineV(screen,(map.width<<4)-camx-1,-camy,(map.height<<4)-camy-1,color);
	for( y=0; y<map.height; y++ ) {
		for( x=0; x<map.width; x++ ) {
			SPG_LineH(screen,(x<<4)-camx,(y<<4)-camy,((x+1)<<4)-camx-1,color);
			SPG_LineV(screen,(x<<4)-camx,(y<<4)-camy,((y+1)<<4)-camy-1,color);
		}
	}
}

/*-------------------------------------------------------------------------------

	drawMinimap
	
	Draws a minimap in the upper right corner of the screen to represent
	the screen's position relative to the rest of the level

-------------------------------------------------------------------------------*/

void drawMinimap(long camx, long camy) {
	SDL_Rect src, osrc;
	
	src.x = xres-120;
	src.y = 24;
	src.w = 112;
	src.h = 112;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,0,0,0));
	
	// initial box dimensions
	src.x = (xres-120) + (((double)camx/16)*112.0)/map.width;
	src.y = 24 + (((double)camy/16)*112.0)/map.height;
	src.w = (112.0/map.width)*((double)xres/16);
	src.h = (112.0/map.height)*((double)yres/16);
	
	// clip at left edge
	if( src.x < xres-120 ) {
		src.w -= (xres-120)-src.x;
		src.x = xres-120;
	}
	
	// clip at right edge
	if( src.x+src.w > xres-8 )
		src.w = xres-8-src.x;
		
	// clip at top edge
	if( src.y < 24 ) {
		src.h -= 24-src.y;
		src.y = 24;
	}
	
	// clip at bottom edge
	if( src.y+src.h > 136 )
		src.h = 136-src.y;
	
	osrc.x = src.x+1; osrc.y = src.y+1;
	osrc.w = src.w-2; osrc.h = src.h-2;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,255,255,255));
	SDL_FillRect(screen,&osrc,SDL_MapRGB(screen->format,0,0,0));
}

/*-------------------------------------------------------------------------------

	drawWindow / drawDepressed

	Draws a rectangular box that fills the area inside the given screen
	coordinates

-------------------------------------------------------------------------------*/

void drawWindow(int x1, int y1, int x2, int y2) {
	SDL_Rect src;
	
	src.x = x1;
	src.y = y1;
	src.w = x2-x1;
	src.h = y2-y1;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,192,192,192));
	src.x = x1+1;
	src.y = y1+1;
	src.w = x2-x1-1;
	src.h = y2-y1-1;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,128,128,128));
	src.x = x1+1;
	src.y = y1+1;
	src.w = x2-x1-2;
	src.h = y2-y1-2;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,64,64,64));
}

void drawDepressed(int x1, int y1, int x2, int y2) {
	SDL_Rect src;
	
	src.x = x1;
	src.y = y1;
	src.w = x2-x1;
	src.h = y2-y1;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,64,64,64));
	src.x = x1+1;
	src.y = y1+1;
	src.w = x2-x1-1;
	src.h = y2-y1-1;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,192,192,192));
	src.x = x1+1;
	src.y = y1+1;
	src.w = x2-x1-2;
	src.h = y2-y1-2;
	SDL_FillRect(screen,&src,SDL_MapRGB(screen->format,128,128,128));
}

/*-------------------------------------------------------------------------------

	printTextFormatted

	Prints formatted text to the screen using a font bitmap

-------------------------------------------------------------------------------*/

void printTextFormatted( SDL_Surface *font_bmp, int x, int y, char *fmt, ... ) {
	int c;
	int numbytes;
	char str[2048];
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
		src.x = (str[c]*src.w)%font_bmp->w;
		src.y = (int)((str[c]*src.w)/font_bmp->w)*src.h;
		if( str[c] != 10 && str[c] != 13 ) { // LF/CR
			odest.x=dest.x; odest.y=dest.y;
			SDL_BlitSurface( font_bmp, &src, screen, &dest );
			dest.x=odest.x+src.w; dest.y=odest.y;
		} else if( str[c]==10 ) {
			dest.x=x;
			dest.y+=src.h;
		}
	}
	va_end( argptr );
}

/*-------------------------------------------------------------------------------

	printText

	Prints unformatted text to the screen using a font bitmap

-------------------------------------------------------------------------------*/

void printText( SDL_Surface *font_bmp, int x, int y, char *str ) {
	int c;
	int numbytes;
	SDL_Rect src, dest, odest;
	
	// format the string
	numbytes = strlen(str);
	
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
		src.y = (int)((str[c]*src.w)/font_bmp->w)*src.h;
		if( str[c] != 10 && str[c] != 13 ) { // LF/CR
			odest.x=dest.x; odest.y=dest.y;
			SDL_BlitSurface( font_bmp, &src, screen, &dest );
			dest.x=odest.x+src.w; dest.y=odest.y;
		} else if( str[c]==10 ) {
			dest.x=x;
			dest.y+=src.h;
		}
	}
}