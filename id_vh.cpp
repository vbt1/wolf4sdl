#include "wl_def.h"


pictabletype	*pictable; //[NUMPICS];
SDL_Surface     *latchpics[NUMLATCHPICS];
int	    px,py;
byte	fontcolor,backcolor;
int	    fontnumber;

//==========================================================================

void VWB_DrawPropString(const char* string)
{
	fontstruct  *font;
	int		    width, step, height;
	byte	    *source, *dest;
	byte	    ch;

    byte *vbuf = LOCK();

	font = (fontstruct *) grsegs[STARTFONT+fontnumber];
	font->height=SWAP_BYTES_16(font->height);
	height =font->height;//SWAP_BYTES_16(font->height);//font->height;//font->height;//font->height;//SWAP_BYTES_16(font->height);

	dest = vbuf + scaleFactor * (py * curPitch + px);

	while ((ch = (byte)*string++)!=0)
	{
		width = step = font->width[ch];
		source = ((byte *)font)+SWAP_BYTES_16(font->location[ch]);

		while (width--)
		{
			for(int i=0;i<height;i++)
			{
				if(source[i*step])
                {
                    for(unsigned sy=0; sy<scaleFactor; sy++)
                        for(unsigned sx=0; sx<scaleFactor; sx++)
        					dest[(scaleFactor*i+sy)*curPitch+sx]=fontcolor;
                }
			}
		
			source++;
			px++;
			dest+=scaleFactor;
		}
	}

	font->height=SWAP_BYTES_16(font->height);

	UNLOCK();
}

/*
=================
=
= VL_MungePic
=
=================
*/
#if 1
void VL_MungePic (byte *source, unsigned width, unsigned height)
{
	unsigned x,y,plane,size,pwidth;
	byte *temp, *dest, *srcline;

	size = width*height;

	if (width&3)
		Quit ("VL_MungePic: Not divisable by 4!");

//
// copy the pic to a temp buffer
//
	temp=(byte *)SATURN_CHUNK_ADDR; //malloc(size);
//	temp=(byte *)malloc(size);
    CHECKMALLOCRESULT(temp);
	memcpy (temp,source,size);
//	memcpyl (temp,source,size);

//
// munge it back into the original buffer
//
	dest = source;
	pwidth = width/4;

	for (plane=0;plane<4;plane++)
	{
		srcline = temp;
		for (y=0;y<height;y++)
		{
			for (x=0;x<pwidth;x++)
				*dest++ = *(srcline+x*4+plane);
			srcline+=width;
		}
	}

//	free(temp);
	temp = NULL;
}
#endif

void VWL_MeasureString (const char *string, word *width, word *height, fontstruct *font)
{
	*height = SWAP_BYTES_16(font->height);

	for (*width = 0;*string;string++)
		*width += font->width[*((byte *)string)];	// proportional width
}

void VW_MeasurePropString (const char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

/*
=============================================================================

				Double buffer management routines

=============================================================================
*/
#ifndef USE_SPRITES
void VH_UpdateScreen()
{
	
//	SDL_BlitSurface(screenBuffer, NULL, screen, NULL);
//	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
#endif

void VWB_DrawTile8 (int x, int y, int tile)
{
	LatchDrawChar(x,y,tile);
}
/*
void VWB_DrawTile8M (int x, int y, int tile)
{
	VL_MemToScreen (((byte *)grsegs[STARTTILE8M])+tile*64,8,8,x,y);
}
*/
void VWB_DrawPic (int x, int y, int chunknum)
{
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	x &= ~7;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VL_MemToScreen (grsegs[chunknum],width,height,x,y);
}

void VWB_DrawPicScaledCoord (int scx, int scy, int chunknum)
{
	
	int	picnum = chunknum - STARTPICS;
	unsigned width,height;

	width = pictable[picnum].width;
	height = pictable[picnum].height;

    VL_MemToScreenScaledCoord (grsegs[chunknum],width,height,scx,scy);
}


void VWB_Bar (int x, int y, int width, int height, int color)
{
	VW_Bar (x,y,width,height,color);
}
/*
void VWB_Plot (int x, int y, int color)
{
    if(scaleFactor == 1)
        VW_Plot(x,y,color);
    else
        VW_Bar(x, y, 1, 1, color);
}
*/
void VWB_Hlin (int x1, int x2, int y, int color)
{
    if(scaleFactor == 1)
    	VW_Hlin(x1,x2,y,color);
    else
        VW_Bar(x1, y, x2-x1+1, 1, color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
    if(scaleFactor == 1)
		VW_Vlin(y1,y2,x,color);
    else
        VW_Bar(x, y1, 1, y2-y1+1, color);
}


/*
=============================================================================

						WOLFENSTEIN STUFF

=============================================================================
*/

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic (unsigned x, unsigned y, unsigned picnum)
{
// pas super utile pour l'instant	
	VL_LatchToScreen (latchpics[2+picnum-LATCHPICS_LUMP_START], x*8, y);
}

void LatchDrawPicScaledCoord (unsigned scx, unsigned scy, unsigned picnum)
{
//	if(2+picnum-LATCHPICS_LUMP_START==1)
//		return;
	VL_LatchToScreenScaledCoord (latchpics[2+picnum-LATCHPICS_LUMP_START], scx*8, scy);
}

void LatchDrawPicScaledCoordIndirect (unsigned scx, unsigned scy, unsigned picnum)
{
//	if(2+picnum-LATCHPICS_LUMP_START==1)
//		return;
	VL_LatchToScreenScaledCoordIndirect (latchpics[2+picnum-LATCHPICS_LUMP_START], scx*8, scy);
//void VL_LatchToScreenScaledCoordIndirect(SDL_Surface *source, int xsrc, int ysrc, int width, int height, int scxdest, int scydest);	
}
/*
void inline VL_LatchToScreen (SDL_Surface *source, int x, int y)
{
   VL_LatchToScreenScaledCoord(source,0,0,source->w,source->h,
        scaleFactor*x,scaleFactor*y);
}
*/


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem (void)
{
	int	i,width,height,start,end;
	byte *src;
	SDL_Surface *surf; //,*surf1;
#if 0
//
// tile 8s
//
   surf = SDL_CreateRGBSurface(SDL_HWSURFACE, 8*8,
        ((NUMTILE8 + 7) / 8) * 8, 8, 0, 0, 0, 0);
    if(surf == NULL)
    {
        Quit("Unable to create surface for tiles!");
    }
    SDL_SetColors(surf, gamepal, 0, 256);

	latchpics[0] = surf;
	
	CA_CacheGrChunk (STARTTILE8);
	src = grsegs[STARTTILE8];
	for (i=0;i<NUMTILE8;i++)
	{
		VL_MemToLatch (src, 8, 8, surf, (i & 7) * 8, (i >> 3) * 8);
		src += 64;
	}	
	UNCACHEGRCHUNK (STARTTILE8);
	
	latchpics[1] = surf;
#endif	
//
// pics
//
	start = LATCHPICS_LUMP_START;
	end = LATCHPICS_LUMP_END;
   
	for (i=start;i<=end;i++)
	{
		width = pictable[i-STARTPICS].width;
		height = pictable[i-STARTPICS].height;
		
		surf = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 8, 0, 0, 0, 0);
        if(surf == NULL)
        {
            Quit("Unable to create surface for picture!");
        }			  
 //       SDL_SetColors(surf, gamepal, 0, 256);
		latchpics[2+i-start] = surf;
		CA_CacheGrChunk (i);
		VL_MemToLatch (grsegs[i], width, height, surf, 0, 0);
		UNCACHEGRCHUNK(i);
// vbt 26/07/2020 free remis	
// vbt 15/08/2020 pas de free c'est les images de la barre de statut	
//		free(surf);
//		surf=NULL;
	}	
// vbt 26/07/2020 free remis	
// vbt 15/08/2020 utilisation de lowworkram
//free(surf1);
//surf1=NULL;
}

//==========================================================================

/*
===================
=
= FizzleFade
=
= returns true if aborted
=
= It uses maximum-length Linear Feedback Shift Registers (LFSR) counters.
= You can find a list of them with lengths from 3 to 168 at:
= http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
= Many thanks to Xilinx for this list!!!
=
===================
*/

// XOR masks for the pseudo-random number sequence starting with n=17 bits
static const uint32_t rndmasks[] = {
                    // n    XNOR from (starting at 1, not 0 as usual)
    0x00012000,     // 17   17,14
    0x00020400,     // 18   18,11
    0x00040023,     // 19   19,6,2,1
    0x00090000,     // 20   20,17
    0x00140000,     // 21   21,19
    0x00300000,     // 22   22,21
    0x00420000,     // 23   23,18
    0x00e10000,     // 24   24,23,22,17
    0x01200000,     // 25   25,22      (this is enough for 8191x4095)
};

static unsigned int rndbits_y;
static unsigned int rndmask;

//extern SDL_Color curpal[256];

// Returns the number of bits needed to represent the given value
static int log2_ceil(uint32_t x)
{
    int n = 0;
    uint32_t v = 1;
    while(v < x)
    {
        n++;
        v <<= 1;
    }
    return n;
}

void VH_Startup()
{
    int rndbits_x = log2_ceil(screenWidth);
    rndbits_y = log2_ceil(screenHeight);

    int rndbits = rndbits_x + rndbits_y;
    if(rndbits < 17)
        rndbits = 17;       // no problem, just a bit slower
    else if(rndbits > 25)
        rndbits = 25;       // fizzle fade will not fill whole screen

    rndmask = rndmasks[rndbits - 17];
}

boolean FizzleFade (SDL_Surface *source, SDL_Surface *dest,	int x1, int y1,
    unsigned width, unsigned height, unsigned frames, boolean abortable)
{
#if 1
    unsigned x, y, frame, pixperframe;
    int32_t  rndval, lastrndval;
    int      first = 1;

    lastrndval = 0;
    pixperframe = width * height / frames;

    IN_StartAck ();

	frame = GetTimeCount();
	byte *srcptr = (byte *)source->pixels;
	byte color = (srcptr[x1+(y1*width)]?0:4);

	SDL_Rect rect = { x1,y1,width,height };
	
	if(curSurface==screen)
	{
		curSurface = screenBuffer;
		VL_BarScaledCoord (x1,y1,width,height,srcptr[x1+(y1*width)]); // vbt ajout
		DrawStatusBar(); // vbt : ajout
	}
	else
	{
		VL_BarScaledCoord (x1,y1,width,height,color); // vbt ajout		
		DrawStatusBar(); // vbt : ajout
		curSurface = screen;
	}

    do
    {
        if(abortable && IN_CheckAck ())
        {
#ifndef USE_SPRITES
//            SDL_BlitSurface(screenBuffer, NULL, screen, NULL);
//            SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif
// xxx			VGAClearScreen(); // vbt : maj du fond d'écran
			//curSurface = source;
			VL_BarScaledCoord (x1,y1,width,height,color); // vbt ajout
            return true;
        }


		byte *destptr = (byte *)dest->pixels;

        rndval = lastrndval;

        // When using double buffering, we have to copy the pixels of the last AND the current frame.
        // Only for the first frame, there is no "last frame"
        for(int i = first; i < 2; i++)
        {

            for(unsigned p = 0; p < pixperframe; p++)
            {
                //
                // seperate random value into x/y pair
                //

                x = rndval >> rndbits_y;
                y = rndval & ((1 << rndbits_y) - 1);

                //
                // advance to next random element
                //

                rndval = (rndval >> 1) ^ (rndval & 1 ? 0 : rndmask);

                if(x >= width || y >= height)
                {
                    if(rndval == 0)     // entire sequence has been completed
                        goto finished;
                    p--;
                    continue;
                }

                //
                // copy one pixel
                //

				*(destptr + (y1 + y) * dest->pitch + x1 + x) = *(srcptr + (y1 + y) * source->pitch + x1 + x);

                if(rndval == 0)		// entire sequence has been completed
                    goto finished;
            }

            if(!i || first) lastrndval = rndval;
        }
		first = 0;
		
		SDL_BlitSurface(dest, &rect, source, &rect);
		frame++;

       Delay(frame-GetTimeCount());        // don't go too fast
	} while (1);
	

finished:
	SDL_BlitSurface(dest, &rect, source, &rect);
	return false;
#endif
}

