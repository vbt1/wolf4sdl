#ifndef		_IMAGES_H_
#define		_IMAGES_H_

#include <SGL.H>

typedef struct {
	Sint16 x, y;
	Uint16 w, h;
} SDL_Rect;

typedef struct {
	Uint16 w, h;
	char *data;
} Image;
Image images[];
/*
extern unsigned short palette[];
*/
enum {img_grid, img_o, img_start_1ply, img_start_2ply, img_title, img_turn_o, img_turn_x, img_win_draw, img_win_o, img_win_x, img_x};




#endif	
/*_IMAGES_H_	*/
