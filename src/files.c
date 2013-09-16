/*-------------------------------------------------------------------------------

	INFANT STAR
	File: files.c
	Desc: contains code for map i/o

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"

/*-------------------------------------------------------------------------------

	loadMap
	
	Loads a map from the given filename

-------------------------------------------------------------------------------*/

int loadMap(char *filename2) {
	FILE *fp;
	char valid_data[11];
	unsigned int numentities;
	unsigned int c;
	entity_t *entity;
	unsigned int sprite;
	char *filename;
	
	if( filename2 != NULL && strcmp(filename2,"") ) {
		c=0;
		while(1) {
			if(filename2[c]==0)
				break;
			c++;
		}
		filename = (char *) malloc(sizeof(char)*(c+14));
		strcpy(filename,"maps/");
		strcat(filename,filename2);
		
		if( strcmp(filename,"..") && strcmp(filename,".") ) {
			// add extension if missing
			if( strstr(filename,".imp") == NULL )
				strcat(filename,".imp");
			
			// load the file!
			fprintf(stderr,"opening map file '%s'...\n",filename);
			if((fp = fopen(filename, "rb")) == NULL) {
				fprintf(stderr,"failed to open file '%s' for map loading!\n",filename);
				return 1;
			}
		} else {
			fprintf(stderr,"failed to open file '%s' for map loading!\n",filename);
			return 1;
		}
		fread(valid_data, sizeof(char), strlen("INFANTSTAR"), fp);
		if( strncmp(valid_data,"INFANTSTAR",strlen("INFANTSTAR")) ) {
			fprintf(stderr,"file '%s' is an invalid map file.\n",filename);
			fclose(fp);
			return 1;
		}
		if( map.tiles != NULL )
			free(map.tiles);
		fread(map.name, sizeof(char), 32, fp); // map name
		fread(map.author, sizeof(char), 32, fp); // map author
		fread(&map.width, sizeof(unsigned int), 1, fp); // map width
		fread(&map.height, sizeof(unsigned int), 1, fp); // map height
		map.tiles = (int *) malloc(sizeof(int)*map.width*map.height*MAPLAYERS);
		fread(map.tiles, sizeof(int), map.width*map.height*MAPLAYERS, fp);
		list_FreeAll(&entity_l);
		fread(&numentities,sizeof(unsigned int), 1, fp); // number of entities on the map
		for(c=0; c<numentities; c++) {
			fread(&sprite,sizeof(unsigned int), 1, fp);
			entity = newEntity(sprite,0);
			fread(&entity->x,sizeof(long), 1, fp);
			fread(&entity->y,sizeof(long), 1, fp);
		}
		free(filename);
		fclose(fp);
		
		// reset camera
		camx=0;
		camy=0;
		return 0;
	} else
		return 1;
}

/*-------------------------------------------------------------------------------

	saveMap
	
	Saves a map to the given filename

-------------------------------------------------------------------------------*/

int saveMap(char *filename2) {
	FILE *fp;
	unsigned int numentities=0;
	node_t *node;
	entity_t *entity;
	char *filename;
	unsigned int c;
	
	if( filename2 != NULL && strcmp(filename2,"") ) {
		c=0;
		while(1) {
			if(filename2[c]==0)
				break;
			c++;
		}
		filename = (char *) malloc(sizeof(char)*(c+10));
		strcpy(filename,"maps/");
		strcat(filename,filename2);
		
		if( strstr(filename,".imp") == NULL )
			strcat(filename,".imp");
		fprintf(stderr,"saving map file '%s'...\n",filename);
		if((fp = fopen(filename, "wb")) == NULL) {
			fprintf(stderr,"failed to open file '%s' for map saving!\n",filename);
			return 1;
		}

		fwrite("INFANTSTAR", sizeof(char), strlen("INFANTSTAR"), fp); // magic code
		fwrite(map.name, sizeof(char), 32, fp); // map filename
		fwrite(map.author, sizeof(char), 32, fp); // map author
		fwrite(&map.width, sizeof(unsigned int), 1, fp); // map width
		fwrite(&map.height, sizeof(unsigned int), 1, fp); // map height
		fwrite(map.tiles, sizeof(int), map.width*map.height*MAPLAYERS, fp);
		for(node=entity_l.first;node!=NULL;node=node->next)
			numentities++;
		fwrite(&numentities,sizeof(unsigned int), 1, fp); // number of entities on the map
		for(node=entity_l.first;node!=NULL;node=node->next) {
			entity = (entity_t *) node->element;
			fwrite(&entity->sprite,sizeof(unsigned int), 1, fp);
			fwrite(&entity->x,sizeof(long), 1, fp);
			fwrite(&entity->y,sizeof(long), 1, fp);
		}
		fclose(fp);
		free(filename);
		return 0;
	} else
		return 1;
}