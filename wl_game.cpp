// WL_GAME.C
//#define USE_SPRITES 1
#include <math.h>
#include "wl_def.h"
//#include <SDL_mixer.h>

#ifdef MYPROFILE
#include <TIME.H>
#endif
extern "C"{
//extern fixed slAtan(fixed y, fixed x);
extern fixed MTH_Atan(fixed y, fixed x);
}
short atan2fix(fixed x, fixed y);

#ifdef USE_SPRITES
unsigned int position_vram=((SATURN_WIDTH+64)*64);
//unsigned int static_items=0;
extern unsigned char wall_buffer[(SATURN_WIDTH+64)*64];
extern TEXTURE tex_spr[SPR_NULLSPRITE+SATURN_WIDTH];
unsigned char texture_list[SPR_NULLSPRITE];
#endif

#ifdef EMBEDDED
//boolean loadedgame;
extern boolean startgame;
#endif
    int ChunksInFile = 0;

void readChunks(Sint32 fileId, uint32_t size, uint32_t *pageOffsets, Uint8 *Chunks, uint8_t *ptr);
//uint8_t *PM_DecodeSprites2(unsigned int start,unsigned int endi,uint32_t* pageOffsets,word *pageLengths,uint8_t *ptr, Sint32 fileId);

#undef atan2
//#define atan2(a,b) slAtan(a,b)
#define atan2(a,b) MTH_Atan(a,b)
/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/


/*
=============================================================================

                             GLOBAL VARIABLES

=============================================================================
*/

boolean         ingame,fizzlein;
gametype        gamestate;
byte            bordercol=VIEWCOLOR;        // color of the Change View/Ingame border

#ifdef SPEAR
int32_t         spearx,speary;
unsigned        spearangle;
boolean         spearflag;
#endif

#ifdef USE_FEATUREFLAGS
int ffDataTopLeft, ffDataTopRight, ffDataBottomLeft, ffDataBottomRight;
#endif

//
// ELEVATOR BACK MAPS - REMEMBER (-1)!!
//
int ElevatorBackTo[]={1,1,7,3,5,3};

//void SetupGameLevel (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
void GameLoop (void);

/*
=============================================================================

                             LOCAL VARIABLES

=============================================================================
*/

/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/




static int ScanInfoPlane(Sint32 fileId,uint32_t* pageOffsets,word *pageLengths)
{
    unsigned x,y;
    int      tile;
	int 	loaded=0;
    word     *start;
int current;
//-----------------------------------------------------------------------------------
	uint8_t *itemmap = (uint8_t *)saturnChunk+0x4000; // ne pas toucher
//-----------------------------------------------------------------------------------

    start = mapsegs[1];
    for (y=0;y<mapheight;y++)
    {
        for (x=0;x<mapwidth;x++)
        {
            tile = *start++;
            if (!tile)
                continue;

            switch (tile)
            {
                case 19:
                case 20:
                case 21:
                case 22:
                    SpawnPlayer(x,y,NORTH+tile-19);
                    break;

                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                case 28:
                case 29:
                case 30:

                case 31:
                case 32:
                case 33:
                case 34:
                case 35:
                case 36:
                case 37:
                case 38:

                case 39:
                case 40:
                case 41:
                case 42:
                case 43:
                case 44:
                case 45:
                case 46:

                case 47:
                case 48:
                case 49:
                case 50:
                case 51:
                case 52:
                case 53:
                case 54:

                case 55:
                case 56:
                case 57:
                case 58:
                case 59:
                case 60:
                case 61:
                case 62:

                case 63:
                case 64:
                case 65:
                case 66:
                case 67:
                case 68:
                case 69:
                case 70:
                case 71:
                case 72:
#ifdef SPEAR
                case 73:                        // TRUCK AND SPEAR!
                case 74:
#elif defined(USE_DIR3DSPR)                     // just for the example
                case 73:
#endif
                    SpawnStatic(x,y,tile-23);
					current = ((int)(laststatobj-&statobjlist[0]))-1;
					loaded += PRELOAD_ITEMS (statobjlist[current].shapenum,statobjlist[current].shapenum);
                    break;

//
// P wall
//
                case 98:
                    //if (!loadedgame)
                        gamestate.secrettotal++;
                    break;

#ifndef SPEAR
                case 99:
					loaded += PRELOAD_ITEMS (SPR_DEATHCAM,SPR_DEATHCAM);
					loaded += PRELOAD_ITEMS (SPR_BJ_W1,SPR_BJ_JUMP4);
                   break;
#endif
//
// guard
//
                case 180:
                case 181:
                case 182:
                case 183:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 144:
                case 145:
                case 146:
                case 147:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 108:
                case 109:
                case 110:
                case 111:
                    SpawnStand(en_guard,x,y,tile-108);
					loaded += PRELOAD_ITEMS (SPR_GRD_S_1,SPR_GRD_SHOOT3);
                    break;
                case 184:
                case 185:
                case 186:
                case 187:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 148:
                case 149:
                case 150:
                case 151:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 112:
                case 113:
                case 114:
                case 115:
                    SpawnPatrol(en_guard,x,y,tile-112);
					loaded += PRELOAD_ITEMS (SPR_GRD_S_1,SPR_GRD_SHOOT3);
                    break;

                case 124:
                    SpawnDeadGuard (x,y);
					loaded += PRELOAD_ITEMS (SPR_GRD_DEAD,SPR_GRD_DEAD);
                    break;
//
// officer
//
                case 188:
                case 189:
                case 190:
                case 191:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 152:
                case 153:
                case 154:
                case 155:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 116:
                case 117:
                case 118:
                case 119:
                    SpawnStand(en_officer,x,y,tile-116);
					loaded += PRELOAD_ITEMS (SPR_OFC_S_1,SPR_OFC_SHOOT3);
                    break;


                case 192:
                case 193:
                case 194:
                case 195:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 156:
                case 157:
                case 158:
                case 159:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 120:
                case 121:
                case 122:
                case 123:
                    SpawnPatrol(en_officer,x,y,tile-120);
					loaded += PRELOAD_ITEMS (SPR_OFC_S_1,SPR_OFC_SHOOT3);
                    break;


//
// ss
//
                case 198:
                case 199:
                case 200:
                case 201:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 162:
                case 163:
                case 164:
                case 165:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 126:
                case 127:
                case 128:
                case 129:
                    SpawnStand(en_ss,x,y,tile-126);
					loaded += PRELOAD_ITEMS (SPR_SS_S_1,SPR_SS_SHOOT3);
                    break;


                case 202:
                case 203:
                case 204:
                case 205:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 166:
                case 167:
                case 168:
                case 169:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 130:
                case 131:
                case 132:
                case 133:
                    SpawnPatrol(en_ss,x,y,tile-130);
					loaded += PRELOAD_ITEMS (SPR_SS_S_1,SPR_SS_SHOOT3);
                    break;

//
// dogs
//
                case 206:
                case 207:
                case 208:
                case 209:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 170:
                case 171:
                case 172:
                case 173:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 134:
                case 135:
                case 136:
                case 137:
                    SpawnStand(en_dog,x,y,tile-134);
					loaded += PRELOAD_ITEMS (SPR_DOG_W1_1,SPR_DOG_JUMP3);
                    break;

                case 210:
                case 211:
                case 212:
                case 213:
                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 36;
                case 174:
                case 175:
                case 176:
                case 177:
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 36;
                case 138:
                case 139:
                case 140:
                case 141:
                    SpawnPatrol(en_dog,x,y,tile-138);
					loaded += PRELOAD_ITEMS (SPR_DOG_W1_1,SPR_DOG_JUMP3);
                    break;

//
// boss
//
#ifndef SPEAR
                case 214:
                    SpawnBoss (x,y);
					loaded += PRELOAD_ITEMS (SPR_BOSS_W1,SPR_BOSS_DIE3);
                    break;
                case 197:
                    SpawnGretel (x,y);
					loaded += PRELOAD_ITEMS (SPR_GRETEL_W1,SPR_GRETEL_DIE3);
                    break;
                case 215:
                    SpawnGift (x,y);
					loaded += PRELOAD_ITEMS (SPR_GIFT_W1,SPR_GIFT_DEAD);
					loaded += PRELOAD_ITEMS (SPR_ROCKET_1,SPR_BOOM_3);
                    break;
                case 179:
                    SpawnFat (x,y);
					loaded += PRELOAD_ITEMS (SPR_FAT_W1,SPR_FAT_DEAD);
					loaded += PRELOAD_ITEMS (SPR_ROCKET_1,SPR_BOOM_3);
                    break;
                case 196:
                    SpawnSchabbs (x,y);
					loaded += PRELOAD_ITEMS (SPR_SCHABB_W1,SPR_HYPO4);
                    break;
                case 160:
                    SpawnFakeHitler (x,y);
					loaded += PRELOAD_ITEMS (SPR_FAKE_W1,SPR_FAKE_DEAD);
                    break;
                case 178:
                    SpawnHitler (x,y);
					loaded += PRELOAD_ITEMS (SPR_MECHA_W1,SPR_HITLER_DIE7);
                    break;
#else
                case 106:
                    SpawnSpectre (x,y);
					loaded += PRELOAD_ITEMS (SPR_SPECTRE_W1,SPR_SPECTRE_F4);
                    break;
                case 107:
                    SpawnAngel (x,y);
					loaded += PRELOAD_ITEMS (SPR_ANGEL_W1,SPR_ANGEL_DEAD);
                    break;
                case 125:
                    SpawnTrans (x,y);
					loaded += PRELOAD_ITEMS (SPR_TRANS_W1,SPR_TRANS_DIE3);
                    break;
                case 142:
                    SpawnUber (x,y);
					loaded += PRELOAD_ITEMS (SPR_UBER_W1,SPR_UBER_DEAD);
                    break;
                case 143:
                    SpawnWill (x,y);
					loaded += PRELOAD_ITEMS (SPR_WILL_W1,SPR_WILL_DEAD);
                    break;
                case 161:
                    SpawnDeath (x,y);
					loaded += PRELOAD_ITEMS (SPR_DEATH_W1,SPR_DEATH_DEAD);
					loaded += PRELOAD_ITEMS (SPR_HROCKET_1,SPR_SPARK4);
                    break;
#endif

//
// mutants
//
                case 252:
                case 253:
                case 254:
                case 255:
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);

                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 18;
                case 234:
                case 235:
                case 236:
                case 237:
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);

                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 18;
                case 216:
                case 217:
                case 218:
                case 219:
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);

                    SpawnStand(en_mutant,x,y,tile-216);
                    break;

                case 256:
                case 257:
                case 258:
                case 259:
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);

                    if (gamestate.difficulty<gd_hard)
                        break;
                    tile -= 18;
                case 238:
                case 239:
                case 240:
                case 241:
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);
				
                    if (gamestate.difficulty<gd_medium)
                        break;
                    tile -= 18;
                case 220:
                case 221:
                case 222:
                case 223:
                    SpawnPatrol(en_mutant,x,y,tile-220);
					loaded += PRELOAD_ITEMS (SPR_MUT_S_1,SPR_MUT_SHOOT4);
                    break;

//
// ghosts
//
#ifndef SPEAR
                case 224:
                    SpawnGhosts (en_blinky,x,y);
					loaded += PRELOAD_ITEMS (SPR_BLINKY_W1,SPR_BLINKY_W2);
                    break;
                case 225:
                    SpawnGhosts (en_clyde,x,y);
					loaded += PRELOAD_ITEMS (SPR_CLYDE_W1,SPR_CLYDE_W2);
                    break;
                case 226:
                    SpawnGhosts (en_pinky,x,y);
					loaded += PRELOAD_ITEMS (SPR_PINKY_W1,SPR_PINKY_W2);
                    break;
                case 227:
                    SpawnGhosts (en_inky,x,y);
					loaded += PRELOAD_ITEMS (SPR_INKY_W1,SPR_INKY_W2);
                    break;
#endif
            }
        }
    }
#ifndef SPEAR	
	/*
	if(gamestate.mapon == 8)
	{
		loaded += PRELOAD_ITEMS (SPR_DEATHCAM,SPR_DEATHCAM);
		loaded += PRELOAD_ITEMS (SPR_BJ_W1,SPR_BJ_JUMP4);
	}
	*/
#endif	
	return loaded;
}

//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/
void VblIn(void);

	uint8_t *wallData = NULL;

int SetupGameLevel (void)
{
    int  x,y;
    word *map;
    word tile;

    //if (!loadedgame)
    {
        gamestate.TimeCount
            = gamestate.secrettotal
            = gamestate.killtotal
            = gamestate.treasuretotal
            = gamestate.secretcount
            = gamestate.killcount
            = gamestate.treasurecount
            = pwallstate = pwallpos = facetimes = 0;
        LastAttacker = NULL;
        killerobj = NULL;
    }

    if (demoplayback)
        US_InitRndT (false);
    else
        US_InitRndT (true);
//
// load the level
//
    CA_CacheMap (gamestate.mapon+10*gamestate.episode);
    mapon-=gamestate.episode*10;
#ifdef USE_FEATUREFLAGS
    // Temporary definition to make things clearer
    #define MXX MAPSIZE - 1

    // Read feature flags data from map corners and overwrite corners with adjacent tiles
    ffDataTopLeft     = MAPSPOT(0,   0,   0); MAPSPOT(0,   0,   0) = MAPSPOT(1,       0,       0);
    ffDataTopRight    = MAPSPOT(MXX, 0,   0); MAPSPOT(MXX, 0,   0) = MAPSPOT(MXX,     1,       0);
    ffDataBottomRight = MAPSPOT(MXX, MXX, 0); MAPSPOT(MXX, MXX, 0) = MAPSPOT(MXX - 1, MXX,     0);
    ffDataBottomLeft  = MAPSPOT(0,   MXX, 0); MAPSPOT(0,   MXX, 0) = MAPSPOT(0,       MXX - 1, 0);

    #undef MXX
#endif
//
// copy the wall data to a data segment array
//
    memset (tilemap,0,sizeof(tilemap));
    memset (actorat,0,sizeof(actorat));
	memset(objactor,0,sizeof(objactor));	

	InitActorList();	/* start spawning things with a clean slate */
	InitDoorList();
	InitStaticList();

/*---------------------------------------------------------------*/
#if 1
    char fname[13] = "VSWAP.";
//	Uint32 i=0;
	Uint8 *Chunks;
	long fileSize;
	
    strcat(fname,extension);
	
	Sint32 fileId;

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

	Chunks=(Uint8*)saturnChunk;
	GFS_Load(fileId, 0, (void *)Chunks, 0x2000);
	ChunksInFile =Chunks[0]|Chunks[1]<<8;
	PMSpriteStart=Chunks[2]|Chunks[3]<<8;

// vbt : on ne charge pas les sons !	
	ChunksInFile=Chunks[4]|Chunks[5]<<8;

	uint32_t* pageOffsets = (uint32_t *)saturnChunk+0x2000; 
	word *pageLengths = (word *)saturnChunk+(ChunksInFile + 1) * sizeof(int32_t);
 
	for(int i=0;i<ChunksInFile+1;i++)
	{
		pageOffsets[i]=Chunks[6]<<0|Chunks[7]<<8|Chunks[8]<<16|Chunks[9]<<24;
		Chunks+=4;
	}

	for(int i=PMSpriteStart;i<ChunksInFile+1;i++)
	{
		pageLengths[i-PMSpriteStart]=Chunks[6]|Chunks[7]<<8;
		Chunks+=2;
	}
	
    //fread(pageLengths, sizeof(word), ChunksInFile, file);
    long pageDataSize = fileSize - pageOffsets[0];
    if(pageDataSize > (size_t) -1)
        Quit("The page file \"%s\" is too large!", fname);

//    pageOffsets[ChunksInFile] = fileSize;

#endif

	uint8_t *itemmap = (uint8_t *)saturnChunk+0x4000; 
	memset(itemmap,0x00,0x2000); // itemmap et itemmap communs, ne pas toucher à la taille du memset

/*---------------------------------------------------------------*/
    map = mapsegs[0];
    for (y=0;y<mapheight;y++)
    {
        for (x=0;x<mapwidth;x++)
        {
            tile = *map++;


			
            if (tile<AREATILE)
            {
				itemmap[tile]=1;				
                // solid wall

                tilemap[x][y] = (byte) tile;
				
#ifdef EMBEDDED
				set_wall_at(x, y, tile);
#else
                actorat[x][y] = (objtype *)(uintptr_t) tile;
#endif				
            }
            else
            {
                // area floor
                tilemap[x][y] = 0;
#ifndef EMBEDDED				
                actorat[x][y] = 0;
#endif				
            }
        }
    }
	
//
// spawn doors
//
    map = mapsegs[0];
    for (y=0;y<mapheight;y++)
    {
        for (x=0;x<mapwidth;x++)
        {
            tile = *map++;
            if (tile >= 90 && tile <= 101)
            {
                // door
                switch (tile)
                {
                    case 90:
                    case 92:
                    case 94:
                    case 96:
                    case 98:
                    case 100:
                        SpawnDoor (x,y,1,(tile-90)/2);
                        break;
                    case 91:
                    case 93:
                    case 95:
                    case 97:
                    case 99:
                    case 101:
                        SpawnDoor (x,y,0,(tile-91)/2);
                        break;
                }
            }
        }
    }
// vbt : on recharge la vram
#ifdef USE_SPRITES
	memset(texture_list,0x00,SPR_NULLSPRITE);
	position_vram = (SATURN_WIDTH+64)*64;

	if(viewheight == screenHeight)
		VL_ClearScreen(0);	
	//	VGAClearScreen ();
#endif
//
// spawn actors
//
	
    int totaloaded = ScanInfoPlane (fileId, pageOffsets, pageLengths); // on charge les persos
//
// take out the ambush markers
//
    map = mapsegs[0];
    for (y=0;y<mapheight;y++)
    {
        for (x=0;x<mapwidth;x++)
        {
            tile = *map++;
			
            if (tile == AMBUSHTILE)
            {
                tilemap[x][y] = 0;
//                if ( (unsigned)(uintptr_t)actorat[x][y] == AMBUSHTILE)
//                    actorat[x][y] = NULL;
				if (get_actor_at(x, y) == AMBUSHTILE)
					clear_actor(x, y);

                if (*map >= AREATILE)
                    tile = *map;
                if (*(map-1-mapwidth) >= AREATILE)
                    tile = *(map-1-mapwidth);
                if (*(map-1+mapwidth) >= AREATILE)
                    tile = *(map-1+mapwidth);
                if ( *(map-2) >= AREATILE)
                    tile = *(map-2);
                *(map-1) = tile;
            }
        }
    }

    int total = (int)(laststatobj-&statobjlist[0]);
//totaloaded=0;
    for (int i=0;i<=total;i++)
    {
        if (statobjlist[i].shapenum != -1)
		{
			itemmap[statobjlist[i].shapenum+PMSpriteStart]=1;
//			totaloaded++;
		}
    }

	slScrTransparent(0);
	slSynch();
	extern const void * TransList;
	memset((void *)TransList,0x00,0xf0*4*3);
	return totaloaded;
}


//==========================================================================


/*
===================
=
= DrawPlayBorderSides
=
= To fix window overwrites
=
===================
*/
void DrawPlayBorderSides(void)
{
    if(viewsize == 21) return;

	const int sw = screenWidth;
	const int sh = screenHeight;
	const int vw = viewwidth;
	const int vh = viewheight;
	const int px = scaleFactor; // size of one "pixel"

	const int h  = sh - px * STATUSLINES;
	const int xl = sw / 2 - vw / 2;
	const int yl = (h - vh) / 2;

    if(xl != 0)
    {
	    VWB_BarScaledCoord(0,            0, xl - px,     h, bordercol);                 // left side
	    VWB_BarScaledCoord(xl + vw + px, 0, xl - px * 2, h, bordercol);                 // right side
    }

    if(yl != 0)
    {
	    VWB_BarScaledCoord(0, 0,            sw, yl - px, bordercol);                    // upper side
	    VWB_BarScaledCoord(0, yl + vh + px, sw, yl - px, bordercol);                    // lower side
    }

    if(xl != 0)
    {
        // Paint game view border lines
	    VWB_BarScaledCoord(xl - px, yl - px, vw + px, px,          160);                      // upper border
	    VWB_BarScaledCoord(xl,      yl + vh, vw + px, px,          bordercol - 2);          // lower border
	    VWB_BarScaledCoord(xl - px, yl - px, px,      vh + px,     160);                      // left border
	    VWB_BarScaledCoord(xl + vw, yl - px, px,      vh + px * 2, bordercol - 2);          // right border
	    VWB_BarScaledCoord(xl - px, yl + vh, px,      px,          bordercol - 3);          // lower left highlight
    }
    else
    {
        // Just paint a lower border line
        VWB_BarScaledCoord(0, yl+vh, vw, px, bordercol-2);       // lower border
    }
}


/*
===================
=
= DrawStatusBorder
=
===================
*/

void DrawStatusBorder (byte color)
{
 /*   int statusborderw = (screenWidth-scaleFactor*SATURN_WIDTH)/2;

    VWB_BarScaledCoord (0,0,screenWidth,screenHeight-scaleFactor*(STATUSLINES-3),color);
    VWB_BarScaledCoord (0,screenHeight-scaleFactor*(STATUSLINES-3),
        statusborderw+scaleFactor*8,scaleFactor*(STATUSLINES-4),color);
    VWB_BarScaledCoord (0,screenHeight-scaleFactor*2,screenWidth,scaleFactor*2,color);
    VWB_BarScaledCoord (screenWidth-statusborderw-scaleFactor*8, screenHeight-scaleFactor*(STATUSLINES-3),
        statusborderw+scaleFactor*8,scaleFactor*(STATUSLINES-4),color);

    VWB_BarScaledCoord (statusborderw+scaleFactor*9, screenHeight-scaleFactor*3,
        scaleFactor*97, scaleFactor*1, color-1);
    VWB_BarScaledCoord (statusborderw+scaleFactor*106, screenHeight-scaleFactor*3,
        scaleFactor*161, scaleFactor*1, color-2);
    VWB_BarScaledCoord (statusborderw+scaleFactor*267, screenHeight-scaleFactor*3,
        scaleFactor*44, scaleFactor*1, color-3);
    VWB_BarScaledCoord (screenWidth-statusborderw-scaleFactor*9, screenHeight-scaleFactor*(STATUSLINES-4),
        scaleFactor*1, scaleFactor*20, color-2);
    VWB_BarScaledCoord (screenWidth-statusborderw-scaleFactor*9, screenHeight-scaleFactor*(STATUSLINES/2-4),
        scaleFactor*1, scaleFactor*14, color-3);*/
}


/*
===================
=
= DrawPlayBorder
=
===================
*/

inline void DrawPlayBorder (void)
{
	const int px = scaleFactor; // size of one "pixel"

    if (bordercol != VIEWCOLOR)
        DrawStatusBorder(bordercol);
    else
    {
        const int statusborderw = (screenWidth-px*SATURN_WIDTH)/2;
#if SATURN_WIDTH==352		
        VWB_BarScaledCoord (0, screenHeight-px*STATUSLINES,
            8+statusborderw+px*8, px*STATUSLINES, bordercol);
        VWB_BarScaledCoord (screenWidth-8-statusborderw-px*8, screenHeight-px*STATUSLINES,
            8+statusborderw+px*8, px*STATUSLINES, bordercol);
#else
        VWB_BarScaledCoord (0, screenHeight-px*STATUSLINES,
            statusborderw+px*8, px*STATUSLINES, bordercol);
        VWB_BarScaledCoord (screenWidth-statusborderw-px*8, screenHeight-px*STATUSLINES,
            statusborderw+px*8, px*STATUSLINES, bordercol);
#endif
    }

    VWB_BarScaledCoord (0,0,screenWidth,screenHeight-px*STATUSLINES,bordercol);

    const int xl = screenWidth/2-viewwidth/2;
    const int yl = (screenHeight-px*STATUSLINES-viewheight)/2;
    VWB_BarScaledCoord (xl,yl,viewwidth,viewheight,0);

    if(xl != 0)
//    if(viewsize == 19)
    {
		SPRITE *sys_clip = (SPRITE *) SpriteVRAM;
		(*sys_clip).XC = (xl+viewwidth)-1;
		(*sys_clip).YC = (yl+viewheight)-1;
	
		slWindow(xl , yl, (xl+viewwidth)-1 , (yl+viewheight)-1, 300 , screenWidth/2, (yl*2+viewheight)/2);
//viewwidth+px =321	
//xl=16	
//xl-px=15
//yl=9
        // Paint game view border lines
        VWB_BarScaledCoord(xl-px, yl-px, viewwidth+px, px, 160);                      // upper border
        VWB_BarScaledCoord(xl, yl+viewheight, viewwidth+px, px, bordercol-2);       // lower border
        VWB_BarScaledCoord(xl-px, yl-px, px, viewheight+px, 160);                     // left border
        VWB_BarScaledCoord(xl+viewwidth, yl-px, px, viewheight+2*px, bordercol-2);  // right border
        VWB_BarScaledCoord(xl-px, yl+viewheight, px, px, bordercol-3);              // lower left highlight
    }
    else
    {
		SPRITE *sys_clip = (SPRITE *) SpriteVRAM;
		(*sys_clip).XC = SATURN_WIDTH-1;
		(*sys_clip).YC = 239;

		slWindow(0 , 0, SATURN_WIDTH-1 , 239 , 300 , screenWidth/2, screenHeight/2);		
        // Just paint a lower border line
        VWB_BarScaledCoord(0, yl+viewheight, viewwidth, px, bordercol-2);       // lower border
    }
}


/*
===================
=
= DrawPlayScreen
=
===================
*/

void DrawPlayScreen (void)
{
	//vbt à remettre
  	//		slPrint("VWB_DrawPicScaledCoord",slLocate(10,10));
	VWB_DrawPicScaledCoord (SATURN_ADJUST+(screenWidth-scaleFactor*SATURN_WIDTH)/2,screenHeight-scaleFactor*STATUSLINES,STATUSBARPIC);
  	//		slPrint("DrawPlayBorder",slLocate(10,11));
    DrawPlayBorder ();
}

void DrawStatusBar (void)
{
	//vbt à remettre
  	//		slPrint("VWB_DrawPicScaledCoord",slLocate(10,10));
//			VWB_DrawPicScaledCoord ((screenWidth-scaleFactor*SATURN_WIDTH)/2,screenHeight-scaleFactor*STATUSLINES,STATUSBARPIC);
  	//		slPrint("DrawPlayBorder",slLocate(10,11));
//    DrawPlayBorder ();
    DrawFace (); // deja fait dans update face
    DrawHealth ();
    DrawLives ();
    DrawLevel ();
    DrawAmmo ();
    DrawKeys ();
    DrawWeapon ();
    DrawScore ();
}

/*
// Uses LatchDrawPic instead of StatusDrawPic
void LatchNumberHERE (int x, int y, unsigned width, int32_t number)
{
    unsigned length,c;
    char str[20];

    ltoa (number,str,10);

    length = (unsigned) strlen (str);

    while (length<width)
    {
        LatchDrawPic (x,y,N_BLANKPIC);
        x++;
        width--;
    }

    c = length <= width ? 0 : length-width;

    while (c<length)
    {
        LatchDrawPic (x,y,str[c]-'0'+ N_0PIC);
        x++;
        c++;
    }
}

void ShowActStatus()
{
    // Draw status bar without borders
    byte *source = grsegs[STATUSBARPIC];
    int	picnum = STATUSBARPIC - STARTPICS;
    int width = pictable[picnum].width;
    int height = pictable[picnum].height;
    int destx = (screenWidth-scaleFactor*SATURN_WIDTH)/2 + 9 * scaleFactor;
    int desty = screenHeight - (height - 4) * scaleFactor;
    VL_MemToScreenScaledCoord(source, width, height, 9, 4, destx, desty, width - 18, height - 7);

    ingame = false;
    DrawFace ();
    DrawHealth ();
    DrawLives ();
    DrawLevel ();
    DrawAmmo ();
    DrawKeys ();
//    DrawWeapon ();
    DrawScore ();
    ingame = true;
}
*/
/*
==================
=
= PlayDemo
=
= Fades the screen out, then starts a demo.  Exits with the screen unfaded
=
==================
*/

void PlayDemo (int demonumber)
{
    int length;
#ifdef DEMOSEXTERN
// debug: load chunk
#ifndef SPEARDEMO
    int dems[4]={T_DEMO0,T_DEMO1,T_DEMO2,T_DEMO3};
#else
    int dems[1]={T_DEMO0};
#endif

    CA_CacheGrChunk(dems[demonumber]);
    demoptr = (int8_t *) grsegs[dems[demonumber]];
#else
    demoname[4] = '0'+demonumber;
    CA_LoadFile (demoname,&demobuffer);
    demoptr = (int8_t *)demobuffer;
#endif

    NewGame (1,0);
    gamestate.mapon = *demoptr++;
    gamestate.difficulty = gd_hard;
    length = READWORD(*(uint8_t **)&demoptr);
    // TODO: Seems like the original demo format supports 16 MB demos
    //       But T_DEM00 and T_DEM01 of Wolf have a 0xd8 as third length size...
    demoptr++;
    lastdemoptr = demoptr-4+length;

    VW_FadeOut ();

    SETFONTCOLOR(0,15);
    DrawPlayScreen ();

    startgame = false;
    demoplayback = true;
    int loaded = SetupGameLevel ();
	PreloadGraphics(loaded);
    StartMusic ();

    PlayLoop ();

#ifdef DEMOSEXTERN
    UNCACHEGRCHUNK(dems[demonumber]);
#else
    MM_FreePtr (&demobuffer);
#endif

    demoplayback = false;

    StopMusic ();
    
}

//==========================================================================

/*
==================
=
= Died
=
==================
*/

#define DEATHROTATE 2

void Died (void)
{
    int32_t dx,dy;
    int     iangle,curangle,clockwise,counter,change;

    if (screenfaded)
    {
        ThreeDRefresh ();
        VW_FadeIn ();
    }

    gamestate.weapon = (weapontype) -1;                     // take away weapon
    SD_PlaySound (PLAYERDEATHSND);

    //
    // swing around to face attacker
    //
    if(killerobj)
    {
        dx = killerobj->x - player->x;
        dy = player->y - killerobj->y;

		iangle = atan2fix(dy,dx);
    }
    else
    {
        iangle = player->angle + ANGLES / 2;
        if(iangle >= ANGLES) iangle -= ANGLES;
    }

    if (player->angle > iangle)
    {
        counter = player->angle - iangle;
        clockwise = ANGLES-player->angle + iangle;
    }
    else
    {
        clockwise = iangle - player->angle;
        counter = player->angle + ANGLES-iangle;
    }
    curangle = player->angle;

    if (clockwise<counter)
    {
        //
        // rotate clockwise
        //
        if (curangle>iangle)
            curangle -= ANGLES;
        do
        {
            change = tics*DEATHROTATE;

            if (curangle + change > iangle)
                change = iangle-curangle;

            curangle += change;
            player->angle += change;
            if (player->angle >= ANGLES)
                player->angle -= ANGLES;

            ThreeDRefresh ();
            CalcTics ();
        } while (curangle != iangle);
    }
    else
    {
        //
        // rotate counterclockwise
        //
        if (curangle<iangle)
            curangle += ANGLES;
        do
        {
            change = -(int)tics*DEATHROTATE;

            if (curangle + change < iangle)
                change = iangle-curangle;

            curangle += change;
            player->angle += change;
            if (player->angle < 0)
                player->angle += ANGLES;

            ThreeDRefresh ();
            CalcTics ();
        } while (curangle != iangle);
    }

    //
    // fade to red
    //
    FinishPaletteShifts ();
	memset (screenBuffer->pixels,4,screenBuffer->pitch*240);

    IN_ClearKeysDown ();

    FizzleFade(screenBuffer,screen,viewscreenx,viewscreeny,viewwidth,viewheight,70,false); // died !!!
	
    IN_UserInput(100);
    SD_WaitSoundDone ();
	
    gamestate.lives--;

    if (gamestate.lives > -1)
    {
        gamestate.health = 100;
        gamestate.weapon = gamestate.bestweapon
            = gamestate.chosenweapon = wp_pistol;
        gamestate.ammo = STARTAMMO;
        gamestate.keys = 0;
        pwallstate = pwallpos = 0;
        gamestate.attackframe = gamestate.attackcount =
            gamestate.weaponframe = 0;

        if(viewsize != 21)
        {
            DrawKeys ();
			if(gamestate.weapon!=-1)
				DrawWeapon ();
            DrawAmmo ();
            DrawHealth ();
            DrawFace ();
            DrawLives ();
        }
    }
}

//==========================================================================

/*
===================
=
= GameLoop
=
===================
*/
void heapWalk();

void GameLoop (void)
{
// vbt dernier niveau
//gamestate.mapon = 8;	
//gamestate.mapon = 3;
//gamestate.episode=3;
//GiveWeapon (gamestate.bestweapon+2);

gamestate.ammo = 99;	
gamestate.keys = 3;
// vbt dernier niveau
		
    boolean died;
#ifdef MYPROFILE
    clock_t start,end;
#endif
  			//slPrint("GameLoop",slLocate(10,7));
restartgame:
    
    SETFONTCOLOR(0,15);
    VW_FadeOut();
  			//slPrint("DrawPlayScreen",slLocate(10,8));
    DrawPlayScreen ();
	  			//slPrint("DrawPlayScreen end",slLocate(10,9));
    died = false;
	int loaded = 0;
	
    do
    {

        //if (!loadedgame)
            gamestate.score = gamestate.oldscore;
        if(!died || viewsize != 21) 
		{
  			//slPrint("DrawScore",slLocate(10,9));
			DrawScore();
		}
        startgame = false;
        //if (!loadedgame)
		{
			//slPrint("SetupGameLevel",slLocate(10,10));
            loaded = SetupGameLevel ();
		}
#ifdef SPEAR
        if (gamestate.mapon == 20)      // give them the key allways
        {
            gamestate.keys |= 1;
            DrawKeys ();
        }
#endif
        ingame = true;
/*        if(loadedgame)
        {
            ContinueMusic(lastgamemusicoffset);
            loadedgame = false;
        }
        else*/ 
        if (!died)
            PreloadGraphics (loaded);             // TODO: Let this do something useful!
        else
        {
            died = false;
            fizzlein = true;
        }
		StartMusic ();
#ifdef SPEAR
startplayloop:
#endif
        PlayLoop ();

#ifdef SPEAR
        if (spearflag)
        {
            SD_StopSound();
            SD_PlaySound(GETSPEARSND);
            if (DigiMode != sds_Off)
            {
                Delay(150);
            }
            else
                SD_WaitSoundDone();

            gamestate.oldscore = gamestate.score;
            gamestate.mapon = 20;
            loaded = SetupGameLevel ();
            StartMusic ();
            player->x = spearx;
            player->y = speary;
            player->angle = (short)spearangle;
            spearflag = false;
            Thrust (0,0);
            goto startplayloop;
        }
#endif

        StopMusic ();
        ingame = false;

        if (startgame)
            goto restartgame;

        switch (playstate)
        {
            case ex_completed:
            case ex_secretlevel:
                if(viewsize == 21) DrawPlayScreen();
                gamestate.keys = 0;
                DrawKeys ();
                VW_FadeOut ();
                LevelCompleted ();              // do the intermission
// vbt pour garder les clefs.
	gamestate.keys = 3;				
                if(viewsize == 21) DrawPlayScreen();

#ifdef SPEARDEMO
                if (gamestate.mapon == 1)
                {
                    died = true;                    // don't "get psyched!"

                    VW_FadeOut ();

                    

                    CheckHighScore (gamestate.score,gamestate.mapon+1);
#ifndef JAPAN
                    strcpy(MainMenu[viewscores].string,STR_VS);
#endif
                    MainMenu[viewscores].routine = CP_ViewScores;
                    return;
                }
#endif

#ifdef JAPDEMO
                if (gamestate.mapon == 3)
                {
                    died = true;                    // don't "get psyched!"

                    VW_FadeOut ();

                    

                    CheckHighScore (gamestate.score,gamestate.mapon+1);
#ifndef JAPAN
                    strcpy(MainMenu[viewscores].string,STR_VS);
#endif
                    MainMenu[viewscores].routine = CP_ViewScores;
                    return;
                }
#endif

                gamestate.oldscore = gamestate.score;

#ifndef SPEAR
                //
                // COMING BACK FROM SECRET LEVEL
                //
                if (gamestate.mapon == 9)
                    gamestate.mapon = ElevatorBackTo[gamestate.episode];    // back from secret
                else
                    //
                    // GOING TO SECRET LEVEL
                    //
                    if (playstate == ex_secretlevel)
                        gamestate.mapon = 9;
#else

#define FROMSECRET1             3
#define FROMSECRET2             11

                //
                // GOING TO SECRET LEVEL
                //
                if (playstate == ex_secretlevel)
                    switch(gamestate.mapon)
                {
                    case FROMSECRET1: gamestate.mapon = 18; break;
                    case FROMSECRET2: gamestate.mapon = 19; break;
                }
                else
                    //
                    // COMING BACK FROM SECRET LEVEL
                    //
                    if (gamestate.mapon == 18 || gamestate.mapon == 19)
                        switch(gamestate.mapon)
                    {
                        case 18: gamestate.mapon = FROMSECRET1+1; break;
                        case 19: gamestate.mapon = FROMSECRET2+1; break;
                    }
#endif
                    else
                        //
                        // GOING TO NEXT LEVEL
                        //
                        gamestate.mapon++;
                break;

            case ex_died:
                Died ();
                died = true;                    // don't "get psyched!"

                if (gamestate.lives > -1)
                    break;                          // more lives left

                VW_FadeOut ();
                if(screenHeight % 200 != 0)
                    VL_ClearScreen(0);

                

                CheckHighScore (gamestate.score,gamestate.mapon+1);
#ifndef JAPAN
                strcpy(MainMenu[viewscores].string,STR_VS);
#endif
                MainMenu[viewscores].routine = CP_ViewScores;
                return;

            case ex_victorious:
                if(viewsize == 21) DrawPlayScreen();
#ifndef SPEAR
                VW_FadeOut ();
#else
                VL_FadeOut (0,255,0,17,17,300);
#endif
                
                Victory ();

                

                CheckHighScore (gamestate.score,gamestate.mapon+1);
#ifndef JAPAN
                strcpy(MainMenu[viewscores].string,STR_VS);
#endif
                MainMenu[viewscores].routine = CP_ViewScores;
                return;

            default:
                if(viewsize == 21) DrawPlayScreen();
                
                break;
        }
//		slSynch(); // vbt ajout 26/05 à remettre
    } while (1);
}
