#include "wl_def.h"

#define LOADADDR 0x00242000
#define READSIZE 0xB0000
#define NB_WALL_HWRAM 44

int PMSpriteStart;
//int PMSoundStart;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
	int PMSoundStart;	
//#define READ16 1
    char fname[13] = "VSWAP.";
	Uint32 i=0,j=0;
	Uint8 *Chunks;
	long fileSize;
	
    strcat(fname,extension);
	
	Sint32 fileId;

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    int ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);

	Chunks=(Uint8*)LOADADDR;
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, fileSize);
	ChunksInFile=Chunks[0]|Chunks[1]<<8;
    PMSpriteStart = 0;
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[2]|Chunks[3]<<8;
    PMSoundStart = 0;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[4]|Chunks[5]<<8;

// vbt : on ne charge pas les sons !	
	ChunksInFile=PMSoundStart;

	uint8_t *wallData = (uint8_t *) malloc((NB_WALL_HWRAM+18)*0x1000);
	CHECKMALLOCRESULT(wallData);
	
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);	

    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);

    word *pageLengths = (word *) malloc((ChunksInFile-PMSpriteStart) * sizeof(word));
    CHECKMALLOCRESULT(pageLengths);

	for(i=0;i<ChunksInFile;i++)
	{
		pageOffsets[i]=Chunks[6]<<0|Chunks[7]<<8|Chunks[8]<<16|Chunks[9]<<24;
		Chunks+=4;
	}

	for(i=PMSpriteStart;i<ChunksInFile;i++)
	{
		pageLengths[i-PMSpriteStart]=Chunks[6]|Chunks[7]<<8;
		Chunks+=2;
	}
	
    //fread(pageLengths, sizeof(word), ChunksInFile, file);
    long pageDataSize = fileSize - pageOffsets[0];
    if(pageDataSize > (size_t) -1)
        Quit("The page file \"%s\" is too large!", fname);

    pageOffsets[ChunksInFile] = fileSize;

    uint32_t dataStart = pageOffsets[0];
	
    // Check that all pageOffsets are valid
/*    for(i = 0; i < ChunksInFile; i++)
    {
        if(!pageOffsets[i]) continue;   // sparse page
        if(pageOffsets[i] < dataStart || pageOffsets[i] >= (size_t) fileSize)
            Quit("Illegal page offset for page %i: %u (filesize: %u)",
                    i, pageOffsets[i], fileSize);
    }	
*/
	Chunks=(Uint8*)LOADADDR;	
    // Load pages and initialize PMPages pointers
	
	uint8_t *ptr = (uint8_t *)wallData;
	
    for(i = 0; i < NB_WALL_HWRAM; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

		memcpyl(ptr,&Chunks[pageOffsets[i]],0x1000);
		ptr+=0x1000;
	}			

    for(i = PMSpriteStart-8; i < PMSpriteStart; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

		memcpyl(ptr,&Chunks[pageOffsets[i]],0x1000);
		ptr+=0x1000;
	}

//    for(i = PMSpriteStart; i < ChunksInFile; i++)
	PM_DecodeSprites(PMSpriteStart,PMSpriteStart+10,ptr,pageOffsets,pageLengths,Chunks);
	ptr = (uint8_t *)0x00202000;
	ptr = PM_DecodeSprites(PMSpriteStart+10,PMSpriteStart+SPR_MUT_S_1,ptr,pageOffsets,pageLengths,Chunks);
	ptr = PM_DecodeSprites(PMSpriteStart+SPR_BOSS_W1,PMSpriteStart+SPR_BOSS_DIE3+1,ptr,pageOffsets,pageLengths,Chunks);
	ptr = PM_DecodeSprites(PMSpriteStart+SPR_BJ_W1,PMSpriteStart+SPR_BJ_JUMP4+1,ptr,pageOffsets,pageLengths,Chunks);
	ptr = PM_DecodeSprites(PMSpriteStart+SPR_KNIFEREADY,PMSpriteStart+SPR_NULLSPRITE,ptr,pageOffsets,pageLengths,Chunks);
/*
SPR_KNIFEREADY, SPR_NULLSPRITE
*/
    // last page points after page buffer
    PMPages[ChunksInFile] = ptr;
	extern Uint8 *lowram;
	lowram = ptr;
	
	int *val = (int *)ptr;
	
	slPrintHex((int)ChunksInFile-PMSpriteStart,slLocate(3,3));	
	slPrintHex((int)val,slLocate(3,4));

	free(pageLengths);
	pageLengths = NULL;	
    free(pageOffsets);
	pageOffsets = NULL;		
	Chunks = NULL;		
}	

uint8_t * PM_DecodeSprites(unsigned int start,unsigned int endi,uint8_t *ptr,uint32_t* pageOffsets,word *pageLengths,Uint8 *Chunks)
{
    for(int i = start; i < endi; i++)
    {
//		if(i>=SPR_MUT_S_1 && i<SPR_INKY_W2)
//			continue;				// vbt : skip mutant loading		
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i-PMSpriteStart];
        else size = pageOffsets[i + 1] - pageOffsets[i];

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memset(ptr,0x00,end);
		memcpy(ptr,&Chunks[pageOffsets[i]],size);

		t_compshape   *shape = (t_compshape   *)ptr;
		shape->leftpix=SWAP_BYTES_16(shape->leftpix);
		shape->rightpix=SWAP_BYTES_16(shape->rightpix);

		for (int x=0;x<(shape->rightpix-shape->leftpix)+1;x++ )
		{
			shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
		}
	
		byte bmpbuff[0x1000];
		byte *bmpptr;
		PICTURE pic_spr;
		unsigned short  *cmdptr, *sprdata;

		// set the texel index to the first texel
		unsigned char  *sprptr = (unsigned char  *)shape+(((((shape->rightpix)-(shape->leftpix))+1)*2)+4);
		// clear the buffers
		memset(bmpbuff,0,0x1000);
		// setup a pointer to the column offsets	
		cmdptr = shape->dataofs;

		for (int x = (shape->leftpix); x <= (shape->rightpix); x++)
		{
			sprdata = (unsigned short *)((unsigned char  *)shape+*cmdptr);
			bmpptr = (byte *)bmpbuff+x;
			
			while (SWAP_BYTES_16(*sprdata) != 0)
			{
				for (int y = SWAP_BYTES_16(sprdata[2])/2; y < SWAP_BYTES_16(*sprdata)/2; y++)
				{
					bmpptr[y<<6] = *sprptr++;
					if(bmpptr[y<<6]==0) bmpptr[y<<6]=0xa0;
				}
				sprdata += 3;
			}
			cmdptr++;
		}			

		memcpy((void *)ptr,bmpbuff,0x1000);
		ptr+=0x1000;
	}
	return ptr;
}

void PM_Shutdown()
{
    free(PMPages);
	PMPages = NULL;	
//    free(PMPageData);
//	PMPageData = NULL;	
}
