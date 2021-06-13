// WL_DRAW.C
//#define USE_SPRITES 1
#include "wl_def.h"

#include "wl_cloudsky.h"
#include "wl_atmos.h"
#include "wl_shade.h"

void heapWalk();
#ifdef USE_SPRITES
unsigned char wall_buffer[(SATURN_WIDTH+64)*64];
SPRITE user_walls[SATURN_WIDTH];
extern 	TEXTURE tex_spr[SPR_TOTAL+SATURN_WIDTH];
extern unsigned char texture_list[SPR_TOTAL];
extern unsigned int position_vram;
#endif

typedef struct
{
	byte *postsource;	
#ifndef EMBEDDED	
	int postx;
	int lasttilehit;
	word lastside;
	word lastintercept;
	word lasttexture;	
#endif	
	int tilehit;
	int xintercept,yintercept;
	short xtile,ytile;	
	short xtilestep,ytilestep;
// wall optimization variables	
} 
ray_struc __attribute__ ((aligned (4)));

/*
=============================================================================

                               LOCAL CONSTANTS

=============================================================================
*/

// the door is the last picture before the sprites
#define DOORWALL        (PMSpriteStart-8)

#define ACTORSIZE       0x4000

/*
=============================================================================

                              GLOBAL VARIABLES

=============================================================================
*/

int32_t    lasttimecount;

short *wallheight;
int min_wallheight;

//
// math tables
//
short *pixelangle;
int32_t finetangent[FINEANGLES/4];
fixed sintable[ANGLES+ANGLES/4];
fixed *costable = sintable+(ANGLES/4);

static short     dirangle[9] = {0,ANGLES/8,2*ANGLES/8,3*ANGLES/8,4*ANGLES/8,
                       5*ANGLES/8,6*ANGLES/8,7*ANGLES/8,ANGLES};

//
// refresh variables
//
fixed   viewx,viewy;                    // the focal point
//short   viewangle;
fixed   viewsin,viewcos;

void    TransformActor (objtype *ob);
void    BuildTables (void);
void    ClearScreen (void);
int     CalcRotate (objtype *ob);
static void    DrawScaleds (void);
void    CalcTics (void);
void    ThreeDRefresh (void);

//
// ray tracing variables
#ifndef EMBEDDED
short   midangle;
int viewangle;
#endif
word horizwall[MAXWALLTILES],vertwall[MAXWALLTILES];

/*
============================================================================

                           3 - D  DEFINITIONS

============================================================================
*/

/*
========================
=
= TransformActor
=
= Takes paramaters:
=   gx,gy               : globalx/globaly of point
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
========================
*/


//
// transform actor
//
inline void TransformActor (objtype *ob)
{
    fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
    gx = ob->x-viewx;
    gy = ob->y-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-ACTORSIZE;         // fudge the shape forward a bit, because
                                    // the midpoint could put parts of the shape
                                    // into an adjacent wall

//
// calculate newy
//
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;

//
// calculate perspective ratio
//
    ob->transx = nx;
    ob->transy = ny;

    if (nx<MINDIST)                 // too close, don't overflow the divide
    {
        ob->viewheight = 0;
        return;
    }

    ob->viewx = (word)(centerx + ny*scale/nx);

//
// calculate height (heightnumerator/(nx>>8))
//
    ob->viewheight = (word)(heightnumerator/(nx>>8));
}

//==========================================================================

/*
========================
=
= TransformTile
=
= Takes paramaters:
=   tx,ty               : tile the object is centered in
=
= globals:
=   viewx,viewy         : point of view
=   viewcos,viewsin     : sin/cos of viewangle
=   scale               : conversion from global value to screen value
=
= sets:
=   screenx,transx,transy,screenheight: projected edge location and size
=
= Returns true if the tile is withing getting distance
=
========================
*/

static boolean inline TransformTile (int tx, int ty, short *dispx, short *dispheight)
{
    fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
    gx = ((int32_t)tx<<TILESHIFT)+0x8000-viewx;
    gy = ((int32_t)ty<<TILESHIFT)+0x8000-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt-0x2000;            // 0x2000 is size of object

//
// calculate newy
//
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;

//
// calculate height / perspective ratio
//
    if (nx<MINDIST)                 // too close, don't overflow the divide
	{
        *dispheight = 0;
		return false;
	}

    *dispx = (short)(centerx + ny*scale/nx);
    *dispheight = (short)(heightnumerator/(nx>>8));

//
// see if it should be grabbed
//
    if (nx<TILEGLOBAL && ny>-TILEGLOBAL/2 && ny<TILEGLOBAL/2)
        return true;
    else
        return false;
}
/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

int CalcHeight(int xintercept, int yintercept)
{
    fixed z = FixedMul(xintercept - viewx, viewcos)
        - FixedMul(yintercept - viewy, viewsin);
    if(z < MINDIST) z = MINDIST;
    int height = heightnumerator / (z >> 8);
    if(height < min_wallheight) min_wallheight = height;
    return height;
}

//==========================================================================

inline void loadActorTexture(int texture);


#ifdef USE_SPRITES
TEXTURE tex_spr[SPR_TOTAL+SATURN_WIDTH];

inline void loadActorTexture(int texture)
{
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+texture];	
	*txptr = TEXDEF(64, 64, position_vram);
	memcpyl((void *)(SpriteVRAM + ((txptr->CGadr) << 3)),(void *)PM_GetSprite(texture),0x1000);
//	slDMACopy((void *)pic_spr.pcsrc,		(void *)(SpriteVRAM + ((txptr->CGadr) << 3)),		(Uint32)((txptr->Hsize * txptr->Vsize * 4) >> (pic_spr.cmode)));
//	slDMACopy((void *)pic_spr.pcsrc,		(void *)(SpriteVRAM + ((txptr->CGadr) << 3)),		(Uint32)((txptr->Hsize * txptr->Vsize * 4) >> (pic_spr.cmode)));
	texture_list[texture]=position_vram/0x800;
	position_vram+=0x800;	
//	slDMAWait();
}
#endif

/*
===================
=
= ScalePost
=
===================
*/
extern int 					nb_unlock;

void ScalePost(int postx,byte *postsource)
{
#ifdef USE_SPRITES	
//--------------------------------------------------------------------------------------------
//	if(postx>=0 && postx<=SATURN_WIDTH)
	{
		memcpyl((void *)(wall_buffer + (postx<<6)),(void *)postsource,64);
//		nb_unlock+=64;
//		slDMACopy((void *)postsource, (void *)(wall_buffer + (postx<<6)), 64);
//	slTransferEntry((void *)postsource,(void *)(wall_buffer + (postx<<6)),64);		

	//  a           b          c             d
	// top left, top right, bottom right, bottom left
		SPRITE *user_wall = &user_walls[postx];
		user_wall->CTRL=FUNC_Texture | _ZmCC;
		user_wall->PMOD=CL256Bnk | ECdis | SPdis | 0x0800; // sans transparence

		user_wall->SRCA=0x2000|(postx*8);
		user_wall->COLR=256;
		user_wall->SIZE=0x801;

		user_wall->XD=postx-(viewwidth/2);
		user_wall->YC=(wallheight[postx] / 8);
		user_wall->YD=-user_wall->YC;
		user_wall->XC=user_wall->XD;
	//	user_wall->YC=(wallheight[postx] / 8);
		user_wall->XA=user_wall->XD;
		user_wall->YA=user_wall->YD;
		user_wall->XB=user_wall->XA;
		user_wall->YB=user_wall->YC;
	//    user_wall->GRDA=0;
	}
//--------------------------------------------------------------------------------------------
#else
    byte *vbuf = LOCK()+screenofs;	
    int ywcount, yoffs, yw, yd, yendoffs;
    byte col;

#ifdef USE_SHADING
    byte *curshades = shadetable[GetShade(wallheight[postx])];
#endif

    ywcount = yd = wallheight[postx] / 8;
    if(yd <= 0) yd = 100;

    yoffs = (viewheight / 2 - ywcount) * curPitch;
    if(yoffs < 0) yoffs = 0;
    yoffs += postx;

    yendoffs = viewheight / 2 + ywcount - 1;
    yw=TEXTURESIZE-1;

    while(yendoffs >= viewheight)
    {
        ywcount -= TEXTURESIZE/2;
        while(ywcount <= 0)
        {
            ywcount += yd;
            yw--;
        }
        yendoffs--;
    }
    if(yw < 0) return;	

#ifdef USE_SHADING
    col = curshades[postsource[yw]];
#else
    col = postsource[yw];
#endif	

    yendoffs = yendoffs * curPitch + postx;
    while(yoffs <= yendoffs)
    {
        vbuf[yendoffs] = col;
        ywcount -= TEXTURESIZE/2;
        if(ywcount <= 0)
        {
            do
            {
                ywcount += yd;
                yw--;
            }
            while(ywcount <= 0);
            if(yw < 0) break;
#ifdef USE_SHADING
            col = curshades[postsource[yw]];
#else
            col = postsource[yw];
#endif
        }
        yendoffs -= curPitch;
    }
#endif	
}

byte vgaCeiling[]=
{
#ifndef SPEAR
 0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0x1d,0xbf,
 0x4e,0x4e,0x4e,0x1d,0x8d,0x4e,0x1d,0x2d,0x1d,0x8d,
 0x1d,0x1d,0x1d,0x1d,0x1d,0x2d,0xdd,0x1d,0x1d,0x98,

 0x1d,0x9d,0x2d,0xdd,0xdd,0x9d,0x2d,0x4d,0x1d,0xdd,
 0x7d,0x1d,0x2d,0x2d,0xdd,0xd7,0x1d,0x1d,0x1d,0x2d,
 0x1d,0x1d,0x1d,0x1d,0xdd,0xdd,0x7d,0xdd,0xdd,0xdd
#else
 0x6f,0x4f,0x1d,0xde,0xdf,0x2e,0x7f,0x9e,0xae,0x7f,
 0x1d,0xde,0xdf,0xde,0xdf,0xde,0xe1,0xdc,0x2e,0x1d,0xdc
#endif
};

/*
=====================
=
= VGAClearScreen
=
=====================
*/
void VGAClearScreen () // vbt : fond d'écran 2 barres grises
{
#ifndef USE_SPRITES	
    byte ceiling=vgaCeiling[gamestate.episode*10+mapon];

    int y;
	byte *vbuf = LOCK()+screenofs;
    byte *ptr = vbuf;
#ifdef USE_SHADING
    for(y = 0; y < viewheight / 2; y++, ptr += curPitch)
        memset(ptr, shadetable[GetShade((viewheight / 2 - y) << 3)][ceiling], viewwidth);
    for(; y < viewheight; y++, ptr += curPitch)
        memset(ptr, shadetable[GetShade((y - viewheight / 2) << 3)][0x19], viewwidth);
#else
    for(y = 0; y < viewheight / 2; y++, ptr += curPitch)
        memset(ptr, ceiling, viewwidth);
    for(; y < viewheight; y++, ptr += curPitch)
        memset(ptr, 0x19, viewwidth);
#endif

#else
	extern byte vgaCeiling[];
	extern SDL_Color curpal[256];
	unsigned char y;
	Uint16	*Line_Color_Pal0	=(Uint16 *)LINE_COLOR_TABLE;

	SDL_Color *sdlCeilingColor = (SDL_Color *)&curpal[vgaCeiling[gamestate.episode*10+mapon]];
	Uint16 ceilingColor = 0x8000 | RGB((*sdlCeilingColor).r>>3,(*sdlCeilingColor).g>>3,(*sdlCeilingColor).b>>3);

	SDL_Color *sdlFloorColor = (SDL_Color *)&curpal[25];
	Uint16 floorColor = 0x8000 | RGB((*sdlFloorColor).r>>3,(*sdlFloorColor).g>>3,(*sdlFloorColor).b>>3);
	
	unsigned int start=0,end=0;

    if(viewsize == 21)
	{
		end=120;
	}
    else if(viewsize == 20)
	{
		end=screenHeight/2;
	}
    else
	{	
		start=viewscreeny;
		end =(viewheight+start*2)/2;
	}	

	for(y = start; y < end; y++)
		Line_Color_Pal0[y] = ceilingColor;
	
	for(; y <= viewheight+viewscreeny; y++)
		Line_Color_Pal0[y] = floorColor;

	start=RGB(0,0,0);
	for(; y < 240; y++)
		Line_Color_Pal0[y] = start;
	
	slBackColTable((void *)LINE_COLOR_TABLE);
#endif
}

//==========================================================================

/*
=====================
=
= CalcRotate
=
=====================
*/

inline int CalcRotate (objtype *ob)
{
    int angle;

    // this isn't exactly correct, as it should vary by a trig value,
    // but it is close enough with only eight rotations

    int viewangle = player->angle + (centerx - ob->viewx)/8;

    if (ob->obclass == rocketobj || ob->obclass == hrocketobj)
        angle = (viewangle-180) - ob->angle;
    else
        angle = (viewangle-180) - dirangle[ob->dir];

    angle+=ANGLES/16;
    while (angle>=ANGLES)
        angle-=ANGLES;
    while (angle<0)
        angle+=ANGLES;

#ifdef EMBEDDED
    if (gamestates[ob->state].rotate == 2)  // 2 rotation pain frame
        return 0;               // pain with shooting frame bugfix
#else
    if (ob->state->rotate == 2)             // 2 rotation pain frame
        return 4*(angle/(ANGLES/2));    // seperated by 3
#endif	
    return angle/(ANGLES/8);
}

inline void ScaleShape (int xcenter, int shapenum, unsigned height)
{
    unsigned scalel,pixheight;

#ifdef USE_SHADING
    byte *curshades;
    if(flags & FL_FULLBRIGHT)
        curshades = shadetable[0];
    else
        curshades = shadetable[GetShade(height)];
#endif
    scalel=height/8;                 // low three bits are fractional
    if(!scalel) return;   // too close or far away
    pixheight=scalel*SPRITESCALEFACTOR;

#ifdef USE_SPRITES
	if(texture_list[shapenum]==0xff)
		loadActorTexture(shapenum);
//--------------------------------------------------------------------------------------------
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+shapenum]; 
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis;// | ECenb | SPdis;  // pas besoin pour les sprites
    user_sprite.SRCA=((txptr->CGadr));
    user_sprite.COLR=256;
    user_sprite.SIZE=0x840;
	user_sprite.XA=(xcenter-centerx);
	user_sprite.YA=0;
	user_sprite.XB=pixheight;
	user_sprite.YB=user_sprite.XB;
    user_sprite.GRDA=0;
	slSetSprite(&user_sprite, toFIXED(0+(SATURN_SORT_VALUE-pixheight/2)));	// à remettre // ennemis et objets
//--------------------------------------------------------------------------------------------	
#else
	byte *vbuf = LOCK()+screenofs;
    t_compshape *shape;

    unsigned starty,endy;
    word *cmdptr;
    byte *cline;
    byte *line;
    byte *vmem;
    unsigned j;
    byte col;
    int actx,i,upperedge;
    short newstart;
    int scrstarty,screndy,lpix,rpix,pixcnt,ycnt;

    actx=xcenter-scale;
    upperedge=viewheight/2-scale;

    shape = (t_compshape *) PM_GetSprite(shapenum);
	
    cmdptr=(word *) shape->dataofs;
	
    for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
    {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixheight;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0)
        {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline=(byte *)shape + *cmdptr;
            while(lpix<rpix)
            {
                if(wallheight[lpix]<=(int)height)
                {
                    line=cline;
                    while((endy = READWORD(line)) != 0)
                    {
                        endy >>= 1;
                        newstart = READWORD(line);
                        starty = READWORD(line) >> 1;
//                        j=starty;
                        ycnt=j*pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(screndy<0) vmem=vbuf+lpix;
                        else vmem=vbuf+screndy*curPitch+lpix;
                        for(;j<endy;j++)
                        {
                            scrstarty=screndy;
                            ycnt+=pixheight;
                            screndy=(ycnt>>6)+upperedge;
                            if(scrstarty!=screndy && screndy>0)
                            {
#ifdef USE_SHADING
                                col=curshades[((byte *)shape)[newstart+j]];
#else
                                col=((byte *)shape)[newstart+j];
#endif
                                if(scrstarty<0) scrstarty=0;
                                if(screndy>viewheight) screndy=viewheight,j=endy;

                                while(scrstarty<screndy)
                                {
                                    *vmem=col;
                                    vmem+=curPitch;
                                    scrstarty++;
                                }
                            }
                        }
                    }
                }
                lpix++;
            }
        }
    }
#endif	
}
#ifdef USE_SPRITES
int old_texture = -1;
void SimpleScaleShape (int xcenter, int shapenum, unsigned height)
#else
void SimpleScaleShape (byte *vbuf, int xcenter, int shapenum, unsigned height,unsigned vbufPitch)
#endif
{
#ifdef USE_SPRITES	
////slPrintHex(shapenum,slLocate(10,4));
	if (old_texture!=shapenum)
	{
		memcpyl((void *)(wall_buffer + (SATURN_WIDTH<<6)),(void *)PM_GetSprite(shapenum),0x1000);
		old_texture=shapenum;
	}	
//--------------------------------------------------------------------------------------------
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis | 0x0800;// | ECenb | SPdis;  // pas besoin pour les sprites
//    user_sprite.SRCA=0x2000|(SATURN_WIDTH*8);
    user_sprite.SRCA=0x2000|(SATURN_WIDTH*8);
    user_sprite.COLR=256;
    user_sprite.SIZE=0x840;
	user_sprite.XA=(xcenter-centerx);
	user_sprite.YA=0;
	user_sprite.XB=height;
	user_sprite.YB=user_sprite.XB;
    user_sprite.GRDA=0;
	slSetSprite(&user_sprite, toFIXED(10));// à remettre	// arme
//--------------------------------------------------------------------------------------------	
#else
	unsigned pixheight=scale*SPRITESCALEFACTOR;
    t_compshape   *shape;

    unsigned starty,endy;
    word *cmdptr;
    byte *cline;
    byte *line;
    int actx,i,upperedge;
    short newstart;
    int scrstarty,screndy,lpix,rpix,pixcnt,ycnt;
    unsigned j;
    byte col;
    byte *vmem;

    shape = (t_compshape *) PM_GetSprite(shapenum);

    scale=height>>1;
    pixheight=scale*SPRITESCALEFACTOR;
    actx=xcenter-scale;
    upperedge=viewheight/2-scale;

    cmdptr=shape->dataofs;

    for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
    {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixheight;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0)
        {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline = (byte *)shape + *cmdptr;
            while(lpix<rpix)
            {
                line=cline;
                while((endy = READWORD(line)) != 0)
                {
                    endy >>= 1;
                    newstart = READWORD(line);
                    starty = READWORD(line) >> 1;
                    j=starty;
                    ycnt=j*pixheight;
                    screndy=(ycnt>>6)+upperedge;
                    if(screndy<0) vmem=vbuf+lpix;
                    else vmem=vbuf+screndy*vbufPitch+lpix;
                    for(;j<endy;j++)
                    {
                        scrstarty=screndy;
                        ycnt+=pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(scrstarty!=screndy && screndy>0)
                        {
                            col=((byte *)shape)[newstart+j];
                            if(scrstarty<0) scrstarty=0;
                            if(screndy>viewheight) screndy=viewheight,j=endy;

                            while(scrstarty<screndy)
                            {
                                *vmem=col;
                                vmem+=vbufPitch;
                                scrstarty++;
                            }
                        }
                    }
                }
                lpix++;
            }
        }
    }
#endif // end use sprites
}

void SimpleScaleShape (byte *vbuf, int xcenter, int shapenum, unsigned height,unsigned vbufPitch)
{
	unsigned pixheight=scale*SPRITESCALEFACTOR;
    t_compshape   *shape;

    unsigned starty,endy;
    word *cmdptr;
    byte *cline;
    byte *line;
    int actx,i,upperedge;
    short newstart;
    int scrstarty,screndy,lpix,rpix,pixcnt,ycnt;
    unsigned j;
    byte col;
    byte *vmem;
/*
    shape = (t_compshape *) PM_GetSprite(shapenum);
*/
//    scale=height>>1;
/* 
 pixheight=scale*SPRITESCALEFACTOR;
    actx=xcenter-scale;
    upperedge=viewheight/2-scale;
*/	
		unsigned char *surfacePtr = (unsigned char*)PM_GetSprite(shapenum); // + ((0) * source->pitch) + 0;
		unsigned char *nextSurfacePtr = (unsigned char*)PM_GetSprite(shapenum+1);
		int size=(nextSurfacePtr-surfacePtr)>>6;
		
		unsigned int *nbg1Ptr = (unsigned int*)(VDP2_VRAM_A0 + ((viewheight-56+(64-size))<<9)+ (viewwidth/2-32));

//if(TransCount!=0)
//			slPrintHex(TransCount,slLocate(10,4));
			
		for( Sint16 i=0;i<size;i++)
		{
			slDMACopy((void*)surfacePtr,(void *)nbg1Ptr,64);
//			memcpy((void *)nbg1Ptr,(void*)surfacePtr,64);
//			slTransferEntry((void *)surfacePtr,(void *)nbg1Ptr,64);
			surfacePtr+=64;
			nbg1Ptr+=128;
		}
	
/*
    cmdptr=shape->dataofs;


    for(i=shape->leftpix,pixcnt=i*pixheight,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
    {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixheight;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0)
        {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline = (byte *)shape + *cmdptr;
            while(lpix<rpix)
            {
                line=cline;
                while((endy = READWORD(line)) != 0)
                {
                    endy >>= 1;
                    newstart = READWORD(line);
                    starty = READWORD(line) >> 1;
                    j=starty;
                    ycnt=j*pixheight;
                    screndy=(ycnt>>6)+upperedge;
                    if(screndy<0) vmem=vbuf+lpix;
                    else vmem=vbuf+screndy*vbufPitch+lpix;
                    for(;j<endy;j++)
                    {
                        scrstarty=screndy;
                        ycnt+=pixheight;
                        screndy=(ycnt>>6)+upperedge;
                        if(scrstarty!=screndy && screndy>0)
                        {
                            col=((byte *)shape)[newstart+j];
                            if(scrstarty<0) scrstarty=0;
                            if(screndy>viewheight) screndy=viewheight,j=endy;

                            while(scrstarty<screndy)
                            {
//                                *vmem=col;
                                *vmem=0x11;
                                vmem+=vbufPitch;
                                scrstarty++;
                            }
                        }
                    }
                }
                lpix++;
            }
        }
    }
*/	
}


/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

#define MAXVISABLE 150

typedef struct
{
    short      viewx,
               viewheight,
               shapenum;
//    short      flags;          // this must be changed to uint32_t, when you
                               // you need more than 16-flags for drawing
#ifdef USE_DIR3DSPR
    statobj_t *transsprite;
#endif
} visobj_t;

visobj_t vislist[MAXVISABLE];
visobj_t *visptr,*visstep,*farthest;

static void DrawScaleds (void)
{
    int      i,least,numvisable,height;
    byte     *tilespot,*visspot;
    unsigned spotloc;

    statobj_t *statptr;
    objtype   *obj;

    visptr = &vislist[0];
//
// place static objects
//
    for (statptr = &statobjlist[0] ; statptr !=laststatobj ; statptr++)
    {
        if ((visptr->shapenum = statptr->shapenum) == -1)
            continue;                                               // object has been deleted

        if (!*statptr->visspot)
            continue;                                               // not visable

        if (TransformTile (statptr->tilex,statptr->tiley,&visptr->viewx,&visptr->viewheight) && statptr->flags & FL_BONUS)
        {
            GetBonus (statptr);
            if(statptr->shapenum == -1)
                continue;                                           // object has been taken
        }
        if (!visptr->viewheight)
            continue;                                               // to close to the object

#ifdef USE_DIR3DSPR
        if(statptr->flags & FL_DIR_MASK)
            visptr->transsprite=statptr;
        else
            visptr->transsprite=NULL;
#endif

        if (visptr < &vislist[MAXVISABLE-1])    // don't let it overflow
        {
//            visptr->flags = (short) statptr->flags;
            visptr++;
        }
    }
//
// place active objects
//
    for (obj = player->next;obj;obj=obj->next)
    {
#ifdef EMBEDDED
        if (!(visptr->shapenum = gamestates[obj->state].shapenum))
#else
        if ((visptr->shapenum = obj->state->shapenum)==0)
#endif
            continue;                                               // no shape

        spotloc = (obj->tilex<<mapshift)+obj->tiley;   // optimize: keep in struct?
        visspot = &spotvis[0][0]+spotloc;
        tilespot = &tilemap[0][0]+spotloc;
        //
        // could be in any of the nine surrounding tiles
        //
        if (*visspot
            || ( *(visspot-1) && !*(tilespot-1) )
            || ( *(visspot+1) && !*(tilespot+1) )
            || ( *(visspot-65) && !*(tilespot-65) )
            || ( *(visspot-64) && !*(tilespot-64) )
            || ( *(visspot-63) && !*(tilespot-63) )
            || ( *(visspot+65) && !*(tilespot+65) )
            || ( *(visspot+64) && !*(tilespot+64) )
            || ( *(visspot+63) && !*(tilespot+63) ) )
        {
            obj->active = ac_yes;
            TransformActor (obj);
            if (!obj->viewheight)
                continue;                                               // too close or far away

            visptr->viewx = obj->viewx;
            visptr->viewheight = obj->viewheight;
            if (visptr->shapenum == -1)
                visptr->shapenum = obj->temp1;  // special shape

#ifdef EMBEDDED
            if (gamestates[obj->state].rotate)
#else
            if (obj->state->rotate)
#endif
                visptr->shapenum += CalcRotate (obj);

            if (visptr < &vislist[MAXVISABLE-1])    // don't let it overflow
            {
//                visptr->flags = (short) obj->flags;
#ifdef USE_DIR3DSPR
                visptr->transsprite = NULL;
#endif
                visptr++;
            }
            obj->flags |= FL_VISABLE;
        }
        else
		{
            obj->flags &= ~FL_VISABLE;
		}
    }
//
// draw from back to front
//
    numvisable = (int) (visptr-&vislist[0]);

    if (!numvisable)
        return;                                                                 // no visable objects

    for (i = 0; i<numvisable; i++)
    {
        least = 32000;
        for (visstep=&vislist[0] ; visstep<visptr ; visstep++)
        {
            height = visstep->viewheight;
            if (height < least)
            {
                least = height;
                farthest = visstep;
            }
        }
        //
        // draw farthest
        //
#ifdef USE_DIR3DSPR
        if(farthest->transsprite)
            Scale3DShape(vbuf, vbufPitch, farthest->transsprite);
        else
#endif
// affiche la version bitmap
		ScaleShape(farthest->viewx, farthest->shapenum, farthest->viewheight);
        farthest->viewheight = 32000;
    }
}
//==========================================================================

/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hands
=
==============
*/

static int weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY, SPR_PISTOLREADY,
    SPR_MACHINEGUNREADY, SPR_CHAINREADY};

#ifdef USE_SPRITES
void DrawPlayerWeapon ()
#else
void DrawPlayerWeapon (byte *vbuf)
#endif
{
    if (gamestate.weapon != -1)
    {
        int shapenum = weaponscale[gamestate.weapon]+gamestate.weaponframe;
/*		
#ifdef USE_SPRITES
		if(viewsize != 20)
			SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
		else
			SimpleScaleShape(viewwidth/2,shapenum,viewheight-41);		
#else
        SimpleScaleShape(vbuf,viewwidth/2,shapenum,viewheight+1,curPitch);
#endif
*/
unsigned int *nbg1Ptr = (unsigned int*)(VDP2_VRAM_A0);

//		SimpleScaleShape((byte *)curSurface->pixels,viewwidth/2,shapenum,viewheight+1,curPitch);
//		memset((byte *)nbg1Ptr,0x11,64*128);
		SimpleScaleShape((byte *)nbg1Ptr,viewwidth/2,shapenum,viewheight+1,curPitch);
//	while(1);	
    }

    if (demoplayback)
#ifdef USE_SPRITES
        SimpleScaleShape(viewwidth/2,SPR_DEMO,viewheight+1);
#else
        SimpleScaleShape(vbuf,viewwidth/2,SPR_DEMO,viewheight+1,curPitch);
#endif		

#ifndef SPEAR
    if (gamestate.victoryflag)
    {
#ifndef APOGEE_1_0
#ifdef EMBEDDED
        if ((player->state == s_deathcam) && (GetTimeCount() & 32))
#else
        if (player->state == &s_deathcam && (GetTimeCount()&32) )
#endif		
#ifdef USE_SPRITES
			SimpleScaleShape(viewwidth/2,SPR_DEATHCAM,viewheight+1);
#else
			SimpleScaleShape(vbuf,viewwidth/2,SPR_DEATHCAM,viewheight+1,curPitch);
#endif

#endif
        return;
    }
#endif	
}


//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
//
// calculate tics since last refresh for adaptive timing
//
    if (lasttimecount > (int32_t) GetTimeCount())
        lasttimecount = GetTimeCount();    // if the game was paused a LONG time

    uint32_t curtime = SDL_GetTicks();
    tics = (curtime * 7) / 100 - lasttimecount;
    if(!tics)
    {
        // wait until end of current tic
        SDL_Delay(((lasttimecount + 1) * 100) / 7 - curtime);
        tics = 1;
    }

    lasttimecount += tics;

    if (tics>MAXTICS)
        tics = MAXTICS;
}

static inline int samex(int xtilestep, int intercept, int tile)
{
    if (xtilestep > 0) {
        if ((intercept>>TILESHIFT) >= tile)
            return 0;
        else
            return 1;
    } else {
        if ((intercept>>TILESHIFT) <= tile)
            return 0;
        else
            return 1;
    }
}

static inline int samey(int ytilestep, int intercept, int tile)
{
    if (ytilestep > 0) {
        if ((intercept>>TILESHIFT) >= tile)
            return 0;
        else
            return 1;
    } else {
        if ((intercept>>TILESHIFT) <= tile)
            return 0;
        else
            return 1;
    }
}
//==========================================================================
#ifdef EMBEDDED

#define DEG90   900
#define DEG180  1800
#define DEG270  2700
#define DEG360  3600

static inline void HitHorizPWall(int postx, ray_struc *ray)
{
	int wallpic;
	unsigned texture, offset;
	
	texture = (ray->xintercept >> 4) & 0xfc0;
	
	offset = pwallpos << 10;
	
	if (ray->ytilestep == -1)
		ray->yintercept += TILEGLOBAL-offset;
	else {
		texture = 0xfc0-texture;
		ray->yintercept += offset;
	}

	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);
//horizwall[ray->tilehit & ~0x40];
	wallpic = horizwall[ray->tilehit&63];
//	wall = PM_GetPage(wallpic);
	ray->postsource = PM_GetTexture(wallpic) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static inline void HitHorizWallNew(int postx, ray_struc *ray)
{
	int wallpic;
	unsigned texture;

	texture = (ray->xintercept >> 4) & 0xfc0;
	
	if (ray->ytilestep == -1)
		ray->yintercept += TILEGLOBAL;
	else
		texture = 0xfc0 - texture;
		
	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);

	if (ray->tilehit & 0x40) { // check for adjacent doors
		ray->xtile = ray->xintercept>>TILESHIFT;
		if (tilemap[ray->xtile][ray->ytile-ray->ytilestep] & 0x80)
			wallpic = DOORWALL+2;
		else
			wallpic = horizwall[ray->tilehit & ~0x40];
	} else
		wallpic = horizwall[ray->tilehit];

//	wall = PM_GetPage(wallpic);
	ray->postsource = PM_GetTexture(wallpic) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static inline void HitHorizDoorNew(int postx, ray_struc *ray)
{
	unsigned texture, doorpage = 0, doornum;
//	byte *wall;

	doornum = ray->tilehit&0x7f;
	texture = ((ray->xintercept-doorposition[doornum]) >> 4) & 0xfc0;

	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);

	switch(doorobjlist[doornum].lock) {
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
	}

//	wall = PM_GetPage(doorpage);
	ray->postsource = PM_GetTexture(doorpage) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static void inline HitVertWallNew(int postx, ray_struc *ray)
{
	int wallpic;
	unsigned texture;
//	byte *wall;

	texture = (ray->yintercept>>4)&0xfc0;
	
	if (ray->xtilestep == -1) {
		texture = 0xfc0-texture;
		ray->xintercept += TILEGLOBAL;
	}
	
	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);

	if (ray->tilehit & 0x40) { // check for adjacent doors
		ray->ytile = ray->yintercept>>TILESHIFT;
		if (tilemap[ray->xtile-ray->xtilestep][ray->ytile] & 0x80)
			wallpic = DOORWALL+3;
		else
			wallpic = vertwall[ray->tilehit & ~0x40];
	} else
		wallpic = vertwall[ray->tilehit];
		
//	wall = PM_GetPage(wallpic);
	ray->postsource = PM_GetTexture(wallpic) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static inline void HitVertPWall(int postx, ray_struc *ray)
{
	int wallpic;
	unsigned texture, offset;
//	byte *wall;
	
	texture = (ray->yintercept >> 4) & 0xfc0;
	offset = pwallpos << 10;
	
	if (ray->xtilestep == -1) {
		ray->xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	} else
		ray->xintercept += offset;

	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);
	
	wallpic = vertwall[ray->tilehit&63];

//	wall = PM_GetPage(wallpic);
	ray->postsource = PM_GetTexture(wallpic) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static inline void HitVertDoorNew(int postx, ray_struc *ray)
{
	unsigned texture, doorpage = 0, doornum;

	doornum = ray->tilehit&0x7f;
	texture = ((ray->yintercept-doorposition[doornum]) >> 4) & 0xfc0;

	wallheight[postx] = CalcHeight(ray->xintercept,ray->yintercept);

	switch(doorobjlist[doornum].lock) {
		case dr_normal:
			doorpage = DOORWALL;
			break;
		case dr_lock1:
		case dr_lock2:
		case dr_lock3:
		case dr_lock4:
			doorpage = DOORWALL+6;
			break;
		case dr_elevator:
			doorpage = DOORWALL+4;
			break;
	}

//	wall = PM_GetPage(doorpage+1);
	ray->postsource = PM_GetTexture(doorpage+1) + texture;
//	ray->postx=postx;
	ScalePost(postx, ray->postsource);
}

static void AsmRefresh()
{
	static	ray_struc my_ray;	
	unsigned xpartialup, xpartialdown, ypartialup, ypartialdown;
	unsigned xpartial, ypartial;
	int doorhit;
	int angle;    /* ray angle through postx */
	int midangle;
	int focaltx, focalty;
	int xstep, ystep;

    int viewangle = player->angle;

    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedMul(focallength, viewcos);
    viewy = player->y + FixedMul(focallength, viewsin);

	
	midangle = viewangle*(FINEANGLES/ANGLES);
	xpartialdown = (viewx&(TILEGLOBAL-1));
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = (viewy&(TILEGLOBAL-1));
	ypartialup = TILEGLOBAL-ypartialdown;

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

#ifdef USE_SLAVE
for (int postx = 0; postx < (viewwidth/2); postx++) 
#else
for (int postx = 0; postx < viewwidth; postx++) 
#endif	
{
	angle = midangle + pixelangle[postx];

	if (angle < 0) {
		/* -90 - -1 degree arc */
		angle += FINEANGLES;
		goto entry360;
	} else if (angle < DEG90) {
		/* 0-89 degree arc */
	entry90:
		my_ray.xtilestep = 1;
		my_ray.ytilestep = -1;
		xstep = finetangent[DEG90-1-angle];
		ystep = -finetangent[angle];
		xpartial = xpartialup;
		ypartial = ypartialdown;
	} else if (angle < DEG180) {
		/* 90-179 degree arc */
		my_ray.xtilestep = -1;
		my_ray.ytilestep = -1;
		xstep = -finetangent[angle-DEG90];
		ystep = -finetangent[DEG180-1-angle];
		xpartial = xpartialdown;
		ypartial = ypartialdown;
	} else if (angle < DEG270) {
		/* 180-269 degree arc */
		my_ray.xtilestep = -1;
		my_ray.ytilestep = 1;
		xstep = -finetangent[DEG270-1-angle];
		ystep = finetangent[angle-DEG180];
		xpartial = xpartialdown;
		ypartial = ypartialup;
	} else if (angle < DEG360) {
		/* 270-359 degree arc */
	entry360:
		my_ray.xtilestep = 1;
		my_ray.ytilestep = 1;
		xstep = finetangent[angle-DEG270];
		ystep = finetangent[DEG360-1-angle];
		xpartial = xpartialup;
		ypartial = ypartialup;
	} else {
		angle -= FINEANGLES;
		goto entry90;
	}
	
	my_ray.yintercept = viewy + FixedMul(xpartial, ystep); // + xtilestep;
	my_ray.xtile = focaltx + my_ray.xtilestep;

	my_ray.xintercept = viewx + FixedMul(ypartial, xstep); // + ytilestep;
	my_ray.ytile = focalty + my_ray.ytilestep;

/* CORE LOOP */

#define TILE(n) ((n)>>TILESHIFT)

	/* check intersections with vertical walls */
vertcheck:
// int ytilestep, int intercept, int tile)
	if (!samey(my_ray.ytilestep, my_ray.yintercept, my_ray.ytile))
		goto horizentry;
		
vertentry:
	my_ray.tilehit = tilemap[my_ray.xtile][TILE(my_ray.yintercept)];
	/* printf("vert: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", postx, tilehit, xtile, ytile, xintercept, yintercept, xpartialup, xpartialdown, ypartialup, ypartialdown, xpartial, ypartial, doorhit, angle, midangle, focaltx, focalty, xstep, ystep); */
	
	if (my_ray.tilehit) {
		if (my_ray.tilehit & 0x80) {
			if (my_ray.tilehit & 0x40) {
				/* vertpushwall */
				doorhit = my_ray.yintercept + (signed)((signed)pwallpos * ystep) / 64;
			
				if (TILE(doorhit) != TILE(my_ray.yintercept)) 
					goto passvert;
					
				my_ray.yintercept = doorhit;
				my_ray.xintercept = my_ray.xtile << TILESHIFT;
				HitVertPWall(postx, &my_ray);
			} else {
				/* vertdoor */
				doorhit = my_ray.yintercept + ystep / 2;

				if (TILE(doorhit) != TILE(my_ray.yintercept))
					goto passvert;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passvert;
				
				my_ray.yintercept = doorhit;
				my_ray.xintercept = (my_ray.xtile << TILESHIFT) + TILEGLOBAL/2;
				HitVertDoorNew(postx, &my_ray);
			}
		} else {
			my_ray.xintercept = my_ray.xtile << TILESHIFT;
			HitVertWallNew(postx, &my_ray);
		}
		continue;
	}
passvert:
//	*((byte *)spotvis+xspot)=1;
//	setspotvis(my_ray.xtile,TILE(my_ray.yintercept));
	spotvis[my_ray.xtile][TILE(my_ray.yintercept)] = 1;
	my_ray.xtile += my_ray.xtilestep;
	my_ray.yintercept += ystep;
	goto vertcheck;
	
horizcheck:
	/* check intersections with horizontal walls */
	
	if (!samex(my_ray.xtilestep, my_ray.xintercept, my_ray.xtile))
		goto vertentry;

horizentry:
	my_ray.tilehit = tilemap[TILE(my_ray.xintercept)][my_ray.ytile];
	/* printf("horz: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", postx, tilehit, xtile, ytile, xintercept, yintercept, xpartialup, xpartialdown, ypartialup, ypartialdown, xpartial, ypartial, doorhit, angle, midangle, focaltx, focalty, xstep, ystep); */
	
	if (my_ray.tilehit) {
		if (my_ray.tilehit & 0x80) {
			if (my_ray.tilehit & 0x40) {
				doorhit = my_ray.xintercept + (signed)((signed)pwallpos * xstep) / 64;
		    	
				/* horizpushwall */
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = my_ray.ytile << TILESHIFT; 
				HitHorizPWall(postx, &my_ray);
			} else {
				doorhit = my_ray.xintercept + xstep / 2;
				
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = (my_ray.ytile << TILESHIFT) + TILEGLOBAL/2;
				HitHorizDoorNew(postx, &my_ray);
			}
		} else {
			my_ray.yintercept = my_ray.ytile << TILESHIFT;
			HitHorizWallNew(postx, &my_ray);
		}
		continue;
	}
passhoriz:
//    *((byte *)spotvis+yspot)=1;
//	setspotvis(TILE(my_ray.xintercept), my_ray.ytile);
	spotvis[TILE(my_ray.xintercept)][my_ray.ytile] = 1;
	my_ray.ytile += my_ray.ytilestep;
	my_ray.xintercept += xstep;
	goto horizcheck;
}
}

#ifdef USE_SLAVE

static void AsmRefreshSlave()
{
	static	ray_struc my_ray;	
	unsigned xpartialup, xpartialdown, ypartialup, ypartialdown;
	unsigned xpartial, ypartial;
	int doorhit;
	int angle;    /* ray angle through postx */
	int midangle;
	int focaltx, focalty;
	int xstep, ystep;

    int viewangle = player->angle;

    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedMul(focallength, viewcos);
    viewy = player->y + FixedMul(focallength, viewsin);

	
	midangle = viewangle*(FINEANGLES/ANGLES);
	xpartialdown = (viewx&(TILEGLOBAL-1));
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = (viewy&(TILEGLOBAL-1));
	ypartialup = TILEGLOBAL-ypartialdown;

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;

for(int postx=viewwidth>>1;postx<viewwidth;postx++)
{
	angle = midangle + pixelangle[postx];

	if (angle < 0) {
		/* -90 - -1 degree arc */
		angle += FINEANGLES;
		goto entry360;
	} else if (angle < DEG90) {
		/* 0-89 degree arc */
	entry90:
		my_ray.xtilestep = 1;
		my_ray.ytilestep = -1;
		xstep = finetangent[DEG90-1-angle];
		ystep = -finetangent[angle];
		xpartial = xpartialup;
		ypartial = ypartialdown;
	} else if (angle < DEG180) {
		/* 90-179 degree arc */
		my_ray.xtilestep = -1;
		my_ray.ytilestep = -1;
		xstep = -finetangent[angle-DEG90];
		ystep = -finetangent[DEG180-1-angle];
		xpartial = xpartialdown;
		ypartial = ypartialdown;
	} else if (angle < DEG270) {
		/* 180-269 degree arc */
		my_ray.xtilestep = -1;
		my_ray.ytilestep = 1;
		xstep = -finetangent[DEG270-1-angle];
		ystep = finetangent[angle-DEG180];
		xpartial = xpartialdown;
		ypartial = ypartialup;
	} else if (angle < DEG360) {
		/* 270-359 degree arc */
	entry360:
		my_ray.xtilestep = 1;
		my_ray.ytilestep = 1;
		xstep = finetangent[angle-DEG270];
		ystep = finetangent[DEG360-1-angle];
		xpartial = xpartialup;
		ypartial = ypartialup;
	} else {
		angle -= FINEANGLES;
		goto entry90;
	}
	
	my_ray.yintercept = viewy + FixedMul(xpartial, ystep); // + xtilestep;
	my_ray.xtile = focaltx + my_ray.xtilestep;

	my_ray.xintercept = viewx + FixedMul(ypartial, xstep); // + ytilestep;
	my_ray.ytile = focalty + my_ray.ytilestep;

/* CORE LOOP */

#define TILE(n) ((n)>>TILESHIFT)

	/* check intersections with vertical walls */
vertcheck:
// int ytilestep, int intercept, int tile)
	if (!samey(my_ray.ytilestep, my_ray.yintercept, my_ray.ytile))
		goto horizentry;
		
vertentry:
	my_ray.tilehit = tilemap[my_ray.xtile][TILE(my_ray.yintercept)];
	/* printf("vert: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", postx, tilehit, xtile, ytile, xintercept, yintercept, xpartialup, xpartialdown, ypartialup, ypartialdown, xpartial, ypartial, doorhit, angle, midangle, focaltx, focalty, xstep, ystep); */
	
	if (my_ray.tilehit) {
		if (my_ray.tilehit & 0x80) {
			if (my_ray.tilehit & 0x40) {
				/* vertpushwall */
				doorhit = my_ray.yintercept + (signed)((signed)pwallpos * ystep) / 64;
			
				if (TILE(doorhit) != TILE(my_ray.yintercept)) 
					goto passvert;
					
				my_ray.yintercept = doorhit;
				my_ray.xintercept = my_ray.xtile << TILESHIFT;
				HitVertPWall(postx, &my_ray);
			} else {
				/* vertdoor */
				doorhit = my_ray.yintercept + ystep / 2;

				if (TILE(doorhit) != TILE(my_ray.yintercept))
					goto passvert;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passvert;
				
				my_ray.yintercept = doorhit;
				my_ray.xintercept = (my_ray.xtile << TILESHIFT) + TILEGLOBAL/2;
				HitVertDoorNew(postx, &my_ray);
			}
		} else {
			my_ray.xintercept = my_ray.xtile << TILESHIFT;
			HitVertWallNew(postx, &my_ray);
		}
		continue;
	}
passvert:
//	setspotvis(my_ray.xtile,TILE(my_ray.yintercept));
	spotvis[my_ray.xtile][TILE(my_ray.yintercept)] = 1;
	my_ray.xtile += my_ray.xtilestep;
	my_ray.yintercept += ystep;
	goto vertcheck;
	
horizcheck:
	/* check intersections with horizontal walls */
	
	if (!samex(my_ray.xtilestep, my_ray.xintercept, my_ray.xtile))
		goto vertentry;

horizentry:
	my_ray.tilehit = tilemap[TILE(my_ray.xintercept)][my_ray.ytile];
	/* printf("horz: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", postx, tilehit, xtile, ytile, xintercept, yintercept, xpartialup, xpartialdown, ypartialup, ypartialdown, xpartial, ypartial, doorhit, angle, midangle, focaltx, focalty, xstep, ystep); */
	
	if (my_ray.tilehit) {
		if (my_ray.tilehit & 0x80) {
			if (my_ray.tilehit & 0x40) {
				doorhit = my_ray.xintercept + (signed)((signed)pwallpos * xstep) / 64;
		    	
				/* horizpushwall */
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = my_ray.ytile << TILESHIFT; 
				HitHorizPWall(postx, &my_ray);
			} else {
				doorhit = my_ray.xintercept + xstep / 2;
				
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = (my_ray.ytile << TILESHIFT) + TILEGLOBAL/2;
				HitHorizDoorNew(postx, &my_ray);
			}
		} else {
			my_ray.yintercept = my_ray.ytile << TILESHIFT;
			HitHorizWallNew(postx, &my_ray);
		}
		continue;
	}
passhoriz:
	spotvis[TILE(my_ray.xintercept)][my_ray.ytile] = 1;
//	setspotvis(TILE(my_ray.xintercept), my_ray.ytile);
	my_ray.ytile += my_ray.ytilestep;
	my_ray.xintercept += xstep;
	goto horizcheck;
}
}


#endif

#else
/*
====================
=
= HitVertWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

static inline void HitVertWall (int pixx, int texdelta, ray_struc *ray)
{
    int wallpic;
    word texture;

    texture = ((ray->yintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    if (ray->xtilestep == -1)
    {
        texture = TEXTUREMASK-texture;
        ray->xintercept += TILEGLOBAL;
    }

    if(ray->lastside==1 && ray->lastintercept==ray->xtile && ray->lasttilehit==ray->tilehit && !(ray->lasttilehit & 0x40))
    {
        ScalePost(ray->postx,ray->postsource);
		ray->postx = pixx;
			
        if((pixx&3) && texture == ray->lasttexture)
        {
//            ScalePost(ray->postx,ray->postsource);
 //           ray->postx = pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
//        ScalePost(ray->postx,ray->postsource);
        wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
        ray->postsource+=texture-ray->lasttexture;
//        postwidth=1;
//        ray->postx=pixx;
        ray->lasttexture=texture;
        return;
    }

    if(ray->lastside!=-1) ScalePost(ray->postx,ray->postsource);

    ray->lastside=1;
    ray->lastintercept=ray->xtile;
    ray->lasttilehit=ray->tilehit;
    ray->lasttexture=texture;
    wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
    ray->postx = pixx;
//    postwidth = 1;

    if (ray->tilehit & 0x40)
    {                                                               // check for adjacent doors
        ray->ytile = (short)(ray->yintercept>>TILESHIFT);
        if ( tilemap[ray->xtile-ray->xtilestep][ray->ytile]&0x80 )
            wallpic = DOORWALL+3;
        else
            wallpic = vertwall[ray->tilehit & ~0x40];
    }
    else
        wallpic = vertwall[ray->tilehit];

    ray->postsource = PM_GetTexture(wallpic) + texture;
}


/*
====================
=
= HitHorizWall
=
= tilehit bit 7 is 0, because it's not a door tile
= if bit 6 is 1 and the adjacent tile is a door tile, use door side pic
=
====================
*/

static inline void HitHorizWall (int pixx, int texdelta, ray_struc *ray)
{
    int wallpic;
    word texture;

    texture = ((ray->xintercept+texdelta)>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;
    if (ray->ytilestep == -1)
        ray->yintercept += TILEGLOBAL;
    else
        texture = TEXTUREMASK-texture;

    if(ray->lastside==0 && ray->lastintercept==ray->ytile && ray->lasttilehit==ray->tilehit && !(ray->lasttilehit & 0x40))
    {
        ScalePost(ray->postx,ray->postsource);
        ray->postx=pixx;
			
        if((pixx&3) && texture == ray->lasttexture)
        {
//            ScalePost(ray->postx,ray->postsource);
//            ray->postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
//        ScalePost(ray->postx,ray->postsource);
        wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
        ray->postsource+=texture-ray->lasttexture;
//        postwidth=1;
//        ray->postx=pixx;
        ray->lasttexture=texture;
        return;
    }

    if(ray->lastside!=-1) ScalePost(ray->postx,ray->postsource);

    ray->lastside=0;
    ray->lastintercept=ray->ytile;
    ray->lasttilehit=ray->tilehit;
    ray->lasttexture=texture;
    wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
    ray->postx = pixx;
//    postwidth = 1;

    if (ray->tilehit & 0x40)
    {                                                               // check for adjacent doors
        ray->xtile = (short)(ray->xintercept>>TILESHIFT);
        if ( tilemap[ray->xtile][ray->ytile-ray->ytilestep]&0x80)
            wallpic = DOORWALL+2;
        else
            wallpic = horizwall[ray->tilehit & ~0x40];
    }
    else
        wallpic = horizwall[ray->tilehit];
    ray->postsource = PM_GetTexture(wallpic) + texture;
}

//==========================================================================

/*
====================
=
= HitHorizDoor
=
====================
*/

void HitHorizDoor2 (int pixx,ray_struc *ray)
{
    unsigned int doorpage=0;
    unsigned int doornum;
    word texture;

    doornum = ray->tilehit&0x7f;
    texture = ((ray->xintercept-doorposition[doornum])>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    if(ray->lasttilehit==ray->tilehit)
    {
        ScalePost(ray->postx,ray->postsource);
        ray->postx=pixx;
 
        if((pixx&3) && texture == ray->lasttexture)
        {
 //           ScalePost(ray->postx,ray->postsource);
 //           ray->postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
//        ScalePost(ray->postx,ray->postsource);
        wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
        ray->postsource+=texture-ray->lasttexture;
//        postwidth=1;
//        ray->postx=pixx;
        ray->lasttexture=texture;
        return;
    }

    if(ray->lastside!=-1) ScalePost(ray->postx,ray->postsource);

    ray->lastside=2;
    ray->lasttilehit=ray->tilehit;
    ray->lasttexture=texture;
    wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
    ray->postx = pixx;
//    postwidth = 1;

    switch(doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL;
            break;
        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL+6;
            break;
        case dr_elevator:
            doorpage = DOORWALL+4;
            break;
    }
    ray->postsource = PM_GetTexture(doorpage) + texture;
}

//==========================================================================

/*
====================
=
= HitVertDoor
=
====================
*/

void HitVertDoor2 (int pixx,ray_struc *ray)
{
    unsigned int doorpage=0;
    unsigned int doornum;
    word texture;

    doornum = ray->tilehit&0x7f;
    texture = ((ray->yintercept-doorposition[doornum])>>TEXTUREFROMFIXEDSHIFT)&TEXTUREMASK;

    if(ray->lasttilehit==ray->tilehit)
    {
		ScalePost(ray->postx,ray->postsource);
		ray->postx=pixx;
			
        if((pixx&3) && texture == ray->lasttexture)
        {
//            ScalePost(ray->postx,ray->postsource);
//            ray->postx=pixx;
            wallheight[pixx] = wallheight[pixx-1];
            return;
        }
//        ScalePost(ray->postx,ray->postsource);
        wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
        ray->postsource+=texture-ray->lasttexture;
//        postwidth=1;
//        ray->postx=pixx;
        ray->lasttexture=texture;
        return;
    }

    if(ray->lastside!=-1) ScalePost(ray->postx,ray->postsource);

    ray->lastside=2;
    ray->lasttilehit=ray->tilehit;
    ray->lasttexture=texture;
    wallheight[pixx] = CalcHeight(ray->xintercept,ray->yintercept);
    ray->postx = pixx;
//    postwidth = 1;

    switch(doorobjlist[doornum].lock)
    {
        case dr_normal:
            doorpage = DOORWALL+1;
            break;
        case dr_lock1:
        case dr_lock2:
        case dr_lock3:
        case dr_lock4:
            doorpage = DOORWALL+7;
            break;
        case dr_elevator:
            doorpage = DOORWALL+5;
            break;
    }
    ray->postsource = PM_GetTexture(doorpage) + texture;
}

//==========================================================================

#define HitHorizBorder HitHorizWall
#define HitVertBorder HitVertWall

//==========================================================================

void AsmRefresh()
{
	static	ray_struc my_ray;
// vbt :moins de variable globale
	longword xpartialup,xpartialdown,ypartialup,ypartialdown;
	int texdelta;
	word xspot,yspot;
	
    xpartialdown = viewx&(TILEGLOBAL-1);
    xpartialup = TILEGLOBAL-xpartialdown;
    ypartialdown = viewy&(TILEGLOBAL-1);
    ypartialup = TILEGLOBAL-ypartialdown;	
	
    int32_t xstep=0,ystep=0;
    longword xpartial=0,ypartial=0;
	
    my_ray.lastside = -1;                  // the first pixel is on a new wall	
	
    short focaltx = (short)(viewx>>TILESHIFT);
    short focalty = (short)(viewy>>TILESHIFT);	
    boolean playerInPushwallBackTile = tilemap[focaltx][focalty] == 64;
#ifdef USE_SLAVE
    for(int pixx=0;pixx<=(viewwidth/2);pixx++)
#else
    for(int pixx=0;pixx<viewwidth;pixx++)
#endif		
    {
        short angl=midangle+pixelangle[pixx];
        if(angl<0) angl+=FINEANGLES;
        if(angl>=3600) angl-=FINEANGLES;
        if(angl<900)
        {
            my_ray.xtilestep=1;
            my_ray.ytilestep=-1;
            xstep=finetangent[900-1-angl];
            ystep=-finetangent[angl];
            xpartial=xpartialup;
            ypartial=ypartialdown;
        }
        else if(angl<1800)
        {
            my_ray.xtilestep=-1;
            my_ray.ytilestep=-1;
            xstep=-finetangent[angl-900];
            ystep=-finetangent[1800-1-angl];
            xpartial=xpartialdown;
            ypartial=ypartialdown;
        }
        else if(angl<2700)
        {
            my_ray.xtilestep=-1;
            my_ray.ytilestep=1;
            xstep=-finetangent[2700-1-angl];
            ystep=finetangent[angl-1800];
            xpartial=xpartialdown;
            ypartial=ypartialup;
        }
        else if(angl<3600)
        {
            my_ray.xtilestep=1;
            my_ray.ytilestep=1;
            xstep=finetangent[angl-2700];
            ystep=finetangent[3600-1-angl];
            xpartial=xpartialup;
            ypartial=ypartialup;
        }
        my_ray.yintercept=FixedMul(ystep,xpartial)+viewy;
        my_ray.xtile=focaltx+my_ray.xtilestep;
        xspot=(word)((my_ray.xtile<<mapshift)+((uint32_t)my_ray.yintercept>>16));
        my_ray.xintercept=FixedMul(xstep,ypartial)+viewx;
        my_ray.ytile=focalty+my_ray.ytilestep;
        yspot=(word)((((uint32_t)my_ray.xintercept>>16)<<mapshift)+my_ray.ytile);
        texdelta=0;

        // Special treatment when player is in back tile of pushwall
        if(playerInPushwallBackTile)
        {
            if(    (pwalldir == di_east && my_ray.xtilestep ==  1)
                || (pwalldir == di_west && my_ray.xtilestep == -1))
            {
                int32_t yintbuf = my_ray.yintercept - ((ystep * (64 - pwallpos)) >> 6);
                if((yintbuf >> 16) == focalty)   // ray hits pushwall back?
                {
                    if(pwalldir == di_east)
                        my_ray.xintercept = (focaltx << TILESHIFT) + (pwallpos << 10);
                    else
                        my_ray.xintercept = (focaltx << TILESHIFT) - TILEGLOBAL + ((64 - pwallpos) << 10);
                    my_ray.yintercept = yintbuf;
                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                    my_ray.tilehit = pwalltile;
                    HitVertWall(pixx,texdelta,&my_ray);
                    continue;
                }
            }
            else if((pwalldir == di_south && my_ray.ytilestep ==  1)
                ||  (pwalldir == di_north && my_ray.ytilestep == -1))
            {
                int32_t xintbuf = my_ray.xintercept - ((xstep * (64 - pwallpos)) >> 6);
                if((xintbuf >> 16) == focaltx)   // ray hits pushwall back?
                {
                    my_ray.xintercept = xintbuf;
                    if(pwalldir == di_south)
                        my_ray.yintercept = (focalty << TILESHIFT) + (pwallpos << 10);
                    else
                        my_ray.yintercept = (focalty << TILESHIFT) - TILEGLOBAL + ((64 - pwallpos) << 10);
// vbt new
                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                    my_ray.tilehit = pwalltile;
                    HitHorizWall(pixx,texdelta,&my_ray);
                    continue;
                }
            }
        }

        do
        {
			if (!samey(my_ray.ytilestep, my_ray.yintercept, my_ray.ytile))        goto horizentry;
/*			
            if(my_ray.ytilestep==-1 && (my_ray.yintercept>>16)<=my_ray.ytile) goto horizentry;
            if(my_ray.ytilestep==1 && (my_ray.yintercept>>16)>=my_ray.ytile) goto horizentry;
*/			
vertentry:
/*            if((uint32_t)my_ray.yintercept>mapheight*65536-1 || (word)my_ray.xtile>=mapwidth)
            {
                if(my_ray.xtile<0) my_ray.xintercept=0, my_ray.xtile=0;
                else if(my_ray.xtile>=mapwidth) my_ray.xintercept=mapwidth<<TILESHIFT, my_ray.xtile=mapwidth-1;
                else my_ray.xtile=(short) (my_ray.xintercept >> TILESHIFT);
                if(my_ray.yintercept<0) my_ray.yintercept=0, my_ray.ytile=0;
                else if(my_ray.yintercept>=(mapheight<<TILESHIFT)) my_ray.yintercept=mapheight<<TILESHIFT, my_ray.ytile=mapheight-1;
                yspot=0xffff;
                my_ray.tilehit=0;
                HitHorizBorder(pixx,texdelta,&my_ray);
                break;
            }
            if(xspot>=maparea) break;
*/			
            my_ray.tilehit=((byte *)tilemap)[xspot];
            if(my_ray.tilehit)
            {
                if(my_ray.tilehit&0x80)
                {
                    int32_t yintbuf=my_ray.yintercept+(ystep>>1);
                    if((yintbuf>>16)!=(my_ray.yintercept>>16))
                        goto passvert;
                    if((word)yintbuf<doorposition[my_ray.tilehit&0x7f])
                        goto passvert;
                    my_ray.yintercept=yintbuf;
                    my_ray.xintercept=(my_ray.xtile<<TILESHIFT)|0x8000;
// vbt new
                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                    HitVertDoor2(pixx,&my_ray);
                }
                else
                {
                    if(my_ray.tilehit==64)
                    {
                        if(pwalldir==di_west || pwalldir==di_east)
                        {
	                        int32_t yintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(pwalldir==di_west)
                            {
                                pwallposnorm = 64-pwallpos;
                                pwallposinv = pwallpos;
                            }
                            else
                            {
                                pwallposnorm = pwallpos;
                                pwallposinv = 64-pwallpos;
                            }
                            if(pwalldir == di_east && my_ray.xtile==pwallx && ((uint32_t)my_ray.yintercept>>16)==pwally
                                || pwalldir == di_west && !(my_ray.xtile==pwallx && ((uint32_t)my_ray.yintercept>>16)==pwally))
                            {
                                yintbuf=my_ray.yintercept+((ystep*pwallposnorm)>>6);
                                if((yintbuf>>16)!=(my_ray.yintercept>>16))
                                    goto passvert;

                                my_ray.xintercept=(my_ray.xtile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                            }
                            else
                            {
                                yintbuf=my_ray.yintercept+((ystep*pwallposinv)>>6);
                                if((yintbuf>>16)!=(my_ray.yintercept>>16))
                                    goto passvert;

                                my_ray.xintercept=(my_ray.xtile<<TILESHIFT)-(pwallposinv<<10);
                            }
							my_ray.yintercept=yintbuf;
// vbt new
							my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
							my_ray.tilehit=pwalltile;
							HitVertWall(pixx,texdelta,&my_ray);							
                        }
                        else
                        {
                            int pwallposi = pwallpos;
                            if(pwalldir==di_north) pwallposi = 64-pwallpos;
                            if(pwalldir==di_south && (word)my_ray.yintercept<(pwallposi<<10)
                                || pwalldir==di_north && (word)my_ray.yintercept>(pwallposi<<10))
                            {
                                if(((uint32_t)my_ray.yintercept>>16)==pwally && my_ray.xtile==pwallx)
                                {
                                    if(pwalldir==di_south && (int32_t)((word)my_ray.yintercept)+ystep<(pwallposi<<10)
                                            || pwalldir==di_north && (int32_t)((word)my_ray.yintercept)+ystep>(pwallposi<<10))
                                        goto passvert;

                                    if(pwalldir==di_south)
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xintercept-((xstep*(64-pwallpos))>>6);
                                }
                                else
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xtile<<TILESHIFT;
                                }
// vbt new
                                my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                                my_ray.tilehit=pwalltile;
                                HitVertWall(pixx,texdelta,&my_ray);								
                            }
                            else
                            {
                                if(((uint32_t)my_ray.yintercept>>16)==pwally && my_ray.xtile==pwallx)
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xtile<<TILESHIFT;

                                }
                                else
                                {
                                    if(pwalldir==di_south && (int32_t)((word)my_ray.yintercept)+ystep>(pwallposi<<10)
                                            || pwalldir==di_north && (int32_t)((word)my_ray.yintercept)+ystep<(pwallposi<<10))
                                        goto passvert;

                                    if(pwalldir==di_south)
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)-((64-pwallpos)<<10);
                                    else
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)+((64-pwallpos)<<10);
                                    my_ray.xintercept=my_ray.xintercept-((xstep*pwallpos)>>6);
                                }
// vbt new
								my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
								my_ray.tilehit=pwalltile;
								HitVertWall(pixx,texdelta,&my_ray);								
                            }
                        }
                    }
                    else
                    {
                        my_ray.xintercept=my_ray.xtile<<TILESHIFT;
// vbt new
                        my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                        HitVertWall(pixx,texdelta,&my_ray);
                    }
                }
                break;
            }
passvert:
            *((byte *)spotvis+xspot)=1;
            my_ray.xtile+=my_ray.xtilestep;
            my_ray.yintercept+=ystep;
            xspot=(word)((my_ray.xtile<<mapshift)+((uint32_t)my_ray.yintercept>>16));
        }
        while(1);
        continue;

        do
        {
			if (!samex(my_ray.xtilestep, my_ray.xintercept, my_ray.xtile))        goto vertentry;			
/*			
            if(my_ray.xtilestep==-1 && (my_ray.xintercept>>16)<=my_ray.xtile) goto vertentry;
            if(my_ray.xtilestep==1 && (my_ray.xintercept>>16)>=my_ray.xtile) goto vertentry;
*/	

horizentry:
 /*           if((uint32_t)my_ray.xintercept>mapwidth*65536-1 || (word)my_ray.ytile>=mapheight)
            {
                if(my_ray.ytile<0) my_ray.yintercept=0, my_ray.ytile=0;
                else if(my_ray.ytile>=mapheight) my_ray.yintercept=mapheight<<TILESHIFT, my_ray.ytile=mapheight-1;
                else my_ray.ytile=(short) (my_ray.yintercept >> TILESHIFT);
                if(my_ray.xintercept<0) my_ray.xintercept=0, my_ray.xtile=0;
                else if(my_ray.xintercept>=(mapwidth<<TILESHIFT)) my_ray.xintercept=mapwidth<<TILESHIFT, my_ray.xtile=mapwidth-1;
                xspot=0xffff;
                my_ray.tilehit=0;
                HitVertBorder(pixx,texdelta,&my_ray);
                break;
            }
            if(yspot>=maparea) break;
*/			
            my_ray.tilehit=((byte *)tilemap)[yspot];
            if(my_ray.tilehit)
            {
                if(my_ray.tilehit&0x80)
                {
                    int32_t xintbuf=my_ray.xintercept+(xstep>>1);
                    if((xintbuf>>16)!=(my_ray.xintercept>>16))
                        goto passhoriz;
                    if((word)xintbuf<doorposition[my_ray.tilehit&0x7f])
                        goto passhoriz;
                    my_ray.xintercept=xintbuf;
                    my_ray.yintercept=(my_ray.ytile<<TILESHIFT)+0x8000;
// vbt new
                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                    HitHorizDoor2(pixx,&my_ray);
                }
                else
                {
                    if(my_ray.tilehit==64)
                    {
                        if(pwalldir==di_north || pwalldir==di_south)
                        {
                            int32_t xintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(pwalldir==di_north)
                            {
                                pwallposnorm = 64-pwallpos;
                                pwallposinv = pwallpos;
                            }
                            else
                            {
                                pwallposnorm = pwallpos;
                                pwallposinv = 64-pwallpos;
                            }
                            if(pwalldir == di_south && my_ray.ytile==pwally && ((uint32_t)my_ray.xintercept>>16)==pwallx
                                || pwalldir == di_north && !(my_ray.ytile==pwally && ((uint32_t)my_ray.xintercept>>16)==pwallx))
                            {
                                xintbuf=my_ray.xintercept+((xstep*pwallposnorm)>>6);
                                if((xintbuf>>16)!=(my_ray.xintercept>>16))
                                    goto passhoriz;

                                my_ray.yintercept=(my_ray.ytile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                                my_ray.xintercept=xintbuf;
// vbt new
                                my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                my_ray.tilehit=pwalltile;
                                HitHorizWall(pixx,texdelta,&my_ray);
                            }
                            else
                            {
                                xintbuf=my_ray.xintercept+((xstep*pwallposinv)>>6);
                                if((xintbuf>>16)!=(my_ray.xintercept>>16))
                                    goto passhoriz;

                                my_ray.yintercept=(my_ray.ytile<<TILESHIFT)-(pwallposinv<<10);
                                my_ray.xintercept=xintbuf;
// vbt new
                                my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                my_ray.tilehit=pwalltile;
                                HitHorizWall(pixx,texdelta,&my_ray);
                            }
                        }
                        else
                        {
                            int pwallposi = pwallpos;
                            if(pwalldir==di_west) pwallposi = 64-pwallpos;
                            if(pwalldir==di_east && (word)my_ray.xintercept<(pwallposi<<10)
                                    || pwalldir==di_west && (word)my_ray.xintercept>(pwallposi<<10))
                            {
                                if(((uint32_t)my_ray.xintercept>>16)==pwallx && my_ray.ytile==pwally)
                                {
                                    if(pwalldir==di_east && (int32_t)((word)my_ray.xintercept)+xstep<(pwallposi<<10)
                                            || pwalldir==di_west && (int32_t)((word)my_ray.xintercept)+xstep>(pwallposi<<10))
                                        goto passhoriz;

                                    if(pwalldir==di_east)
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    my_ray.yintercept=my_ray.yintercept-((ystep*(64-pwallpos))>>6);
// vbt new
                                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitVertWall(pixx,texdelta,&my_ray);
                                }
                                else
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.yintercept=my_ray.ytile<<TILESHIFT;
// vbt new
                                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitHorizWall(pixx,texdelta,&my_ray);
                                }
                            }
                            else
                            {
                                if(((uint32_t)my_ray.xintercept>>16)==pwallx && my_ray.ytile==pwally)
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.yintercept=my_ray.ytile<<TILESHIFT;
                                }
                                else
                                {
                                    if(pwalldir==di_east && (int32_t)((word)my_ray.xintercept)+xstep>(pwallposi<<10)
                                            || pwalldir==di_west && (int32_t)((word)my_ray.xintercept)+xstep<(pwallposi<<10))
                                        goto passhoriz;

                                    if(pwalldir==di_east)
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)-((64-pwallpos)<<10);
                                    else
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)+((64-pwallpos)<<10);
                                    my_ray.yintercept=my_ray.yintercept-((ystep*pwallpos)>>6);
                                }
// vbt new
								my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
								my_ray.tilehit=pwalltile;
								HitVertWall(pixx,texdelta,&my_ray);								
                            }
                        }
                    }
                    else
                    {
                        my_ray.yintercept=my_ray.ytile<<TILESHIFT;
// vbt new
                        my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                        HitHorizWall(pixx,texdelta,&my_ray);
                    }
                }
                break;
            }
passhoriz:
            *((byte *)spotvis+yspot)=1;
            my_ray.ytile+=my_ray.ytilestep;
            my_ray.xintercept+=xstep;
            yspot=(word)((((uint32_t)my_ray.xintercept>>16)<<mapshift)+my_ray.ytile);
        }
        while(1);
    }
// vbt : uniquement à la fin de tous les strips	
#ifndef USE_SLAVE
	    ScalePost(my_ray.postx,my_ray.postsource);                   // no more optimization on last post
#endif		
}
#ifdef USE_SLAVE
//==========================================================================

void AsmRefreshSlave()
{
// vbt :moins de variable globale
	longword xpartialup,xpartialdown,ypartialup,ypartialdown;
//	word tilehit;
	int texdelta;
	word xspot,yspot;
	
    xpartialdown = viewx&(TILEGLOBAL-1);
    xpartialup = TILEGLOBAL-xpartialdown;
    ypartialdown = viewy&(TILEGLOBAL-1);
    ypartialup = TILEGLOBAL-ypartialdown;	
	
    int32_t xstep=0,ystep=0;
    longword xpartial=0,ypartial=0;
	static ray_struc my_ray;
	
//    my_ray.lastside = -1;                  // the first pixel is on a new wall
    short focaltx = (short)(viewx>>TILESHIFT);
    short focalty = (short)(viewy>>TILESHIFT);	
    boolean playerInPushwallBackTile = tilemap[focaltx][focalty] == 64;

    for(int pixx=viewwidth>>1;pixx<viewwidth;pixx++)
    {
        short angl=midangle+pixelangle[pixx];
        if(angl<0) angl+=FINEANGLES;
        if(angl>=3600) angl-=FINEANGLES;
        if(angl<900)
        {
            my_ray.xtilestep=1;
            my_ray.ytilestep=-1;
            xstep=finetangent[900-1-angl];
            ystep=-finetangent[angl];
            xpartial=xpartialup;
            ypartial=ypartialdown;
        }
        else if(angl<1800)
        {
            my_ray.xtilestep=-1;
            my_ray.ytilestep=-1;
            xstep=-finetangent[angl-900];
            ystep=-finetangent[1800-1-angl];
            xpartial=xpartialdown;
            ypartial=ypartialdown;
        }
        else if(angl<2700)
        {
            my_ray.xtilestep=-1;
            my_ray.ytilestep=1;
            xstep=-finetangent[2700-1-angl];
            ystep=finetangent[angl-1800];
            xpartial=xpartialdown;
            ypartial=ypartialup;
        }
        else if(angl<3600)
        {
            my_ray.xtilestep=1;
            my_ray.ytilestep=1;
            xstep=finetangent[angl-2700];
            ystep=finetangent[3600-1-angl];
            xpartial=xpartialup;
            ypartial=ypartialup;
        }
        my_ray.yintercept=FixedMul(ystep,xpartial)+viewy;
        my_ray.xtile=focaltx+my_ray.xtilestep;
        xspot=(word)((my_ray.xtile<<mapshift)+((uint32_t)my_ray.yintercept>>16));
        my_ray.xintercept=FixedMul(xstep,ypartial)+viewx;
        my_ray.ytile=focalty+my_ray.ytilestep;
        yspot=(word)((((uint32_t)my_ray.xintercept>>16)<<mapshift)+my_ray.ytile);
        texdelta=0;

        // Special treatment when player is in back tile of pushwall
        if(playerInPushwallBackTile)
        {
            if(    pwalldir == di_east && my_ray.xtilestep ==  1
                || pwalldir == di_west && my_ray.xtilestep == -1)
            {
                int32_t yintbuf = my_ray.yintercept - ((ystep * (64 - pwallpos)) >> 6);
                if((yintbuf >> 16) == focalty)   // ray hits pushwall back?
                {
                    if(pwalldir == di_east)
                        my_ray.xintercept = (focaltx << TILESHIFT) + (pwallpos << 10);
                    else
                        my_ray.xintercept = (focaltx << TILESHIFT) - TILEGLOBAL + ((64 - pwallpos) << 10);
                    my_ray.yintercept = yintbuf;
                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                    my_ray.tilehit = pwalltile;
                    HitVertWall(pixx,texdelta,&my_ray);
                    continue;
                }
            }
            else if(pwalldir == di_south && my_ray.ytilestep ==  1
                ||  pwalldir == di_north && my_ray.ytilestep == -1)
            {
                int32_t xintbuf = my_ray.xintercept - ((xstep * (64 - pwallpos)) >> 6);
                if((xintbuf >> 16) == focaltx)   // ray hits pushwall back?
                {
                    my_ray.xintercept = xintbuf;
                    if(pwalldir == di_south)
                        my_ray.yintercept = (focalty << TILESHIFT) + (pwallpos << 10);
                    else
                        my_ray.yintercept = (focalty << TILESHIFT) - TILEGLOBAL + ((64 - pwallpos) << 10);
// vbt new
                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                    my_ray.tilehit = pwalltile;
                    HitHorizWall(pixx,texdelta,&my_ray);
                    continue;
                }
            }
        }

        do
        {
			if (!samey(my_ray.ytilestep, my_ray.yintercept, my_ray.ytile))        goto horizentry;
/*			
            if(my_ray.ytilestep==-1 && (my_ray.yintercept>>16)<=my_ray.ytile) goto horizentry;
            if(my_ray.ytilestep==1 && (my_ray.yintercept>>16)>=my_ray.ytile) goto horizentry;
*/			
vertentry:
/*            if((uint32_t)my_ray.yintercept>mapheight*65536-1 || (word)my_ray.xtile>=mapwidth)
            {
                if(my_ray.xtile<0) my_ray.xintercept=0, my_ray.xtile=0;
                else if(my_ray.xtile>=mapwidth) my_ray.xintercept=mapwidth<<TILESHIFT, my_ray.xtile=mapwidth-1;
                else my_ray.xtile=(short) (my_ray.xintercept >> TILESHIFT);
                if(my_ray.yintercept<0) my_ray.yintercept=0, my_ray.ytile=0;
                else if(my_ray.yintercept>=(mapheight<<TILESHIFT)) my_ray.yintercept=mapheight<<TILESHIFT, my_ray.ytile=mapheight-1;
                yspot=0xffff;
                my_ray.tilehit=0;
                HitHorizBorder(pixx,texdelta,&my_ray);
                break;
            }
 */         if(xspot>=maparea) break;
			
            my_ray.tilehit=((byte *)tilemap)[xspot];
            if(my_ray.tilehit)
            {
                if(my_ray.tilehit&0x80)
                {
                    int32_t yintbuf=my_ray.yintercept+(ystep>>1);
                    if((yintbuf>>16)!=(my_ray.yintercept>>16))
                        goto passvert;
                    if((word)yintbuf<doorposition[my_ray.tilehit&0x7f])
                        goto passvert;
                    my_ray.yintercept=yintbuf;
                    my_ray.xintercept=(my_ray.xtile<<TILESHIFT)|0x8000;
// vbt new
                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                    HitVertDoor2(pixx,&my_ray);
                }
                else
                {
                    if(my_ray.tilehit==64)
                    {
                        if(pwalldir==di_west || pwalldir==di_east)
                        {
	                        int32_t yintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(pwalldir==di_west)
                            {
                                pwallposnorm = 64-pwallpos;
                                pwallposinv = pwallpos;
                            }
                            else
                            {
                                pwallposnorm = pwallpos;
                                pwallposinv = 64-pwallpos;
                            }
                            if(pwalldir == di_east && my_ray.xtile==pwallx && ((uint32_t)my_ray.yintercept>>16)==pwally
                                || pwalldir == di_west && !(my_ray.xtile==pwallx && ((uint32_t)my_ray.yintercept>>16)==pwally))
                            {
                                yintbuf=my_ray.yintercept+((ystep*pwallposnorm)>>6);
                                if((yintbuf>>16)!=(my_ray.yintercept>>16))
                                    goto passvert;

                                my_ray.xintercept=(my_ray.xtile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                            }
                            else
                            {
                                yintbuf=my_ray.yintercept+((ystep*pwallposinv)>>6);
                                if((yintbuf>>16)!=(my_ray.yintercept>>16))
                                    goto passvert;

                                my_ray.xintercept=(my_ray.xtile<<TILESHIFT)-(pwallposinv<<10);
                            }
							my_ray.yintercept=yintbuf;
// vbt new
							my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
							my_ray.tilehit=pwalltile;
							HitVertWall(pixx,texdelta,&my_ray);							
                        }
                        else
                        {
                            int pwallposi = pwallpos;
                            if(pwalldir==di_north) pwallposi = 64-pwallpos;
                            if(pwalldir==di_south && (word)my_ray.yintercept<(pwallposi<<10)
                                || pwalldir==di_north && (word)my_ray.yintercept>(pwallposi<<10))
                            {
                                if(((uint32_t)my_ray.yintercept>>16)==pwally && my_ray.xtile==pwallx)
                                {
                                    if(pwalldir==di_south && (int32_t)((word)my_ray.yintercept)+ystep<(pwallposi<<10)
                                            || pwalldir==di_north && (int32_t)((word)my_ray.yintercept)+ystep>(pwallposi<<10))
                                        goto passvert;

                                    if(pwalldir==di_south)
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xintercept-((xstep*(64-pwallpos))>>6);
                                }
                                else
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xtile<<TILESHIFT;
                                }
// vbt new
								my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
								my_ray.tilehit=pwalltile;
								HitVertWall(pixx,texdelta,&my_ray);								
                            }
                            else
                            {
                                if(((uint32_t)my_ray.yintercept>>16)==pwally && my_ray.xtile==pwallx)
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.xintercept=my_ray.xtile<<TILESHIFT;
                                }
                                else
                                {
                                    if(pwalldir==di_south && (int32_t)((word)my_ray.yintercept)+ystep>(pwallposi<<10)
                                            || pwalldir==di_north && (int32_t)((word)my_ray.yintercept)+ystep<(pwallposi<<10))
                                        goto passvert;

                                    if(pwalldir==di_south)
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)-((64-pwallpos)<<10);
                                    else
                                        my_ray.yintercept=(my_ray.yintercept&0xffff0000)+((64-pwallpos)<<10);
                                    my_ray.xintercept=my_ray.xintercept-((xstep*pwallpos)>>6);
                                }
// vbt new
								my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
								my_ray.tilehit=pwalltile;
								HitHorizWall(pixx,texdelta,&my_ray);								
                            }
                        }
                    }
                    else
                    {
                        my_ray.xintercept=my_ray.xtile<<TILESHIFT;
// vbt new
                        my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                        HitVertWall(pixx,texdelta,&my_ray);
                    }
                }
                break;
            }
passvert:
            *((byte *)spotvis+xspot)=1;
            my_ray.xtile+=my_ray.xtilestep;
            my_ray.yintercept+=ystep;
            xspot=(word)((my_ray.xtile<<mapshift)+((uint32_t)my_ray.yintercept>>16));
        }
        while(1);
        continue;

        do
        {
			if (!samex(my_ray.xtilestep, my_ray.xintercept, my_ray.xtile))        goto vertentry;			
/*			
            if(my_ray.xtilestep==-1 && (my_ray.xintercept>>16)<=my_ray.xtile) goto vertentry;
            if(my_ray.xtilestep==1 && (my_ray.xintercept>>16)>=my_ray.xtile) goto vertentry;
*/			
horizentry:
/*            if((uint32_t)my_ray.xintercept>mapwidth*65536-1 || (word)my_ray.ytile>=mapheight)
            {
                if(my_ray.ytile<0) my_ray.yintercept=0, my_ray.ytile=0;
                else if(my_ray.ytile>=mapheight) my_ray.yintercept=mapheight<<TILESHIFT, my_ray.ytile=mapheight-1;
                else my_ray.ytile=(short) (my_ray.yintercept >> TILESHIFT);
                if(my_ray.xintercept<0) my_ray.xintercept=0, my_ray.xtile=0;
                else if(my_ray.xintercept>=(mapwidth<<TILESHIFT)) my_ray.xintercept=mapwidth<<TILESHIFT, my_ray.xtile=mapwidth-1;
                xspot=0xffff;
                my_ray.tilehit=0;
                HitVertBorder(pixx,texdelta,&my_ray);
                break;
            }
*/
            if(yspot>=maparea) break;
			
            my_ray.tilehit=((byte *)tilemap)[yspot];
            if(my_ray.tilehit)
            {
                if(my_ray.tilehit&0x80)
                {
                    int32_t xintbuf=my_ray.xintercept+(xstep>>1);
                    if((xintbuf>>16)!=(my_ray.xintercept>>16))
                        goto passhoriz;
                    if((word)xintbuf<doorposition[my_ray.tilehit&0x7f])
                        goto passhoriz;
                    my_ray.xintercept=xintbuf;
                    my_ray.yintercept=(my_ray.ytile<<TILESHIFT)+0x8000;
// vbt new
                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                    HitHorizDoor2(pixx,&my_ray);
                }
                else
                {
                    if(my_ray.tilehit==64)
                    {
                        if(pwalldir==di_north || pwalldir==di_south)
                        {
                            int32_t xintbuf;
                            int pwallposnorm;
                            int pwallposinv;
                            if(pwalldir==di_north)
                            {
                                pwallposnorm = 64-pwallpos;
                                pwallposinv = pwallpos;
                            }
                            else
                            {
                                pwallposnorm = pwallpos;
                                pwallposinv = 64-pwallpos;
                            }
                            if(pwalldir == di_south && my_ray.ytile==pwally && ((uint32_t)my_ray.xintercept>>16)==pwallx
                                || pwalldir == di_north && !(my_ray.ytile==pwally && ((uint32_t)my_ray.xintercept>>16)==pwallx))
                            {
                                xintbuf=my_ray.xintercept+((xstep*pwallposnorm)>>6);
                                if((xintbuf>>16)!=(my_ray.xintercept>>16))
                                    goto passhoriz;

                                my_ray.yintercept=(my_ray.ytile<<TILESHIFT)+TILEGLOBAL-(pwallposinv<<10);
                                my_ray.xintercept=xintbuf;
// vbt new
                                my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                my_ray.tilehit=pwalltile;
                                HitHorizWall(pixx,texdelta,&my_ray);
                            }
                            else
                            {
                                xintbuf=my_ray.xintercept+((xstep*pwallposinv)>>6);
                                if((xintbuf>>16)!=(my_ray.xintercept>>16))
                                    goto passhoriz;

                                my_ray.yintercept=(my_ray.ytile<<TILESHIFT)-(pwallposinv<<10);
                                my_ray.xintercept=xintbuf;
// vbt new
                                my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                my_ray.tilehit=pwalltile;
                                HitHorizWall(pixx,texdelta,&my_ray);
                            }
                        }
                        else
                        {
                            int pwallposi = pwallpos;
                            if(pwalldir==di_west) pwallposi = 64-pwallpos;
                            if(pwalldir==di_east && (word)my_ray.xintercept<(pwallposi<<10)
                                    || pwalldir==di_west && (word)my_ray.xintercept>(pwallposi<<10))
                            {
                                if(((uint32_t)my_ray.xintercept>>16)==pwallx && my_ray.ytile==pwally)
                                {
                                    if(pwalldir==di_east && (int32_t)((word)my_ray.xintercept)+xstep<(pwallposi<<10)
                                            || pwalldir==di_west && (int32_t)((word)my_ray.xintercept)+xstep>(pwallposi<<10))
                                        goto passhoriz;

                                    if(pwalldir==di_east)
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)+(pwallposi<<10);
                                    else
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)-TILEGLOBAL+(pwallposi<<10);
                                    my_ray.yintercept=my_ray.yintercept-((ystep*(64-pwallpos))>>6);
// vbt new
                                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitVertWall(pixx,texdelta,&my_ray);
                                }
                                else
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.yintercept=my_ray.ytile<<TILESHIFT;
// vbt new
                                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitHorizWall(pixx,texdelta,&my_ray);
                                }
                            }
                            else
                            {
                                if(((uint32_t)my_ray.xintercept>>16)==pwallx && my_ray.ytile==pwally)
                                {
                                    texdelta = -(pwallposi<<10);
                                    my_ray.yintercept=my_ray.ytile<<TILESHIFT;
// vbt new
                                    my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitHorizWall(pixx,texdelta,&my_ray);
                                }
                                else
                                {
                                    if(pwalldir==di_east && (int32_t)((word)my_ray.xintercept)+xstep>(pwallposi<<10)
                                            || pwalldir==di_west && (int32_t)((word)my_ray.xintercept)+xstep<(pwallposi<<10))
                                        goto passhoriz;

                                    if(pwalldir==di_east)
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)-((64-pwallpos)<<10);
                                    else
                                        my_ray.xintercept=(my_ray.xintercept&0xffff0000)+((64-pwallpos)<<10);
                                    my_ray.yintercept=my_ray.yintercept-((ystep*pwallpos)>>6);
// vbt new
                                    my_ray.ytile = (short) (my_ray.yintercept >> TILESHIFT);
                                    my_ray.tilehit=pwalltile;
                                    HitVertWall(pixx,texdelta,&my_ray);
                                }
                            }
                        }
                    }
                    else
                    {
                        my_ray.yintercept=my_ray.ytile<<TILESHIFT;
// vbt new
                        my_ray.xtile = (short) (my_ray.xintercept >> TILESHIFT);
                        HitHorizWall(pixx,texdelta,&my_ray);
                    }
                }
                break;
            }
passhoriz:
            *((byte *)spotvis+yspot)=1;
            my_ray.ytile+=my_ray.ytilestep;
            my_ray.xintercept+=xstep;
            yspot=(word)((((uint32_t)my_ray.xintercept>>16)<<mapshift)+my_ray.ytile);
        }
        while(1);
    }
	
	ScalePost(my_ray.postx,my_ray.postsource);                   // no more optimization on last post
}
#endif


#endif


/*
====================
=
= WallRefresh
=
====================
*/

inline void WallRefresh (void)
{
//	slCashPurge();
#ifdef USE_SLAVE	
	slSlaveFunc(AsmRefreshSlave,(void*)NULL);
#endif	
	AsmRefresh();
}

inline void CalcViewVariables()
{
    int viewangle = player->angle;
#ifndef EMBEDDED
    midangle = viewangle*(FINEANGLES/ANGLES);
#endif	
    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedMul(focallength,viewcos);
    viewy = player->y + FixedMul(focallength,viewsin);

//    focaltx = (short)(viewx>>TILESHIFT);
//    focalty = (short)(viewy>>TILESHIFT);

//    viewtx = (short)(player->x >> TILESHIFT);
//    viewty = (short)(player->y >> TILESHIFT);
}

//==========================================================================

/*
========================
=
= ThreeDRefresh
=
========================
*/

void    ThreeDRefresh (void)
{
//
// clear out the traced array
//
    memset(spotvis,0,maparea);
    spotvis[player->tilex][player->tiley] = 1;       // Detect all sprites over player fix
#ifndef USE_SPRITES
    byte *vbuf = (byte *)screenBuffer->pixels;
    vbuf += screenofs;
#endif	
    CalcViewVariables();
//
// follow the walls from there to the right, drawing as we go
//
//    VGAClearScreen (); // vbt : maj du fond d'écran
//  on deplace dans le vblank
#if defined(USE_FEATUREFLAGS) && defined(USE_STARSKY)
    if(GetFeatureFlags() & FF_STARSKY)
        DrawStarSky(vbuf, vbufPitch);
#endif

    WallRefresh ();

#if defined(USE_FEATUREFLAGS) && defined(USE_PARALLAX)
    if(GetFeatureFlags() & FF_PARALLAXSKY)
        DrawParallax(vbuf, vbufPitch);
#endif
#if defined(USE_FEATUREFLAGS) && defined(USE_CLOUDSKY)
    if(GetFeatureFlags() & FF_CLOUDSKY)
        DrawClouds(vbuf, vbufPitch, min_wallheight);
#endif
#ifdef USE_FLOORCEILINGTEX
    DrawFloorAndCeiling(vbuf, vbufPitch, min_wallheight);
#endif

//---------------------------------------------------------------------------	
//
// draw all the scaled images
//
    DrawScaleds();                  // draw scaled stuff

#if defined(USE_FEATUREFLAGS) && defined(USE_RAIN)
    if(GetFeatureFlags() & FF_RAIN)
        DrawRain(vbuf, vbufPitch);
#endif
#if defined(USE_FEATUREFLAGS) && defined(USE_SNOW)
    if(GetFeatureFlags() & FF_SNOW)
        DrawSnow(vbuf, vbufPitch);
#endif
 // vbt : à remettre pour afficher l'arme
#ifdef USE_SPRITES
//    DrawPlayerWeapon ();    // draw player's hands
#else
    DrawPlayerWeapon (vbuf);    // draw player's hands
#endif

  /*  if(Keyboard[sc_Tab] && viewsize == 21 && gamestate.weapon != -1)
	{
        ShowActStatus();
	}*/

#ifdef USE_SPRITES
//		slDMACopy((void *)wall_buffer,(void *)(SpriteVRAM + cgaddress),(SATURN_WIDTH+64) * 64);
		slTransferEntry((void *)wall_buffer,(void *)(SpriteVRAM + cgaddress),(SATURN_WIDTH+64) * 64);

	//	extern int vbt;
		SPRITE *user_wall = user_walls;

		for(unsigned int pixx=0;pixx<viewwidth;pixx++)
		{
			slSetSprite(user_wall++, toFIXED(0+(SATURN_SORT_VALUE-user_wall->YC)));	// à remettre // murs
//			user_wall++;
		}
//		memcpy((void *)(SpriteVRAM + cgaddress),(void *)&wall_buffer[0],(SATURN_WIDTH+64) * 64);
		if(position_vram>0x38000)
		{
			memset(texture_list,0xFF,SPR_TOTAL);
//			position_vram = (SATURN_WIDTH+64)*32+static_items*0x800;
			position_vram = (SATURN_WIDTH+64)*32;
		}
//		slDMAWait();
#else
	VL_UnlockSurface(screenBuffer); // met à jour l'affichage de la barre de statut
	vbuf = NULL;
#endif

//
// show screen and time last cycle
//

    if (fizzlein)
    {
//		SDL_Rect destrect = { viewscreenx, viewscreeny, viewwidth, viewheight }; 
//		SDL_FillRect (screenBuffer, &destrect, 0);

//		VGAClearScreen(); // vbt : maj du fond d'écran
//		VL_BarScaledCoord (viewscreenx,viewscreeny,viewwidth,viewheight,0);

        FizzleFade(screenBuffer, screen, viewscreenx,viewscreeny,viewwidth,viewheight, 70, false);
//		VL_BarScaledCoord (viewscreenx,viewscreeny,viewwidth,viewheight,0);
        fizzlein = false;
		DrawStatusBar(); // vbt ajout
        lasttimecount = GetTimeCount();          // don't make a big tic count
    }
#ifndef USE_SPRITES		
	else
	{

        SDL_BlitSurface(screenBuffer, NULL, screen, NULL);
//        SDL_UpdateRect(screen, 0, 0, 0, 0);
    }
#endif
DrawPlayerWeapon ();    // draw player's hands
		slSynch(); // vbt ajout 26/05 à remettre // utile ingame !!	
}
