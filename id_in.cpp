//
//	ID Engine
//	ID_IN.c - Input Manager
//	v1.0d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with the various input devices
//
//	Depends on: Memory Mgr (for demo recording), Sound Mgr (for timing stuff),
//				User Mgr (for command line parms)
//
//	Globals:
//		LastScan - The keyboard scan code of the last key pressed
//		LastASCII - The ASCII value of the last key pressed
//	DEBUG - there are more globals
//

#include "wl_def.h"


/*
=============================================================================

					GLOBAL VARIABLES

=============================================================================
*/


//
// configuration variables
//

// 	Global variables
volatile boolean    Keyboard[SDLK_LAST];
volatile boolean	Paused;
volatile ScanCode	LastScan;

static KeyboardDef KbdDefs = {
    sc_Control,             // button0
    sc_Alt,                 // button1
    sc_Home,                // upleft
    sc_UpArrow,             // up
    sc_PgUp,                // upright
    sc_LeftArrow,           // left
    sc_RightArrow,          // right
    sc_End,                 // downleft
    sc_DownArrow,           // down
    sc_PgDn                 // downright
};
/*
=============================================================================

					LOCAL VARIABLES

=============================================================================
*/
static	boolean		IN_Started;

static	Direction	DirTable[] =		// Quick lookup for total direction
{
    dir_NorthWest,	dir_North,	dir_NorthEast,
    dir_West,		dir_None,	dir_East,
    dir_SouthWest,	dir_South,	dir_SouthEast
};

static void processEvent(SDL_Event *event)
{
    switch (event->type)
    {
        // exit if the window is closed
        case SDL_QUIT:
            Quit(NULL);

        // check for keypresses
        case SDL_KEYDOWN:
        {
            LastScan = event->key.keysym.sym;

            if(LastScan == SDLK_KP_ENTER) LastScan = SDLK_RETURN;
            else if(LastScan == SDLK_RSHIFT) LastScan = SDLK_LSHIFT;
            else if(LastScan == SDLK_RALT) LastScan = SDLK_LALT;
            else if(LastScan == SDLK_RCTRL) LastScan = SDLK_LCTRL;

            if(LastScan<SDLK_LAST)
                Keyboard[LastScan] = 1;
            if(LastScan == SDLK_PAUSE)
                Paused = true;
            break;
		}

        case SDL_KEYUP:
        {
            int key = event->key.keysym.sym;
            if(key == SDLK_KP_ENTER) key = SDLK_RETURN;
            else if(key == SDLK_RSHIFT) key = SDLK_LSHIFT;
            else if(key == SDLK_RALT) key = SDLK_LALT;
            else if(key == SDLK_RCTRL) key = SDLK_LCTRL;			   

            if(key<SDLK_LAST)
                Keyboard[key] = 0;
            break;
        }

#ifdef GP2X
        case SDL_JOYBUTTONDOWN:
            GP2X_ButtonDown(event->jbutton.button);
            break;

        case SDL_JOYBUTTONUP:
            GP2X_ButtonUp(event->jbutton.button);
            break;
#endif
    }
}

void IN_WaitAndProcessEvents()
{
    SDL_Event event;
	SDL_PollEvent(0,13,&event);
//    if(!SDL_WaitEvent(&event)) return;
//    do
    {
        processEvent(&event);
    }
//    while(SDL_PollEvent(&event));
	IN_ClearKeysDown();
}

void IN_ProcessEvents()
{
    SDL_Event event1,event2;

	SDL_PollEvent(4,13,&event1);
    processEvent(&event1);
	SDL_PollEvent(0,4,&event2);
    processEvent(&event2);
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_Startup() - Starts up the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
void
IN_Startup(void)
{
	if (IN_Started)
		return;
    IN_ClearKeysDown();
    IN_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_Shutdown() - Shuts down the Input Mgr
//
///////////////////////////////////////////////////////////////////////////
/*
void
IN_Shutdown(void)
{
	if (!IN_Started)
		return;
	IN_Started = false;
}
*/
///////////////////////////////////////////////////////////////////////////
//
//	IN_ClearKeysDown() - Clears the keyboard array
//
///////////////////////////////////////////////////////////////////////////
void
IN_ClearKeysDown(void)
{
	LastScan = sc_None;
	memset ((void *) Keyboard,0,sizeof(Keyboard));
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_ReadControl() - Reads the device associated with the specified
//		player and fills in the control info struct
//
///////////////////////////////////////////////////////////////////////////
void
IN_ReadControl(int player,ControlInfo *info)
{
	word		buttons;
	int			dx,dy;
	Motion		mx,my;

	dx = dy = 0;
	mx = my = motion_None;
	buttons = 0;

	IN_ProcessEvents();

    if (Keyboard[KbdDefs.upleft])
        mx = motion_Left,my = motion_Up;
    else if (Keyboard[KbdDefs.upright])
        mx = motion_Right,my = motion_Up;
    else if (Keyboard[KbdDefs.downleft])
        mx = motion_Left,my = motion_Down;
    else if (Keyboard[KbdDefs.downright])
        mx = motion_Right,my = motion_Down;

    if (Keyboard[KbdDefs.up])
        my = motion_Up;
    else if (Keyboard[KbdDefs.down])
        my = motion_Down;

    if (Keyboard[KbdDefs.left])
        mx = motion_Left;
    else if (Keyboard[KbdDefs.right])
        mx = motion_Right;

    if (Keyboard[KbdDefs.button0])
        buttons += 1 << 0;
    if (Keyboard[KbdDefs.button1])
        buttons += 1 << 1;

	dx = mx * 127;
	dy = my * 127;

	info->x = dx;
	info->xaxis = mx;
	info->y = dy;
	info->yaxis = my;
	info->button0 = (buttons & (1 << 0)) != 0;
	info->button1 = (buttons & (1 << 1)) != 0;
	info->button2 = (buttons & (1 << 2)) != 0;
	info->button3 = (buttons & (1 << 3)) != 0;
	info->dir = DirTable[((my + 1) * 3) + (mx + 1)];
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_WaitForKey() - Waits for a scan code, then clears LastScan and
//		returns the scan code
//
///////////////////////////////////////////////////////////////////////////
/*
ScanCode
IN_WaitForKey(void)
{
	ScanCode	result;

	while ((result = LastScan)==0)
		IN_WaitAndProcessEvents();
	LastScan = 0;
	return(result);
}
*/
///////////////////////////////////////////////////////////////////////////
//
//	IN_Ack() - waits for a button or key press.  If a button is down, upon
// calling, it must be released for it to be recognized
//
///////////////////////////////////////////////////////////////////////////
void IN_StartAck(void)
{
    IN_ProcessEvents();
//
// get initial state of everything
//
	IN_ClearKeysDown();
}


boolean IN_CheckAck (void)
{
    IN_ProcessEvents();
//
// see if something has been pressed
//
	if(LastScan)
		return true;
	return false;
}


void IN_Ack (void)
{
	IN_StartAck ();

    do
    {
        IN_WaitAndProcessEvents();
    }
	while(!IN_CheckAck ());
}


///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. If the clear flag is set, it
//		then either clears the key or waits for the user to let the mouse
//		button up.
//
///////////////////////////////////////////////////////////////////////////
boolean IN_UserInput(longword delay)
{
	longword	lasttime;

	lasttime = GetTimeCount();
	IN_StartAck ();
	do
	{
        IN_ProcessEvents();
		if (IN_CheckAck())
			return true;
        SDL_Delay(5);
	} while (GetTimeCount() - lasttime < delay);
	return(false);
}

//===========================================================================
