#include "wl_def.h"

int PMSpriteStart;
int PMSoundStart;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
//#define READ16 1
    char fname[13] = "vswap.";
	Uint32 i=0,j=0;
#ifdef READ16	
	Uint16 *Chunks;
#else
	Uint8 *Chunks;
#endif	
	long fileSize;
    strcat(fname,extension);
	
	Sint32 fileId;

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 

	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    int ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);
#ifdef READ16	
	Chunks=(Uint16*)0x00230000;
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, fileSize); //(ChunksInFile*3)+3);
	ChunksInFile=SWAP_BYTES_16(Chunks[0]);
    PMSpriteStart = 0;
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=SWAP_BYTES_16(Chunks[1]);
    PMSoundStart = 0;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=SWAP_BYTES_16(Chunks[2]);

    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);
	
	for(i=0;i<(ChunksInFile*2);i+=2,j++)
	{
		pageOffsets[j]=SWAP_BYTES_16(Chunks[i+3]) | (SWAP_BYTES_16(Chunks[i+4])<<16);
	} 

    //fread(pageOffsets, sizeof(uint32_t), ChunksInFile, file);
    word *pageLengths = (word *) malloc(ChunksInFile * sizeof(word));
    CHECKMALLOCRESULT(pageLengths);

	for(j=0;i<(ChunksInFile*3);i++,j++)
	{
		pageLengths[j]=SWAP_BYTES_16(Chunks[i+3]);
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
#else
	Chunks=(Uint8*)0x00240000;
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, fileSize); //(ChunksInFile*3)+3);
	ChunksInFile=Chunks[0]|Chunks[1]<<8;
    PMSpriteStart = 0;
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[2]|Chunks[3]<<8;
    PMSoundStart = 0;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[4]|Chunks[5]<<8;

    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);

	for(i=0;i<(ChunksInFile*4);i+=4,j++)
	{
		pageOffsets[j]=Chunks[6+i]<<0|Chunks[7+i]<<8|Chunks[8+i]<<16|Chunks[9+i]<<24;
	} 

    //fread(pageOffsets, sizeof(uint32_t), ChunksInFile, file);
    word *pageLengths = (word *) malloc(ChunksInFile * sizeof(word));
    CHECKMALLOCRESULT(pageLengths);

	for(j=0;i<(ChunksInFile*6);i+=2,j++)
	{
		pageLengths[j]=Chunks[6+i]|Chunks[7+i]<<8; //SWAP_BYTES_16(Chunks[i+3]);
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
#endif
    uint8_t *PMPageData = (uint8_t *) 0x00202000;
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);
    // Load pages and initialize PMPages pointers
    uint8_t *ptr = (uint8_t *) PMPageData;
	
    for(i = 0; i < ChunksInFile; i++)
    {
        PMPages[i] = ptr;
	
        if(!pageOffsets[i])
            continue;               // sparse page

		uint8_t *Chunks8=(uint8_t*)Chunks;		
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
		memcpy(ptr,&Chunks8[pageOffsets[i]],size);
		
		if(i >= PMSpriteStart && i < PMSoundStart)
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
