#ifndef		_SDL_H_
#define		_SDL_H_


//#ifdef __cplusplus
extern "C" {
//#endif
	   
#include "sgl.h"

#include "sega_sys.h"
#include "sega_gfs.h"
#include "sega_tim.h"
//#ifdef __cplusplus

 void SYS_CHGSCUIM_NR(Uint32 AndMask,Uint32 OrMask);
}
//#endif

#undef cgaddress
#undef pal
#undef TEXDEF

#define	cgaddress	0x8000 //SpriteBufSize
#define	cgaddress8	cgaddress/8
#define pal1 COL_256
#define TEXDEF(h,v,presize)		{h,v,(cgaddress+(((presize)*4)>>(pal1)))/8,(((h)&0x1f8)<<5 | (v))}

//#include "images.h"
#include "SDL_keysym.h"
#include "SDL_types.h"

#define		NBG1_MAP_ADR		( VDP2_VRAM_B1 + 0x18000 )
#define     BACK_COL_ADR    (VDP2_VRAM_A1 + 0x1fffe)
#define     NBG1_CEL_ADR    (VDP2_VRAM_B0 + 0x00000)
#define     NBG1_COL_ADR    (VDP2_COLRAM  + 0x00200)
#define		TEX_COL_ADR		(VDP2_COLRAM  + 0x00200)
#define		NBG0_COL_ADR    (VDP2_COLRAM  + 0x00000)
#define     NBG0_CEL_ADR    (VDP2_VRAM_A0 + 0x00000)

#define PCM_MSK4(a)				((a)&0x000F)
#define PCM_MSK10(a)			((a)&0x03FF)

#define PCM_SCSP_FREQUENCY					(44100L)

#define PCM_CALC_OCT(smpling_rate) 											\
		((Sint32)logtbl[PCM_SCSP_FREQUENCY / ((smpling_rate) + 1)])

/* ?V?t?g?õT???g????v?Z */
#define PCM_CALC_SHIFT_FREQ(oct)											\
		(PCM_SCSP_FREQUENCY >> (oct))

/* ?e?m?r??v?Z */
#define PCM_CALC_FNS(smpling_rate, shift_freq)								\
		((((smpling_rate) - (shift_freq)) << 10) / (shift_freq))

/* SATURN Sound Driver ?p?????[?^???l */
#define PCM_SET_STMNO(para)													\
		((Uint8)PCM_MSK3((para)->pcm_stream_no))
#define PCM_SET_LEVEL_PAN(para)												\
		((Uint8)((PCM_MSK3((para)->pcm_level) << 5) | PCM_MSK5((para)->pcm_pan)))
#define PCM_SET_LEVEL_PAN2(level, pan)										\
		((Uint8)((PCM_MSK3(level) << 5) | PCM_MSK5(pan)))
#define PCM_SET_PITCH_WORD(oct, fns)										\
		((Uint16)((PCM_MSK4(-(oct)) << 11) | PCM_MSK10(fns)))
#define PCM_SET_PCM_SIZE(para) 	((para)->pcm_size)


//#define  ACTION_REPLAY



#ifndef ACTION_REPLAY
#define MAX_OPEN        24 //a ne pas modifier
#define MAX_DIR         96//384
#else
#define MAX_OPEN        2
#define MAX_DIR         1
#endif

#ifndef ACTION_REPLAY
extern GfsDirName dir_name[MAX_DIR];
#endif

//#define VBT

#ifdef VBT
#define SYS_CDINIT1(i) \
((**(void(**)(int))0x60002dc)(i))

#define SYS_CDINIT2() \
((**(void(**)(void))0x600029c)())
#endif


//enum {NONE, SDL_QUIT, SDL_KEYDOWN, SDL_KEYUP,SDLK_ESCAPE2, SDL_MOUSEBUTTONDOWN};
/* Event enumerations */
enum { SDL_NOEVENT = 0,			/* Unused (do not remove) */
       SDL_ACTIVEEVENT,			/* Application loses/gains visibility */
       SDL_KEYDOWN,			/* Keys pressed */
       SDL_KEYUP,			/* Keys released */
       SDL_MOUSEMOTION,			/* Mouse moved */
       SDL_MOUSEBUTTONDOWN,		/* Mouse button pressed */
       SDL_MOUSEBUTTONUP,		/* Mouse button released */
       SDL_JOYAXISMOTION,		/* Joystick axis motion */
       SDL_JOYBALLMOTION,		/* Joystick trackball motion */
       SDL_JOYHATMOTION,		/* Joystick hat position change */
       SDL_JOYBUTTONDOWN,		/* Joystick button pressed */
       SDL_JOYBUTTONUP,			/* Joystick button released */
       SDL_QUIT,			/* User-requested quit */
       SDL_SYSWMEVENT,			/* System specific event */
       SDL_EVENT_RESERVEDA,		/* Reserved for future use.. */
       SDL_EVENT_RESERVEDB,		/* Reserved for future use.. */
       SDL_VIDEORESIZE,			/* User resized video mode */
       SDL_VIDEOEXPOSE,			/* Screen needs to be redrawn */
       SDL_EVENT_RESERVED2,		/* Reserved for future use.. */
       SDL_EVENT_RESERVED3,		/* Reserved for future use.. */
       SDL_EVENT_RESERVED4,		/* Reserved for future use.. */
       SDL_EVENT_RESERVED5,		/* Reserved for future use.. */
       SDL_EVENT_RESERVED6,		/* Reserved for future use.. */
       SDL_EVENT_RESERVED7,		/* Reserved for future use.. */
       /* Events SDL_USEREVENT through SDL_MAXEVENTS-1 are for your use */
       SDL_USEREVENT = 24,
       /* This last event is only for bounding internal arrays
	  It is the number of bits in the event mask datatype -- Uint32
        */
       SDL_NUMEVENTS = 32
};

typedef enum {
	SDL_GRAB_QUERY = -1,
	SDL_GRAB_OFF = 0,
	SDL_GRAB_ON = 1,
	SDL_GRAB_FULLSCREEN	/* Used internally */
} SDL_GrabMode;

#define SDL_FULLSCREEN 0
#define SDL_DISABLE 0
#define SDL_SRCCOLORKEY 0
#define SDL_HWPALETTE	0x20000000	/* Surface has exclusive palette */

#define	SDL_INIT_TIMER		0x00000001
#define SDL_INIT_AUDIO		0x00000010
#define SDL_INIT_VIDEO		0x00000020
#define SDL_INIT_CDROM		0x00000100
#define SDL_INIT_JOYSTICK	0x00000200
#define SDL_INIT_NOPARACHUTE	0x00100000	/* Don't catch fatal signals */
#define SDL_INIT_EVENTTHREAD	0x01000000	/* Not supported on all OS's */
#define SDL_INIT_EVERYTHING	0x0000FFFF
#define SDL_MIX_MAXVOLUME 128

#define AUDIO_U8	0x0008	/* Unsigned 8-bit samples */
#define AUDIO_S8	0x8008	/* Signed 8-bit samples */
#define AUDIO_U16LSB	0x0010	/* Unsigned 16-bit samples */
#define AUDIO_S16LSB	0x8010	/* Signed 16-bit samples */
#define AUDIO_U16MSB	0x1010	/* As above, but big-endian byte order */
#define AUDIO_S16MSB	0x9010	/* As above, but big-endian byte order */
#define AUDIO_U16       AUDIO_U16LSB
#define AUDIO_S16       AUDIO_S16LSB

#define SDL_SWSURFACE	0x00000000	/* Surface is in system memory */
#define SDL_HWSURFACE	0x00000001	/* Surface is in video memory */
#define SDL_ASYNCBLIT	0x00000004	/* Use asynchronous blits if possible */

/* flags for SDL_SetPalette() */
#define SDL_LOGPAL 0x01
#define SDL_PHYSPAL 0x02

#define SDL_IGNORE	 0

 //enum { SDL_PRESSED = 0x01, SDL_RELEASED = 0x00 };

#define SDL_BUTTON(X)		(SDL_PRESSED<<(X-1))
#define SDL_BUTTON_LEFT		1
#define SDL_BUTTON_MIDDLE	2
#define SDL_BUTTON_RIGHT	3
#define SDL_BUTTON_WHEELUP	4
#define SDL_BUTTON_WHEELDOWN	5
#define SDL_BUTTON_LMASK	SDL_BUTTON(SDL_BUTTON_LEFT)
#define SDL_BUTTON_MMASK	SDL_BUTTON(SDL_BUTTON_MIDDLE)
#define SDL_BUTTON_RMASK	SDL_BUTTON(SDL_BUTTON_RIGHT)

#define SDL_HAT_CENTERED	0x00
#define SDL_HAT_UP		0x01
#define SDL_HAT_RIGHT		0x02
#define SDL_HAT_DOWN		0x04
#define SDL_HAT_LEFT		0x08
#define SDL_HAT_RIGHTUP		(SDL_HAT_RIGHT|SDL_HAT_UP)
#define SDL_HAT_RIGHTDOWN	(SDL_HAT_RIGHT|SDL_HAT_DOWN)
#define SDL_HAT_LEFTUP		(SDL_HAT_LEFT|SDL_HAT_UP)
#define SDL_HAT_LEFTDOWN	(SDL_HAT_LEFT|SDL_HAT_DOWN)


#define MIX_CHANNELS 8
 
#define SDL_LockMutex(m)	SDL_mutexP(m)
#define SDL_UnlockMutex(m)	SDL_mutexV(m)

typedef struct {
	Sint16 x, y;
	Uint16 w, h;
} SDL_Rect;

typedef struct {
	Uint8 type;
	struct {
		struct {
			Uint16 sym;
		} keysym;
	} key;
/*	struct {
		Uint16 x,y;
	} button;*/
} SDL_Event;

typedef struct {
	Uint8 format;
} SDL_Screen;

typedef struct {
	Uint8 r;
	Uint8 g;
	Uint8 b;
//	Uint8 unused;
} SDL_Color;

//typedef Uint16 SDL_Color;

typedef struct {
//	int       ncolors;
	SDL_Color *colors;
} SDL_Palette;

/* Everything in the pixel format structure is read-only */
typedef struct SDL_PixelFormat {
	Uint8  BitsPerPixel;
#if 0	
	SDL_Palette *palette;	
	Uint8  BytesPerPixel;
	Uint8  Rloss;
	Uint8  Gloss;
	Uint8  Bloss;
	Uint8  Aloss;
	Uint8  Rshift;
	Uint8  Gshift;
	Uint8  Bshift;
	Uint8  Ashift;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;

	/* RGB color key information */
	Uint32 colorkey;
	/* Alpha value information (per-surface alpha) */
	Uint8  alpha;
#endif	
} SDL_PixelFormat;

/*
extern SDL_Screen *screen;
 */
typedef struct SDL_Surface {
//	Uint32 flags;				/* Read-only */
//	SDL_PixelFormat *format;		/* Read-only */
	unsigned int w, h;				/* Read-only */
	Uint16 pitch;				/* Read-only */
	void *pixels;				/* Read-write */
//	int offset;				/* Private */

	/* Hardware-specific surface info */
	//struct private_hwdata *hwdata;

	/* clipping information */
//	SDL_Rect clip_rect;			/* Read-only */
	//Uint32 unused1;				/* for binary compatibility */

	/* Allow recursive locks */
//	Uint32 locked;				/* Private */

	/* info for fast blit mapping to other surfaces */
//	struct SDL_BlitMap *map;		/* Private */

	/* format version, bumped at every change to invalidate blit maps */
//	unsigned int format_version;		/* Private */

	/* Reference count -- used when freeing surface */
//	int refcount;				/* Read-mostly */
} SDL_Surface;

/* The calculated values in this structure are calculated by SDL_OpenAudio() */
	
typedef struct {
	int freq;		/* DSP frequency -- samples per second */
	Uint16 format;		/* Audio data format */
	Uint8  channels;	/* Number of channels: 1 mono, 2 stereo */
//	Uint8  silence;		/* Audio buffer silence value (calculated) */
//	Uint16 samples;		/* Audio buffer size in samples (power of 2) */
//	Uint16 padding;		/* Necessary for some compile environments */
//	Uint32 size;		/* Audio buffer size in bytes (calculated) */
	/* This function is called when the audio device needs more data.
	   'stream' is a pointer to the audio data buffer
	   'len' is the length of that buffer in bytes.
	   Once the callback returns, the buffer will no longer be valid.
	   Stereo samples are stored in a LRLRLR ordering.
	*/
//	void (*callback)(void *userdata, Uint8 *stream, int len);
//	void  *userdata;
} SDL_AudioSpec;

/*
struct SDL_mutex;
typedef struct SDL_mutex SDL_mutex;
struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;
*/
/* Useful for determining the video hardware capabilities */
typedef struct {
#if 0	
	Uint32 hw_available :1;	/* Flag: Can you create hardware surfaces? */
	Uint32 wm_available :1;	/* Flag: Can you talk to a window manager? */
	Uint32 UnusedBits1  :6;
	Uint32 UnusedBits2  :1;
	Uint32 blit_hw      :1;	/* Flag: Accelerated blits HW --> HW */
	Uint32 blit_hw_CC   :1;	/* Flag: Accelerated blits with Colorkey */
	Uint32 blit_hw_A    :1;	/* Flag: Accelerated blits with Alpha */
	Uint32 blit_sw      :1;	/* Flag: Accelerated blits SW --> HW */
	Uint32 blit_sw_CC   :1;	/* Flag: Accelerated blits with Colorkey */
	Uint32 blit_sw_A    :1;	/* Flag: Accelerated blits with Alpha */
	Uint32 blit_fill    :1;	/* Flag: Accelerated color fill */
	Uint32 UnusedBits3  :16;
	Uint32 video_mem;	/* The total amount of video memory (in K) */
#endif	
	SDL_PixelFormat *vfmt;	/* Value: The format of the video surface */
} SDL_VideoInfo;

 typedef struct Mix_Chunk {
//    int allocated;
    Uint8 *abuf;
    Uint32 alen;
//    Uint8 volume;     /* Per-sample volume, 0-128 */
} Mix_Chunk;

#define SDL_RLEACCEL	0x00004000	/* Surface is RLE encoded */
#define SDL_MUSTLOCK(surface)	\
  (surface->offset ||		\
  ((surface->flags & (SDL_HWSURFACE|SDL_ASYNCBLIT|SDL_RLEACCEL)) != 0))

#define SDL_ShowCursor(arg1){}
#define SDL_BlitSurface SDL_UpperBlit

extern int SDL_WaitEvent(SDL_Event *event);
extern void SDL_PollEvent(int start,int end, SDL_Event *event);
//extern void SDL_Delay(int delay);
extern void SDL_WM_SetCaption(const char *title, const char *icon);
extern const SDL_VideoInfo * SDL_GetVideoInfo(void);
extern SDL_Surface * SDL_SetVideoMode	 (int width, int height, int bpp, Uint32 flags);
extern char * SDL_GetError(void);
extern  int SDL_SetPalette(SDL_Surface *surface, int flags, SDL_Color *colors, int firstcolor, int ncolors);
extern int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
extern SDL_Surface * SDL_CreateRGBSurface (Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
extern void SDL_UpdateRects (SDL_Surface *screen, int numrects, SDL_Rect *rects);
extern void SDL_UpdateRect	(SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);
extern int SDL_UpperBlit (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
extern Uint32 SDL_GetTicks(void);
extern void SDL_Delay(long delay);
extern int SDL_LockSurface(SDL_Surface *surface);
extern void SDL_UnlockSurface(SDL_Surface *surface);
extern int SDL_Init(Uint32 flags);
extern void SDL_Quit(void);
extern int SDL_NumJoysticks(void);
extern int SDL_FillRect (SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
extern Uint32 SDL_MapRGB (SDL_PixelFormat *format, Uint8 r, Uint8 g, Uint8 b);
//extern int SDL_SaveBMP_RW (SDL_Surface *surface, SDL_RWops *dst, int freedst);

/* Convenience macro -- save a surface to a file */
#define SDL_SaveBMP(surface, file) \
		SDL_SaveBMP_RW(surface, SDL_RWFromFile(file, "wb"), 1)
/*
extern Uint8 SDL_GetMouseState(int *x, int *y);
extern void SDL_JoystickUpdate(void);
extern Sint16 SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis);
extern Uint8 SDL_JoystickGetHat(SDL_Joystick *joystick, int hat);
extern Uint8 SDL_JoystickGetButton(SDL_Joystick *joystick, int button);
extern SDL_Joystick * SDL_JoystickOpen(int device_index);
extern int SDL_JoystickNumHats(SDL_Joystick *joystick);
extern int SDL_JoystickNumButtons(SDL_Joystick *joystick);
extern void SDL_JoystickClose(SDL_Joystick *joystick);
extern void SDL_WarpMouse(Uint16 x, Uint16 y);
extern SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode);
extern SDLMod SDL_GetModState(void);
*/
extern Uint8 SDL_EventState(Uint8 type, int state);
extern void Mix_FreeChunk(Mix_Chunk *chunk);
//extern Mix_Chunk *Mix_LoadWAV_RW(SDL_RWops *src, int freesrc);
extern int Mix_PlayChannel (int channel, Mix_Chunk *chunk, int loops);
extern int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize);
extern void Mix_ChannelFinished(void (*channel_finished)(int channel));
extern int Mix_SetPanning(int channel, Uint8 left, Uint8 right);
extern int Mix_GroupAvailable(int tag);
extern int Mix_ReserveChannels(int num);
extern int Mix_GroupOldest(int tag);
extern int Mix_GroupChannels(int from, int to, int tag);
extern void Mix_HookMusic(void (*mix_func)(void *udata, Uint8 *stream, int len),void *arg);
extern char *Mix_GetError();
extern int Mix_HaltChannel(int channel);
//extern SDL_RWops *SDL_RWFromMem(void *mem, int size);
//extern SDL_RWops *SDL_RWFromFile(const char *file, const char *mode);


extern GfsHn GFS_Open(Sint32 fid);


#endif	// _SDL_H_

#ifdef VBT
/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL.h,v 1.7 2002/04/11 14:35:13 slouken Exp $";
#endif

/* Main include header for the SDL library */

#ifndef _SDL_H
#define _SDL_H

#include "SDL_main.h"
#include "SDL_types.h"
#include "SDL_getenv.h"
#include "SDL_error.h"
#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_cdrom.h"
#include "SDL_joystick.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "SDL_byteorder.h"
#include "SDL_version.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* As of version 0.5, SDL is loaded dynamically into the application */

/* These are the flags which may be passed to SDL_Init() -- you should
   specify the subsystems which you will be using in your application.
*/
#define	SDL_INIT_TIMER		0x00000001
#define SDL_INIT_AUDIO		0x00000010
#define SDL_INIT_VIDEO		0x00000020
#define SDL_INIT_CDROM		0x00000100
#define SDL_INIT_JOYSTICK	0x00000200
#define SDL_INIT_NOPARACHUTE	0x00100000	/* Don't catch fatal signals */
#define SDL_INIT_EVENTTHREAD	0x01000000	/* Not supported on all OS's */
#define SDL_INIT_EVERYTHING	0x0000FFFF



/* This function loads the SDL dynamically linked library and initializes 
 * the subsystems specified by 'flags' (and those satisfying dependencies)
 * Unless the SDL_INIT_NOPARACHUTE flag is set, it will install cleanup
 * signal handlers for some commonly ignored fatal signals (like SIGSEGV)
 */
extern DECLSPEC int SDLCALL SDL_Init(Uint32 flags);

/* This function initializes specific SDL subsystems */
extern DECLSPEC int SDLCALL SDL_InitSubSystem(Uint32 flags);

/* This function cleans up specific SDL subsystems */
extern DECLSPEC void SDLCALL SDL_QuitSubSystem(Uint32 flags);

/* This function returns mask of the specified subsystems which have
   been initialized.
   If 'flags' is 0, it returns a mask of all initialized subsystems.
*/
extern DECLSPEC Uint32 SDLCALL SDL_WasInit(Uint32 flags);

/* This function cleans up all initialized subsystems and unloads the
 * dynamically linked library.  You should call it upon all exit conditions.
 */
extern DECLSPEC void SDLCALL SDL_Quit(void);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif 
/* _SDL_H */

#endif