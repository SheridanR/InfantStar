/*-------------------------------------------------------------------------------

	INFANT STAR
	File: behaviors.c
	Desc: contains all code for entity behaviors

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"
#include "game.h"

int sgn(int x) {
	return (x>0)-(x<0);
}

/*-------------------------------------------------------------------------------

	checkObstacle

	Checks the environment at the given pixel coordinates for obstacles

-------------------------------------------------------------------------------*/

entity_t *hitEntity = NULL;

int checkObstacle(long x, long y, entity_t *my) {
	entity_t *entity;
	node_t *node;

	// check entities
	hitEntity=NULL;
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity = (entity_t *)node->element;
		if( !entity->PASSABLE && entity != my ) {
			if( entity->sprite >= 0 && entity->sprite < numsprites ) {
				if( sprites[entity->sprite] != NULL ) {
					if( x >= entity->x-entity->focalx && x < entity->x-entity->focalx+sprites[entity->sprite]->w )
						if( y >= entity->y-entity->focaly && y < entity->y-entity->focaly+sprites[entity->sprite]->h ) {
							hitEntity=entity;
							break;
						}
				} else {
					if( x >= entity->x && x < entity->x+16 )
						if( y >= entity->y && y < entity->y+16 ) {
							hitEntity=entity;
							break;
						}
				}
			} else {
				if( x >= entity->x && x < entity->x+16 )
					if( y >= entity->y && y < entity->y+16 ) {
						hitEntity=entity;
						break;
					}
			}
		}
	}

	// check map
	if( x>=0 && x<map.width<<4 ) {
		if( y>=0 && y<map.height<<4 ) {
			if(map.tiles[OBSTACLELAYER+(y>>4)*MAPLAYERS+(x>>4)*MAPLAYERS*map.height])
				return 1;
			else
				return 0;
		} else {
			return 0;
		}
	} else {
		return 1;
	}
}

/*-------------------------------------------------------------------------------

	act*

	The following functions describe various entity behaviors.

-------------------------------------------------------------------------------*/

void actAnimator(entity_t *my) {
	my->skill[3]++;
	if( my->skill[3] >= 4 ) {
		my->skill[3]=0;
		map.tiles[my->skill[0]+my->y*MAPLAYERS+my->x*MAPLAYERS*map.height]++;
		my->skill[2]++;
		if(my->skill[2]==my->skill[1]) {
			my->skill[2]=0;
			map.tiles[my->skill[0]+my->y*MAPLAYERS+my->x*MAPLAYERS*map.height] -= my->skill[1];
		}
	}
}

#define PLAYER_DIR my->skill[0]
#define PLAYER_FRAME my->skill[1]
#define PLAYER_ONGROUND my->skill[2]
#define PLAYER_ANIMATE my->skill[3]
#define PLAYER_FALL my->skill[4]
#define PLAYER_JUMPING my->skill[5]
#define PLAYER_TURN my->skill[6]
#define PLAYER_JUMPED my->skill[7]
#define PLAYER_FELL my->skill[8]
#define PLAYER_LASTDIR my->skill[9]
#define PLAYER_SHOOT my->skill[10]
#define PLAYER_RECOIL my->skill[11]

int maxbullets=2;
int bullets=0;

void actPlayer(entity_t *my) {
	entity_t *entity;
	int c;
	
	if( PLAYER_LASTDIR == 0 )
		PLAYER_LASTDIR=1;
	
	// check for ground
	PLAYER_ONGROUND=1;
	if(!checkObstacle(my->x,my->y,my))
		if(!checkObstacle(my->x-8,my->y,my)) {
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
		PLAYER_JUMPING = 0;
		if(!checkObstacle(my->x,my->y-33,my))
			if(!checkObstacle(my->x-7,my->y-33,my))
				my->y += PLAYER_FALL;
		for( c=0; c>=PLAYER_FALL; c-- ) {
			if( checkObstacle(my->x,my->y-32+c,my) || checkObstacle(my->x-7,my->y-32+c,my) ) {
				my->y += c;
				PLAYER_FALL = 0;
			}
		}
	}
	
	// gravity
	if( !PLAYER_ONGROUND ) {
		// don't fall through the floor...
		if( PLAYER_FALL > 0 ) {
			if( checkObstacle(my->x,my->y+PLAYER_FALL,my) || checkObstacle(my->x-8,my->y+PLAYER_FALL,my) )
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
	} else {
		if( PLAYER_FALL > 0 ) {
			Mix_PlayChannel(-1, sounds[1], 0);
			if( PLAYER_FALL == 16 ) {
				PLAYER_JUMPING = -6;
				PLAYER_FELL = 1;
			} else {
				PLAYER_JUMPING = -3;
				PLAYER_FELL = 2;
			}
			PLAYER_FALL = 0;
			PLAYER_JUMPED = 0;
		}
		my->y += PLAYER_FALL;
			
		// jump command
		if( keystatus[SDLK_LCTRL] && PLAYER_JUMPING>=-2 && PLAYER_JUMPING<=0 && !PLAYER_FALL && !PLAYER_JUMPED ) {
			Mix_PlayChannel(-1, sounds[0], 0);
			keystatus[SDLK_LCTRL]=0;
			PLAYER_JUMPING = 1;
			PLAYER_JUMPED = 1;
			PLAYER_SHOOT = 0;
			if( !keystatus[SDLK_RIGHT] && !keystatus[SDLK_LEFT] )
				PLAYER_DIR = 0;
		}
	}
	
	// jumping
	if( PLAYER_JUMPING ) {
		if( PLAYER_JUMPING < 0 ) {
			PLAYER_JUMPING++;
			if( PLAYER_FELL == 2 || PLAYER_JUMPING == -5 || ((PLAYER_JUMPING == -1 || PLAYER_JUMPING == -2)&&!keystatus[SDLK_DOWN]) )
				PLAYER_FRAME = 12;
			else
				PLAYER_FRAME = 13;
		}
		if( PLAYER_JUMPING == 1 ) {
			PLAYER_FRAME = 9;
			PLAYER_JUMPING++;
		}
		else if( PLAYER_JUMPING == 2 ) {
			PLAYER_FRAME = 10;
			PLAYER_JUMPING++;
		}
		else if( PLAYER_JUMPING == 3 ) {
			PLAYER_FRAME = 11;
			PLAYER_FALL = -12;
			PLAYER_ONGROUND = 0;
		}
	}
	
	// move to the right
	if( keystatus[SDLK_RIGHT] && !keystatus[SDLK_LEFT] && !PLAYER_JUMPING ) {
		if(PLAYER_ONGROUND) {
			PLAYER_DIR = 1; // change direction
			PLAYER_LASTDIR = 1;
			PLAYER_TURN = 0;
			
			// animate
			if( PLAYER_FALL == 0 ) {
				PLAYER_ANIMATE=6;
				PLAYER_FRAME++;
				if( PLAYER_FRAME<1 || PLAYER_FRAME>8 )
					PLAYER_FRAME = 1;
			}
			
			// check for an obstacle and move if clear
			if(!checkObstacle(my->x+8,my->y-1,my))
				if(!checkObstacle(my->x+8,my->y-17,my))
					if(!checkObstacle(my->x+8,my->y-25,my))
						my->x += 8;
		}
		else {
			if(!checkObstacle(my->x+8,my->y-1,my)) {
				if(!checkObstacle(my->x+8,my->y-17,my)) {
					if(!checkObstacle(my->x+8,my->y-25,my)) {
						my->x += 8;
						PLAYER_DIR = 1; // change direction
						PLAYER_LASTDIR = 1;
					}
					else
						PLAYER_DIR = 0;
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
			PLAYER_LASTDIR = 2;
			PLAYER_TURN = 0;
			
			// animate
			if( PLAYER_FALL == 0 ) {
				PLAYER_ANIMATE=6;
				PLAYER_FRAME++;
				if( PLAYER_FRAME<1 || PLAYER_FRAME>8 )
					PLAYER_FRAME = 1;
			}
			
			// check for an obstacle and move if clear
			if(!checkObstacle(my->x-16,my->y-1,my))
				if(!checkObstacle(my->x-16,my->y-17,my))
					if(!checkObstacle(my->x-16,my->y-25,my))
						my->x -= 8;
		}
		else {
			if(!checkObstacle(my->x-16,my->y-1,my)) {
				if(!checkObstacle(my->x-16,my->y-17,my)) {
					if(!checkObstacle(my->x-16,my->y-25,my)) {
						my->x -= 8;
						PLAYER_DIR = 2; // change direction
						PLAYER_LASTDIR = 2;
					}
					else
						PLAYER_DIR = 0;
				}
				else
					PLAYER_DIR = 0;
			}
			else
				PLAYER_DIR = 0;
		}
	}
	
	// standing still
	if( ((!keystatus[SDLK_LEFT] && !keystatus[SDLK_RIGHT]) || (keystatus[SDLK_LEFT] && keystatus[SDLK_RIGHT])) && !PLAYER_JUMPING && !PLAYER_FALL && !PLAYER_JUMPED ) {
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
				if( PLAYER_TURN > 30 ) {
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
	my->focalx = 8;
	if( PLAYER_DIR == 0 ) {
		if( PLAYER_FRAME==0 )
			my->sprite = 1;
		else
			my->sprite = 1+PLAYER_FRAME-8;
	}
	else {
		my->sprite = 7+PLAYER_FRAME+(PLAYER_DIR-1)*23;
		if( PLAYER_SHOOT && PLAYER_FRAME >= 0 && PLAYER_FRAME < 9 ) {
			my->sprite += 14;
			if( PLAYER_DIR == 2 )
				my->focalx = 12;
		}
	}
	
	// move camera
	if( my->x-camx >= xres-128 )
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
	
	if( my->y-camy > yres-64 )
		newcamy = my->y-camy-yres+64;
	else if( my->y-camy < 96 )
		newcamy = my->y-camy-96;
	else
		newcamy = 0;

	camx = min(max(0,camx+newcamx),(map.width-20)<<4);
	camy = min(max(0,camy+newcamy),(map.height<<4)-yres);
	
	// attack
	if( PLAYER_SHOOT > 0 )
		PLAYER_SHOOT--;
	if( PLAYER_RECOIL > 0 )
		PLAYER_RECOIL--;
	if( !keystatus[SDLK_UP] && !keystatus[SDLK_DOWN] )
		if( keystatus[SDLK_LALT] && !PLAYER_RECOIL && PLAYER_ONGROUND && !PLAYER_JUMPED ) {
			PLAYER_DIR=PLAYER_LASTDIR;
			PLAYER_SHOOT=14;
			PLAYER_TURN=0;
			if( bullets < maxbullets ) {
				bullets++;
				Mix_PlayChannel(-1, sounds[11], 0);
				entity=newEntity(-1,0);
				entity->skill[0]=PLAYER_LASTDIR-1; // bullet direction
				entity->skill[4]=16; // bullet speed
				entity->x=my->x;
				entity->y=my->y-24;
				entity->focalx=8;
				entity->focaly=4;
				entity->behavior=&actBullet;
			} else {
				Mix_PlayChannel(-1, sounds[2], 0);
			}
			//PLAYER_RECOIL=2;
			keystatus[SDLK_LALT]=0;
		}
}

#define GEM_FRAME my->skill[0]
#define GEM_ANIMTIME my->skill[1]

void actPGem(entity_t *my) {
	node_t *node;
	entity_t *entity;
	
	GEM_ANIMTIME++;
	if( GEM_ANIMTIME>=2 ) {
		GEM_ANIMTIME=0;
		GEM_FRAME++;
		if(GEM_FRAME>3)
			GEM_FRAME=0;
	}
	my->sprite=53+GEM_FRAME;
	
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity=(entity_t *)node->element;
		if( entity->behavior == &actPlayer ) {
			// touched by the player
			if( abs(my->x-entity->x) < 16 && entity->y > my->y-8 && entity->y-32 < my->y+8 ) {
				entity=newEntity(66,0);
				entity->x=my->x;
				entity->y=my->y;
				entity->behavior=&actGlean;
				
				Mix_PlayChannel(-1, sounds[3], 0);
				list_RemoveNode(my->node);
			}
		}
	}
}

void actRGem(entity_t *my) {
	node_t *node;
	entity_t *entity;
	
	GEM_ANIMTIME++;
	if( GEM_ANIMTIME>=2 ) {
		GEM_ANIMTIME=0;
		GEM_FRAME++;
		if(GEM_FRAME>3)
			GEM_FRAME=0;
	}
	my->sprite=57+GEM_FRAME;
	
	for( node=entity_l.first; node!=NULL; node=node->next ) {
		entity=(entity_t *)node->element;
		if( entity->behavior == &actPlayer ) {
			// touched by the player
			if( abs(my->x-entity->x) < 16 && entity->y > my->y-8 && entity->y-32 < my->y+8 ) {
				entity=newEntity(66,0);
				entity->x=my->x;
				entity->y=my->y;
				entity->behavior=&actGlean;
				
				Mix_PlayChannel(-1, sounds[3], 0);
				list_RemoveNode(my->node);
			}
		}
	}
}

#define BULLET_DIR my->skill[0]
#define BULLET_DEATH my->skill[1]
#define BULLET_ANIM my->skill[2]
#define BULLET_ADJUST my->skill[3]
#define BULLET_SPEED my->skill[4]

void actBullet(entity_t *my) {
	entity_t *entity;
	int c, x;

	if( !BULLET_DEATH ) {
		if( my->x < camx-80 || my->x >= camx+xres+80 ) {
			bullets--; // retrieve this bullet for the player
			list_RemoveNode(my->node);
			return;
		}
		my->sprite=61+BULLET_DIR;
		if( !BULLET_DIR ) {
			for( c=0; c<BULLET_SPEED; c++ ) {
				if( checkObstacle(my->x+8,my->y,my) || ( hitEntity != NULL && hitEntity->HURTABLE ) ) {
					BULLET_DEATH=1;
					bullets--; // retrieve this bullet for the player
					if( hitEntity != NULL && hitEntity->HURTABLE ) {
						for(x=0; x<2; x++) {
							entity = newEntity(70+rand()%4,1);
							entity->x = my->x+4+rand()%8;
							entity->y = my->y-4+rand()%8;
							entity->skill[0] = rand()%4-2;
							entity->skill[1] = -4-rand()%2;
							entity->behavior = &actParticle;
						}
						Mix_PlayChannel(-1, sounds[12], 0);
						hitEntity->HEALTH--;
						entity=hitEntity;
						if( entity->behavior == &actTroll )
							if( checkObstacle(entity->x+20,entity->y,entity) && !checkObstacle(entity->x+20,entity->y-16,entity) && !checkObstacle(entity->x+20,entity->y-32,entity) )
								entity->x+=4;
					} else {
						Mix_PlayChannel(-1, sounds[9], 0);
					}
					break;
				}
				else
					my->x++; // move right
			}
		} else {
			for( c=0; c<BULLET_SPEED; c++ ) {
				if( checkObstacle(my->x-9,my->y,my) || ( hitEntity != NULL && hitEntity->HURTABLE ) ) {
					BULLET_DEATH=1;
					bullets--; // retrieve this bullet for the player
					if( hitEntity != NULL && hitEntity->HURTABLE ) {
						for(x=0; x<2; x++) {
							entity = newEntity(70+rand()%4,1);
							entity->x = my->x-5-rand()%8;
							entity->y = my->y-4+rand()%8;
							entity->skill[0] = rand()%4-2;
							entity->skill[1] = -4-rand()%2;
							entity->behavior = &actParticle;
						}
						Mix_PlayChannel(-1, sounds[12], 0);
						hitEntity->HEALTH--;
						entity=hitEntity;
						if( entity->behavior == &actTroll )
							if( checkObstacle(entity->x-21,entity->y,entity) && !checkObstacle(entity->x-21,entity->y-16,entity) && !checkObstacle(entity->x-21,entity->y-32,entity) )
								entity->x-=4;
					} else {
						Mix_PlayChannel(-1, sounds[9], 0);
					}
					break;
				}
				else
					my->x--; // move left
			}
		}
	} else {
		if( !BULLET_ADJUST ) {
			BULLET_ADJUST=1;
			my->focalx=4;
			my->x -= 4*(BULLET_DIR*2-1);
		}
		my->sprite=63+BULLET_ANIM;
		BULLET_ANIM++;
		if( BULLET_ANIM > 3 )
			list_RemoveNode(my->node);
	}
}

#define GLEAN_FRAME my->skill[0]
#define GLEAN_ANIM my->skill[1]

void actGlean(entity_t *my) {
	if( !GLEAN_ANIM ) {
		GLEAN_FRAME++;
		if( GLEAN_FRAME > 3 ) {
			GLEAN_FRAME=3;
			GLEAN_ANIM=1;
		}
	} else {
		GLEAN_FRAME--;
		if( GLEAN_FRAME < 0 )
			list_RemoveNode(my->node);
	}
	my->sprite=66+GLEAN_FRAME;
}

#define TROLL_DIR my->skill[0]
#define TROLL_DYING my->skill[1]

void actTroll(entity_t *my) {
	entity_t *entity;
	int c;

	my->sprite = 74+TROLL_DIR;
	if( !TROLL_DYING ) {
		my->HURTABLE = 1;
		if( my->HEALTH <= 0 ) {
			Mix_PlayChannel(0, sounds[13], 0);
			TROLL_DYING=1;
			for(c=0; c<4; c++) {
				entity = newEntity(70+rand()%4,1);
				entity->x = my->x-12+rand()%24;
				entity->y = my->y-4-rand()%24;
				entity->skill[0] = rand()%4-2;
				entity->skill[1] = -4-rand()%2;
				entity->behavior = &actParticle;
			}
		} else {
			if( checkObstacle(my->x+16-32*TROLL_DIR,my->y,my) && !checkObstacle(my->x+16-32*TROLL_DIR,my->y-16,my) && !checkObstacle(my->x+16-32*TROLL_DIR,my->y-32,my) )
				my->x += 2-4*TROLL_DIR;
			else
				TROLL_DIR = (TROLL_DIR==0);
		}
	} else {
		if( TROLL_DYING >= 20 ) {
			list_RemoveNode(my->node);
			return;
		}
		my->HURTABLE = 0;
		TROLL_DYING++;
		TROLL_DIR=(TROLL_DIR==0);
	}
}

#define PARTICLE_VELX my->skill[0]
#define PARTICLE_VELY my->skill[1]
#define PARTICLE_LIFE my->skill[2]
#define PARTICLE_DROP my->skill[3]

void actParticle(entity_t *my) {
	int c;
	
	// lifespan
	PARTICLE_LIFE++;
	if( PARTICLE_LIFE > 20 ) {
		list_RemoveNode(my->node);
		return;
	}
	
	// gravity
	if( !checkObstacle(my->x,my->y+2,my) ) {
		PARTICLE_DROP++;
		if( PARTICLE_DROP >= 2 ) {
			PARTICLE_DROP=0;
			PARTICLE_VELY++;
			if( PARTICLE_VELY > 16 )
				PARTICLE_VELY=16;
		}
	}
	else
		PARTICLE_VELY=0;
	
	// ceiling bump
	if( checkObstacle(my->x,my->y-3,my) && PARTICLE_VELY<0 )
		PARTICLE_VELY=0;

	// move horizontally
	for( c=min(PARTICLE_VELX,0); c!=max(PARTICLE_VELX,0); c++ ) {
		if( !checkObstacle(my->x+2*sgn(PARTICLE_VELX)-(PARTICLE_VELX<0),my->y,my) )
			my->x += sgn(PARTICLE_VELX); // move particle horizontally
		else
			break;
	}
	
	// move vertically
	for( c=min(PARTICLE_VELY,0); c!=max(PARTICLE_VELY,0); c++ ) {
		if( !checkObstacle(my->x,my->y+2*sgn(PARTICLE_VELY)-(PARTICLE_VELY<0),my) )
			my->y += sgn(PARTICLE_VELY); // move particle vertically
		else
			break;
	}
}