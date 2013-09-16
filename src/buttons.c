/*-------------------------------------------------------------------------------

	INFANT STAR
	File: buttons.c
	Desc: contains code for all buttons in the editor and game

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "infantstar.h"
#include "editor.h"

// Corner buttons

void buttonExit(button_t *my) {
	button_t *button;
	
	// this shouldn't work if a window is already open
	if( subwindow )
		return;
	
	menuVisible = 0;
	subwindow=1;
	subx1=xres/2-128;
	subx2=xres/2+128;
	suby1=yres/2-32;
	suby2=yres/2+32;
	strcpy(subtext,"Are you sure you want to quit?\n\nAny unsaved work will be lost.");
	
	button = newButton();
	strcpy(button->label,"Yes");
	button->x=subx1+32; button->y=suby2-24;
	button->sizex=32; button->sizey=16;
	button->action=&buttonExitConfirm;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"No");
	button->x=subx2-56; button->y=suby2-24;
	button->sizex=24; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
}

void buttonExitConfirm(button_t *my) {
	mainloop=0; // gracefully stops the game/editor
}

void buttonIconify(button_t *my) {
	// aka minimize
	SDL_WM_IconifyWindow();
}

// Toolbox buttons

void buttonTilePalette(button_t *my) {
	tilepalette=1;
}

void buttonSprite(button_t *my) {
	spritepalette=1;
}

void buttonPoint(button_t *my) {
	selectedTool=0;
}

void buttonBrush(button_t *my) {
	selectedTool=1;
}

void buttonSelect(button_t *my) {
	selectedTool=2;
}

void buttonFill(button_t *my) {
	selectedTool=3;
}

// File menu

void buttonFile(button_t *my) {
	if( menuVisible != 1 )
		menuVisible = 1;
	else
		menuVisible = 0;
}

void buttonNew(button_t *my) {
	button_t *button;
	
	snprintf(widthtext, 4, "%d", map.width);
	snprintf(heighttext, 4, "%d", map.height);
	strcpy(nametext,map.name);
	strcpy(authortext,map.author);
	cursorflash=0;
	menuVisible=0;
	subwindow=1;
	newwindow=1;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-80;
	suby2=yres/2+80;
	strcpy(subtext,"New map:");
	
	button = newButton();
	strcpy(button->label,"Create");
	button->x=subx2-64; button->y=suby2-48;
	button->sizex=56; button->sizey=16;
	button->action=&buttonNewConfirm;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"Cancel");
	button->x=subx2-64; button->y=suby2-24;
	button->sizex=56; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
}

void buttonNewConfirm(button_t *my) {
	int x, y, z;
	free(map.tiles);
	list_FreeAll(&entity_l);
	strcpy(map.name,nametext);
	strcpy(map.author,authortext);
	map.width=atoi(widthtext);
	map.height=atoi(heighttext);
	map.width=min(max(MINWIDTH,map.width),MAXWIDTH);
	map.height=min(max(MINHEIGHT,map.height),MAXHEIGHT);
	map.tiles=(int *) malloc(sizeof(int)*MAPLAYERS*map.height*map.width);
	for( z=0; z<MAPLAYERS; z++ ) {
		for( y=0; y<map.height; y++ ) {
			for( x=0; x<map.width; x++ ) {
				if(z==0)
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 1;
				else if(z==2) {
					if(x==0||y==0||x==map.width-1||y==map.height-1)
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 2;
					else
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
				}
				else
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
			}
		}
	}
	strcpy(message,"                             Created a new map.");
	messagetime=60;
	buttonCloseSubwindow(my);
}

void buttonOpen(button_t *my) {
	button_t *button;
	DIR *dir=NULL;
	struct dirent *ent=NULL;
	unsigned long c=0;

	cursorflash=0;
	d_names_length=0;
	menuVisible=0;
	subwindow=1;
	openwindow=1;
	slidery=0;
	selectedFile=0;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-120;
	suby2=yres/2+120;
	strcpy(subtext,"Open file:");
	
	button = newButton();
	strcpy(button->label," Open ");
	button->x=subx2-64; button->y=suby2-48;
	button->sizex=56; button->sizey=16;
	button->action=&buttonOpenConfirm;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"Cancel");
	button->x=subx2-64; button->y=suby2-24;
	button->sizex=56; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	// file list
	if( (dir=opendir("maps/")) != NULL ) {
		while( (ent=readdir(dir)) != NULL ) {
			if( strstr(ent->d_name,".imp") != NULL || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".") )
				d_names_length++;
		}
		closedir(dir);
	} else {
		// could not open directory
		fprintf(stderr,"failed to open map directory for viewing!\n");
	}
	d_names = (char **) malloc(sizeof(char *)*d_names_length);
	for( c=0; c<d_names_length; c++ )
		d_names[c] = (char *) malloc(sizeof(char)*FILENAME_MAX);
	c=0;
	if( (dir=opendir("maps/")) != NULL ) {
		while( (ent=readdir(dir)) != NULL ) {
			if( strstr(ent->d_name,".imp") != NULL || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".") ) {
				strcpy(d_names[c],ent->d_name);
				c++;
			}
		}
		closedir(dir);
	} else {
		// could not open directory
		fprintf(stderr,"failed to open map directory for viewing!\n");
	}
}

void buttonOpenConfirm(button_t *my) {
	int c, c2;
	strcpy(message,"");
	for( c=0; c<32; c++ ) {
		if(filename[c]==0)
			break;
	}
	for( c2=0; c2<32-c; c2++ )
		strcat(message," ");
	if(loadMap(filename)) {
		strcat(message,"Failed to open ");
		strcat(message,filename);
	} else {
		strcat(message,"      Opened '");
		strcat(message,filename);
		strcat(message,"'");
	}
	messagetime=60; // 60*50 ms = 3000 ms (3 seconds)
	buttonCloseSubwindow(my);
}

void buttonSave(button_t *my) {
	int c, c2;
	menuVisible=0;
	if(!strcmp(filename,""))
		buttonSaveAs(my);
	else {
		strcpy(message,"");
		for( c=0; c<32; c++ ) {
			if(filename[c]==0)
				break;
		}
		for( c2=0; c2<32-c; c2++ )
			strcat(message," ");
		if(saveMap(filename)) {
			strcat(message,"Failed to save ");
			strcat(message,filename);
		} else {
			strcat(message,"       Saved '");
			strcat(message,filename);
			strcat(message,"'");
		}
		messagetime=60; // 60*50 ms = 3000 ms (3 seconds)
		buttonCloseSubwindow(my);
	}
}

void buttonSaveAs(button_t *my) {
	button_t *button;
	DIR *dir;
	struct dirent *ent;
	unsigned long c=0;

	cursorflash=0;
	d_names_length=0;
	menuVisible=0;
	subwindow=1;
	savewindow=1;
	slidery=0;
	selectedFile=0;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-120;
	suby2=yres/2+120;
	strcpy(subtext,"Save file:");
	
	button = newButton();
	strcpy(button->label," Save ");
	button->x=subx2-64; button->y=suby2-48;
	button->sizex=56; button->sizey=16;
	button->action=&buttonSave;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"Cancel");
	button->x=subx2-64; button->y=suby2-24;
	button->sizex=56; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	// file list
	if( (dir=opendir("maps/")) != NULL ) {
		while( (ent=readdir(dir)) != NULL ) {
			if( strstr(ent->d_name,".imp") != NULL || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".") )
				d_names_length++;
		}
		closedir(dir);
	} else {
		// could not open directory
		fprintf(stderr,"failed to open map directory for viewing!\n");
	}
	d_names = (char **) malloc(sizeof(char *)*d_names_length);
	for( c=0; c<d_names_length; c++ )
		d_names[c] = (char *) malloc(sizeof(char)*FILENAME_MAX);
	c=0;
	if( (dir=opendir("maps/")) != NULL ) {
		while( (ent=readdir(dir)) != NULL ) {
			if( strstr(ent->d_name,".imp") != NULL || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".") ) {
				strcpy(d_names[c],ent->d_name);
				c++;
			}
		}
		closedir(dir);
	} else {
		// could not open directory
		fprintf(stderr,"failed to open map directory for viewing!\n");
	}
}

// Edit menu

void buttonEdit(button_t *my) {
	if( menuVisible != 2 )
		menuVisible = 2;
	else
		menuVisible = 0;
}

void buttonCut(button_t *my) {
	menuVisible=0;
}

void buttonCopy(button_t *my) {
	menuVisible=0;
}

void buttonPaste(button_t *my) {
	menuVisible=0;
}

void buttonDelete(button_t *my) {
	menuVisible=0;
	
	// delete the selected entity, if there is one
	if(selectedEntity != NULL) {
		list_RemoveNode(selectedEntity->node);
		selectedEntity=NULL;
	}
}

void buttonSelectAll(button_t *my) {
	menuVisible=0;
}

// View menu

void buttonView(button_t *my) {
	if( menuVisible != 3 )
		menuVisible = 3;
	else
		menuVisible = 0;
}

void buttonToolbox(button_t *my) {
	toolbox = (toolbox==0);
	butTilePalette->visible = (butTilePalette->visible==0);
	butSprite->visible = (butSprite->visible==0);
	butPoint->visible = (butPoint->visible==0);
	butBrush->visible = (butBrush->visible==0);
	butSelect->visible = (butSelect->visible==0);
	butFill->visible = (butFill->visible==0);
}

void buttonStatusBar(button_t *my) {
	statusbar = (statusbar==0);
}

void buttonAllLayers(button_t *my) {
	alllayers = (alllayers==0);
}

void buttonViewSprites(button_t *my) {
	viewsprites = (viewsprites==0);
}

void buttonGrid(button_t *my) {
	showgrid = (showgrid==0);
}

void buttonFullscreen(button_t *my) {
	fullscreen = (fullscreen==0);
	if(!fullscreen) {
		free(palette);
		palette = (int *) malloc(sizeof(unsigned int)*xres*yres);
		if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_RESIZABLE )) == NULL) {
			fprintf(stderr, "failed to set video mode.\n");
		}
	}
	else {
		xres=640; yres=400;
		free(palette);
		palette = (int *) malloc(sizeof(unsigned int)*xres*yres);
		if((screen=SDL_SetVideoMode( xres, yres, 32, SDL_HWSURFACE | SDL_FULLSCREEN )) == NULL) {
			fprintf(stderr, "failed to set video mode.\n");
		}
	}
}

// Map menu

void buttonMap(button_t *my) {
	if( menuVisible != 4 )
		menuVisible = 4;
	else
		menuVisible = 0;
}

void buttonAttributes(button_t *my) {
	button_t *button;
	
	snprintf(widthtext, 4, "%d", map.width);
	snprintf(heighttext, 4, "%d", map.height);
	strcpy(nametext,map.name);
	strcpy(authortext,map.author);
	cursorflash=0;
	menuVisible=0;
	subwindow=1;
	newwindow=1;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-80;
	suby2=yres/2+80;
	strcpy(subtext,"Map properties:");
	
	button = newButton();
	strcpy(button->label,"  OK  ");
	button->x=subx2-64; button->y=suby2-48;
	button->sizex=56; button->sizey=16;
	button->action=&buttonAttributesConfirm;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"Cancel");
	button->x=subx2-64; button->y=suby2-24;
	button->sizex=56; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
}

void buttonAttributesConfirm(button_t *my) {
	int x, y, z;
	map_t mapcopy;
	
	// make a copy of the current map
	mapcopy.width = map.width;
	mapcopy.height = map.height;
	mapcopy.tiles = (int *) malloc(sizeof(int)*MAPLAYERS*mapcopy.width*mapcopy.height);
	memcpy(mapcopy.tiles,map.tiles,sizeof(int)*MAPLAYERS*mapcopy.width*mapcopy.height);
	
	// allocate memory for a new map
	free(map.tiles);
	map.width=atoi(widthtext);
	map.height=atoi(heighttext);
	map.width=min(max(MINWIDTH,map.width),MAXWIDTH);
	map.height=min(max(MINHEIGHT,map.height),MAXHEIGHT);
	map.tiles=(int *) malloc(sizeof(int)*MAPLAYERS*map.height*map.width);
	strcpy(map.name,nametext);
	strcpy(map.author,authortext);
	
	// transfer data from the new map to the old map and fill extra space with empty data
	for( z=0; z<MAPLAYERS; z++ ) {
		for( y=0; y<map.height; y++ ) {
			for( x=0; x<map.width; x++ ) {
				if( x >= mapcopy.width ) {
					if(z==0)
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 1;
					else
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
				}
				else if( y >= mapcopy.height ) {
					if(z==0)
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 1;
					else
						map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
				}
				else if( x < mapcopy.width && y < mapcopy.height ) {
					map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = mapcopy.tiles[z + y*MAPLAYERS + x*MAPLAYERS*mapcopy.height];
				}
			}
		}
	}
	free(mapcopy.tiles);
	strcpy(message,"                       Modified map attributes.");
	messagetime=60;
	buttonCloseSubwindow(my);
}

void buttonClearMap(button_t *my) {
	button_t *button;
	
	menuVisible = 0;
	subwindow=1;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-56;
	suby2=yres/2+56;
	strcpy(subtext,"Warning:\n\nThis option will completely erase your\nentire map.\n\nAre you sure you want to continue?\n");
	
	button = newButton();
	strcpy(button->label,"OK");
	button->x=subx1+64; button->y=suby2-24;
	button->sizex=24; button->sizey=16;
	button->action=&buttonClearMapConfirm;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"Cancel");
	button->x=subx2-112; button->y=suby2-24;
	button->sizex=56; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
}

void buttonClearMapConfirm(button_t *my) {
	long x, y, z;
	for( z=0; z<MAPLAYERS; z++ ) {
		for( y=0; y<map.height; y++ ) {
			for( x=0; x<map.width; x++ ) {
				map.tiles[z + y*MAPLAYERS + x*MAPLAYERS*map.height] = 0;
			}
		}
	}
	buttonCloseSubwindow(my);
}

// Help menu

void buttonHelp(button_t *my) {
	if( menuVisible != 5 )
		menuVisible = 5;
	else
		menuVisible = 0;
}

void buttonAbout(button_t *my) {
	button_t *button;
	
	menuVisible = 0;
	subwindow=1;
	subx1=xres/2-160;
	subx2=xres/2+160;
	suby1=yres/2-56;
	suby2=yres/2+56;
	strcpy(subtext,"Infant Star: Map Editor v0.8\n\nSee EDITING for full documentation.\n\nThis software is copyright 2013 (c)\nSheridan Rathbun, all rights reserved.\n\nSee LICENSE for details.\n");
	
	button = newButton();
	strcpy(button->label,"OK");
	button->x=xres/2-12; button->y=suby2-24;
	button->sizex=24; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
	
	button = newButton();
	strcpy(button->label,"X");
	button->x=subx2-16; button->y=suby1;
	button->sizex=16; button->sizey=16;
	button->action=&buttonCloseSubwindow;
	button->visible=1;
	button->focused=1;
}

// Subwindows
void buttonCloseSubwindow(button_t *my) {
	int c;
	
	// close window
	subwindow=0;
	newwindow=0;
	openwindow=0;
	savewindow=0;
	editproperty=0;
	if( d_names != NULL ) {
		for( c=0; c<d_names_length; c++ )
			if( d_names[c] != NULL ) {
				free(d_names[c]);
				d_names[c]=NULL;
			}
		free(d_names);
		d_names=NULL;
	}
}