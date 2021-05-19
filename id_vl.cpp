// ID_VL.C

//#include <string.h>
#include "wl_def.h"

// Uncomment the following line, if you get destination out of bounds
// assertion errors and want to ignore them during debugging
#define IGNORE_BAD_DEST

#ifdef IGNORE_BAD_DEST
//#undef assert
//#define assert(x) if(!(x))  { slPrint((char *)"asset test failed0", slLocate(10,20));return;}
#define assert1(x) if(!(x)) { slPrint((char *)"asset test failed1", slLocate(10,20));return;}
#define assert2(x) if(!(x)) { slPrint((char *)"asset test failed2", slLocate(10,20));return;}
#define assert3(x) if(!(x)) { slPrint((char *)"asset test failed3", slLocate(10,20));return;}
#define assert4(x) if(!(x)) { slPrint((char *)"asset test failed4", slLocate(10,20));return;}
#define assert5(x) if(!(x)) { slPrint((char *)"asset test failed5", slLocate(10,20));return;}
#define assert6(x) if(!(x)) { slPrint((char *)"asset test failed6", slLocate(10,20));return;}
//#define assert7(x) if(!(x)) { slPrint((char *)"asset test failed7", slLocate(10,20));return;}
#define assert8(x) if(!(x)) { slPrint((char *)"asset test failed8", slLocate(10,20));return;}
#define assert_ret(x) if(!(x)) return 0
#else
#define assert_ret(x) assert(x)
#endif

//boolean fullscreen = true;
#if defined(_arch_dreamcast)
unsigned screenWidth = 320;
unsigned screenHeight = 200;
unsigned screenBits = 8;
#elif defined(GP2X)
unsigned screenWidth = 320;
unsigned screenHeight = 240;
unsigned screenBits = 8;
#else
unsigned screenWidth = SATURN_WIDTH;
unsigned screenHeight = 240;
unsigned screenBits = 8;      // use "best" color depth according to libSDL
#endif

SDL_Surface *screen = NULL;
SDL_Surface *screenBuffer = NULL;
SDL_Surface *curSurface = NULL;

unsigned curPitch;

unsigned scaleFactor;

boolean	 screenfaded;

SDL_Color curpal[256];

#define CASSERT(x) extern int ASSERT_COMPILE[((x) != 0) * 2 - 1];
#undef RGB
#define RGB(r, g, b) {(r)*255/63, (g)*255/63, (b)*255/63, 0}
//#define RGB(r, g, b) (r<<2,g<<2,b<<2,0)

SDL_Color gamepal[]={
#ifdef SPEAR
    #include "sodpal.inc"
#else
    #include "wolfpal.inc"
#endif
};

//CASSERT(lengthof(gamepal) == 256)

//===========================================================================


/*
=======================
=
= VL_Shutdown
=
=======================
*/

void	VL_Shutdown (void)
{
	//VL_SetTextMode ();
}


/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void	VL_SetVGAPlaneMode (void)
{
    if(screenBits == -1)
    {
        const SDL_VideoInfo *vidInfo = SDL_GetVideoInfo();
        screenBits = vidInfo->vfmt->BitsPerPixel;
    }
	
	if(screen==NULL)
	{
		screen = SDL_SetVideoMode(screenWidth, screenHeight, screenBits,
			SDL_SWSURFACE | (screenBits == 8 ? SDL_HWPALETTE : 0) );
	}
//slPrintHex(screen->pixels,slLocate(20,8));	
    if(!screen)
    {
//        printf("Unable to set %ix%ix%i video mode: %s\n", screenWidth,
//            screenHeight, screenBits, SDL_GetError());
        SYS_Exit(1);
    }

    screenBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, screenWidth, screenHeight, 8, 0, 0, 0, 0);
    if(!screenBuffer)
    {
//        printf("Unable to create screen buffer surface: %s\n", SDL_GetError());
        SYS_Exit(1);
    }

//    SDL_ShowCursor(SDL_DISABLE);
    curSurface = screenBuffer;
    curPitch = screenBuffer->pitch;
//    SDL_SetColors(screen, gamepal, 0, 256);
    SDL_SetColors(curSurface, gamepal, 0, 256);
    memcpy(curpal, gamepal, sizeof(SDL_Color) * 256);


//    SDL_SetColors(screenBuffer, gamepal, 0, 256);

    scaleFactor = screenWidth/320;
    if(screenHeight/200 < scaleFactor) scaleFactor = screenHeight/200;
    pixelangle = (short *) malloc(screenWidth * sizeof(short));
    CHECKMALLOCRESULT(pixelangle);
    wallheight = (short *) malloc(screenWidth * sizeof(short));
    CHECKMALLOCRESULT(wallheight);
}

/*
=============================================================================

						PALETTE OPS

		To avoid snow, do a WaitVBL BEFORE calling these

=============================================================================
*/

/*
=================
=
= VL_ConvertPalette
=
=================
*/

void VL_ConvertPalette(byte *srcpal, SDL_Color *destpal, int numColors)
{
    for(int i=0; i<numColors; i++)
    {
        destpal[i].r = *srcpal++ * 255 / 63;
        destpal[i].g = *srcpal++ * 255 / 63;
        destpal[i].b = *srcpal++ * 255 / 63;
    }
}

/*
=================
=
= VL_FillPalette
=
=================
*/

void VL_FillPalette (int red, int green, int blue)
{
	unsigned short i;
	SDL_Color palo[256];

	for(i=0; i<256; i++)
	{
	    palo[i].r = red;
	    palo[i].g = green;
	    palo[i].b = blue;
	}

	VL_SetPalette(palo);
}

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette (SDL_Color *palette)
{
    memcpyl(curpal, palette, sizeof(SDL_Color) * 256);
 //   SDL_SetPalette(screen, SDL_PHYSPAL, palette, 0, 256);
//    SDL_SetPalette(curSurface, SDL_PHYSPAL, palette, 0, 256);
}


//===========================================================================

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette (SDL_Color *palette)
{
    memcpyl(palette, curpal, sizeof(SDL_Color) * 256);
}


//===========================================================================

/*
=================
=
= VL_FadeOut
=
= Fades the current palette to the given color in the given number of steps
=
=================
*/

void VL_FadeOut (int start, int end, int red, int green, int blue, int steps)
{
	int		    i,j,orig,delta;
	SDL_Color palette1[256], palette2[256];	
	SDL_Color   *origptr, *newptr;

    red = red * 255 / 63;
    green = green * 255 / 63;
    blue = blue * 255 / 63;

	VL_GetPalette(palette1);
	VL_WaitVBL(1);	
	memcpyl(palette2, palette1, sizeof(SDL_Color) * 256);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		origptr = &palette1[start];
		newptr = &palette2[start];
		for (j=start;j<=end;j++)
		{
			orig = origptr->r;
			delta = red-orig;
			newptr->r = orig + delta * i / steps;
			orig = origptr->g;
			delta = green-orig;
			newptr->g = orig + delta * i / steps;
			orig = origptr->b;
			delta = blue-orig;
			newptr->b = orig + delta * i / steps;
			origptr++;
			newptr++;
		}

		VL_SetPalette (palette2);
		VL_WaitVBL(1);		
// xxx		VGAClearScreen(); // vbt : maj du fond d'écran
	}

//
// final color
//
	VL_FillPalette (red,green,blue);

	screenfaded = true;
}


/*
=================
=
= VL_FadeIn
=
=================
*/

void VL_FadeIn (int start, int end, SDL_Color *palette, int steps)
{
	SDL_Color palette1[256], palette2[256];	
	int i,j,delta;

	VL_GetPalette(palette1);
	VL_WaitVBL(1);	
	memcpyl(palette2, palette1, sizeof(SDL_Color) * 256);

//
// fade through intermediate frames
//
	for (i=0;i<steps;i++)
	{
		for (j=start;j<=end;j++)
		{
			delta = palette[j].r-palette1[j].r;
			palette2[j].r = palette1[j].r + delta * i / steps;
			delta = palette[j].g-palette1[j].g;
			palette2[j].g = palette1[j].g + delta * i / steps;
			delta = palette[j].b-palette1[j].b;
			palette2[j].b = palette1[j].b + delta * i / steps;
		}

		VL_SetPalette(palette2);
		VL_WaitVBL(1);		
// xxx		VGAClearScreen(); // vbt : maj du fond d'écran
	}

//
// final color
//
	VL_SetPalette (palette);
	screenfaded = false;
}

/*
=============================================================================

							PIXEL OPS

=============================================================================
*/

byte *VL_LockSurface(SDL_Surface *surface)
{
    return (byte *) surface->pixels;
}

/*
=================
=
= VL_GetPixel
=
=================
*/

byte VL_GetPixel (int x, int y)
{
    assert_ret(x >= 0 && (unsigned) x < screenWidth
            && y >= 0 && (unsigned) y < screenHeight
            && "VL_GetPixel: Pixel out of bounds!");

//    VL_LockSurface(curSurface);
	return ((byte *) curSurface->pixels)[y * curPitch + x];
//    VL_UnlockSurface(curSurface);
//	return col;
}

/*
=================
=
= VL_Hlin
=
=================
*/

void VL_Hlin (unsigned x, unsigned y, unsigned width, int color)
{
    assert1(x >= 0 && x + width <= screenWidth
            && y >= 0 && y < screenHeight
            && "VL_Hlin: Destination rectangle out of bounds!");

    VL_LockSurface(curSurface);
    Uint8 *dest = ((byte *) curSurface->pixels) + y * curPitch + x;
    memset(dest, color, width);
//    VL_UnlockSurface(curSurface); // vbt on vire pour voir
}

/*
=================
=
= VL_Vlin
=
=================
*/

void VL_Vlin (int x, int y, int height, int color)
{
	assert2(x >= 0 && (unsigned) x < screenWidth
			&& y >= 0 && (unsigned) y + height <= screenHeight
			&& "VL_Vlin: Destination rectangle out of bounds!");

	VL_LockSurface(curSurface);
	Uint8 *dest = ((byte *) curSurface->pixels) + y * curPitch + x;

	while (height--)
	{
		*dest = color;
		dest += curPitch;
	}
//	VL_UnlockSurface(curSurface);// vbt on vire pour voir
}

/*
=================
=
= VL_Bar
=
=================
*/

void VL_BarScaledCoord (int scx, int scy, int scwidth, int scheight, int color)
{
	assert3(scx >= 0 && (unsigned) scx + scwidth <= screenWidth
			&& scy >= 0 && (unsigned) scy + scheight <= screenHeight
			&& "VL_BarScaledCoord: Destination rectangle out of bounds!");

	VL_LockSurface(curSurface);
	Uint8 *dest = ((byte *) curSurface->pixels) + scy * curPitch + scx;

	while (scheight--)
	{
		memset(dest, color, scwidth);
		dest += curPitch;
	}
	VL_UnlockSurface(curSurface);// vbt utile
}

/*
============================================================================

							MEMORY OPS

============================================================================
*/

/*
=================
=
= VL_MemToLatch
=
=================
*/

void VL_MemToLatch(byte *source, int width, int height,
    SDL_Surface *destSurface, int x, int y)
{
    assert4(x >= 0 && (unsigned) x + width <= screenWidth
            && y >= 0 && (unsigned) y + height <= screenHeight
            && "VL_MemToLatch: Destination rectangle out of bounds!");

//    VL_LockSurface(destSurface);
    int pitch = destSurface->pitch;
    byte *dest = (byte *) destSurface->pixels + y * pitch + x;
    for(int ysrc = 0; ysrc < height; ysrc++)
    {
        for(int xsrc = 0; xsrc < width; xsrc++)
        {
            dest[xsrc] = source[(ysrc * (width >> 2) + (xsrc >> 2))
                + (xsrc & 3) * (width >> 2) * height];
        }
		dest+=pitch;
    }	 
//    VL_UnlockSurface(destSurface); // vbt : evite l'affichage des chargements au boot
}

//===========================================================================


/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a block of data to the screen with scaling according to scaleFactor.
=
=================
*/
// vbt : à améliorer
void VL_MemToScreenScaledCoord (byte *source, int width, int height, int destx, int desty)
{
    assert5(destx >= 0 && destx + width * scaleFactor <= screenWidth
            && desty >= 0 && desty + height * scaleFactor <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    VL_LockSurface(curSurface);
	byte *vbuf = (byte *)curSurface->pixels+(desty*curPitch)+destx;
	
	if(scaleFactor == 1)
    {
		for(unsigned int j=0; j<height; j++)
		{
			for(unsigned int i=0; i<width; i++)
			{
				byte col = source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
				vbuf[i] = col;
			}
			vbuf+=curPitch;
		}
    }		
	else
	{
		for(unsigned int j=0,scj=0; j<height; j++, scj+=scaleFactor)
		{
			for(unsigned int i=0,sci=0; i<width; i++, sci+=scaleFactor)
			{
				byte col = source[(j*(width>>2)+(i>>2))+(i&3)*(width>>2)*height];
				for(unsigned m=0; m<scaleFactor; m++)
				{
					for(unsigned n=0; n<scaleFactor; n++)
					{
						vbuf[(scj+m)*curPitch+sci+n] = col;
					}
				}
			}
		}
	}
    VL_UnlockSurface(curSurface); // vbt utile pour signon screen
}

/*
=================
=
= VL_MemToScreenScaledCoord
=
= Draws a part of a block of data to the screen.
= The block has the size origwidth*origheight.
= The part at (srcx, srcy) has the size width*height
= and will be painted to (destx, desty) with scaling according to scaleFactor.
=
=================
*/
// vbt : à améliorer
void VL_MemToScreenScaledCoord (byte *source, int origwidth, int origheight, int srcx, int srcy,
                                int destx, int desty, int width, int height)
{
    assert6(destx >= 0 && destx + width * scaleFactor <= screenWidth
            && desty >= 0 && desty + height * scaleFactor <= screenHeight
            && "VL_MemToScreenScaledCoord: Destination rectangle out of bounds!");

    VL_LockSurface(curSurface);
    byte *vbuf = (byte *) curSurface->pixels;
    for(int j=0,scj=0; j<height; j++, scj+=scaleFactor)
    {
        for(int i=0,sci=0; i<width; i++, sci+=scaleFactor)
        {
            byte col = source[((j+srcy)*(origwidth>>2)+((i+srcx)>>2))+((i+srcx)&3)*(origwidth>>2)*origheight];
            for(unsigned m=0; m<scaleFactor; m++)
            {
                for(unsigned n=0; n<scaleFactor; n++)
                {
                    vbuf[(scj+m+desty)*curPitch+sci+n+destx] = col;
                }
            }
        }
    }
    VL_UnlockSurface(curSurface); // vbt utile
}
//==========================================================================

/*
=================
=
= VL_LatchToScreen
=
=================
*/
// vbt à améliorer
void VL_LatchToScreenScaledCoord(SDL_Surface *source, int xsrc, int ysrc,
    int width, int height, int scxdest, int scydest)
{
	assert(scxdest >= 0 && scxdest + width * scaleFactor <= screenWidth
		&& scydest >= 0 && scydest + height * scaleFactor <= screenHeight
		&& "VL_LatchToScreenScaledCoord: Destination rectangle out of bounds!");
		
	if(scaleFactor == 1)
    {
		SDL_Rect srcrect = { xsrc, ysrc, width, height };
		SDL_Rect destrect = { scxdest, scydest, 0, 0 }; // width and height are ignored
		SDL_BlitSurface(source, &srcrect, curSurface, &destrect);
    }
    else
    {
        VL_LockSurface(source);
        byte *src = (byte *) source->pixels;
        unsigned srcPitch = source->pitch;

        VL_LockSurface(curSurface);
        byte *vbuf = (byte *) curSurface->pixels;
        for(int j=0,scj=0; j<height; j++, scj+=scaleFactor)
        {
            for(int i=0,sci=0; i<width; i++, sci+=scaleFactor)
            {
                byte col = src[(ysrc + j)*srcPitch + xsrc + i];
				for(unsigned m=0; m<scaleFactor; m++)
                {
                    for(unsigned n=0; n<scaleFactor; n++)
                    {
                        vbuf[(scydest+scj+m)*curPitch+scxdest+sci+n] = col;
                    }
                }
            }
        }
        VL_UnlockSurface(curSurface);
 //       VL_UnlockSurface(source);
    }	
}
