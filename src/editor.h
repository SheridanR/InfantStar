/*-------------------------------------------------------------------------------

	INFANT STAR
	File: editor.h
	Desc: header file for the editor

	Copyright 2013 (c) Sheridan Rathbun, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

// for file browsing
#include <dirent.h>

#define MAXWIDTH 2000
#define MAXHEIGHT 2000
#define MINWIDTH 4
#define MINHEIGHT 4

// button definitions
extern button_t *butX;
extern button_t *but_;
extern button_t *butTilePalette;
extern button_t *butSprite;
extern button_t *butPoint;
extern button_t *butBrush;
extern button_t *butSelect;
extern button_t *butFill;
extern button_t *butFile;
extern button_t *butNew;
extern button_t *butOpen;
extern button_t *butSave;
extern button_t *butSaveAs;
extern button_t *butExit;
extern button_t *butEdit;
extern button_t *butCut;
extern button_t *butCopy;
extern button_t *butPaste;
extern button_t *butDelete;
extern button_t *butSelectAll;
extern button_t *butView;
extern button_t *butToolbox;
extern button_t *butStatusBar;
extern button_t *butAllLayers;
extern button_t *butViewSprites;
extern button_t *butGrid;
extern button_t *butFullscreen;
extern button_t *butMap;
extern button_t *butAttributes;
extern button_t *butClearMap;
extern button_t *butHelp;
extern button_t *butAbout;
extern int menuVisible;
extern int subwindow;
extern int subx1, subx2, suby1, suby2;
extern char subtext[1024];
extern int toolbox;
extern int statusbar;
extern int viewsprites;
extern int alllayers;
extern int showgrid;
extern int selectedTile;
extern int tilepalette;
extern int spritepalette;
extern int selectedTool;
extern int openwindow, savewindow, newwindow;
extern int slidery;
extern int slidersize;
extern int selectedFile;
extern char **d_names;
unsigned long d_names_length;
extern char filename[32];
extern int messagetime;
extern char message[48];
extern int cursorflash;
extern char widthtext[4], heighttext[4], nametext[32], authortext[32];
extern int editproperty;
extern int *palette;

// function prototypes for buttons.c:
void buttonExit(button_t *my);
void buttonExitConfirm(button_t *my);
void buttonIconify(button_t *my);
void buttonTilePalette(button_t *my);
void buttonSprite(button_t *my);
void buttonPoint(button_t *my);
void buttonBrush(button_t *my);
void buttonSelect(button_t *my);
void buttonFill(button_t *my);
void buttonFile(button_t *my);
void buttonNewConfirm(button_t *my);
void buttonNew(button_t *my);
void buttonOpenConfirm(button_t *my);
void buttonOpen(button_t *my);
void buttonSaveConfirm(button_t *my);
void buttonSave(button_t *my);
void buttonSaveAs(button_t *my);
void buttonExit(button_t *my);
void buttonEdit(button_t *my);
void buttonCut(button_t *my);
void buttonCopy(button_t *my);
void buttonPaste(button_t *my);
void buttonDelete(button_t *my);
void buttonSelectAll(button_t *my);
void buttonView(button_t *my);
void buttonToolbox(button_t *my);
void buttonStatusBar(button_t *my);
void buttonAllLayers(button_t *my);
void buttonViewSprites(button_t *my);
void buttonGrid(button_t *my);
void buttonFullscreen(button_t *my);
void buttonMap(button_t *my);
void buttonAttributes(button_t *my);
void buttonAttributesConfirm(button_t *my);
void buttonClearMap(button_t *my);
void buttonClearMapConfirm(button_t *my);
void buttonHelp(button_t *my);
void buttonAbout(button_t *my);
void buttonCloseSubwindow(button_t *my);