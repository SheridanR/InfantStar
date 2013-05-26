/*-------------------------------------------------------------------------------

	INFANT STAR
	File: behaviors.c
	Desc: contains all code for entity behaviors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

#define PLAYER_DIR my->skill[0]
#define PLAYER_FRAME my->skill[1]
#define PLAYER_ONGROUND my->skill[2]
#define PLAYER_ANIMATE my->skill[3]
#define PLAYER_FALL my->skill[4]
#define PLAYER_JUMPING my->skill[5]
#define PLAYER_TURN my->skill[6]

void actPlayer(entity_t *my) {
	// check for ground
	PLAYER_ONGROUND=1;
	if(!map.tiles[OBSTACLELAYER+((my->y)>>4)*MAPLAYERS+((my->x)>>4)*MAPLAYERS*map.height])
		if(!map.tiles[OBSTACLELAYER+((my->y)>>4)*MAPLAYERS+((my->x-8)>>4)*MAPLAYERS*map.height]) {
			PLAYER_ONGROUND=0;
			if( PLAYER_FALL==0 )
				PLAYER_FALL=2;
		}
	if( PLAYER_FALL ) {
		PLAYER_ANIMATE=0;
		PLAYER_TURN = 0;
	}
		
	// don't jump through the ceiling...
	if( PLAYER_FALL < 0 ) {
		if(map.tiles[OBSTACLELAYER+(int)floor((my->y-33)/16.0)*MAPLAYERS+((my->x)>>4)*MAPLAYERS*map.height]) {
			my->y = (((my->y+15)>>4))<<4;
			PLAYER_FALL = 0;
		}
		else if(map.tiles[OBSTACLELAYER+(int)floor((my->y-33)/16.0)*MAPLAYERS+((my->x-8)>>4)*MAPLAYERS*map.height]) {
			my->y = (((my->y+15)>>4))<<4;
			PLAYER_FALL = 0;
		}
		else {
			my->y += PLAYER_FALL;
			if(map.tiles[OBSTACLELAYER+(int)floor((my->y+PLAYER_FALL-33)/16.0)*MAPLAYERS+((my->x)>>4)*MAPLAYERS*map.height]) {
				my->y = (((my->y+8)>>4))<<4;
				PLAYER_FALL = 0;
			}
			else if(map.tiles[OBSTACLELAYER+(int)floor((my->y+PLAYER_FALL-33)/16.0)*MAPLAYERS+((my->x-8)>>4)*MAPLAYERS*map.height]) {
				my->y = (((my->y+8)>>4))<<4;
				PLAYER_FALL = 0;
			}
		}
	}
	
	// gravity
	if( !PLAYER_ONGROUND ) {
		// don't fall through the floor...
		if( PLAYER_FALL > 0 ) {
			if(map.tiles[OBSTACLELAYER+((my->y+PLAYER_FALL)>>4)*MAPLAYERS+((my->x)>>4)*MAPLAYERS*map.height])
				my->y = (((my->y+PLAYER_FALL)>>4)<<4);
			else if(map.tiles[OBSTACLELAYER+((my->y+PLAYER_FALL)>>4)*MAPLAYERS+((my->x-8)>>4)*MAPLAYERS*map.height])
				my->y = (((my->y+PLAYER_FALL)>>4)<<4);
			else
				my->y += PLAYER_FALL;
		}
			
		// increase vertical velocity
		if( PLAYER_FALL < 16 )
			PLAYER_FALL += 2;
		
		// animate
		if( PLAYER_FALL >= 0 )
			PLAYER_FRAME = 12;
		if( PLAYER_FALL < 0 )
			PLAYER_FRAME = 11;
	}
	else {
		if( PLAYER_FALL > 0 ) {
			Mix_PlayChannel(-1, sounds[1], 0);
			PLAYER_FALL = 0;
			PLAYER_JUMPING = -4;
		}
		my->y += PLAYER_FALL;
			
		// jump command
		if( keystatus[SDLK_LCTRL] && !PLAYER_JUMPING && !PLAYER_FALL ) {
			Mix_PlayChannel(-1, sounds[0], 0);
			PLAYER_JUMPING = 1;
			if( !keystatus[SDLK_RIGHT] && !keystatus[SDLK_LEFT] )
				PLAYER_DIR = 0;
		}
	}
	
	// jumping
	if( PLAYER_JUMPING ) {
		if( PLAYER_JUMPING < 0 ) {
			PLAYER_FRAME = 13;
			PLAYER_JUMPING++;
		}
		if( PLAYER_JUMPING == 1 || PLAYER_JUMPING == 2 ) {
			PLAYER_FRAME = 9;
			PLAYER_JUMPING++;
		}
		else if( PLAYER_JUMPING == 3 ) {
			PLAYER_FRAME = 10;
			PLAYER_JUMPING++;
		}
		else if( PLAYER_JUMPING == 4 ) {
			PLAYER_JUMPING = 0;
			PLAYER_FRAME = 11;
			PLAYER_FALL = -12;
			PLAYER_ONGROUND = 0;
		}
	}
	
	// move to the right
	if( keystatus[SDLK_RIGHT] && !keystatus[SDLK_LEFT] && !PLAYER_JUMPING ) {
		if(PLAYER_ONGROUND) {
			PLAYER_DIR = 1; // change direction
			PLAYER_TURN = 0;
			
			// animate
			if( PLAYER_FALL == 0 ) {
				PLAYER_ANIMATE=6;
				if( PLAYER_FRAME<1 || PLAYER_FRAME>8 )
					PLAYER_FRAME = 1;
				else
					PLAYER_FRAME++;
			}
			
			// check for an obstacle and move if clear
			if(!map.tiles[OBSTACLELAYER+((my->y-1)>>4)*MAPLAYERS+((my->x+8)>>4)*MAPLAYERS*map.height])
				if(!map.tiles[OBSTACLELAYER+((my->y-17)>>4)*MAPLAYERS+((my->x+8)>>4)*MAPLAYERS*map.height])
					my->x += 8;
		}
		else {
			if(!map.tiles[OBSTACLELAYER+((my->y-1)>>4)*MAPLAYERS+((my->x+8)>>4)*MAPLAYERS*map.height]) {
				if(!map.tiles[OBSTACLELAYER+((my->y-17)>>4)*MAPLAYERS+((my->x+8)>>4)*MAPLAYERS*map.height]) {
					my->x += 8;
					PLAYER_DIR = 1; // change direction
				}
				else
					PLAYER_DIR = 0;
			}
			else
				PLAYER_DIR = 0;
		}
	}
	
	// move to the left
	if( keystatus[SDLK_LEFT] && !keystatus[SDLK_RIGHT] && !PLAYER_JUMPING ) {
		if(PLAYER_ONGROUND) {
			PLAYER_DIR = 2; // change direction
			PLAYER_TURN = 0;
			
			// animate
			if( PLAYER_FALL == 0 ) {
				PLAYER_ANIMATE=4;
				if( PLAYER_FRAME<1 || PLAYER_FRAME>8 )
					PLAYER_FRAME = 1;
				else
					PLAYER_FRAME++;
			}
			
			// check for an obstacle and move if clear
			if(!map.tiles[OBSTACLELAYER+((my->y-1)>>4)*MAPLAYERS+((my->x-16)>>4)*MAPLAYERS*map.height])
				if(!map.tiles[OBSTACLELAYER+((my->y-17)>>4)*MAPLAYERS+((my->x-16)>>4)*MAPLAYERS*map.height])
					my->x -= 8;
		}
		else {
			if(!map.tiles[OBSTACLELAYER+((my->y-1)>>4)*MAPLAYERS+((my->x-16)>>4)*MAPLAYERS*map.height]) {
				if(!map.tiles[OBSTACLELAYER+((my->y-17)>>4)*MAPLAYERS+((my->x-16)>>4)*MAPLAYERS*map.height]) {
					my->x -= 8;
					PLAYER_DIR = 2; // change direction
				}
				else
					PLAYER_DIR = 0;
			}
			else
				PLAYER_DIR = 0;
		}
	}
	
	// standing still
	if( !keystatus[SDLK_LEFT] && !keystatus[SDLK_RIGHT] && !PLAYER_JUMPING && !PLAYER_FALL ) {
		if(PLAYER_ONGROUND) {
			if( PLAYER_ANIMATE > 0 ) {
				PLAYER_ANIMATE--;
				if( PLAYER_ANIMATE>0 && PLAYER_ANIMATE<=2 )
					PLAYER_FRAME=8;
				if( PLAYER_ANIMATE>2 && PLAYER_ANIMATE<=4 )
					PLAYER_FRAME=7;
				if( PLAYER_ANIMATE>4 && PLAYER_ANIMATE<=6 )
					PLAYER_FRAME=6;
			}
			else {
				PLAYER_TURN++;
				if( PLAYER_TURN > 20 ) {
					PLAYER_DIR = 0;
					PLAYER_FRAME = 0;
					PLAYER_TURN = 0;
				}
				if( keystatus[SDLK_UP] ) {
					PLAYER_DIR = 0;
					PLAYER_FRAME = 0;
				}
				else if( keystatus[SDLK_DOWN] ) {
					PLAYER_DIR = 0;
					PLAYER_FRAME = 13;
				}
				else
					PLAYER_FRAME = 0;
			}
		}
	}
	
	// animate me
	if( PLAYER_DIR == 0 && PLAYER_FRAME==0 ) {
		my->sprite = 1;
	}
	else {
		my->sprite = 1+PLAYER_FRAME-8+PLAYER_DIR*14;
	}
	
	// move camera
	if( my->x-camx >= 192 )
		newcamx += 2;
	else {
		if( my->x-camx <= 128 )
			newcamx -= 2;
		else {
			if( newcamx > 0 )
				newcamx -= 2;
			else if( newcamx < 0 )
				newcamx += 2;
		}
	}
	newcamx = min(max(-8,newcamx),8);
	
	if( my->y-camy >= 168 )
		newcamy += 2;
	else {
		if( my->y-camy <= 96 )
			newcamy -= 2;
		else {
			if( newcamx > 0 )
				newcamy-=2;
			else if( newcamx < 0 )
				newcamy+=2;
			if( keystatus[SDLK_DOWN] )
				newcamy += 4;
			if( keystatus[SDLK_UP] )
				newcamy -= 4;
		}
	}
	newcamy = min(max(-8,newcamy),8);

	camx = min(max(0,camx+newcamx),(map.width-20)<<4);
	camy = min(max(0,camy+newcamy),(map.height-13)<<4);
}	
