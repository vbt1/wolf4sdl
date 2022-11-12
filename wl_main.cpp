// WL_MAIN.C

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include "wl_def.h"
#include "wl_atmos.h"
#include "sdl/SDL_syswm.h"
#include "pcmsys.h"
#include "SEGA_PER.H"
/*
=============================================================================

                             WOLFENSTEIN 3-D

                        An Id Software production

                             by John Carmack

=============================================================================
*/

extern byte signon[];

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/


#define FOCALLENGTH     (0x5700l)               // in global coordinates
#define VIEWGLOBAL      0x10000                 // globals visable flush to wall

#define VIEWWIDTH       256                     // size of view window
#define VIEWHEIGHT      144

/*
=============================================================================

                            GLOBAL VARIABLES

=============================================================================
*/

//char    str[80];

//
// proejection variables
//
fixed    focallength;
int      viewscreenx, viewscreeny;
int      viewwidth;
int      viewheight;
short    centerx;
int      shootdelta;           // pixels away from centerx a target can be
fixed    scale;
int32_t  heightnumerator;

void    Quit (const char *error,...);

boolean startgame;
//boolean loadedgame;

//
// Command line parameter variables
//

static boolean param_nowait = true;
int     param_difficulty = 1;           // default is "normal"
//int     param_tedlevel = -1;            // default is not to start a level

int     param_mission = 1;
boolean param_goodtimes = false;
//boolean param_ignorenumchunks = false;

//#ifndef REMDEBUG
unsigned frame_x=0,frame_y=0;
unsigned char hz=0;
extern SDL_Color curpal[256];
#ifndef EMBEDDED
extern int min_wallheight;
#endif
//extern int nb_unlock;

void VblIn(void);

void VblIn(void)
{
	char buffer[8];/*
extern Uint8 TransRequest;
TransRequest*=2;
extern Uint16 TransCount;
TransCount*=2;	*/
	frame_y++;
	
	if(frame_y==hz)
	{
		slPrint((char*)ltoa(frame_x,buffer),slLocate(14,1));
//		slPrintHex(nb_unlock,slLocate(14,2));		
		frame_y=frame_x=0;
//		nb_unlock=0;
	}
//	SDL_SetPalette(curSurface, SDL_PHYSPAL, curpal, 0, 256);
	SDL_SetColors(curSurface, curpal, 0, 256);
	VGAClearScreen (); // vbt : maj du fond d'écran
#ifdef PONY
	m68k_com->start = (m68k_com->start != 0xFFFF) ? 1 : m68k_com->start;
#endif
}


//#endif

/*
=============================================================================

                            LOCAL VARIABLES

=============================================================================
*/

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (int difficulty,int episode)
{
    memset (&gamestate,0,sizeof(gamestate));
    gamestate.difficulty = difficulty;
    gamestate.weapon = gamestate.bestweapon
            = gamestate.chosenweapon = wp_pistol;
    gamestate.health = 100;
    gamestate.ammo = STARTAMMO;
    gamestate.lives = 3;
    gamestate.nextextra = EXTRAPOINTS;
    gamestate.episode=episode;

    startgame = true;
}

//===========================================================================
#if 0
void DiskFlopAnim(int x,int y)
{
    static int8_t which=0;
    if (!x && !y)
        return;
    VWB_DrawPic(x,y,C_DISKLOADING1PIC+which);
    VW_UpdateScreen();
    which^=1;
}

int32_t DoChecksum(byte *source,unsigned size,int32_t checksum)
{
    unsigned i;

    for (i=0;i<size-1;i++)
    checksum += source[i]^source[i+1];

    return checksum;
}

/*
==================
=
= SaveTheGame
=
==================
*/
#ifndef EMBEDDED
extern statetype s_grdstand;
extern statetype s_player;
#endif
boolean SaveTheGame(FILE *file,int x,int y)
{
//    struct diskfree_t dfree;
//    int32_t avail,size,checksum;
    int checksum;
    objtype *ob;
    objtype nullobj;
    statobj_t nullstat;

/*    if (_dos_getdiskfree(0,&dfree))
        Quit("Error in _dos_getdiskfree call");

    avail = (int32_t)dfree.avail_clusters *
                  dfree.bytes_per_sector *
                  dfree.sectors_per_cluster;

    size = 0;
    for (ob = player; ob ; ob=ob->next)
        size += sizeof(*ob);
    size += sizeof(nullobj);

    size += sizeof(gamestate) +
            sizeof(LRstruct)*LRpack +
            sizeof(tilemap) +
            sizeof(actorat) +
            sizeof(laststatobj) +
            sizeof(statobjlist) +
            sizeof(doorposition) +
            sizeof(pwallstate) +
            sizeof(pwalltile) +
            sizeof(pwallx) +
            sizeof(pwally) +
            sizeof(pwalldir) +
            sizeof(pwallpos);

    if (avail < size)
    {
        Message(STR_NOSPACE1"\n"STR_NOSPACE2);
        return false;
    }*/

    checksum = 0;

    DiskFlopAnim(x,y);
    fwrite(&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum((byte *)&gamestate,sizeof(gamestate),checksum);

    DiskFlopAnim(x,y);
    fwrite(&LevelRatios[0],sizeof(LRstruct)*LRpack,1,file);
    checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*LRpack,checksum);

    DiskFlopAnim(x,y);
    fwrite(tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum((byte *)tilemap,sizeof(tilemap),checksum);
    DiskFlopAnim(x,y);

    int i;
    for(i=0;i<MAPSIZE;i++)
    {
        for(int j=0;j<MAPSIZE;j++)
        {
            word actnum;
            objtype *objptr=actorat[i][j];
            if(ISPOINTER(objptr))
                actnum=0x8000 | (word)(objptr-objlist);
            else
                actnum=(word)(uintptr_t)objptr;
            fwrite(&actnum,sizeof(actnum),1,file);
            checksum = DoChecksum((byte *)&actnum,sizeof(actnum),checksum);
        }
    }

    fwrite (areaconnect,sizeof(areaconnect),1,file);
    fwrite (areabyplayer,sizeof(areabyplayer),1,file);

    // player object needs special treatment as it's in WL_AGENT.CPP and not in
    // WL_ACT2.CPP which could cause problems for the relative addressing

    ob = player;
    DiskFlopAnim(x,y);
    memcpy(&nullobj,ob,sizeof(nullobj));
    nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_player);
    fwrite(&nullobj,sizeof(nullobj),1,file);
    ob = ob->next;

    DiskFlopAnim(x,y);
    for (; ob ; ob=ob->next)
    {
        memcpy(&nullobj,ob,sizeof(nullobj));
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state-(uintptr_t)&s_grdstand);
        fwrite(&nullobj,sizeof(nullobj),1,file);
    }
    nullobj.active = ac_badobject;          // end of file marker
    DiskFlopAnim(x,y);
    fwrite(&nullobj,sizeof(nullobj),1,file);

    DiskFlopAnim(x,y);
    word laststatobjnum=(word) (laststatobj-statobjlist);
    fwrite(&laststatobjnum,sizeof(laststatobjnum),1,file);
    checksum = DoChecksum((byte *)&laststatobjnum,sizeof(laststatobjnum),checksum);

    DiskFlopAnim(x,y);
    for(i=0;i<MAXSTATS;i++)
    {
        memcpy(&nullstat,statobjlist+i,sizeof(nullstat));
        nullstat.visspot=(byte *) ((uintptr_t) nullstat.visspot-(uintptr_t)spotvis);
        fwrite(&nullstat,sizeof(nullstat),1,file);
        checksum = DoChecksum((byte *)&nullstat,sizeof(nullstat),checksum);
    }

    DiskFlopAnim(x,y);
    fwrite (doorposition,sizeof(doorposition),1,file);
    checksum = DoChecksum((byte *)doorposition,sizeof(doorposition),checksum);
    DiskFlopAnim(x,y);
    fwrite (doorobjlist,sizeof(doorobjlist),1,file);
    checksum = DoChecksum((byte *)doorobjlist,sizeof(doorobjlist),checksum);

    DiskFlopAnim(x,y);
    fwrite (&pwallstate,sizeof(pwallstate),1,file);
    checksum = DoChecksum((byte *)&pwallstate,sizeof(pwallstate),checksum);
    fwrite (&pwalltile,sizeof(pwalltile),1,file);
    checksum = DoChecksum((byte *)&pwalltile,sizeof(pwalltile),checksum);
    fwrite (&pwallx,sizeof(pwallx),1,file);
    checksum = DoChecksum((byte *)&pwallx,sizeof(pwallx),checksum);
    fwrite (&pwally,sizeof(pwally),1,file);
    checksum = DoChecksum((byte *)&pwally,sizeof(pwally),checksum);
    fwrite (&pwalldir,sizeof(pwalldir),1,file);
    checksum = DoChecksum((byte *)&pwalldir,sizeof(pwalldir),checksum);
    fwrite (&pwallpos,sizeof(pwallpos),1,file);
    checksum = DoChecksum((byte *)&pwallpos,sizeof(pwallpos),checksum);

    //
    // WRITE OUT CHECKSUM
    //
    fwrite (&checksum,sizeof(checksum),1,file);

    fwrite (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);

    return(true);
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

boolean LoadTheGame(FILE *file,int x,int y)
{
    int32_t checksum,oldchecksum;
    objtype nullobj;
    statobj_t nullstat;

    checksum = 0;

    DiskFlopAnim(x,y);
    fread (&gamestate,sizeof(gamestate),1,file);
    checksum = DoChecksum((byte *)&gamestate,sizeof(gamestate),checksum);

    DiskFlopAnim(x,y);
    fread (&LevelRatios[0],sizeof(LRstruct)*LRpack,1,file);
    checksum = DoChecksum((byte *)&LevelRatios[0],sizeof(LRstruct)*LRpack,checksum);

    DiskFlopAnim(x,y);
    SetupGameLevel ();

    DiskFlopAnim(x,y);
    fread (tilemap,sizeof(tilemap),1,file);
    checksum = DoChecksum((byte *)tilemap,sizeof(tilemap),checksum);

    DiskFlopAnim(x,y);

    int actnum=0, i;
    for(i=0;i<MAPSIZE;i++)
    {
        for(int j=0;j<MAPSIZE;j++)
        {
            fread (&actnum,sizeof(word),1,file);
            checksum = DoChecksum((byte *) &actnum,sizeof(word),checksum);
            if(actnum&0x8000)
                actorat[i][j]=objlist+(actnum&0x7fff);
            else
                actorat[i][j]=(objtype *)(uintptr_t) actnum;
        }
    }

    fread (areaconnect,sizeof(areaconnect),1,file);
    fread (areabyplayer,sizeof(areabyplayer),1,file);

    InitActorList ();
    DiskFlopAnim(x,y);
    fread (player,sizeof(*player),1,file);
    player->state=(statetype *) ((uintptr_t)player->state+(uintptr_t)&s_player);

    while (1)
    {
        DiskFlopAnim(x,y);
        fread (&nullobj,sizeof(nullobj),1,file);
        if (nullobj.active == ac_badobject)
            break;
        GetNewActor ();
        nullobj.state=(statetype *) ((uintptr_t)nullobj.state+(uintptr_t)&s_grdstand);
        // don't copy over the links
        memcpy (newobj,&nullobj,sizeof(nullobj)-8);
    }

    DiskFlopAnim(x,y);
    word laststatobjnum;
    fread (&laststatobjnum,sizeof(laststatobjnum),1,file);
    laststatobj=statobjlist+laststatobjnum;
    checksum = DoChecksum((byte *)&laststatobjnum,sizeof(laststatobjnum),checksum);

    DiskFlopAnim(x,y);
    for(i=0;i<MAXSTATS;i++)
    {
        fread(&nullstat,sizeof(nullstat),1,file);
        checksum = DoChecksum((byte *)&nullstat,sizeof(nullstat),checksum);
        nullstat.visspot=(byte *) ((uintptr_t)nullstat.visspot+(uintptr_t)spotvis);
        memcpy(statobjlist+i,&nullstat,sizeof(nullstat));
    }

    DiskFlopAnim(x,y);
    fread (doorposition,sizeof(doorposition),1,file);
    checksum = DoChecksum((byte *)doorposition,sizeof(doorposition),checksum);
    DiskFlopAnim(x,y);
    fread (doorobjlist,sizeof(doorobjlist),1,file);
    checksum = DoChecksum((byte *)doorobjlist,sizeof(doorobjlist),checksum);

    DiskFlopAnim(x,y);
    fread (&pwallstate,sizeof(pwallstate),1,file);
    checksum = DoChecksum((byte *)&pwallstate,sizeof(pwallstate),checksum);
    fread (&pwalltile,sizeof(pwalltile),1,file);
    checksum = DoChecksum((byte *)&pwalltile,sizeof(pwalltile),checksum);
    fread (&pwallx,sizeof(pwallx),1,file);
    checksum = DoChecksum((byte *)&pwallx,sizeof(pwallx),checksum);
    fread (&pwally,sizeof(pwally),1,file);
    checksum = DoChecksum((byte *)&pwally,sizeof(pwally),checksum);
    fread (&pwalldir,sizeof(pwalldir),1,file);
    checksum = DoChecksum((byte *)&pwalldir,sizeof(pwalldir),checksum);
    fread (&pwallpos,sizeof(pwallpos),1,file);
    checksum = DoChecksum((byte *)&pwallpos,sizeof(pwallpos),checksum);

    if (gamestate.secretcount)      // assign valid floorcodes under moved pushwalls
    {
        word *map, *obj; word tile, sprite;
        map = mapsegs[0]; obj = mapsegs[1];
        for (y=0;y<mapheight;y++)
            for (x=0;x<mapwidth;x++)
            {
                tile = *map++; sprite = *obj++;
                if (sprite == PUSHABLETILE && !tilemap[x][y]
                    && (tile < AREATILE || tile >= (AREATILE+NUMMAPS)))
                {
                    if (*map >= AREATILE)
                        tile = *map;
                    if (*(map-1-mapwidth) >= AREATILE)
                        tile = *(map-1-mapwidth);
                    if (*(map-1+mapwidth) >= AREATILE)
                        tile = *(map-1+mapwidth);
                    if ( *(map-2) >= AREATILE)
                        tile = *(map-2);

                    *(map-1) = tile; *(obj-1) = 0;
                }
            }
    }

    Thrust(0,0);    // set player->areanumber to the floortile you're standing on

    fread (&oldchecksum,sizeof(oldchecksum),1,file);

    fread (&lastgamemusicoffset,sizeof(lastgamemusicoffset),1,file);
    if(lastgamemusicoffset<0) lastgamemusicoffset=0;


    if (oldchecksum != checksum)
    {
        Message(STR_SAVECHT1"\n"
                STR_SAVECHT2"\n"
                STR_SAVECHT3"\n"
                STR_SAVECHT4);

        IN_ClearKeysDown();
        IN_Ack();

        gamestate.oldscore = gamestate.score = 0;
        gamestate.lives = 1;
        gamestate.weapon =
            gamestate.chosenweapon =
            gamestate.bestweapon = wp_pistol;
        gamestate.ammo = 8;
    }

    return true;
}
#endif
//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/
/*
void ShutdownId (void)
{
    US_Shutdown ();         // This line is completely useless...
    SD_Shutdown ();
    PM_Shutdown ();
    IN_Shutdown ();
    VW_Shutdown ();
    CA_Shutdown ();
#if defined(GP2X)
    GP2X_Shutdown();
#endif
}
*/

//===========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/

const float radtoint = (float)(FINEANGLES/2/PI);

void BuildTables (void)
{
#ifndef EMBEDDED2
    //
    // calculate fine tangents
    //

    int i;
    for(i=0;i<FINEANGLES/8;i++)
    {
        double tang=tan((i+0.5)/radtoint);
        finetangent[i]=(int32_t)(tang*GLOBAL1);
        finetangent[FINEANGLES/4-1-i]=(int32_t)((1/tang)*GLOBAL1);
    }

    //
    // costable overlays sintable with a quarter phase shift
    // ANGLES is assumed to be divisable by four
    //

    float angle=0;
    float anglestep=(float)(PI/2/ANGLEQUAD);
    for(i=0; i<ANGLEQUAD; i++)
    {
        fixed value=(int32_t)(GLOBAL1*sin(angle));
        sintable[i]=sintable[i+ANGLES]=sintable[ANGLES/2-i]=value;
        sintable[ANGLES-i]=sintable[ANGLES/2+i]=-value;
        angle+=anglestep;
    }
    sintable[ANGLEQUAD] = 65536;
    sintable[3*ANGLEQUAD] = -65536;

#if defined(USE_STARSKY) || defined(USE_RAIN) || defined(USE_SNOW)
    Init3DPoints();
#endif

#else
	int i;
	long    intang;
	int     halfview;
	double tang, angle, anglestep, facedist;
	fixed value;
	fixed scale;

	/* FIXME: Use linear interpolation to reduce size of trig tables. */
/* calculate fine tangents */

	angle = 0.0;
	anglestep = PI/2.0/ANGLEQUAD;

    for(i=0; i<ANGLEQUAD; i++)
    {
        fixed value=(int32_t)(GLOBAL1*sin(angle));
        sintable[i]=sintable[i+ANGLES]=sintable[ANGLES/2-i]=value;
        sintable[ANGLES-i]=sintable[ANGLES/2+i]=-value;
        angle+=anglestep;
    }
    sintable[ANGLEQUAD] = 65536;
    sintable[3*ANGLEQUAD] = -65536;

	finetangent[0] = 0;
	for (i = 1; i < FINEANGLES/8; i++) {
		tang = tan((double)i/radtoint);
		finetangent[i] = tang*TILEGLOBAL;
		finetangent[FINEANGLES/4-1-i] = TILEGLOBAL/tang;
	}
	
	/* fight off asymptotic behaviour at 90 degrees */
	finetangent[FINEANGLES/4-1] = finetangent[FINEANGLES/4-2]+1;
	
//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//

	angle = 0.0;
	anglestep = PI/2.0/ANGLEQUAD;

	for (i = 0; i < ANGLEQUAD; i++) {
	    value = GLOBAL1*sin(angle);
	    angle += anglestep;
	}

	facedist = 0x5800+MINDIST;
	halfview = 64;               /* half view in pixels */

/*
 calculate scale value for vertical height calculations
 and sprite x calculations
*/
	scale = halfview*facedist/(VIEWGLOBAL/2);

/* calculate the angle offset from view angle of each pixel's ray */
	for (i = 0; i < halfview; i++) 
	{
		tang = ((double)i)*VIEWGLOBAL/128/facedist;
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[halfview-1-i] = intang;
		pixelangle[halfview+i] = -intang;
	}
#endif
}

//===========================================================================


/*
====================
=
= CalcProjection
=
= Uses focallength
=
====================
*/

inline double FastArcTan(double x)
{
    return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
}

inline void CalcProjection (int32_t focal)
{
#ifdef EMBEDDED2
	long facedist;
#else	
    int     i;
    int    intang;
    //float   angle;
    //double  tang;
    //double  facedist;
	double  angle, tang;
	double facedist;
#endif
    int     halfview;

    focallength = focal;
    facedist = focal+MINDIST;
    halfview = viewwidth/2;                                 // half view in pixels

    //
    // calculate scale value for vertical height calculations
    // and sprite x calculations
    //
    scale = (fixed) (halfview*facedist/(VIEWGLOBAL/2));

    //
    // divide heightnumerator by a posts distance to get the posts height for
    // the heightbuffer.  The pixel height is height>>2
    //
    heightnumerator = (TILEGLOBAL*scale)>>6;

    //
    // calculate the angle offset from view angle of each pixel's ray
    //
#ifndef EMBEDDED2
    for (i=0;i<halfview;i++)
    {
        // start 1/2 pixel over, so viewangle bisects two middle pixels
        tang = (int32_t)i*VIEWGLOBAL/viewwidth/facedist;
// vbt
//

//		angle = /*(float)*/ atan(tang);

        angle = (double) FastArcTan(tang);
        intang = (int) (angle*radtoint);
        pixelangle[halfview-1-i] = intang;
        pixelangle[halfview+i] = -intang;
    }
#endif	
}



//===========================================================================

/*
===================
=
= SetupWalls
=
= Map tile values to scaled pics
=
===================
*/
#ifndef EMBEDDED
void SetupWalls (void)
{
    int     i;

    horizwall[0]=0;
    vertwall[0]=0;

    for (i=1;i<MAXWALLTILES;i++)
    {
        horizwall[i]=(i-1)*2;
        vertwall[i]=(i-1)*2+1;
    }
}
#endif
//===========================================================================

/*
==========================
=
= SignonScreen
=
==========================
*/

void SignonScreen (void)                        // VGA version
{
    VL_SetVGAPlaneMode ();
#define	SDDRV_ADDR	0x00202000
	GFS_Load(GFS_NameToId((Sint8*)"BOOTPAL.BIN"),0,(void *) SDDRV_ADDR,512);
//	VL_SetPalette ((SDL_Color *)SDDRV_ADDR);
//    SDL_SetColors(curSurface, SDDRV_ADDR, 0, 256);
extern void Pal2CRAM( Uint16 *Pal_Data , void *Col_Adr , Uint32 suu );
//	Pal2CRAM(SDDRV_ADDR , (void *)NBG1_COL_ADR , 256);
	Uint16 *Pal_Data  = (Uint16 *)SDDRV_ADDR;
	Uint16 *VRAM = (Uint16 *)NBG1_COL_ADR;
	
	for(unsigned int  i = 0; i < 256; i++ )
		*(VRAM++) = *(Pal_Data++);

	GFS_Load(GFS_NameToId((Sint8*)"BOOT.BIN"),0,(void *) SDDRV_ADDR,64000);

    VL_MungePic ((void *) SDDRV_ADDR,320,200);
//    VL_MemToScreen (signon,320,200,0,0);
    VL_MemToScreen ((byte*)SDDRV_ADDR,320,200,16,0);
}


/*
==========================
=
= FinishSignon
=
==========================
*/

void FinishSignon (void)
{
#ifndef SPEAR
	// vbt : fait un fond violet
    VW_Bar (0,189,300,11,VL_GetPixel(0,0));
    WindowX = 0;
    WindowW = SATURN_WIDTH;
    PrintY = 190;

    #ifndef JAPAN
    SETFONTCOLOR(14,4);

    #ifdef SPANISH
    US_CPrint ("Oprima una tecla");
    #else
    US_CPrint ("Press a key");
    #endif

    #endif
#ifndef USE_SPRITES	
    VH_UpdateScreen();
#endif	
    if (!param_nowait)
	{
        IN_Ack ();
	}

    #ifndef JAPAN
    VW_Bar (0,189,300,11,VL_GetPixel(0,0));

    PrintY = 190;
    SETFONTCOLOR(10,4);

    #ifdef SPANISH
    US_CPrint ("pensando...");
    #else
    US_CPrint ("Working...");
	
    #endif
#ifndef USE_SPRITES
    VH_UpdateScreen();
#endif
    #endif
    SETFONTCOLOR(0,15);
#else
#ifndef USE_SPRITES	
    VH_UpdateScreen();
#endif
    if (!param_nowait)
        VW_WaitVBL(3*70);
#endif
}

//===========================================================================

/*
=====================
=
= InitDigiMap
=
=====================
*/

// channel mapping:
//  -1: any non reserved channel
//   0: player weapons
//   1: boss weapons
//vbtvbt sound reference
#ifndef USE_ADX
static int wolfdigimap[] =
    {
        // These first sounds are in the upload version
#ifndef SPEAR
        HALTSND,                0,  -1,
        DOGBARKSND,             1,  -1,
        CLOSEDOORSND,           2,  -1,
        OPENDOORSND,            3,  -1,
        ATKMACHINEGUNSND,       4,   0,
        ATKPISTOLSND,           5,   0,
        ATKGATLINGSND,          6,   0,
        SCHUTZADSND,            7,  -1,
        GUTENTAGSND,            8,  -1,
        MUTTISND,               9,  -1,
        BOSSFIRESND,            10,  1,
        SSFIRESND,              11, -1,
        DEATHSCREAM1SND,        12, -1,
        DEATHSCREAM2SND,        13, -1,
        DEATHSCREAM3SND,        13, -1,
        TAKEDAMAGESND,          14, -1,
        PUSHWALLSND,            15, -1,

        LEBENSND,               20, -1,
        NAZIFIRESND,            21, -1,
        SLURPIESND,             22, -1,

        YEAHSND,                32, -1,

        // These are in all other episodes
        DOGDEATHSND,            16, -1,
        AHHHGSND,               17, -1,
        DIESND,                 18, -1,
        EVASND,                 19, -1,

        TOT_HUNDSND,            23, -1,
        MEINGOTTSND,            24, -1,
        SCHABBSHASND,           25, -1,
        HITLERHASND,            26, -1,
        SPIONSND,               27, -1,
        NEINSOVASSND,           28, -1,
        DOGATTACKSND,           29, -1,
        LEVELDONESND,           30, -1,
        MECHSTEPSND,            31, -1,

        SCHEISTSND,             33, -1,
#ifndef APOGEE_1_0		
        DEATHSCREAM4SND,        34, -1,         // AIIEEE
        DEATHSCREAM5SND,        35, -1,         // DEE-DEE
        DONNERSND,              36, -1,         // EPISODE 4 BOSS DIE
        EINESND,                37, -1,         // EPISODE 4 BOSS SIGHTING
        ERLAUBENSND,            38, -1,         // EPISODE 6 BOSS SIGHTING
        DEATHSCREAM6SND,        39, -1,         // FART
        DEATHSCREAM7SND,        40, -1,         // GASP
        DEATHSCREAM8SND,        41, -1,         // GUH-BOY!
        DEATHSCREAM9SND,        42, -1,         // AH GEEZ!
        KEINSND,                43, -1,         // EPISODE 5 BOSS SIGHTING
        MEINSND,                44, -1,         // EPISODE 6 BOSS DIE
        ROSESND,                45, -1,         // EPISODE 5 BOSS DIE
#endif		
#else
//
// SPEAR OF DESTINY DIGISOUNDS
//
        HALTSND,                0,  -1,
        CLOSEDOORSND,           2,  -1,
        OPENDOORSND,            3,  -1,
        ATKMACHINEGUNSND,       4,   0,
        ATKPISTOLSND,           5,   0,
        ATKGATLINGSND,          6,   0,
        SCHUTZADSND,            7,  -1,
        BOSSFIRESND,            8,   1,
        SSFIRESND,              9,  -1,
        DEATHSCREAM1SND,        10, -1,
        DEATHSCREAM2SND,        11, -1,
        TAKEDAMAGESND,          12, -1,
        PUSHWALLSND,            13, -1,
        AHHHGSND,               15, -1,
        LEBENSND,               16, -1,
        NAZIFIRESND,            17, -1,
        SLURPIESND,             18, -1,
        LEVELDONESND,           22, -1,
        DEATHSCREAM4SND,        23, -1,         // AIIEEE
        DEATHSCREAM3SND,        23, -1,         // DOUBLY-MAPPED!!!
        DEATHSCREAM5SND,        24, -1,         // DEE-DEE
        DEATHSCREAM6SND,        25, -1,         // FART
        DEATHSCREAM7SND,        26, -1,         // GASP
        DEATHSCREAM8SND,        27, -1,         // GUH-BOY!
        DEATHSCREAM9SND,        28, -1,         // AH GEEZ!
        GETGATLINGSND,          38, -1,         // Got Gat replacement

#ifndef SPEARDEMO
        DOGBARKSND,             1,  -1,
        DOGDEATHSND,            14, -1,
        SPIONSND,               19, -1,
        NEINSOVASSND,           20, -1,
        DOGATTACKSND,           21, -1,
        TRANSSIGHTSND,          29, -1,         // Trans Sight
        TRANSDEATHSND,          30, -1,         // Trans Death
        WILHELMSIGHTSND,        31, -1,         // Wilhelm Sight
        WILHELMDEATHSND,        32, -1,         // Wilhelm Death
        UBERDEATHSND,           33, -1,         // Uber Death
        KNIGHTSIGHTSND,         34, -1,         // Death Knight Sight
        KNIGHTDEATHSND,         35, -1,         // Death Knight Death
        ANGELSIGHTSND,          36, -1,         // Angel Sight
        ANGELDEATHSND,          37, -1,         // Angel Death
        GETSPEARSND,            39, -1,         // Got Spear replacement
#endif
#endif
        LASTSOUND
    };
#endif

void InitDigiMap (void)
{
#ifdef PONY	
#ifndef USE_ADX
    int *map;
	
	for (int i = 0; i < LASTSOUND; i++)
		DigiMap[i] = -1;	
	
    for (map = wolfdigimap; *map != LASTSOUND; map += 3)
    {
        DigiMap[map[0]] = map[1];
    }
#endif	
    for (int i = 0; i<73; i++)
    {
        SD_PrepareSound(i);
    }
#else
    int *map;
    for (map = wolfdigimap; *map != LASTSOUND; map += 3)
    {
        DigiMap[map[0]] = map[1];
//        DigiChannel[map[1]] = map[2];
        SD_PrepareSound(map[1]);
    }
#endif	
}
/*
#ifndef SPEAR
CP_iteminfo MusicItems={CTL_X,CTL_Y,6,0,32};
CP_itemtype MusicMenu[]=
    {
        {1,"Get Them!",0},
        {1,"Searching",0},
        {1,"P.O.W.",0},
        {1,"Suspense",0},
        {1,"War March",0},
        {1,"Around The Corner!",0},

        {1,"Nazi Anthem",0},
        {1,"Lurking...",0},
        {1,"Going After Hitler",0},
        {1,"Pounding Headache",0},
        {1,"Into the Dungeons",0},
        {1,"Ultimate Conquest",0},

        {1,"Kill the S.O.B.",0},
        {1,"The Nazi Rap",0},
        {1,"Twelfth Hour",0},
        {1,"Zero Hour",0},
        {1,"Ultimate Conquest",0},
        {1,"Wolfpack",0}
    };
#else
CP_iteminfo MusicItems={CTL_X,CTL_Y-20,9,0,32};
CP_itemtype MusicMenu[]=
    {
        {1,"Funky Colonel Bill",0},
        {1,"Death To The Nazis",0},
        {1,"Tiptoeing Around",0},
        {1,"Is This THE END?",0},
        {1,"Evil Incarnate",0},
        {1,"Jazzin' Them Nazis",0},
        {1,"Puttin' It To The Enemy",0},
        {1,"The SS Gonna Get You",0},
        {1,"Towering Above",0}
    };
#endif

#ifndef SPEARDEMO
void DoJukebox(void)
{
    int which,lastsong=-1;
    unsigned start;
    unsigned songs[]=
        {
#ifndef SPEAR
            GETTHEM_MUS,
            SEARCHN_MUS,
            POW_MUS,
            SUSPENSE_MUS,
            WARMARCH_MUS,
            CORNER_MUS,

            NAZI_OMI_MUS,
            PREGNANT_MUS,
            GOINGAFT_MUS,
            HEADACHE_MUS,
            DUNGEON_MUS,
            ULTIMATE_MUS,

            INTROCW3_MUS,
            NAZI_RAP_MUS,
            TWELFTH_MUS,
            ZEROHOUR_MUS,
            ULTIMATE_MUS,
            PACMAN_MUS
#else
            XFUNKIE_MUS,             // 0
            XDEATH_MUS,              // 2
            XTIPTOE_MUS,             // 4
            XTHEEND_MUS,             // 7
            XEVIL_MUS,               // 17
            XJAZNAZI_MUS,            // 18
            XPUTIT_MUS,              // 21
            XGETYOU_MUS,             // 22
            XTOWER2_MUS              // 23
#endif
        };

    IN_ClearKeysDown();

    MenuFadeOut();

#ifndef SPEAR
#ifndef UPLOAD
    start = ((SDL_GetTicks()/10)%3)*6;
#else
    start = 0;
#endif
#else
    start = 0;
#endif

    CA_CacheGrChunk (STARTFONT+1);
#ifdef SPEAR
    CacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
    CacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif

    fontnumber=1;
    ClearMScreen ();
    VWB_DrawPic(112,184,C_MOUSELBACKPIC);
    DrawStripes (10);
    SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);

#ifndef SPEAR
    DrawWindow (CTL_X-2,CTL_Y-6,280,13*7,BKGDCOLOR);
#else
    DrawWindow (CTL_X-2,CTL_Y-26,280,13*10,BKGDCOLOR);
#endif

    DrawMenu (&MusicItems,&MusicMenu[start]);

    SETFONTCOLOR (READHCOLOR,BKGDCOLOR);
    PrintY=15;
    WindowX = 0;
    WindowY = SATURN_WIDTH;
    US_CPrint ("Robert's Jukebox");

    SETFONTCOLOR (TEXTCOLOR,BKGDCOLOR);
    VW_UpdateScreen();
    MenuFadeIn();

    do
    {
        which = HandleMenu(&MusicItems,&MusicMenu[start],NULL);
        if (which>=0)
        {
            if (lastsong >= 0)
                MusicMenu[start+lastsong].active = 1;

            StartCPMusic(songs[start + which]);
            MusicMenu[start+which].active = 2;
            DrawMenu (&MusicItems,&MusicMenu[start]);
            VW_UpdateScreen();
            lastsong = which;
        }
    } while(which>=0);

    MenuFadeOut();
    IN_ClearKeysDown();
#ifdef SPEAR
    UnCacheLump (BACKDROP_LUMP_START,BACKDROP_LUMP_END);
#else
    UnCacheLump (CONTROLS_LUMP_START,CONTROLS_LUMP_END);
#endif
}
#endif
*/
/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

static void InitGame()
{
#ifndef SPEARDEMO
    boolean didjukebox=false;
#endif

#ifdef EMBEDDED
	for (int i = 0;i < MAPSIZE; i++)
	{
		farmapylookup[i] = i*64;
	}
#endif
    // initialize SDL
#if defined _WIN32
    putenv("SDL_VIDEODRIVER=directx");
#endif
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
//        printf("Unable to init SDL: %s\n", SDL_GetError());
        SYS_Exit(1);
    }
	SDL_SetVideoMode  (screenWidth, screenHeight, screenBits, NULL);
	SDL_Init(SDL_INIT_AUDIO);
//#ifndef REMDEBUG

//#endif
//    atexit(SDL_Quit);
    SignonScreen ();
//slPrint("slScrTransparent4",slLocate(1,17));		
	 slScrTransparent(NBG1ON);
	 
#if defined _WIN32
    if(!fullscreen)
    {
        struct SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        if(SDL_GetWMInfo(&wmInfo) != -1)
        {
            HWND hwndSDL = wmInfo.window;
            DWORD style = GetWindowLong(hwndSDL, GWL_STYLE) & ~WS_SYSMENU;
            SetWindowLong(hwndSDL, GWL_STYLE, style);
            SetWindowPos(hwndSDL, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
    }
#endif

//VL_SetPalette (gamepal);
//param_nowait = true;
////slPrint((char *)"VH_Startup     ",slLocate(10,12));
    VH_Startup ();
//    IN_Startup (); // VBT à remettre
////slPrint((char *)"PM_Startup     ",slLocate(10,12));
    PM_Startup ();
////slPrint((char *)"SD_Startup     ",slLocate(10,12));
    SD_Startup ();
////slPrint((char *)"CA_Startup     ",slLocate(10,12));
    CA_Startup ();
////slPrint((char *)"US_Startup     ",slLocate(10,12));
    US_Startup ();
    // TODO: Will any memory checking be needed someday??
//
// build some tables
////slPrint((char *)"InitDigiMap     ",slLocate(10,12));
    InitDigiMap ();
	viewsize = 19;                          // start with a good size

//    SetupSaveGames();

//
// HOLDING DOWN 'M' KEY?
//
/*
#ifndef SPEARDEMO
    if (Keyboard[sc_M])
    {
        DoJukebox();
        didjukebox=true;
    }
    else
#endif
*/
//
// draw intro screen stuff
//
  // VBT dessine les barres
//    IntroScreen ();
//
// load in and lock down some basic chunks
//
//slPrint((char *)"CA_CacheGrChunk1     ",slLocate(10,12));
    CA_CacheGrChunk(STARTFONT);
//slPrint((char *)"CA_CacheGrChunk2     ",slLocate(10,12));	
    CA_CacheGrChunk(STATUSBARPIC);
//slPrint((char *)"LoadLatchMem     ",slLocate(10,12));	
//	slTVOff();
   LoadLatchMem ();	   
//slPrint((char *)"BuildTables     ",slLocate(10,12));   
   BuildTables ();          // trig tables
#ifndef EMBEDDED   
    SetupWalls ();
#endif
//slPrint((char *)"NewViewSize     ",slLocate(10,12));   
    NewViewSize (viewsize);
//slPrint((char *)"slIntFunction     ",slLocate(10,12));	
	slIntFunction(VblIn) ;
//		slTVOn();
		slSynch();
//ajout VBT
slTVOff();
SDL_SetColors(curSurface, gamepal, 0, 256);
//	//slPrint("C4 - 2008-2009     www.rockin-b.de",slLocate(2,29));
//
// initialize variables
//
    InitRedShifts ();	
#ifndef SPEARDEMO
//    if(!didjukebox)
#endif
        FinishSignon();
#ifdef NOTYET
    vdisp = (byte *) (0xa0000+PAGE1START);
    vbuf = (byte *) (0xa0000+PAGE2START);
#endif
}

//===========================================================================

/*
==========================
=
= SetViewSize
=
==========================
*/

boolean SetViewSize (unsigned width, unsigned height)
{
    viewwidth = width&~15;                  // must be divisable by 16
    viewheight = height&~1;                 // must be even
    centerx = viewwidth/2-1;
    shootdelta = viewwidth/10;
    if((unsigned) viewheight == screenHeight)
        viewscreenx = viewscreeny = 0;
    else
    {
        viewscreenx = (screenWidth-viewwidth) / 2;
        viewscreeny = (screenHeight-scaleFactor*STATUSLINES-viewheight)/2;
    }
// calculate trace angles and projection constants
//
    CalcProjection (FOCALLENGTH);
#ifndef EMBEDDED	
	min_wallheight = viewheight;
#endif

#ifdef USE_SPRITES
//	VGAClearScreen ();
#endif
    return true;
}


void ShowViewSize (int width)
{
    int oldwidth,oldheight;

    oldwidth = viewwidth;
    oldheight = viewheight;

    if(width == 21)
    {
        viewwidth = screenWidth;
        viewheight = screenHeight;
        VWB_BarScaledCoord (0, 0, screenWidth, screenHeight, 0);
    }
    else if(width == 20)
    {
        viewwidth = screenWidth;
        viewheight = screenHeight - scaleFactor*STATUSLINES;
        DrawPlayScreen ();
    }
    else
    {
        viewwidth = width*16*screenWidth/SATURN_WIDTH;
        viewheight = (int) (width_to_height(width*16)*screenHeight/200);
        DrawPlayScreen ();
    }

    viewwidth = oldwidth;
    viewheight = oldheight;
}


void NewViewSize (int width)
{
    viewsize = width;
    if(viewsize == 21)
        SetViewSize(screenWidth, screenHeight);
    else if(viewsize == 20)
        SetViewSize(screenWidth, screenHeight - scaleFactor * STATUSLINES);
    else
        SetViewSize(width*16*screenWidth/SATURN_WIDTH, (unsigned) (width_to_height(width*16)*screenHeight/200));
// xxx	VGAClearScreen ();
}



//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit (const char *errorStr, ...)
{

#ifdef NOTYET
    byte *screen;
#endif
    char error[256];
	
    if(errorStr != NULL)
    {
        va_list vlist;
        va_start(vlist, errorStr);
        vsprintf(error, errorStr, vlist);
        va_end(vlist);
    }
    else error[0] = 0;
	
    if (!pictable)  // don't try to display the red box before it's loaded
    {
//        ShutdownId();
        if (error && *error)
        {
#ifdef NOTYET
            SetTextCursor(0,0);
#endif
            puts(error);
#ifdef NOTYET
            SetTextCursor(0,2);
#endif
            VW_WaitVBL(100);
        }
        SYS_Exit(1);
    }

    if (!error || !*error)
    {
#ifdef NOTYET
        #ifndef JAPAN
        CA_CacheGrChunk (ORDERSCREEN);
        screen = grsegs[ORDERSCREEN];
        #endif
#endif
    }
#ifdef NOTYET
    else
    {
        CA_CacheGrChunk (ERRORSCREEN);
        screen = grsegs[ERRORSCREEN];
    }
#endif

//    ShutdownId ();

    if (error && *error)
    {
#ifdef NOTYET
        memcpy((byte *)0xb8000,screen+7,7*160);
        SetTextCursor(9,3);
#endif
        puts(error);
#ifdef NOTYET
        SetTextCursor(0,7);
#endif
        VW_WaitVBL(200);
        SYS_Exit(1);
    }
    else
    if (!error || !(*error))
    {
#ifdef NOTYET
        #ifndef JAPAN
        memcpy((byte *)0xb8000,screen+7,24*160); // 24 for SPEAR/UPLOAD compatibility
        #endif
        SetTextCursor(0,23);
#endif
    }
    SYS_Exit(0);
}

//===========================================================================



/*
=====================
=
= DemoLoop
=
=====================
*/


static void DemoLoop()
{
#ifndef SPEARDEMO
    int LastDemo = 0;
#endif	
//
// check for launch from ted
//
/*    if (param_tedlevel != -1)
    {
        param_nowait = true;
        EnableEndGameMenuItem();
        NewGame(param_difficulty,0);

#ifndef SPEAR
        gamestate.episode = param_tedlevel/10;
        gamestate.mapon = param_tedlevel%10;
#else
        gamestate.episode = 0;
        gamestate.mapon = param_tedlevel;
#endif
        GameLoop();
        Quit (NULL);
    }
*/

//
// main game cycle
//

#ifndef DEMOTEST
/*
    #ifndef UPLOAD

        #ifndef GOODTIMES
        #ifndef SPEAR
        #ifndef JAPAN
		#if 0
        if (!param_nowait)
            NonShareware();
		#endif
        #endif
        #else
            #ifndef GOODTIMES
            #ifndef SPEARDEMO
            extern void CopyProtection(void);
            if(!param_goodtimes)
                CopyProtection();
            #endif
            #endif
        #endif
        #endif
    #endif
	*/
    //StartCPMusic(INTROSONG);
#ifndef JAPAN
//slPrint((char*)"PG13 or StartCPMusic",slLocate(10,22));	

//    if (!param_nowait)
//        PG13 ();
//    else
//slPrint((char*)"StartCPMusic",slLocate(10,22));	

		StartCPMusic(INTROSONG);
		
//slPrint((char*)"StartCPMusic end",slLocate(10,22));			
#endif

#endif
	param_nowait = false;
	
    while (1)
    {
        while (!param_nowait)
        {
//slPrint("slScrTransparent5",slLocate(1,17));				
			slScrTransparent(NBG1ON);
			slSynch(); // applique la non transparence
//
// title page
//
#ifndef DEMOTEST

#ifdef SPEAR
            SDL_Color pal[256];
            CA_CacheGrChunk (TITLEPALETTE);
            VL_ConvertPalette(grsegs[TITLEPALETTE], pal, 256);

            CA_CacheGrChunk (TITLE1PIC);
            VWB_DrawPic (SATURN_ADJUST,0,TITLE1PIC);
            UNCACHEGRCHUNK (TITLE1PIC);

            CA_CacheGrChunk (TITLE2PIC);
            VWB_DrawPic (SATURN_ADJUST,80,TITLE2PIC);
            UNCACHEGRCHUNK (TITLE2PIC);
#ifndef USE_SPRITES			
            VW_UpdateScreen ();
#endif
			slTVOn();
            VL_FadeIn(0,255,pal,30);

            UNCACHEGRCHUNK (TITLEPALETTE);
#else
//slPrint((char*)"CA_CacheScreen1",slLocate(10,22));	
            CA_CacheScreen (TITLEPIC);
//slPrint((char*)"VW_UpdateScreen1",slLocate(10,22));

#ifndef USE_SPRITES			
            VW_UpdateScreen ();
#endif
			slTVOn();
//slPrint((char*)"VW_FadeIn1",slLocate(10,22));				
            VW_FadeIn();
		
#endif
	// VBT déplacé
//slPrint((char*)"StartCPMusic",slLocate(10,22));	
	StartCPMusic(INTROSONG);
//slPrint((char*)"IN_UserInput1",slLocate(10,22));		
            if (IN_UserInput(TickBase*15))
                break;
//slPrint((char*)"VW_FadeOut1",slLocate(10,22));					
            VW_FadeOut();
//
// credits page
//
//slPrint((char*)"CA_CacheScreen",slLocate(10,22));
            CA_CacheScreen (CREDITSPIC);
#ifndef USE_SPRITES			
            VW_UpdateScreen();
#endif
//slPrint((char*)"VW_FadeIn2",slLocate(10,22));				
            VW_FadeIn ();
            if (IN_UserInput(TickBase*10))
                break;
//slPrint((char*)"VW_FadeOut2",slLocate(10,22));			
            VW_FadeOut ();
//
// high scores
//
            DrawHighScores ();
#ifndef USE_SPRITES			
            VW_UpdateScreen ();
#endif			
            VW_FadeIn ();
            if (IN_UserInput(TickBase*10))
                break;
#endif
//
// demo
//
//slPrint((char*)"VW_FadeOut2",slLocate(10,22));
            #ifndef SPEARDEMO
            PlayDemo (LastDemo++%4);
            #else
            PlayDemo (0);
            #endif

            if (playstate == ex_abort)
                break;
            VW_FadeOut();
            if(screenHeight % 200 != 0)
                VL_ClearScreen(0);
            StartCPMusic(INTROSONG);
        }
        VW_FadeOut ();
#ifdef DEBUGKEYS
        if (Keyboard[sc_Tab] && param_debugmode)
            RecordDemo ();
        else
		{
            US_ControlPanel (0);
		}
#else
        US_ControlPanel (0);
#endif
        if (startgame)
        {
//			slIntFunction(VblIn) ;
            GameLoop ();
            if(!param_nowait)
            {
                VW_FadeOut();
                StartCPMusic(INTROSONG);
            }
        }
    }
}


/*
==========================
=
= main
=
==========================
*/

int main (int argc, char *argv[])
{
	
#define		SystemWork		0x060ffc00		/* System Variable Address */
#define		SystemSize		(0x06100000-0x060ffc00)		/* System Variable Size */
	
	extern Uint32 _bstart, _bend;
	Uint8	*dst;
	Uint32	i;

	/* 1.Zero Set .bss Section */
	for( dst = (Uint8 *)&_bstart; dst < (Uint8 *)&_bend; dst++ ) {
		*dst = 0;
	}
	/* 2.ROM has data at end of text; copy it. */

	/* 3.SGL System Variable Clear */
	for( dst = (Uint8 *)SystemWork, i = 0;i < SystemSize; i++) {
		*dst = 0;
	}	
	
	
#if defined(_arch_dreamcast)
    DC_Main();
    DC_CheckParameters();
#elif defined(GP2X)
    GP2X_Init();
#else
	if ( SDL_Init(NULL) < 0 ) {
		return(FALSE);
	}
#ifndef REMDEBUG
//	slIntFunction(VblIn) ;
#endif	
    //CheckParameters(argc, argv);
#endif
// vbt : invincible
	godmode = 1;
    CheckForEpisodes();

    InitGame();
//slPrintHex(screen->pixels,slLocate(20,14));
//slPrint((char*)"DemoLoop",slLocate(10,22));	

    DemoLoop();

    Quit("Demo loop exited???");
    return 1;
}

