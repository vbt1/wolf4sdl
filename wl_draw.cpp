// WL_DRAW.C
//#define USE_SPRITES 1
#include "wl_def.h"

//#define USE_SLAVE 1

#include "wl_cloudsky.h"
#include "wl_atmos.h"
#include "wl_shade.h"

void heapWalk();
#ifdef USE_SPRITES
static unsigned char wall_buffer[(SATURN_WIDTH+64)*64];

 SPRITE user_walls[MAX_WALLS];
extern 	TEXTURE tex_spr[SPR_NULLSPRITE+SATURN_WIDTH];
extern unsigned char texture_list[SPR_NULLSPRITE];
extern unsigned int position_vram;
#endif

//remplacer byte *tilemapaddr par ray->tilehit!!!!!! ... à réfléchir

typedef struct
{
//	byte *postsource;
	int id;
	int texture;
	unsigned tilehit;
	fixed xintercept,yintercept;
	int xtile,ytile;
	byte *tilemapaddr;
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

#define horizwall(x) ((x)-1)*2
#define vertwall(x) ((x)-1)*2+1

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

    if (gamestates[ob->state].rotate == 2)  // 2 rotation pain frame
        return 4*(angle/(ANGLES/2));        // pain with shooting frame bugfix

    return angle/(ANGLES/8);
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
static int CalcHeight(fixed xintercept, fixed yintercept)
{
	fixed gxt,gyt,nx,gx,gy;

	gx = xintercept - viewx;
	gxt = FixedMul(gx, viewcos);

	gy = yintercept - viewy;
	gyt = FixedMul(gy, viewsin);

	nx = gxt-gyt;

  //
  // calculate perspective ratio (heightnumerator/(nx>>8))
  //
	if (nx < MINDIST)
		nx = MINDIST;			/* don't let divide overflow */

	return heightnumerator/(nx>>8);
}
//==========================================================================

inline void loadActorTexture(int texture,unsigned int height,unsigned char *surfacePtr);

#ifdef USE_SPRITES
TEXTURE tex_spr[SPR_NULLSPRITE+SATURN_WIDTH];

inline void loadActorTexture(int texture,unsigned int height,unsigned char *surfacePtr)
{
	
//	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+texture];
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+texture];

//	slPrintHex(texture,slLocate(10,18));
	slPrintHex(position_vram+cgaddress,slLocate(10,20));
//if (position_vram<0x36000)
{
	/*
	Line  854: 	int *val = (int *)ptr;	
	Line  855: 	slPrintHex((int)val,slLocate(10,21));	
	*/
	int *val = (int *)surfacePtr;
//slPrintHex((int)val,slLocate(10,15));	
//	slPrintHex(height,slLocate(10,16));
//slPrintHex(texture,slLocate(10,17));	
	*txptr = TEXDEF(64, (height>>6), position_vram);
//	if(height<=64*64)
		memcpy((void *)(SpriteVRAM + ((txptr->CGadr) << 3)),(void *)surfacePtr,height);
//	memset((void *)(SpriteVRAM + ((txptr->CGadr) << 3)),texture,height);
	texture_list[texture]=1;//position_vram/height/2;
//	position_vram+=height/2;	
	position_vram+=height;	
//	position_vram+=0x800;
}
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
//extern int 					nb_unlock;
static int tutu=0;

void ScalePost(int postx, int texture, byte *postsource, byte *tilemapaddr, ray_struc *ray)
{
#ifdef USE_SPRITES	
//--------------------------------------------------------------------------------------------
	SPRITE *user_wall;
//	slDMACopy((void *)postsource, (void *)(wall_buffer + (postx<<6)), 64);  // vérifier si ca coule les perfs
//	slTransferEntry((void *)postsource,(void *)(wall_buffer + (postx<<6)),64);		
	memcpyl((void *)(wall_buffer + (postx<<6)),(void *)postsource,64);
	int wallheight = CalcHeight(ray->xintercept,ray->yintercept)/8;

	if(tilemapaddr!=ray->tilemapaddr || texture!=ray->texture)
	{
		ray->id++;
	//  a           b          c             d
	// top left, top right, bottom right, bottom left
		user_wall = (SPRITE *)user_walls+ray->id;
//		SPRITE *user_wall = user_walls+postx;
		user_wall->CTRL=FUNC_Texture | _ZmCC;
		user_wall->PMOD=CL256Bnk | ECdis | SPdis | 0x0800; // sans transparence

		user_wall->SRCA=cgaddress8|(postx*8);
		user_wall->COLR=256;
		user_wall->SIZE=0x801;
		
		user_wall->XC=postx-(viewwidth/2);
		user_wall->YC=wallheight;
		user_wall->YB=wallheight;
//		user_wall->XD=user_wall->XC;
		user_wall->YD=-wallheight;
		user_wall->XA=user_wall->XC;
		user_wall->YA=user_wall->YD;
		user_wall->XB=user_wall->XA;
		
		ray->tilemapaddr=tilemapaddr;
		ray->texture=texture;
	}
	else
	{
		user_wall = (SPRITE *)user_walls+ray->id;
		user_wall->SIZE++;
		user_wall->XC=postx-(viewwidth/2);
//		user_wall->XD=user_wall->XC;		
		user_wall->YC=wallheight;
		user_wall->YD=-wallheight;
	}
	user_wall->XD=user_wall->XC;

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

static const byte vgaCeiling[]=
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
	
//if(viewsize!=-1)
{
//	extern byte vgaCeiling[];
	extern SDL_Color curpal[256];
	unsigned char y;

	SDL_Color *sdlCeilingColor = (SDL_Color *)&curpal[vgaCeiling[gamestate.episode*10+mapon]];
	Uint16 ceilingColor = RGB((*sdlCeilingColor).r>>3,(*sdlCeilingColor).g>>3,(*sdlCeilingColor).b>>3);

	SDL_Color *sdlFloorColor = (SDL_Color *)&curpal[25];
	Uint16 floorColor = RGB((*sdlFloorColor).r>>3,(*sdlFloorColor).g>>3,(*sdlFloorColor).b>>3);
	
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
	Uint16	*Line_Color_Pal0	=(Uint16 *)LINE_COLOR_TABLE+start;

	for(y = start; y < end; y++)
		*Line_Color_Pal0++ = ceilingColor;
	
	for(; y <= viewheight+viewscreeny; y++)
		*Line_Color_Pal0++ = floorColor;

//	start=RGB(0,0,0);
	start = RGB((*curpal).r>>3,(*curpal).g>>3,(*curpal).b>>3);

	for(; y < 240; y++)
		*Line_Color_Pal0++ = start;
	
	slBackColTable((void *)LINE_COLOR_TABLE);
//	viewsize=-1;
}	
#endif
}

//==========================================================================
inline void ScaleShapeDemo (int xcenter, int shapenum, unsigned width)
{
	unsigned char *surfacePtr = (unsigned char*)PM_GetSprite(shapenum); // + ((0) * source->pitch) + 0;
	unsigned char *nextSurfacePtr = (unsigned char*)PM_GetSprite(shapenum+1);
	unsigned int height=(nextSurfacePtr-surfacePtr)>>6;
	
	if(!texture_list[shapenum])
	{
		loadActorTexture(shapenum,height<<6,surfacePtr);
	}
//--------------------------------------------------------------------------------------------
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+shapenum]; 
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis;// | ECenb | SPdis;  // pas besoin pour les sprites
    user_sprite.SRCA=((txptr->CGadr));
    user_sprite.COLR=256;

    user_sprite.SIZE=0x800+height;
	user_sprite.XA=(xcenter-centerx);
	user_sprite.YA=width*(32-height/2)/64;
	user_sprite.XB=width;
	user_sprite.YB=width*height/64;
    user_sprite.GRDA=0;
	slSetSprite(&user_sprite, toFIXED(10));	// à remettre // ennemis et objets
//--------------------------------------------------------------------------------------------	
}
inline void ScaleShape (int xcenter, int shapenum, unsigned width)
{
    unsigned scalel,pixwidth;

#ifdef USE_SHADING
    byte *curshades;
    if(flags & FL_FULLBRIGHT)
        curshades = shadetable[0];
    else
        curshades = shadetable[GetShade(width)];
#endif
    scalel=width/8;                 // low three bits are fractional
    if(!scalel) return;   // too close or far away
    pixwidth=scalel*SPRITESCALEFACTOR;

//shapenum=SPR_DEMO+45;
//shapenum=SPR_STAT_49-1;
#ifdef USE_SPRITES
	unsigned char *surfacePtr = (unsigned char*)PM_GetSprite(shapenum); // + ((0) * source->pitch) + 0;
	unsigned char *nextSurfacePtr = (unsigned char*)PM_GetSprite(shapenum+1);
	unsigned int height=(nextSurfacePtr-surfacePtr)>>6;
//	unsigned int height=64;

	int *val = (int *)surfacePtr;
slPrintHex((int)val,slLocate(10,15));	
	slPrintHex(height,slLocate(10,16));
slPrintHex(shapenum,slLocate(10,17));
	
	if(!texture_list[shapenum])
	{
		loadActorTexture(shapenum,height<<6,surfacePtr);
	}
//--------------------------------------------------------------------------------------------
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1+shapenum]; 
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis;// | ECenb | SPdis;  // pas besoin pour les sprites
    user_sprite.SRCA=((txptr->CGadr));
    user_sprite.COLR=256;

    user_sprite.SIZE=0x800+height;
	user_sprite.XA=(xcenter-centerx);
	user_sprite.YA=pixwidth*(32-height/2)/64;
	user_sprite.XB=pixwidth;
	user_sprite.YB=pixwidth*height/64;
    user_sprite.GRDA=0;
	slSetSprite(&user_sprite, toFIXED(0+(SATURN_SORT_VALUE-pixwidth/2)));	// à remettre // ennemis et objets
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
    upperedge=viewwidth/2-scale;

    shape = (t_compshape *) PM_GetSprite(shapenum);
	
    cmdptr=(word *) shape->dataofs;
	
    for(i=shape->leftpix,pixcnt=i*pixwidth,rpix=(pixcnt>>6)+actx;i<=shape->rightpix;i++,cmdptr++)
    {
        lpix=rpix;
        if(lpix>=viewwidth) break;
        pixcnt+=pixwidth;
        rpix=(pixcnt>>6)+actx;
        if(lpix!=rpix && rpix>0)
        {
            if(lpix<0) lpix=0;
            if(rpix>viewwidth) rpix=viewwidth,i=shape->rightpix+1;
            cline=(byte *)shape + *cmdptr;
            while(lpix<rpix)
            {
                if(wallwidth[lpix]<=(int)width)
                {
                    line=cline;
                    while((endy = READWORD(line)) != 0)
                    {
                        endy >>= 1;
                        newstart = READWORD(line);
                        starty = READWORD(line) >> 1;
//                        j=starty;
                        ycnt=j*pixwidth;
                        screndy=(ycnt>>6)+upperedge;
                        if(screndy<0) vmem=vbuf+lpix;
                        else vmem=vbuf+screndy*curPitch+lpix;
                        for(;j<endy;j++)
                        {
                            scrstarty=screndy;
                            ycnt+=pixwidth;
                            screndy=(ycnt>>6)+upperedge;
                            if(scrstarty!=screndy && screndy>0)
                            {
#ifdef USE_SHADING
                                col=curshades[((byte *)shape)[newstart+j]];
#else
                                col=((byte *)shape)[newstart+j];
#endif
                                if(scrstarty<0) scrstarty=0;
                                if(screndy>viewwidth) screndy=viewwidth,j=endy;

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
void SimpleScaleShape (int xcenter, int shapenum, unsigned pixwidth)
#else
void SimpleScaleShape (byte *vbuf, int xcenter, int shapenum, unsigned height,unsigned vbufPitch)
#endif
{
#ifdef USE_SPRITES	
//slPrintHex(shapenum,slLocate(10,4));
/*
	TEXTURE *txptr = &tex_spr[SATURN_WIDTH+2+shapenum]; 
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis;// | ECenb | SPdis;  // pas besoin pour les sprites
    user_sprite.SRCA=((txptr->CGadr));
*/

	unsigned char *surfacePtr = (unsigned char*)PM_GetSprite(shapenum);
	unsigned char *nextSurfacePtr = (unsigned char*)PM_GetSprite(shapenum+1);
	int height=(nextSurfacePtr-surfacePtr);
//slPrintHex(height,slLocate(10,5));
		
//TEXTURE *txptr = &tex_spr[SATURN_WIDTH+1];
		
	if (old_texture!=shapenum)
	{
//		*txptr = TEXDEF(64, (height>>6), (SATURN_WIDTH+1)*64);
		memcpy((void *)(wall_buffer + ((SATURN_WIDTH+1)<<6)),(void *)surfacePtr,height);
		old_texture=shapenum;
	}
	height>>=6;
//--------------------------------------------------------------------------------------------
// correct on touche pas		
    SPRITE user_sprite;
    user_sprite.CTRL = FUNC_Sprite | _ZmCC;
    user_sprite.PMOD=CL256Bnk| ECdis | 0x0800;// | ECenb | SPdis;  // pas besoin pour les sprites
//    user_sprite.SRCA==((txptr->CGadr)); //cgaddress8|(SATURN_WIDTH*8);
    user_sprite.SRCA=cgaddress8+0xB08;//cgaddress8|((SATURN_WIDTH+1)*8);
    user_sprite.COLR=256;
    user_sprite.SIZE=0x800+height;
	user_sprite.XA=(xcenter-centerx);
	user_sprite.YA=pixwidth*(32-height/2)/64;
	user_sprite.XB=pixwidth;
	user_sprite.YB=pixwidth*height/64;
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
#ifdef EMBEDDED222
	for (obj = player->next;obj;obj=obj->next)
	{
		uint32_t spotmask;
		uint32_t spota;
		uint32_t spotb;
		uint32_t spotc;
		if (!(visptr->shapenum = gamestates[obj->state].shapenum))
			continue;  // no shape

		spotloc = (obj->tilex << 6) + obj->tiley;
		tilespot = &tilemap[0][0] + spotloc;
		spota = spotvis[obj->tilex-1];
		spotb = spotvis[obj->tilex];
		spotc = spotvis[obj->tilex+1];
		spotmask = 1ull << obj->tiley;

		//
		// could be in any of the nine surrounding tiles
		//
		if ((spotb & spotmask)
		|| ((spotb & (spotmask >> 1)) && !*(tilespot-1))
		|| ((spotb & (spotmask << 1)) && !*(tilespot+1))
		|| ((spota & (spotmask >> 1)) && !*(tilespot-65))
		|| ((spota & spotmask) && !*(tilespot-64))
		|| ((spota & (spotmask << 1)) && !*(tilespot-63))
		|| ((spotc & (spotmask >> 1)) && !*(tilespot+63))
		|| ((spotc & spotmask) && !*(tilespot+64))
		|| ((spotc & (spotmask << 1)) && !*(tilespot+65)))
		{
			obj->active = ac_yes;
			TransformActor(obj);
			if (!obj->viewheight)
				continue;						// too close or far away

			visptr->viewx = obj->viewx;
			visptr->viewheight = obj->viewheight;
			if (visptr->shapenum == -1)
				visptr->shapenum = obj->temp1;	// special shape

			if (gamestates[obj->state].rotate)
				visptr->shapenum += CalcRotate(obj);

			if (visptr < &vislist[MAXVISABLE-1])	/* don't let it overflow */
				visptr++;
			obj->flags |= FL_VISABLE;
		} else
			obj->flags &= ~FL_VISABLE;
	}
#else
    for (obj = player->next;obj;obj=obj->next)
    {
        if (!(visptr->shapenum = gamestates[obj->state].shapenum))
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

            if (gamestates[obj->state].rotate)
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
#endif
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

static short weaponscale[NUMWEAPONS] = {SPR_KNIFEREADY, SPR_PISTOLREADY,
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

#ifdef USE_SPRITES
		if(viewsize != 20)
			SimpleScaleShape(viewwidth/2,shapenum,viewheight+1);
		else
			SimpleScaleShape(viewwidth/2,shapenum,viewheight-41);		
#else
        SimpleScaleShape(vbuf,viewwidth/2,shapenum,viewheight+1,curPitch);
#endif
    }

    if (demoplayback)
#ifdef USE_SPRITES
		ScaleShapeDemo(viewwidth/2, SPR_DEMO, viewheight+1);
#else
        SimpleScaleShape(vbuf,viewwidth/2,SPR_DEMO,viewheight+1,curPitch);
#endif		

#ifndef SPEAR
    if (gamestate.victoryflag)
    {
#ifndef APOGEE_1_0
        if ((player->state == s_deathcam) && (GetTimeCount() & 32))
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

//==========================================================================

#define DEG90   900
#define DEG180  1800
#define DEG270  2700
#define DEG360  3600

//static inline void HitHorizDoorNew(int postx, byte *tilemapaddr, ray_struc *ray)
static inline void HitHorizDoorNew(int postx, byte *tilemapaddr, ray_struc *ray)
{
	unsigned texture, doorpage = 0, doornum;

	doornum = ray->tilehit&0x7f;
	texture = ((ray->xintercept-doorposition[doornum]) >> 4) & 0xfc0;

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

	byte *postsource = PM_GetTexture(doorpage) + texture;
	ScalePost(postx, doorpage, postsource, tilemapaddr, ray);
}

static inline void HitVertDoorNew(int postx, byte *tilemapaddr, ray_struc *ray)
{
	unsigned texture, doorpage = 0, doornum;

	doornum = ray->tilehit&0x7f;
	texture = ((ray->yintercept-doorposition[doornum]) >> 4) & 0xfc0;

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

	byte *postsource = PM_GetTexture(doorpage+1) + texture;
	ScalePost(postx, doorpage+1, postsource, tilemapaddr, ray);
}

static void inline HitVertWallNew(int postx, short xtilestep, byte *tilemapaddr, ray_struc *ray)
{
	int wallpic;
	unsigned texture;

	texture = (ray->yintercept>>4)&0xfc0;
	
	if (xtilestep == -1) {
		texture = 0xfc0-texture;
		ray->xintercept += TILEGLOBAL;
	}
	
	if (ray->tilehit & 0x40) { // check for adjacent doors
		ray->ytile = ray->yintercept>>TILESHIFT;
		if (tilemap[ray->xtile-xtilestep][ray->ytile] & 0x80)
			wallpic = DOORWALL+3;
		else
			wallpic = vertwall(ray->tilehit & ~0x40);
	} else
		wallpic = vertwall(ray->tilehit);
	
	byte *postsource = PM_GetTexture(wallpic) + texture;
	ScalePost(postx, wallpic, postsource, tilemapaddr, ray);
}

static inline void HitHorizWallNew(int postx, short ytilestep, byte *tilemapaddr,ray_struc *ray)
{
	int wallpic;
	unsigned texture;

	texture = (ray->xintercept >> 4) & 0xfc0;
	
	if (ytilestep == -1)
		ray->yintercept += TILEGLOBAL;
	else
		texture = 0xfc0 - texture;
		
	if (ray->tilehit & 0x40) { // check for adjacent doors
		ray->xtile = ray->xintercept>>TILESHIFT;
		if (tilemap[ray->xtile][ray->ytile-ytilestep] & 0x80)
			wallpic = DOORWALL+2;
		else
			wallpic = horizwall(ray->tilehit & ~0x40);
	} else
		wallpic = horizwall(ray->tilehit);

	byte *postsource = PM_GetTexture(wallpic) + texture;
	ScalePost(postx, wallpic, postsource, tilemapaddr, ray);
}

static inline void HitHorizPWall(int postx, short ytilestep, byte *tilemapaddr, ray_struc *ray)
{
	int wallpic;
	unsigned texture, offset;
	
	texture = (ray->xintercept >> 4) & 0xfc0;
	
	offset = pwallpos << 10;
	
	if (ytilestep == -1)
		ray->yintercept += TILEGLOBAL-offset;
	else {
		texture = 0xfc0-texture;
		ray->yintercept += offset;
	}

	wallpic = horizwall(ray->tilehit&63);
	byte *postsource = PM_GetTexture(wallpic) + texture;
	ScalePost(postx, wallpic, postsource, tilemapaddr, ray);
}

static inline void HitVertPWall(int postx, short xtilestep, byte *tilemapaddr, ray_struc *ray)
{
	int wallpic;
	unsigned texture, offset;
	
	texture = (ray->yintercept >> 4) & 0xfc0;
	offset = pwallpos << 10;
	
	if (xtilestep == -1) {
		ray->xintercept += TILEGLOBAL-offset;
		texture = 0xfc0-texture;
	} else
		ray->xintercept += offset;

	wallpic = vertwall(ray->tilehit&63);
	byte *postsource = PM_GetTexture(wallpic) + texture;
	ScalePost(postx, wallpic, postsource, tilemapaddr, ray);
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

static unsigned int AsmRefresh(int midangle)
{
	static	ray_struc my_ray;	
	unsigned xpartialup, xpartialdown, ypartialup, ypartialdown;
	unsigned xpartial, ypartial;
	int doorhit;
	int angle;    /* ray angle through postx */
//	int midangle;
	int focaltx, focalty;
	int xstep, ystep;
	short xtilestep, ytilestep;
	byte *tilemapaddr;
	
//	midangle = player->angle*(FINEANGLES/ANGLES);
	xpartialdown = (viewx&(TILEGLOBAL-1));
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = (viewy&(TILEGLOBAL-1));
	ypartialup = TILEGLOBAL-ypartialdown;

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;
	my_ray.texture = my_ray.id = -1;
	my_ray.tilehit = NULL;

#ifdef USE_SLAVE
for (int postx = 0; postx < (viewwidth/2)+28; postx++) 
//for (int postx = 0; postx < (viewwidth); postx++) 
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
		xtilestep = 1;
		ytilestep = -1;
		xstep = finetangent[DEG90-1-angle];
		ystep = -finetangent[angle];
		xpartial = xpartialup;
		ypartial = ypartialdown;
	} else if (angle < DEG180) {
		/* 90-179 degree arc */
		xtilestep = -1;
		ytilestep = -1;
		xstep = -finetangent[angle-DEG90];
		ystep = -finetangent[DEG180-1-angle];
		xpartial = xpartialdown;
		ypartial = ypartialdown;
	} else if (angle < DEG270) {
		/* 180-269 degree arc */
		xtilestep = -1;
		ytilestep = 1;
		xstep = -finetangent[DEG270-1-angle];
		ystep = finetangent[angle-DEG180];
		xpartial = xpartialdown;
		ypartial = ypartialup;
	} else if (angle < DEG360) {
		/* 270-359 degree arc */
	entry360:
		xtilestep = 1;
		ytilestep = 1;
		xstep = finetangent[angle-DEG270];
		ystep = finetangent[DEG360-1-angle];
		xpartial = xpartialup;
		ypartial = ypartialup;
	} else {
		angle -= FINEANGLES;
		goto entry90;
	}
	
	my_ray.yintercept = viewy + FixedMul(xpartial, ystep); // + xtilestep;
	my_ray.xtile = focaltx + xtilestep;

	my_ray.xintercept = viewx + FixedMul(ypartial, xstep); // + ytilestep;
	my_ray.ytile = focalty + ytilestep;

/* CORE LOOP */

#define TILE(n) ((n)>>TILESHIFT)

	/* check intersections with vertical walls */
vertcheck:
// int ytilestep, int intercept, int tile)
	if (!samey(ytilestep, my_ray.yintercept, my_ray.ytile))
//	{
//		my_ray.samey=0;
		goto horizentry;
//	}
//	my_ray.samey=1;
		
vertentry:
	tilemapaddr = &tilemap[my_ray.xtile][TILE(my_ray.yintercept)];
	my_ray.tilehit = *tilemapaddr;
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
				HitVertPWall(postx, xtilestep, tilemapaddr, &my_ray);
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
				HitVertDoorNew(postx, tilemapaddr, &my_ray);
			}
		} else {
			my_ray.xintercept = my_ray.xtile << TILESHIFT;
			HitVertWallNew(postx, xtilestep, tilemapaddr, &my_ray);
		}
		continue;
	}
passvert:
//	*((byte *)spotvis+xspot)=1;
//	setspotvis(my_ray.xtile,TILE(my_ray.yintercept));
	spotvis[my_ray.xtile][TILE(my_ray.yintercept)] = 1;
	my_ray.xtile += xtilestep;
	my_ray.yintercept += ystep;
	goto vertcheck;
	
horizcheck:
	/* check intersections with horizontal walls */
	
	if (!samex(xtilestep, my_ray.xintercept, my_ray.xtile))
		goto vertentry;

horizentry:
	tilemapaddr = &tilemap[TILE(my_ray.xintercept)][my_ray.ytile];
	my_ray.tilehit = *tilemapaddr;
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
				HitHorizPWall(postx, ytilestep, tilemapaddr, &my_ray);
			} else {
				doorhit = my_ray.xintercept + xstep / 2;
				
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = (my_ray.ytile << TILESHIFT) + TILEGLOBAL/2;
				HitHorizDoorNew(postx, tilemapaddr, &my_ray);
			}
		} else {
			my_ray.yintercept = my_ray.ytile << TILESHIFT;
			HitHorizWallNew(postx, ytilestep, tilemapaddr, &my_ray);
		}
		continue;
	}
passhoriz:
//    *((byte *)spotvis+yspot)=1;
//	setspotvis(TILE(my_ray.xintercept), my_ray.ytile);
	spotvis[TILE(my_ray.xintercept)][my_ray.ytile] = 1;
	my_ray.ytile += ytilestep;
	my_ray.xintercept += xstep;
	goto horizcheck;
}
	return my_ray.id;
}

#ifdef USE_SLAVE

static void AsmRefreshSlave(int *midangle)
{
	static	ray_struc my_ray;	
	unsigned xpartialup, xpartialdown, ypartialup, ypartialdown;
	unsigned xpartial, ypartial;
	int doorhit;
	int angle;    /* ray angle through postx */
//	int midangle;
	int focaltx, focalty;
	int xstep, ystep;
	short xtilestep, ytilestep;
	byte *tilemapaddr;
	
//	midangle = player->angle*(FINEANGLES/ANGLES);
	xpartialdown = (viewx&(TILEGLOBAL-1));
	xpartialup = TILEGLOBAL-xpartialdown;
	ypartialdown = (viewy&(TILEGLOBAL-1));
	ypartialup = TILEGLOBAL-ypartialdown;

	focaltx = viewx>>TILESHIFT;
	focalty = viewy>>TILESHIFT;
	my_ray.id = 59;
	my_ray.texture = -1;
	
for(int postx=(viewwidth>>1)+28;postx<viewwidth;postx++)
{
	angle = *midangle + pixelangle[postx];

	if (angle < 0) {
		/* -90 - -1 degree arc */
		angle += FINEANGLES;
		goto entry360;
	} else if (angle < DEG90) {
		/* 0-89 degree arc */
	entry90:
		xtilestep = 1;
		ytilestep = -1;
		xstep = finetangent[DEG90-1-angle];
		ystep = -finetangent[angle];
		xpartial = xpartialup;
		ypartial = ypartialdown;
	} else if (angle < DEG180) {
		/* 90-179 degree arc */
		xtilestep = -1;
		ytilestep = -1;
		xstep = -finetangent[angle-DEG90];
		ystep = -finetangent[DEG180-1-angle];
		xpartial = xpartialdown;
		ypartial = ypartialdown;
	} else if (angle < DEG270) {
		/* 180-269 degree arc */
		xtilestep = -1;
		ytilestep = 1;
		xstep = -finetangent[DEG270-1-angle];
		ystep = finetangent[angle-DEG180];
		xpartial = xpartialdown;
		ypartial = ypartialup;
	} else if (angle < DEG360) {
		/* 270-359 degree arc */
	entry360:
		xtilestep = 1;
		ytilestep = 1;
		xstep = finetangent[angle-DEG270];
		ystep = finetangent[DEG360-1-angle];
		xpartial = xpartialup;
		ypartial = ypartialup;
	} else {
		angle -= FINEANGLES;
		goto entry90;
	}
	
	my_ray.yintercept = viewy + FixedMul(xpartial, ystep); // + xtilestep;
	my_ray.xtile = focaltx + xtilestep;

	my_ray.xintercept = viewx + FixedMul(ypartial, xstep); // + ytilestep;
	my_ray.ytile = focalty + ytilestep;

/* CORE LOOP */

#define TILE(n) ((n)>>TILESHIFT)

	/* check intersections with vertical walls */
vertcheck:
	if (!samey(ytilestep, my_ray.yintercept, my_ray.ytile))
		goto horizentry;

vertentry:
	tilemapaddr = &tilemap[my_ray.xtile][TILE(my_ray.yintercept)];
	my_ray.tilehit = *tilemapaddr;
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
				HitVertPWall(postx, xtilestep, tilemapaddr, &my_ray);
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
				HitVertDoorNew(postx, tilemapaddr, &my_ray);
			}
		} else {
			my_ray.xintercept = my_ray.xtile << TILESHIFT;
			HitVertWallNew(postx, xtilestep, tilemapaddr, &my_ray);
		}
		continue;
	}
passvert:
//	setspotvis(my_ray.xtile,TILE(my_ray.yintercept));
	spotvis[my_ray.xtile][TILE(my_ray.yintercept)] = 1;
	my_ray.xtile += xtilestep;
	my_ray.yintercept += ystep;
	goto vertcheck;
	
horizcheck:
	/* check intersections with horizontal walls */
	
	if (!samex(xtilestep, my_ray.xintercept, my_ray.xtile))
		goto vertentry;

horizentry:
	tilemapaddr = &tilemap[TILE(my_ray.xintercept)][my_ray.ytile];
	my_ray.tilehit = *tilemapaddr;
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
				HitHorizPWall(postx, ytilestep, tilemapaddr, &my_ray);
			} else {
				doorhit = my_ray.xintercept + xstep / 2;
				
				if (TILE(doorhit) != TILE(my_ray.xintercept))
					goto passhoriz;
				
				/* check door position */
				if ((doorhit&0xFFFF) < doorposition[my_ray.tilehit&0x7f])
					goto passhoriz;
				
				my_ray.xintercept = doorhit;
				my_ray.yintercept = (my_ray.ytile << TILESHIFT) + TILEGLOBAL/2;
				HitHorizDoorNew(postx, tilemapaddr, &my_ray);
			}
		} else {
			my_ray.yintercept = my_ray.ytile << TILESHIFT;
			HitHorizWallNew(postx, ytilestep, tilemapaddr, &my_ray);
		}
		continue;
	}
passhoriz:
	spotvis[TILE(my_ray.xintercept)][my_ray.ytile] = 1;
//	setspotvis(TILE(my_ray.xintercept), my_ray.ytile);
	my_ray.ytile += ytilestep;
	my_ray.xintercept += xstep;
	goto horizcheck;
}
	tutu=my_ray.id;
}


#endif



/*
====================
=
= WallRefresh
=
====================
*/

inline int WallRefresh (void)
{
//	slCashPurge();
    int viewangle = player->angle;
	int midangle = viewangle*(FINEANGLES/ANGLES);
    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    viewx = player->x - FixedMul(focallength, viewcos);
    viewy = player->y + FixedMul(focallength, viewsin);

#ifdef USE_SLAVE	
	extern void AsmRefreshSlave(int *midangle);


	slSlaveFunc(AsmRefreshSlave,(void *)&midangle);
//	AsmRefreshSlave(&midangle);
#endif	
	return AsmRefresh(midangle);
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
    memset(spotvis,0,sizeof(spotvis));
    spotvis[player->tilex][player->tiley] = 1;       // Detect all sprites over player fix
#ifndef USE_SPRITES
    byte *vbuf = (byte *)screenBuffer->pixels;
    vbuf += screenofs;
#endif
//
// follow the walls from there to the right, drawing as we go
//
//    VGAClearScreen (); // vbt : maj du fond d'écran
//  on deplace dans le vblank
#if defined(USE_FEATUREFLAGS) && defined(USE_STARSKY)
    if(GetFeatureFlags() & FF_STARSKY)
        DrawStarSky(vbuf, vbufPitch);
#endif

    unsigned int nb = WallRefresh ();

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
    DrawPlayerWeapon ();    // draw player's hands
#endif

  /*  if(Keyboard[sc_Tab] && viewsize == 21 && gamestate.weapon != -1)
	{
        ShowActStatus();
	}*/

#ifdef USE_SPRITES
//		slDMACopy((void *)wall_buffer,(void *)(SpriteVRAM + cgaddress),(SATURN_WIDTH+64) * 64);
		slTransferEntry((void *)wall_buffer,(void *)(SpriteVRAM + cgaddress),(SATURN_WIDTH+64) * 64);

		SPRITE *user_wall = user_walls;

		for(unsigned int pixx=0;pixx<=nb;pixx++)
		{
			int depth = (user_wall->YB+user_wall->YC)/2;
			slSetSprite(user_wall++, toFIXED(SATURN_SORT_VALUE-depth));	// à remettre // murs
		}
		
		if(position_vram>0x77000)
		{
			memset(texture_list,0x00,SPR_NULLSPRITE);
			position_vram = (SATURN_WIDTH+64)*64;
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

		user_wall = (SPRITE *)user_walls+(MAX_WALLS/2);
		
//		while(tutu==0);
		
		
		nb=tutu-(MAX_WALLS/2);

		for(unsigned int pixx=0;pixx<=nb;pixx++)
		{
			int depth = (user_wall->YB+user_wall->YC)/2;			
			slSetSprite(user_wall, toFIXED(SATURN_SORT_VALUE-depth));	// à remettre // murs
			user_wall++;
		}

		
		DrawPlayerWeapon ();    // draw player's hands
		
		slSynch(); // vbt ajout 26/05 à remettre // utile ingame !!	
}
