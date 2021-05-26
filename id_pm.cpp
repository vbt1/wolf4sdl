#include "wl_def.h"

int PMSpriteStart;
int PMSoundStart;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
//#define READ16 1
    char fname[13] = "VSWAP.";
	Uint32 i=0,j=0;
#ifdef READ16	
	Uint16 *Chunks;
#else
	Uint8 *Chunks;
#endif	
	long fileSize;
    strcat(fname,extension);
	
	Sint32 fileId;

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    int ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);

	Chunks=(Uint8*)0x00242000;
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
	
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);	

    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);

    word *pageLengths = (word *) malloc(ChunksInFile * sizeof(word));
    CHECKMALLOCRESULT(pageLengths);

	uint8_t *wallData = (uint8_t *) malloc(48*0x1000);
	CHECKMALLOCRESULT(wallData);
	
	for(i=0;i<ChunksInFile;i++)
	{
		pageOffsets[i]=Chunks[6]<<0|Chunks[7]<<8|Chunks[8]<<16|Chunks[9]<<24;
		Chunks+=4;
	}

	for(i=0;i<ChunksInFile;i++)
	{
		pageLengths[i]=Chunks[6]|Chunks[7]<<8;
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

    uint8_t *PMPageData = (uint8_t *)0x00202000;
//    CHECKMALLOCRESULT(PMPageData);

	Chunks=(Uint8*)0x00242000;	
    // Load pages and initialize PMPages pointers
	
	uint8_t *ptr = (uint8_t *)wallData;
	
    for(i = SPR_STAT_0; i < 40; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memset(ptr,0x00,end);
		memcpy(ptr,&Chunks[pageOffsets[i]],size);
		ptr+=end;
	}			

    for(i = PMSpriteStart-8; i < PMSpriteStart; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memset(ptr,0x00,end);
		memcpy(ptr,&Chunks[pageOffsets[i]],size);
		ptr+=end;
	}
	
    ptr = (uint8_t *) PMPageData;	
	
    for(i = PMSpriteStart; i < ChunksInFile; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memset(ptr,0x00,end);
		memcpy(ptr,&Chunks[pageOffsets[i]],size);
		
//		if(i >= PMSpriteStart && i < PMSoundStart)
        {
			t_compshape   *shape = (t_compshape   *)ptr;
			shape->leftpix=SWAP_BYTES_16(shape->leftpix);
			shape->rightpix=SWAP_BYTES_16(shape->rightpix);

			for (int x=0;x<(shape->rightpix-shape->leftpix)+1;x++ )
			{
				shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
			}
		}
		ptr+=end;
	}
	
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

void PM_Shutdown()
{
    free(PMPages);
	PMPages = NULL;	
//    free(PMPageData);
//	PMPageData = NULL;	
}
