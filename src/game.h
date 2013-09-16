/*-------------------------------------------------------------------------------

	INFANT STAR
	File: game.h
	Desc: header file for the game

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#define HEALTH skill[20]
#define PASSABLE skill[21]
#define HURTABLE skill[22]

// function prototypes for behaviors.c:
int checkObstacle(long x, long y, entity_t *my);
void actAnimator(entity_t *my);
void actPlayer(entity_t *my);
void actPGem(entity_t *my);
void actRGem(entity_t *my);
void actBullet(entity_t *my);
void actGlean(entity_t *my);
void actTroll(entity_t *my);
void actParticle(entity_t *my);