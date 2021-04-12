////////////////////////////////////////////////////////////////////
//
// WL_MENU.C
// by John Romero (C) 1992 Id Software, Inc.
//
////////////////////////////////////////////////////////////////////

#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

#include "wl_def.h"
#pragma hdrstop

extern void SYS_Exit(Sint32 code);

//extern int lastgamemusicoffset;
extern int numEpisodesMissing;

//
// PRIVATE PROTOTYPES
//
int CP_ReadThis (int);

#ifdef SPEAR
#define STARTITEM       newgame

#else
#ifdef GOODTIMES
#define STARTITEM       newgame

#else
#define STARTITEM       readthis
#endif
#endif

char endStrings[9][80] = {
#ifndef SPEAR
    {"Dost thou wish to\nleave with such hasty\nabandon?"},
    {"Chickening out...\nalready?"},
    {"Press B for more carnage.\nPress C to be a weenie."},
    {"So, you think you can\nquit this easily, huh?"},
    {"Press B to save the world.\nPress C to abandon it in\nits hour of need."},
    {"Press B if you are brave.\nPress C to cower in shame."},
    {"Heroes, Press B.\nWimps, press C."},
    {"You are at an intersection.\nA sign says, 'Press C to quit.'\n>"},
    {"For guns and glory, Press B.\nFor work and worry, press C."}
#else
    ENDSTR1,
    ENDSTR2,
    ENDSTR3,
    ENDSTR4,
    ENDSTR5,
    ENDSTR6,
    ENDSTR7,
    ENDSTR8,
    ENDSTR9
#endif
};

CP_itemtype MainMenu[] = {
#ifdef JAPAN
    {1, "", CP_NewGame},
    {1, "", CP_Sound},
    {1, "", CP_Control},
    {1, "", CP_LoadGame},
    {0, "", CP_SaveGame},
    {1, "", CP_ChangeView},
    {2, "", CP_ReadThis},
    {1, "", CP_ViewScores},
    {1, "", 0},
    {1, "", 0}
#else

    {1, STR_NG, CP_NewGame},
    {0, STR_SD, 0/*CP_Sound*/},
    {0, STR_CL, 0/*CP_Control*/},
    {0, STR_LG, 0/*CP_LoadGame*/},
    {0, STR_SG, 0/*CP_SaveGame*/},
    {1, STR_CV, CP_ChangeView},

#ifndef GOODTIMES
#ifndef SPEAR

#ifdef SPANISH
    {2, "Ve esto!", CP_ReadThis},
#else
    {2, "Read This!", CP_ReadThis},
#endif

#endif
#endif

    {1, STR_VS, CP_ViewScores},
    {0, STR_BD, 0},
    {0, STR_QT, 0}
#endif
};
/*
CP_itemtype SndMenu[] = {
#ifdef JAPAN
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {1, "", 0},
#else
    {1, STR_NONE, 0},
    {0, STR_PC, 0},
    {1, STR_ALSB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {0, STR_DISNEY, 0},
    {1, STR_SB, 0},
    {0, "", 0},
    {0, "", 0},
    {1, STR_NONE, 0},
    {1, STR_ALSB, 0}
#endif
};
*/
/*
enum { CTL_MOUSEENABLE, CTL_MOUSESENS, CTL_JOYENABLE, CTL_CUSTOMIZE };

CP_itemtype CtlMenu[] = {
#ifdef JAPAN
    {0, "", 0},
    {0, "", MouseSensitivity},
    {0, "", 0},
    {1, "", CustomControls}
#else
    {0, STR_MOUSEEN, 0},
    {0, STR_SENS, 0},
    {0, STR_JOYEN, 0},
    {1, STR_CUSTOM, 0}
#endif
};
*/
#ifndef SPEAR
CP_itemtype NewEmenu[] = {
#ifdef JAPAN
#ifdef JAPDEMO
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
    {0, "", 0},
#else
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0}
#endif
#else
#ifdef SPANISH
    {1, "Episodio 1\n" "Fuga desde Wolfenstein", 0},
    {0, "", 0},
    {3, "Episodio 2\n" "Operacion Eisenfaust", 0},
    {0, "", 0},
    {3, "Episodio 3\n" "Muere, Fuhrer, Muere!", 0},
    {0, "", 0},
    {3, "Episodio 4\n" "Un Negro Secreto", 0},
    {0, "", 0},
    {3, "Episodio 5\n" "Huellas del Loco", 0},
    {0, "", 0},
    {3, "Episodio 6\n" "Confrontacion", 0}
#else
    {1, "Episode 1\n" "Escape from Wolfenstein", 0},
    {0, "", 0},
    {3, "Episode 2\n" "Operation: Eisenfaust", 0},
    {0, "", 0},
    {3, "Episode 3\n" "Die, Fuhrer, Die!", 0},
    {0, "", 0},
    {3, "Episode 4\n" "A Dark Secret", 0},
    {0, "", 0},
    {3, "Episode 5\n" "Trail of the Madman", 0},
    {0, "", 0},
    {3, "Episode 6\n" "Confrontation", 0}
#endif
#endif
};
#endif


CP_itemtype NewMenu[] = {
#ifdef JAPAN
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0}
#else
    {1, STR_DADDY, 0},
    {1, STR_HURTME, 0},
    {1, STR_BRINGEM, 0},
    {1, STR_DEATH, 0}
#endif
};
/*
CP_itemtype LSMenu[] = {
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0},
    {1, "", 0}
};
*/
CP_itemtype CusMenu[] = {
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {0, "", 0},
    {1, "", 0},
    {0, "", 0},
    {1, "", 0}
};

// CP_iteminfo struct format: short x, y, amount, curpos, indent;
CP_iteminfo MainItems = { MENU_X, MENU_Y, lengthof(MainMenu), STARTITEM, 24 },
//            SndItems  = { SM_X, SM_Y1, lengthof(SndMenu), 0, 52 },
//            LSItems   = { LSM_X, LSM_Y, lengthof(LSMenu), 0, 24 },
//            CtlItems  = { CTL_X, CTL_Y, lengthof(CtlMenu), -1, 56 },
            CusItems  = { 8, CST_Y + 13 * 2, lengthof(CusMenu), -1, 0},
#ifndef SPEAR
            NewEitems = { NE_X, NE_Y, lengthof(NewEmenu), 0, 88 },
#endif
            NewItems  = { NM_X, NM_Y, lengthof(NewMenu), 2, 24 };

int color_hlite[] = {
    DEACTIVE,
    HIGHLIGHT,
    READHCOLOR,
    0x67
};

int color_norml[] = {
    DEACTIVE,
    TEXTCOLOR,
    READCOLOR,
    0x6b
};

int EpisodeSelect[6] = { 1 };

static int StartGame;
static int SoundStatus = 1;

////////////////////////////////////////////////////////////////////
//
// Wolfenstein Control Panel!  Ta Da!
//
////////////////////////////////////////////////////////////////////
void
US_ControlPanel (ScanCode scancode)
{
    int which;

    if (ingame)
    {
//        if (CP_CheckQuick (scancode))
//            return;
        StartCPMusic (MENUSONG);
    }
    else
        StartCPMusic (MENUSONG);
    SetupControlPanel ();
//VBT déplacé
	StartCPMusic (MENUSONG);
    //
    // F-KEYS FROM WITHIN GAME
    //
/*
    switch (scancode)
    {

        case sc_F1:
#ifdef SPEAR
            BossKey ();
#else
#ifdef GOODTIMES
            BossKey ();
#else
            HelpScreens ();
#endif
#endif
            goto finishup;
	   
        case sc_F2:
            CP_SaveGame (0);
            goto finishup;
		
        case sc_F3:
            CP_LoadGame (0);
            goto finishup;
		
        case sc_F4:
            CP_Sound (0);
            goto finishup;
		
        case sc_F5:
            CP_ChangeView (0);
            goto finishup;

        case sc_F6:
            CP_Control (0);
            goto finishup;
		 
        finishup:
            CleanupControlPanel ();
#ifdef SPEAR
            UnCacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif
            return;
    }
*/
#ifdef SPEAR
    CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif

    DrawMainMenu ();
    MenuFadeIn ();
    StartGame = 0;

    //
    // MAIN MENU LOOP
    //
    do
    {
        which = HandleMenu (&MainItems, &MainMenu[0], NULL);
#ifdef SPEAR
#ifndef SPEARDEMO
        IN_ProcessEvents();

        //
        // EASTER EGG FOR SPEAR OF DESTINY!
        //
        if (Keyboard[sc_I] && Keyboard[sc_D])
        {
            VW_FadeOut ();
            StartCPMusic (XJAZNAZI_MUS);
            UnCacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
            UnCacheLump (BACKDROP_LUMP_START, BACKDROP_LUMP_END);
            


            CA_CacheGrChunk (IDGUYS1PIC);
            VWB_DrawPic (0, 0, IDGUYS1PIC);
            UNCACHEGRCHUNK (IDGUYS1PIC);

            CA_CacheGrChunk (IDGUYS2PIC);
            VWB_DrawPic (0, 80, IDGUYS2PIC);
            UNCACHEGRCHUNK (IDGUYS2PIC);
#ifndef USE_SPRITES
            VW_UpdateScreen ();
#endif
            SDL_Color pal[256];
            CA_CacheGrChunk (IDGUYSPALETTE);
            VL_ConvertPalette(grsegs[IDGUYSPALETTE], pal, 256);
            VL_FadeIn (0, 255, pal, 30);
            UNCACHEGRCHUNK (IDGUYSPALETTE);

            while (Keyboard[sc_I] || Keyboard[sc_D])
                IN_WaitAndProcessEvents();
            IN_ClearKeysDown ();
            IN_Ack ();

            VW_FadeOut ();

            CacheLump (BACKDROP_LUMP_START, BACKDROP_LUMP_END);
            CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
            DrawMainMenu ();
            StartCPMusic (MENUSONG);
            MenuFadeIn ();
        }
#endif
#endif

        switch (which)
        {
            case viewscores:
                if (MainMenu[viewscores].routine == NULL)
                {
                    if (CP_EndGame (0))
	//slPrint("exit game 5!!!!",slLocate(10,11));	
						
                        StartGame = 1;
                }
                else
                {
                    DrawMainMenu();
                    MenuFadeIn ();
                }
                break;

            case backtodemo:
                StartGame = 1;
                if (!ingame)
                    StartCPMusic (INTROSONG);
                VL_FadeOut (0, 255, 0, 0, 0, 10);
                break;

            case -1:
            case quit:
				//slPrint("exit game !!!!",slLocate(10,11));
                CP_Quit (0);
                break;

            default:
                if (!StartGame)
                {
                    DrawMainMenu ();
                    MenuFadeIn ();
                }
        }

        //
        // "EXIT OPTIONS" OR "NEW GAME" EXITS
        //

		slSynch(); // vbt ajout 26/05
    }
    while (!StartGame);
	//slPrint("exit game 6!!!!",slLocate(10,11));	

    //
    // DEALLOCATE EVERYTHING
    //
    CleanupControlPanel ();
	//slPrint("exit game 7!!!!",slLocate(10,11));	

    //
    // CHANGE MAINMENU ITEM
    //
    if (startgame)
        EnableEndGameMenuItem();
	//slPrint("exit game 8!!!!",slLocate(10,11));	
	
    // RETURN/START GAME EXECUTION
//slPrint("slScrTransparent6",slLocate(1,17));		
	slScrTransparent(0);
// vbt : nettoyage ecran en sortie de menu	
	curSurface = screen;
	VL_BarScaledCoord (viewscreenx,viewscreeny,viewwidth,viewheight,0);
	curSurface = screenBuffer;
	VL_BarScaledCoord (viewscreenx,viewscreeny,viewwidth,viewheight,0); // vbt nettoie l'écran, à mettre en sortant du resize	
#ifdef SPEAR
    UnCacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif
}

void EnableEndGameMenuItem()
{
    MainMenu[viewscores].routine = NULL;
#ifndef JAPAN
    strcpy (MainMenu[viewscores].string, STR_EG);
#endif
}

////////////////////////
//
// DRAW MAIN MENU SCREEN
//
void
DrawMainMenu (void)
{
//slPrint("slScrTransparent7",slLocate(1,17));		
	slScrTransparent(NBG1ON);
//	slPrint("DrawMainMenu",slLocate(10,15));
#ifdef JAPAN
    CA_CacheScreen (S_OPTIONSPIC);
#else
    ClearMScreen ();

    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);
    DrawStripes (10);
    VWB_DrawPic (84, 0, C_OPTIONSPIC);

#ifdef SPANISH
    DrawWindow (MENU_X - 8, MENU_Y - 3, MENU_W + 8, MENU_H, BKGDCOLOR);
#else
    DrawWindow (MENU_X - 8, MENU_Y - 3, MENU_W, MENU_H, BKGDCOLOR);
#endif
#endif

    //
    // CHANGE "GAME" AND "DEMO"
    //
    if (ingame)
    {
#ifndef JAPAN

#ifdef SPANISH
        strcpy (&MainMenu[backtodemo].string, STR_GAME);
#else
        strcpy (&MainMenu[backtodemo].string[8], STR_GAME);
#endif

#else
        CA_CacheGrChunk (C_MRETGAMEPIC);
        VWB_DrawPic (12 * 8, 20 * 8, C_MRETGAMEPIC);
        UNCACHEGRCHUNK (C_MRETGAMEPIC);
        CA_CacheGrChunk (C_MENDGAMEPIC);
        VWB_DrawPic (12 * 8, 18 * 8, C_MENDGAMEPIC);
        UNCACHEGRCHUNK (C_MENDGAMEPIC);
#endif
        MainMenu[backtodemo].active = 2;
    }
    else
    {
#ifndef JAPAN
#ifdef SPANISH
        strcpy (&MainMenu[backtodemo].string, STR_BD);
#else
        strcpy (&MainMenu[backtodemo].string[8], STR_DEMO);
#endif
#else
        CA_CacheGrChunk (C_MRETDEMOPIC);
        VWB_DrawPic (12 * 8, 20 * 8, C_MRETDEMOPIC);
        UNCACHEGRCHUNK (C_MRETDEMOPIC);
        CA_CacheGrChunk (C_MSCORESPIC);
        VWB_DrawPic (12 * 8, 18 * 8, C_MSCORESPIC);
        UNCACHEGRCHUNK (C_MSCORESPIC);
#endif
        MainMenu[backtodemo].active = 1;
    }

    DrawMenu (&MainItems, &MainMenu[0]);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
}

#ifndef GOODTIMES
#ifndef SPEAR
////////////////////////////////////////////////////////////////////
//
// READ THIS!
//
////////////////////////////////////////////////////////////////////
/*
int
CP_ReadThis (int)
{
    StartCPMusic (CORNER_MUS);
    HelpScreens ();
    StartCPMusic (MENUSONG);
    return true;
}		  */
#endif
#endif


#ifdef GOODTIMES
////////////////////////////////////////////////////////////////////
//
// BOSS KEY
//
////////////////////////////////////////////////////////////////////
void
BossKey (void)
{
#ifdef NOTYET
    byte palette1[256][3];
    SD_MusicOff ();
/*       _AX = 3;
        geninterrupt(0x10); */
    _asm
    {
    mov eax, 3 int 0x10}
    puts ("C>");
    SetTextCursor (2, 0);
//      while (!Keyboard[sc_Escape])
    IN_Ack ();
    IN_ClearKeysDown ();

    SD_MusicOn ();
    VL_SetVGAPlaneMode ();
    for (int i = 0; i < 768; i++)
        palette1[0][i] = 0;

    VL_SetPalette (&palette1[0][0]);
    LoadLatchMem ();
#endif
}
#else
#ifdef SPEAR
void
BossKey (void)
{
#ifdef NOTYET
    byte palette1[256][3];
    SD_MusicOff ();
/*       _AX = 3;
        geninterrupt(0x10); */
    _asm
    {
    mov eax, 3 int 0x10}
    puts ("C>");
    SetTextCursor (2, 0);
//      while (!Keyboard[sc_Escape])
    IN_Ack ();
    IN_ClearKeysDown ();

    SD_MusicOn ();
    VL_SetVGAPlaneMode ();
    for (int i = 0; i < 768; i++)
        palette1[0][i] = 0;

    VL_SetPalette (&palette1[0][0]);
    LoadLatchMem ();
#endif
}
#endif
#endif

#if 0
////////////////////////////////////////////////////////////////////
//
// CHECK QUICK-KEYS & QUIT (WHILE IN A GAME)
//
////////////////////////////////////////////////////////////////////
int
CP_CheckQuick (ScanCode scancode)
{
    switch (scancode)
    {
        //
        // END GAME
        //
        case sc_F7:
            CA_CacheGrChunk (STARTFONT + 1);

            WindowH = 160;
#ifdef JAPAN
            if (GetYorN (7, 8, C_JAPQUITPIC))
#else
            if (Confirm (ENDGAMESTR))
#endif
            {
                playstate = ex_died;
                killerobj = NULL;
                /*pickquick =*/ gamestate.lives = 0;
            }

            WindowH = 200;
            fontnumber = 0;
            MainMenu[savegame].active = 0;
            return 1;
        //
        // QUIT
        //
        case sc_F10:
            CA_CacheGrChunk (STARTFONT + 1);

            WindowX = WindowY = 0;
            WindowW = 320;
            WindowH = 160;
#ifdef JAPAN
            if (GetYorN (7, 8, C_QUITMSGPIC))
#else
#ifdef SPANISH
            if (Confirm (ENDGAMESTR))
#else
            if (Confirm (endStrings[US_RndT () & 0x7 + (US_RndT () & 1)]))
#endif
#endif
            {
#ifndef USE_SPRITES				
                VW_UpdateScreen ();
#endif				
                SD_MusicOff ();
                SD_StopSound ();
                MenuFadeOut ();

                Quit (NULL);
            }

            DrawPlayBorder ();
            WindowH = 200;
            fontnumber = 0;
            return 1;
    }

    return 0;
}
#endif

////////////////////////////////////////////////////////////////////
//
// END THE CURRENT GAME
//
////////////////////////////////////////////////////////////////////
int
CP_EndGame (int)
{
    int res;
#ifdef JAPAN
    res = GetYorN (7, 8, C_JAPQUITPIC);
#else
    res = Confirm (ENDGAMESTR);
#endif
	//slPrint("exit game !!!!",slLocate(10,11));	

    DrawMainMenu();
	//slPrint("exit game 2!!!!",slLocate(10,11));	

    if(!res) return 0;
	//slPrint("exit game 3!!!!",slLocate(10,11));	

    /*pickquick =*/ gamestate.lives = 0;
    playstate = ex_died;
    killerobj = NULL;

    MainMenu[savegame].active = 0;
    MainMenu[viewscores].routine = CP_ViewScores;
#ifndef JAPAN
    strcpy (MainMenu[viewscores].string, STR_VS);
#endif
	//slPrint("exit game 4!!!!",slLocate(10,11));	

    return 1;
}


////////////////////////////////////////////////////////////////////
//
// VIEW THE HIGH SCORES
//
////////////////////////////////////////////////////////////////////
int
CP_ViewScores (int)
{
    fontnumber = 0;

#ifdef SPEAR
    UnCacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
    StartCPMusic (XAWARD_MUS);
#else
    StartCPMusic (ROSTER_MUS);
#endif

    DrawHighScores ();
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
    MenuFadeIn ();
    fontnumber = 1;

    IN_Ack ();

    StartCPMusic (MENUSONG);
    MenuFadeOut ();

#ifdef SPEAR
    CacheLump (BACKDROP_LUMP_START, BACKDROP_LUMP_END);
    CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif
    return 0;
}


////////////////////////////////////////////////////////////////////
//
// START A NEW GAME
//
////////////////////////////////////////////////////////////////////
int
CP_NewGame (int)
{
    int which, episode;

#ifdef SPEAR
    UnCacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif

//  			slPrint("CP_NewGame",slLocate(10,7));
#ifndef SPEAR
  firstpart:
  			//slPrint("DrawNewEpisode",slLocate(10,8));
    DrawNewEpisode ();
    do
    {
  			//slPrint("HandleMenu",slLocate(10,9))	;
        which = HandleMenu (&NewEitems, &NewEmenu[0], NULL);
        switch (which)
        {
            case -1:
                MenuFadeOut ();
                return 0;

            default:
// VBT plante ici pour les niveaux suivants
                if (!EpisodeSelect[which / 2])
                {
  			//slPrint("SD_PlaySound",slLocate(10,9))	;
                    SD_PlaySound (NOWAYSND);
                    Message ("Please select \"Read This!\"\n"
                             "from the Options menu to\n"
                             "find out how to order this\n" "episode from Apogee.");	
  			//slPrint("IN_ClearKeysDown",slLocate(10,9));
                    IN_ClearKeysDown ();
  			//slPrint("IN_Ack",slLocate(10,9));	
                    IN_Ack ();
  			//slPrint("DrawNewEpisode",slLocate(10,9));	
                    DrawNewEpisode ();
                    which = 0;
                }
                else
                {
                    episode = which / 2;
                    which = 1;
                }
                break;
        }

    }
    while (!which);
  			//slPrint("ShootSnd",slLocate(10,9));
    ShootSnd ();
  			//slPrint("ShootSndEnd",slLocate(10,10));
    //
    // ALREADY IN A GAME?
    //
    if (ingame)
#ifdef JAPAN
        if (!GetYorN (7, 8, C_JAPNEWGAMEPIC))
#else
        if (!Confirm (CURGAME))
#endif
        {
            MenuFadeOut ();
            return 0;
        }
   			//slPrint("MenuFadeOut",slLocate(10,8));
    MenuFadeOut ();

#else
    episode = 0;

    //
    // ALREADY IN A GAME?
    //
  			//slPrint("CacheLump",slLocate(10,8));
    CacheLump (NEWGAME_LUMP_START, NEWGAME_LUMP_END);
    DrawNewGame ();
    if (ingame)
        if (!Confirm (CURGAME))
        {
            MenuFadeOut ();
            UnCacheLump (NEWGAME_LUMP_START, NEWGAME_LUMP_END);
            CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
            return 0;
        }

#endif
  			//slPrint("DrawNewGame2",slLocate(10,8));
    DrawNewGame ();
    which = HandleMenu (&NewItems, &NewMenu[0], DrawNewGameDiff);
    if (which < 0)
    {
        MenuFadeOut ();
#ifndef SPEAR
        goto firstpart;
#else
        UnCacheLump (NEWGAME_LUMP_START, NEWGAME_LUMP_END);
        CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
        return 0;
#endif
    }

    ShootSnd ();
    NewGame (which, episode);
    StartGame = 1;
    MenuFadeOut ();

    //
    // CHANGE "READ THIS!" TO NORMAL COLOR
    //
#ifndef SPEAR
#ifndef GOODTIMES
    MainMenu[readthis].active = 1;
#endif
#endif

//    pickquick = 0;

#ifdef SPEAR
    UnCacheLump (NEWGAME_LUMP_START, NEWGAME_LUMP_END);
    CacheLump (OPTIONS_LUMP_START, OPTIONS_LUMP_END);
#endif

    return 0;
}


#ifndef SPEAR
/////////////////////
//
// DRAW NEW EPISODE MENU
//
void
DrawNewEpisode (void)
{
    int i;

#ifdef JAPAN
    CA_CacheScreen (S_EPISODEPIC);
#else
    ClearMScreen ();
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);

    DrawWindow (NE_X - 4, NE_Y - 4, NE_W + 8, NE_H + 8, BKGDCOLOR);
    SETFONTCOLOR (READHCOLOR, BKGDCOLOR);
    PrintY = 2;
    WindowX = 0;
#ifdef SPANISH
    US_CPrint ("Cual episodio jugar?");
#else
    US_CPrint ("Which episode to play?");
#endif
#endif

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
    DrawMenu (&NewEitems, &NewEmenu[0]);

    for (i = 0; i < 6; i++)
        VWB_DrawPic (NE_X + 32, NE_Y + i * 26, C_EPISODE1PIC + i);
#ifndef USE_SPRITES
    VW_UpdateScreen ();
#endif	
    MenuFadeIn ();
    WaitKeyUp ();
}
#endif

/////////////////////
//
// DRAW NEW GAME MENU
//
void
DrawNewGame (void)
{
#ifdef JAPAN
    CA_CacheScreen (S_SKILLPIC);
#else
    ClearMScreen ();
    VWB_DrawPic (112, 184, C_MOUSELBACKPIC);

    SETFONTCOLOR (READHCOLOR, BKGDCOLOR);
    PrintX = NM_X + 20;
    PrintY = NM_Y - 32;

#ifndef SPEAR
#ifdef SPANISH
    US_Print ("Eres macho?");
#else
    US_Print ("How tough are you?");
#endif
#else
    VWB_DrawPic (PrintX, PrintY, C_HOWTOUGHPIC);
#endif

    DrawWindow (NM_X - 5, NM_Y - 10, NM_W, NM_H, BKGDCOLOR);
#endif

    DrawMenu (&NewItems, &NewMenu[0]);
    DrawNewGameDiff (NewItems.curpos);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
    MenuFadeIn ();
    WaitKeyUp ();
}


////////////////////////
//
// DRAW NEW GAME GRAPHIC
//
void
DrawNewGameDiff (int w)
{
    VWB_DrawPic (NM_X + 185, NM_Y + 7, w + C_BABYMODEPIC);
}

////////////////////////////////////////////////////////////////////
//
// CHANGE SCREEN VIEWING SIZE
//
////////////////////////////////////////////////////////////////////
int
CP_ChangeView (int)
{
    int exit = 0, oldview, newview;
    ControlInfo ci;

    WindowX = WindowY = 0;
    WindowW = 320;
    WindowH = 200;
    newview = oldview = viewsize;
    DrawChangeView (oldview);
    MenuFadeIn ();

    do
    {
        CheckPause ();
        SDL_Delay(5);
        ReadAnyControl (&ci);
        switch (ci.dir)
        {
            case dir_South:
            case dir_West:
                newview--;
                if (newview < 4)
                    newview = 4;
                if(newview >= 19) DrawChangeView(newview);
                else ShowViewSize (newview);
#ifndef USE_SPRITES				
                VW_UpdateScreen ();
#endif				
                SD_PlaySound (HITWALLSND);
                TicDelay (10);
                break;

            case dir_North:
            case dir_East:
                newview++;
                if (newview >= 21)
                {
                    newview = 21;
                    DrawChangeView(newview);
                }
                else ShowViewSize (newview);
#ifndef USE_SPRITES				
                VW_UpdateScreen ();
#endif				
                SD_PlaySound (HITWALLSND);
                TicDelay (10);
                break;
        }

        if (ci.button0 || Keyboard[sc_Enter])
            exit = 1;
        else if (ci.button1 || Keyboard[sc_Escape])
        {
            SD_PlaySound (ESCPRESSEDSND);
            MenuFadeOut ();
            if(screenHeight % 200 != 0)
                VL_ClearScreen(0);
            return 0;
        }
    }
    while (!exit);
	
    if (oldview != newview)
    {
        SD_PlaySound (SHOOTSND);
        Message (STR_THINK "...");
        NewViewSize (newview);
    }

    ShootSnd ();
    MenuFadeOut ();
	
    if(screenHeight % 200 != 0)
        VL_ClearScreen(0);

    return 0;
}


/////////////////////////////
//
// DRAW THE CHANGEVIEW SCREEN
//
void
DrawChangeView (int view)
{
//slPrint("slScrTransparent8",slLocate(1,17));		
	slScrTransparent(0);
    int rescaledHeight = screenHeight / scaleFactor;
    if(view != 21) VWB_Bar (0, rescaledHeight - 40, 320, 40, bordercol);

#ifdef JAPAN
    CA_CacheScreen (S_CHANGEPIC);

    ShowViewSize (view);
#else
    ShowViewSize (view);

    PrintY = (screenHeight / scaleFactor) - 39;
    WindowX = 0;
    WindowY = 320;                                  // TODO: Check this!
    SETFONTCOLOR (HIGHLIGHT, BKGDCOLOR);

    US_CPrint (STR_SIZE1 "\n");
    US_CPrint (STR_SIZE2 "\n");
    US_CPrint (STR_SIZE3);
#endif
#ifndef USE_SPRITES
    VW_UpdateScreen ();
#endif	
}


////////////////////////////////////////////////////////////////////
//
// QUIT THIS INFERNAL GAME!
//
////////////////////////////////////////////////////////////////////
int
CP_Quit (int)
{
		//slPrint("exit game 1 !!!!",slLocate(10,11));
#ifdef JAPAN
    if (GetYorN (7, 11, C_QUITMSGPIC))
#else

#ifdef SPANISH
    if (Confirm (ENDGAMESTR))
#else
    if (Confirm (endStrings[US_RndT () & 0x7 + (US_RndT () & 1)]))
#endif

#endif
    {
#ifndef USE_SPRITES		
        VW_UpdateScreen ();
#endif		
        SD_MusicOff ();
        SD_StopSound ();
        MenuFadeOut ();
        Quit (NULL);
    }

    DrawMainMenu ();
    return 0;
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
// SUPPORT ROUTINES
//
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//
// Clear Menu screens to dark red
//
////////////////////////////////////////////////////////////////////
void
ClearMScreen (void)
{
#ifndef SPEAR
    VWB_Bar (0, 0, 320, 200, BORDCOLOR);
#else
    VWB_DrawPic (0, 0, C_BACKDROPPIC);
#endif
}


////////////////////////////////////////////////////////////////////
//
// Un/Cache a LUMP of graphics
//
////////////////////////////////////////////////////////////////////
void
CacheLump (int lumpstart, int lumpend)
{
    int i;

    for (i = lumpstart; i <= lumpend; i++)
	{
		//slPrintHex(i,slLocate(2,11));
        CA_CacheGrChunk (i);
	}
}


void
UnCacheLump (int lumpstart, int lumpend)
{
    int i;

    for (i = lumpstart; i <= lumpend; i++)
        if (grsegs[i])
            UNCACHEGRCHUNK (i);
}


////////////////////////////////////////////////////////////////////
//
// Draw a window for a menu
//
////////////////////////////////////////////////////////////////////
void
DrawWindow (int x, int y, int w, int h, int wcolor)
{
    VWB_Bar (x, y, w, h, wcolor);
    DrawOutline (x, y, w, h, BORD2COLOR, DEACTIVE);
}


void
DrawOutline (int x, int y, int w, int h, int color1, int color2)
{
    VWB_Hlin (x, x + w, y, color2);
    VWB_Vlin (y, y + h, x, color2);
    VWB_Hlin (x, x + w, y + h, color1);
    VWB_Vlin (y, y + h, x + w, color1);
}


////////////////////////////////////////////////////////////////////
//
// Setup Control Panel stuff - graphics, etc.
//
////////////////////////////////////////////////////////////////////
void
SetupControlPanel (void)
{
    //
    // CACHE GRAPHICS & SOUNDS
    //
    CA_CacheGrChunk (STARTFONT + 1);
#ifndef SPEAR
    CacheLump (CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#else
    CacheLump (BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#endif

    SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
    fontnumber = 1;
    WindowH = 200;
    if(screenHeight % 200 != 0)
        VL_ClearScreen(0);

    if (ingame)
        MainMenu[savegame].active = 0;
}

////////////////////////////////////////////////////////////////////
//
// Clean up all the Control Panel stuff
//
////////////////////////////////////////////////////////////////////
void
CleanupControlPanel (void)
{
#ifndef SPEAR
    UnCacheLump (CONTROLS_LUMP_START, CONTROLS_LUMP_END);
#else
    UnCacheLump (BACKDROP_LUMP_START, BACKDROP_LUMP_END);
#endif

    fontnumber = 0;
// vbt 13/08/2020 : ajout	
//		byte *vbuf = (byte *)curSurface->pixels;
//		byte *ptr = vbuf;

	//	for(int y = 0; y < screenHeight; y++, ptr += curPitch)
	//		memset(ptr, 0x00, screenWidth);	
	memset((byte *)curSurface->pixels, 0x00, screenWidth*screenHeight);	
}


////////////////////////////////////////////////////////////////////
//
// Handle moving gun around a menu
//
////////////////////////////////////////////////////////////////////
int
HandleMenu (CP_iteminfo * item_i, CP_itemtype * items, void (*routine) (int w))
{
    char key;
    static int redrawitem = 1, lastitem = -1;
    int i, x, y, basey, exit, which, shape;
    int32_t lastBlinkTime, timer;
    ControlInfo ci;


    which = item_i->curpos;
    x = item_i->x & -8;
    basey = item_i->y - 2;
    y = basey + which * 13;

    VWB_DrawPic (x, y, C_CURSOR1PIC);
    SetTextColor (items + which, 1);
    if (redrawitem)
    {
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print ((items + which)->string);
    }
    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //
    if (routine)
        routine (which);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif
    shape = C_CURSOR1PIC;
    timer = 8;
    exit = 0;
    lastBlinkTime = GetTimeCount ();
    IN_ClearKeysDown ();


    do
    {
        //
        // CHANGE GUN SHAPE
        //
        if ((int32_t)GetTimeCount () - lastBlinkTime > timer)
        {
            lastBlinkTime = GetTimeCount ();
            if (shape == C_CURSOR1PIC)
            {
                shape = C_CURSOR2PIC;
                timer = 8;
            }
            else
            {
                shape = C_CURSOR1PIC;
                timer = 70;
            }
            VWB_DrawPic (x, y, shape);
            if (routine)
                routine (which);
#ifndef USE_SPRITES			
            VW_UpdateScreen ();
#endif			
        }
        else SDL_Delay(5);

        CheckPause ();

        //
        // SEE IF ANY KEYS ARE PRESSED FOR INITIAL CHAR FINDING
        //
        key = LastASCII;
        if (key)
        {
            int ok = 0;

            if (key >= 'a')
                key -= 'a' - 'A';

            for (i = which + 1; i < item_i->amount; i++)
                if ((items + i)->active && (items + i)->string[0] == key)
                {
                    EraseGun (item_i, items, x, y, which);
                    which = i;
                    DrawGun (item_i, items, x, &y, which, basey, routine);
                    ok = 1;
                    IN_ClearKeysDown ();
                    break;
                }

            //
            // DIDN'T FIND A MATCH FIRST TIME THRU. CHECK AGAIN.
            //
            if (!ok)
            {
                for (i = 0; i < which; i++)
                    if ((items + i)->active && (items + i)->string[0] == key)
                    {
                        EraseGun (item_i, items, x, y, which);
                        which = i;
                        DrawGun (item_i, items, x, &y, which, basey, routine);
                        IN_ClearKeysDown ();
                        break;
                    }
            }
        }

        //
        // GET INPUT
        //
        ReadAnyControl (&ci);
        switch (ci.dir)
        {
                ////////////////////////////////////////////////
                //
                // MOVE UP
                //
            case dir_North:

                EraseGun (item_i, items, x, y, which);

                //
                // ANIMATE HALF-STEP
                //
                if (which && (items + which - 1)->active)
                {
                    y -= 6;
                    DrawHalfStep (x, y);
                }

                //
                // MOVE TO NEXT AVAILABLE SPOT
                //
                do
                {
                    if (!which)
                        which = item_i->amount - 1;
                    else
                        which--;
                }
                while (!(items + which)->active);

                DrawGun (item_i, items, x, &y, which, basey, routine);
                //
                // WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
                //
                TicDelay (20);
                break;

                ////////////////////////////////////////////////
                //
                // MOVE DOWN
                //
            case dir_South:

                EraseGun (item_i, items, x, y, which);
                //
                // ANIMATE HALF-STEP
                //
                if (which != item_i->amount - 1 && (items + which + 1)->active)
                {
                    y += 6;
                    DrawHalfStep (x, y);
                }

                do
                {
                    if (which == item_i->amount - 1)
                        which = 0;
                    else
                        which++;
                }
                while (!(items + which)->active);

                DrawGun (item_i, items, x, &y, which, basey, routine);

                //
                // WAIT FOR BUTTON-UP OR DELAY NEXT MOVE
                //
                TicDelay (20);
                break;
        }

        if (ci.button0 || Keyboard[sc_Space] || Keyboard[sc_Enter])
            exit = 1;

        if (ci.button1 && !Keyboard[sc_Alt] || Keyboard[sc_Escape])
            exit = 2;

    }
    while (!exit);


    IN_ClearKeysDown ();

    //
    // ERASE EVERYTHING
    //
    if (lastitem != which)
    {
        VWB_Bar (x - 1, y, 25, 16, BKGDCOLOR);
        PrintX = item_i->x + item_i->indent;
        PrintY = item_i->y + which * 13;
        US_Print ((items + which)->string);
        redrawitem = 1;
    }
    else
        redrawitem = 0;

    if (routine)
        routine (which);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif
    item_i->curpos = which;

    lastitem = which;
    switch (exit)
    {
        case 1:
            //
            // CALL THE ROUTINE
            //
            if ((items + which)->routine != NULL)
            {
                ShootSnd ();
                MenuFadeOut ();
                (items + which)->routine (0);
            }
            return which;

        case 2:
            SD_PlaySound (ESCPRESSEDSND);
            return -1;
    }

    return 0;                   // JUST TO SHUT UP THE ERROR MESSAGES!
}


//
// ERASE GUN & DE-HIGHLIGHT STRING
//
void
EraseGun (CP_iteminfo * item_i, CP_itemtype * items, int x, int y, int which)
{
    VWB_Bar (x - 1, y, 25, 16, BKGDCOLOR);
    SetTextColor (items + which, 0);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print ((items + which)->string);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
}


//
// DRAW HALF STEP OF GUN TO NEXT POSITION
//
void
DrawHalfStep (int x, int y)
{
    VWB_DrawPic (x, y, C_CURSOR1PIC);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
    SD_PlaySound (MOVEGUN1SND);
    SDL_Delay (8 * 100 / 7);
}


//
// DRAW GUN AT NEW POSITION
//
void
DrawGun (CP_iteminfo * item_i, CP_itemtype * items, int x, int *y, int which, int basey,
         void (*routine) (int w))
{
    VWB_Bar (x - 1, *y, 25, 16, BKGDCOLOR);
    *y = basey + which * 13;
    VWB_DrawPic (x, *y, C_CURSOR1PIC);
    SetTextColor (items + which, 1);

    PrintX = item_i->x + item_i->indent;
    PrintY = item_i->y + which * 13;
    US_Print ((items + which)->string);

    //
    // CALL CUSTOM ROUTINE IF IT IS NEEDED
    //
    if (routine)
        routine (which);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
    SD_PlaySound (MOVEGUN2SND);
}

////////////////////////////////////////////////////////////////////
//
// DELAY FOR AN AMOUNT OF TICS OR UNTIL CONTROLS ARE INACTIVE
//
////////////////////////////////////////////////////////////////////
void
TicDelay (int count)
{
    ControlInfo ci;

    int32_t startTime = GetTimeCount ();
    do
    {
        SDL_Delay(5);
        ReadAnyControl (&ci);
    }
    while ((int32_t) GetTimeCount () - startTime < count && ci.dir != dir_None);
}


////////////////////////////////////////////////////////////////////
//
// Draw a menu
//
////////////////////////////////////////////////////////////////////
void
DrawMenu (CP_iteminfo * item_i, CP_itemtype * items)
{
    int i, which = item_i->curpos;
	//slPrint("DrawMenu",slLocate(10,16));

    WindowX = PrintX = item_i->x + item_i->indent;
    WindowY = PrintY = item_i->y;
    WindowW = 320;
    WindowH = 200;

    for (i = 0; i < item_i->amount; i++)
    {
        SetTextColor (items + i, which == i);
		//slPrint((items + i)->string,slLocate(20,i+15));
        PrintY = item_i->y + i * 13;
        if ((items + i)->active)
            US_Print ((items + i)->string);
        else
        {
            SETFONTCOLOR (DEACTIVE, BKGDCOLOR);
            US_Print ((items + i)->string);
            SETFONTCOLOR (TEXTCOLOR, BKGDCOLOR);
        }

        US_Print ("\n");
    }
}


////////////////////////////////////////////////////////////////////
//
// SET TEXT COLOR (HIGHLIGHT OR NO)
//
////////////////////////////////////////////////////////////////////
void
SetTextColor (CP_itemtype * items, int hlight)
{
    if (hlight)
    {
        SETFONTCOLOR (color_hlite[items->active], BKGDCOLOR);
    }
    else
    {
        SETFONTCOLOR (color_norml[items->active], BKGDCOLOR);
    }
}


////////////////////////////////////////////////////////////////////
//
// WAIT FOR CTRLKEY-UP OR BUTTON-UP
//
////////////////////////////////////////////////////////////////////
void
WaitKeyUp (void)
{
    ControlInfo ci;
    while (ReadAnyControl (&ci), ci.button0 |
           ci.button1 |
           ci.button2 | ci.button3 | Keyboard[sc_Space] | Keyboard[sc_Enter] | Keyboard[sc_Escape])
    {
        IN_WaitAndProcessEvents();
    }
}


////////////////////////////////////////////////////////////////////
//
// READ KEYBOARD, JOYSTICK AND MOUSE FOR INPUT
//
////////////////////////////////////////////////////////////////////
void
ReadAnyControl (ControlInfo * ci)
{
    IN_ReadControl (0, ci);
}


////////////////////////////////////////////////////////////////////
//
// DRAW DIALOG AND CONFIRM YES OR NO TO QUESTION
//
////////////////////////////////////////////////////////////////////
int
Confirm (const char *string)
{
    int xit = 0, x, y, tick = 0, lastBlinkTime;
    int whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };
    ControlInfo ci;

    Message (string);
    IN_ClearKeysDown ();
    WaitKeyUp ();

    //
    // BLINK CURSOR
    //
    x = PrintX;
    y = PrintY;
    lastBlinkTime = GetTimeCount();

    do
    {
        ReadAnyControl(&ci);

        if (GetTimeCount() - lastBlinkTime >= 10)
        {
            switch (tick)
            {
                case 0:
                    VWB_Bar (x, y, 8, 13, TEXTCOLOR);
                    break;
                case 1:
                    PrintX = x;
                    PrintY = y;
                    US_Print ("_");
            }
#ifndef USE_SPRITES			
            VW_UpdateScreen ();
#endif			
            tick ^= 1;
            lastBlinkTime = GetTimeCount();
        }
        else SDL_Delay(5);

#ifdef SPANISH
    }
    while (!Keyboard[sc_S] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
#else
    }
    while (!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape] && !ci.button0 && !ci.button1);
#endif

#ifdef SPANISH
    if (Keyboard[sc_S] || ci.button0)
    {
        xit = 1;
        ShootSnd ();
    }
#else
    if (Keyboard[sc_Y] || ci.button0)
    {
        xit = 1;
        ShootSnd ();
    }
#endif

    IN_ClearKeysDown ();
    WaitKeyUp ();

    SD_PlaySound ((soundnames) whichsnd[xit]);

    return xit;
}

#ifdef JAPAN
////////////////////////////////////////////////////////////////////
//
// DRAW MESSAGE & GET Y OR N
//
////////////////////////////////////////////////////////////////////
int
GetYorN (int x, int y, int pic)
{
    int xit = 0, whichsnd[2] = { ESCPRESSEDSND, SHOOTSND };


    CA_CacheGrChunk (pic);
    VWB_DrawPic (x * 8, y * 8, pic);
    UNCACHEGRCHUNK (pic);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
    IN_ClearKeysDown ();

    do
    {
        IN_WaitAndProcessEvents();
#ifndef SPEAR
        if (Keyboard[sc_Tab] && Keyboard[sc_P] && param_debugmode)
            PicturePause ();
#endif

#ifdef SPANISH
    }
    while (!Keyboard[sc_S] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
#else
    }
    while (!Keyboard[sc_Y] && !Keyboard[sc_N] && !Keyboard[sc_Escape]);
#endif

#ifdef SPANISH
    if (Keyboard[sc_S])
    {
        xit = 1;
        ShootSnd ();
    }

    while (Keyboard[sc_S] || Keyboard[sc_N] || Keyboard[sc_Escape])
        IN_WaitAndProcessEvents();

#else

    if (Keyboard[sc_Y])
    {
        xit = 1;
        ShootSnd ();
    }

    while (Keyboard[sc_Y] || Keyboard[sc_N] || Keyboard[sc_Escape])
        IN_WaitAndProcessEvents();
#endif

    IN_ClearKeysDown ();
    SD_PlaySound (whichsnd[xit]);
    return xit;
}
#endif


////////////////////////////////////////////////////////////////////
//
// PRINT A MESSAGE IN A WINDOW
//
////////////////////////////////////////////////////////////////////
void
Message (const char *string)
{
    int h = 0, w = 0, mw = 0, i, len = (int) strlen(string);
    fontstruct *font;

    CA_CacheGrChunk (STARTFONT + 1);
    fontnumber = 1;
    font = (fontstruct *) grsegs[STARTFONT + fontnumber];
    h = SWAP_BYTES_16(font->height);
 
    for (i = 0; i < len; i++)
    {
        if (string[i] == '\n')
        {
            if (w > mw)
                mw = w;
            w = 0;
            h += SWAP_BYTES_16(font->height);
        }
        else
            w += font->width[string[i]];
    }
    if (w + 10 > mw)
        mw = w + 10;

    PrintY = (WindowH / 2) - h / 2;
    PrintX = WindowX = 160 - mw / 2;
    DrawWindow (WindowX - 5, PrintY - 5, mw + 10, h + 10, TEXTCOLOR);
    DrawOutline (WindowX - 5, PrintY - 5, mw + 10, h + 10, 0, HIGHLIGHT);
    SETFONTCOLOR (0, TEXTCOLOR);

    US_Print (string);
#ifndef USE_SPRITES	
    VW_UpdateScreen ();
#endif	
}

////////////////////////////////////////////////////////////////////
//
// THIS MAY BE FIXED A LITTLE LATER...
//
////////////////////////////////////////////////////////////////////
static int lastmusic;

int
StartCPMusic (int song)
{
    int lastoffs;

    lastmusic = song;
    lastoffs = SD_MusicOff ();
    //UNCACHEAUDIOCHUNK (STARTMUSIC + lastmusic);

	//slPrint("StartCPMusic",slLocate(10,10));
	//slPrintHex(STARTMUSIC,slLocate(10,11));
	//slPrintHex(song,slLocate(10,12));

    SD_StartMusic(STARTMUSIC + song);


    return lastoffs;
}
/*
void
FreeMusic (void)
{
    //UNCACHEAUDIOCHUNK (STARTMUSIC + lastmusic);
}
*/
///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR PAUSE KEY (FOR MUSIC ONLY)
//
///////////////////////////////////////////////////////////////////////////
void
CheckPause (void)
{
    if (Paused)
    {
        switch (SoundStatus)
        {
            case 0:
                SD_MusicOn ();
                break;
            case 1:
                SD_MusicOff ();
                break;
        }

        SoundStatus ^= 1;
        VW_WaitVBL (3);
        IN_ClearKeysDown ();
        Paused = false;
    }
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW GUN CURSOR AT CORRECT POSITION IN MENU
//
///////////////////////////////////////////////////////////////////////////
void
DrawMenuGun (CP_iteminfo * iteminfo)
{
    int x, y;


    x = iteminfo->x;
    y = iteminfo->y + iteminfo->curpos * 13 - 2;
    VWB_DrawPic (x, y, C_CURSOR1PIC);
}


///////////////////////////////////////////////////////////////////////////
//
// DRAW SCREEN TITLE STRIPES
//
///////////////////////////////////////////////////////////////////////////
void
DrawStripes (int y)
{
#ifndef SPEAR
    VWB_Bar (0, y, 320, 24, 0);
    VWB_Hlin (0, 319, y + 22, STRIPE);
#else
    VWB_Bar (0, y, 320, 22, 0);
    VWB_Hlin (0, 319, y + 23, 0);
#endif
}

void
ShootSnd (void)
{
    SD_PlaySound (SHOOTSND);
}


///////////////////////////////////////////////////////////////////////////
//
// CHECK FOR EPISODES
//
///////////////////////////////////////////////////////////////////////////				   
#undef stat
/*
#undef stat
#define stat (a,statbuf) GFS_NameToId((unsigned char*)a)
	   */
	   /*struct stat*/

int stat(const char *_path, struct stat *_sbuf)
{
	char path[15];
	unsigned int i=0;
	strcpy(path,_path);
	while (path[i])
	{
		path[i]= toupper(path[i]);
		i++;
	}
	return !(GFS_NameToId((signed char *)path));
}

void
CheckForEpisodes (void)
{
    struct stat statbuf;

//
// JAPANESE VERSION
//
#ifdef JAPAN
#ifdef JAPDEMO
    if(!stat("vswap.wj1", &statbuf))
    {
        strcpy (extension, "wj1");
        numEpisodesMissing = 5;
#else
    if(!stat("vswap.wj6", &statbuf))
    {
        strcpy (extension, "wj6");
#endif
//        strcat (configname, extension);
//        strcat (SaveName, extension);
//        strcat (demoname, extension);
        EpisodeSelect[1] =
            EpisodeSelect[2] = EpisodeSelect[3] = EpisodeSelect[4] = EpisodeSelect[5] = 1;
    }
    else
        Quit ("NO JAPANESE WOLFENSTEIN 3-D DATA FILES to be found!");
#else

//
// ENGLISH
//
#ifdef UPLOAD
    if(!stat("vswap.wl1", &statbuf))
    {
        strcpy (extension, "WL1");
        numEpisodesMissing = 5;
    }
    else
        Quit ("NO WOLFENSTEIN 3-D DATA FILES to be found!");
#else
#ifndef SPEAR
    if(!stat("vswap.wl6", &statbuf))
    {
        strcpy (extension, "wl6");
        NewEmenu[2].active =
            NewEmenu[4].active =
            NewEmenu[6].active =
            NewEmenu[8].active =
            NewEmenu[10].active =
            EpisodeSelect[1] =
            EpisodeSelect[2] = EpisodeSelect[3] = EpisodeSelect[4] = EpisodeSelect[5] = 1;
    }
    else
    {
        if(!stat("vswap.wl3", &statbuf))
        {
            strcpy (extension, "wl3");
            numEpisodesMissing = 3;
            NewEmenu[2].active = NewEmenu[4].active = EpisodeSelect[1] = EpisodeSelect[2] = 1;
        }
        else
        {
            if(!stat("VSWAP.WL1", &statbuf))
            {
                strcpy (extension, "WL1");
                numEpisodesMissing = 5;
            }
            else
                Quit ("NO WOLFENSTEIN 3-D DATA FILES to be found!");
        }
    }
#endif
#endif


#ifdef SPEAR
#ifndef SPEARDEMO
    if(param_mission == 1)
    {
        if(!stat("vswap.sod", &statbuf))
            strcpy (extension, "sod");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else if(param_mission == 2)
    {
        if(!stat("vswap.sd2", &statbuf))
            strcpy (extension, "sd2");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else if(param_mission == 3)
    {
        if(!stat("vswap.sd3", &statbuf))
            strcpy (extension, "sd3");
        else
            Quit ("NO SPEAR OF DESTINY DATA FILES TO BE FOUND!");
    }
    else
        Quit ("UNSUPPORTED MISSION!");
    strcpy (graphext, "sod");
//    strcpy (audioext, "sod");
#else
    if(!stat("vswap.sdm", &statbuf))
    {
        strcpy (extension, "sdm");
    }
    else
        Quit ("NO SPEAR OF DESTINY DEMO DATA FILES TO BE FOUND!");
    strcpy (graphext, "sdm");
//    strcpy (audioext, "sdm");
#endif
#else
//    strcpy (graphext, extension);
//    strcpy (audioext, extension);
#endif

//    strcat (configname, extension);
//    strcat (SaveName, extension);
//    strcat (demoname, extension);

#ifndef SPEAR
#ifndef GOODTIMES
//    strcat (helpfilename, extension);
#endif
//    strcat (endfilename, extension);
#endif
#endif
}
