#include "wl_def.h"

#define LOADADDR 0x00242000
#define NB_WALL_HWRAM 50
//#define NB_WALL_HWRAM 39

int PMSpriteStart;
//int PMSoundStart;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;
uint8_t * PM_DecodeSprites2(unsigned int start,unsigned int endi,uint32_t* pageOffsets,word *pageLengths,Uint8 *Chunks);

void PM_Startup()
{
	unsigned int PMSoundStart;	
    char fname[13] = "VSWAP.";
	Uint32 i=0;
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
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[2]|Chunks[3]<<8;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[4]|Chunks[5]<<8;

// vbt : on ne charge pas les sons !	
	ChunksInFile=PMSoundStart;

	uint8_t *wallData = (uint8_t *) malloc((NB_WALL_HWRAM+8)*0x1000);
	CHECKMALLOCRESULT(wallData);
	
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);	

	uint32_t* pageOffsets = (uint32_t *)saturnChunk; 
	word *pageLengths = (word *)saturnChunk+(ChunksInFile + 1) * sizeof(int32_t);
 
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
    for(i = 0; i < ChunksInFile; i++)
    {
        if(!pageOffsets[i]) continue;   // sparse page
        if(pageOffsets[i] < dataStart || pageOffsets[i] >= (size_t) fileSize)
            Quit("Illegal page offset for page %i: %u (filesize: %u)",
                    i, pageOffsets[i], fileSize);
    }	

	Chunks=(Uint8*)LOADADDR;	
    // Load pages and initialize PMPages pointers
	
	uint8_t *ptr = (uint8_t *)wallData;
	
///------------------------ début murs
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
///------------------------ fin murs
//slPrint((char *)"PM_DecodeSprites2     ",slLocate(10,12));	
//	ptr = (uint8_t *)0x00202000;
//	ptr = PM_DecodeSprites2(PMSpriteStart,PMSpriteStart+SPR_NULLSPRITE,ptr,pageOffsets,pageLengths,Chunks);
	ptr = PM_DecodeSprites2(PMSpriteStart,PMSpriteStart+SPR_NULLSPRITE,pageOffsets,pageLengths,Chunks);
//slPrint((char *)"end PM_DecodeSprites2     ",slLocate(10,12));	
    // last page points after page buffer
    PMPages[ChunksInFile] = ptr;

	int *val = (int *)ptr;
	
//	slPrintHex((int)ChunksInFile-PMSpriteStart,slLocate(3,3));	
	slPrintHex((int)val,slLocate(3,4));

//	free(pageLengths);
	pageLengths = NULL;	
//    free(pageOffsets);
	pageOffsets = NULL;		
	Chunks = NULL;		
}	

uint8_t * PM_DecodeSprites2(unsigned int start,unsigned int endi,uint32_t* pageOffsets,word *pageLengths,Uint8 *Chunks)
{
	uint8_t *bmpbuff  = (uint8_t *)0x00202000;
	uint8_t *ptr      = (uint8_t *)0x00203000;
	
    for(unsigned int i = start; i < endi; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i-PMSpriteStart];
        else size = pageOffsets[i + 1] - pageOffsets[i];

        if(!size)
            continue;

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memcpy(ptr,&Chunks[pageOffsets[i]],size);
		memset(&ptr[size],0x00,end-size);

		t_compshape   *shape = (t_compshape   *)ptr;
		shape->leftpix=SWAP_BYTES_16(shape->leftpix);
		shape->rightpix=SWAP_BYTES_16(shape->rightpix);

		for (int x=0;x<(shape->rightpix-shape->leftpix)+1;x++ )
		{
			shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
		}

//		static byte bmpbuff[0x1000];
		byte *bmpptr,*sprdata8;
		unsigned short  *cmdptr;

		// set the texel index to the first texel
		unsigned char  *sprptr = (unsigned char  *)shape+(((((shape->rightpix)-(shape->leftpix))+1)*2)+4);
		// clear the buffers
		
		// setup a pointer to the column offsets	
		cmdptr = shape->dataofs;
		int count_00=63;
//		int count_01=0;

		for (unsigned int x = (shape->leftpix); x <= (shape->rightpix); x++)
		{
			sprdata8 = ((unsigned char  *)shape+*cmdptr);			
			
			while ((sprdata8[0]|sprdata8[1]<<8) != 0)
			{
				for (int y = (sprdata8[4]|sprdata8[5]<<8)/2; y < (sprdata8[0]|sprdata8[1]<<8)/2; y++)
				{
					unsigned int min_y=(sprdata8[4]|sprdata8[5]<<8)/2;
					if(min_y<count_00)
						count_00=min_y;
					
//					if(min_y>count_01)
//						count_01=min_y;					
				}
				sprdata8 += 6;
			}			
			cmdptr++;
		}
		memset(bmpbuff,0x00,(64-count_00)<<6);

		sprptr = (unsigned char  *)shape+(((((shape->rightpix)-(shape->leftpix))+1)*2)+4);

		cmdptr = shape->dataofs;		

//char toto[100];

		for (int x = (shape->leftpix); x <= (shape->rightpix); x++)
		{
			byte *sprdata8 = ((unsigned char  *)shape+*cmdptr);
			bmpptr = (byte *)bmpbuff+x;
/*
sprintf(toto,"l %04x r %04x min %02d       ",shape->leftpix,shape->rightpix,count_00);//,count_01);
//		if(i==SPR_STAT_0)
slPrint(toto,slLocate(5,6));
			
sprintf(toto,"%03d str %04x end %04x  ",i-start,(sprdata8[4]|sprdata8[5]<<8)/2,(sprdata8[0]|sprdata8[1]<<8)/2);
slPrint(toto,slLocate(5,9));

//sprintf(toto,"%03d spdt %04x r %04x  ",i-start,SWAP_BYTES_16(sprdata[2]));
sprintf(toto,"%03d dtofs %04x  sz %04x",i-start,*cmdptr,size);
slPrint(toto,slLocate(5,11));
*/
			while ((sprdata8[0]|sprdata8[1]<<8) != 0)
			{
				for (int y = (sprdata8[4]|sprdata8[5]<<8)/2; y < (sprdata8[0]|sprdata8[1]<<8)/2; y++)
//				for (int y = (sprdata8[4]|sprdata8[5]<<8)/2; y < count_01; y++)
				{
//					if(i-start!=186)
					{
						bmpptr[(y-count_00)<<6] = *sprptr++;
						if(bmpptr[(y-count_00)<<6]==0) bmpptr[(y-count_00)<<6]=0xa0;					
					}
				}
/*		sprintf(toto,"%03d str %04x end %04x  ",i-start,(sprdata8[4]|sprdata8[5]<<8)/2,(sprdata8[0]|sprdata8[1]<<8)/2);
slPrint(toto,slLocate(5,10));		*/
				sprdata8 += 6;
			}
			cmdptr++;
		}
/*		sprintf(toto,"%03d memcpy  ",i-start);
slPrint(toto,slLocate(5,11));*/

		memcpyl((void *)ptr,bmpbuff,(64-count_00)<<6);

/*		sprintf(toto,"%03d ptr++  ",i-start);
slPrint(toto,slLocate(5,12));*/
		
		ptr+=((64-count_00)<<6);

/*		sprintf(toto,"%03d end  ",i-start);
slPrint(toto,slLocate(5,12));*/
	}
	return ptr;
}
/*
void PM_Shutdown()
{
    free(PMPages);
	PMPages = NULL;	
//    free(PMPageData);
//	PMPageData = NULL;	
}
*/