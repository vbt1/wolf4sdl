#include "wl_def.h"

int PMSpriteStart;
//int PMSoundStart;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;
uint8_t *PM_DecodeSprites2(unsigned int i,/*unsigned int endi,*/uint32_t* pageOffsets,word *pageLengths,uint8_t *ptr, Sint32 fileId);


void readChunks(Sint32 fileId, uint32_t size, uint32_t *pageOffsets, Uint8 *Chunks, uint8_t *ptr)
{
	uint16_t delta  = (uint16_t)(*pageOffsets/2048);
	uint32_t delta2 = (*pageOffsets - delta*2048);

	GFS_Load(fileId, delta, (void *)Chunks, size+delta2);
	
	memcpy(ptr,&Chunks[delta2],size);	
}

void PM_Startup()
{
	
	unsigned int PMSoundStart;	
    char fname[13] = "VSWAP.";
	Uint32 i=0;
	Uint8 *Chunks;
//	long fileSize;
	
    strcat(fname,extension);
	
	Sint32 fileId;

	fileId = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(fileId);

    int ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);

	Chunks=(Uint8*)saturnChunk;
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, 0x80);
	ChunksInFile=Chunks[0]|Chunks[1]<<8;
//	slPrintHex(ChunksInFile,slLocate(10,18));	
	
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[2]|Chunks[3]<<8;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[4]|Chunks[5]<<8;

// vbt : on ne charge pas les sons !	
	ChunksInFile=PMSoundStart;

    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
//    CHECKMALLOCRESULT(PMPages);	
/*
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
            Quit("Illegal page offset for page %i: %u (filesize: %u)", i, pageOffsets[i], fileSize);
    }	
	pageLengths = NULL;	
	pageOffsets = NULL;		
	Chunks = NULL;
*/	
}	

uint8_t *PM_DecodeSprites2(unsigned int i,uint32_t* pageOffsets,word *pageLengths,uint8_t *ptr, Sint32 fileId)
{
	uint8_t *Chunks   = (uint8_t *)saturnChunk+0x9000;
	uint8_t *bmpbuff  = (uint8_t *)saturnChunk+0xC000;
    uint32_t size;
		
	PMPages[i] = ptr;

	if(!pageOffsets[i])
		return ptr;               // sparse page

	// Use specified page length, when next page is sparse page.
	// Otherwise, calculate size from the offset difference between this and the next page.

	if(!pageOffsets[i + 1]) 
	{	
		size = pageLengths[i-PMSpriteStart];
	}
	else 
	{
		size = pageOffsets[i + 1] - pageOffsets[i];
	}
	
	if(!size)
		return ptr;

	if (size % 4 != 0)
	{
		int end = ((size + (4 - 1)) & -4);
		memset(&ptr[size],0x00,end-size);
	}
	readChunks(fileId, size, &pageOffsets[i], Chunks, ptr);
	t_compshape   *shape = (t_compshape   *)ptr;
	shape->leftpix =SWAP_BYTES_16(shape->leftpix);
	shape->rightpix=SWAP_BYTES_16(shape->rightpix);

	byte *bmpptr,*sprdata8;
	unsigned short  *cmdptr;
	
	// setup a pointer to the column offsets	
	cmdptr = shape->dataofs;
	int count_00=63;

	for (int x=0;x<=(shape->rightpix-shape->leftpix);x++ )	
	{
		shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
		sprdata8 = ((unsigned char  *)shape+*cmdptr);			
		
		while ((sprdata8[0]|sprdata8[1]<<8) != 0)
		{
			for (int y = (sprdata8[4]|sprdata8[5]<<8)/2; y < (sprdata8[0]|sprdata8[1]<<8)/2; y++)
			{
				unsigned int min_y=(sprdata8[4]|sprdata8[5]<<8)/2;
				if(min_y<count_00)
					count_00=min_y;
			}
			sprdata8 += 6;
		}			
		cmdptr++;
	}
	memset(bmpbuff,0x00,(64-count_00)<<6);

	unsigned char *sprptr = (unsigned char  *)shape+(((((shape->rightpix)-(shape->leftpix))+1)*2)+4);

	cmdptr = shape->dataofs;		

	for (int x = (shape->leftpix); x <= (shape->rightpix); x++)
	{
		sprdata8 = ((unsigned char  *)shape+*cmdptr);

		while ((sprdata8[0]|sprdata8[1]<<8) != 0)
		{
			int y = ((sprdata8[4]|sprdata8[5]<<8)/2)-count_00;
			bmpptr = (byte *)bmpbuff+x+(y<<6);
			
			for (; y < ((sprdata8[0]|sprdata8[1]<<8)/2)-count_00; y++)
			{
				*bmpptr = *sprptr++;
				if(*bmpptr==0) *bmpptr=0xa0;
				bmpptr+=64;
			}
			sprdata8 += 6;
		}
		cmdptr++;
	}
	memcpyl((void *)ptr,bmpbuff,(64-count_00)<<6);
	ptr+=((64-count_00)<<6);
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
