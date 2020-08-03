#include "wl_def.h"

void set_sprite(PICTURE *pcptr);

int ChunksInFile;
int PMSpriteStart;
int PMSoundStart;
int currentPage;

bool PMSoundInfoPagePadded = false;

// holds the whole VSWAP
uint32_t *PMPageData;
size_t PMPageDataSize;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
    char fname[13] = "vswap.";
	Uint32 i,j=0;
	Sint16 *Chunks;
	long fileSize;
    strcat(fname,extension);
	
    //FILE *file = fopen(fname,"rb");
	Sint32 fileId;

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 

    if(stat(fname, NULL))
        CA_CannotOpen(fname);

	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);
	//Chunks = LoadFile(fname,&fileSize);

	//GfsHn gfs;
    //gfs = GFS_Open(fileId);

    ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);
	Chunks=(Sint16*)malloc(8192);
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, 8192);
	ChunksInFile=Chunks[i];
	ChunksInFile = SWAP_BYTES_16(ChunksInFile);
	i++;
    PMSpriteStart = 0;
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[i];
	PMSpriteStart = SWAP_BYTES_16(PMSpriteStart);
	i++;
    PMSoundStart = 0;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[i];
	PMSoundStart = SWAP_BYTES_16(PMSoundStart);
	i++;
    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);
   
	for(;i<ChunksInFile*2+3;i+=2)
	{
		pageOffsets[j]=SWAP_BYTES_16(Chunks[i]) | (SWAP_BYTES_16(Chunks[i+1])<<16);
		j++;
	} 
    //fread(pageOffsets, sizeof(uint32_t), ChunksInFile, file);
    word *pageLengths = (word *) malloc(ChunksInFile * sizeof(word));
    CHECKMALLOCRESULT(pageLengths);
	j=0;
	for(;i<ChunksInFile*3+3;i++)
	{
		pageLengths[j]=SWAP_BYTES_16(Chunks[i]);
		j++;
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
    // Calculate total amount of padding needed for sprites and sound info page
    int alignPadding = 0;
    for(i = PMSpriteStart; i < PMSoundStart; i++)
    {
        if(!pageOffsets[i]) continue;   // sparse page
        uint32_t offs = pageOffsets[i] - dataStart + alignPadding;
        if(offs & 1)
            alignPadding++;
    }
    if((pageOffsets[ChunksInFile - 1] - dataStart + alignPadding) & 1)
        alignPadding++;

    PMPageDataSize = (size_t) pageDataSize + alignPadding;
    PMPageData = (uint32_t *) 0x00202000;//malloc(PMPageDataSize);
    CHECKMALLOCRESULT(PMPageData);
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);
    // Load pages and initialize PMPages pointers
    uint8_t *ptr = (uint8_t *) PMPageData;
////////////
//unsigned int position=0;
	
	

    for(i = 0; i < ChunksInFile; i++)
    {
        if(i >= PMSpriteStart && i < PMSoundStart || i == ChunksInFile - 1)
        {
            size_t offs = ptr - (uint8_t *) PMPageData;

            // pad with zeros to make it 2-byte aligned
            if(offs & 1)
            {
                *ptr++ = 0;
                if(i == ChunksInFile - 1) PMSoundInfoPagePadded = true;
            }
        }

        PMPages[i] = ptr;

        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
		uint16_t delta;
		uint32_t delta2;
		uint8_t *x;
		x = (uint8_t *)malloc(4096*2);
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];
		delta = (uint16_t)(pageOffsets[i]/2048);
		delta2 = pageOffsets[i] - delta*2048; 
		GFS_Load(fileId, delta, (void *)x, 4096*2);
		
		if(i >= PMSpriteStart && i < PMSoundStart || i == ChunksInFile - 1)
			memcpy(ptr,&x[delta2],size);
		else
		{
			memcpy(ptr,&x[delta2],size);
			//vbt :  contient les muurs !!!!
		}
	
		free(x);
		x = NULL;		
        ptr += size;
    }


    // last page points after page buffer
    PMPages[ChunksInFile] = ptr;

    for (i = PMSpriteStart;i < PMSoundStart;i++)
	{
		t_compshape   *shape = (t_compshape   *)PMPages[i];
		shape->leftpix=SWAP_BYTES_16(shape->leftpix);
		shape->rightpix=SWAP_BYTES_16(shape->rightpix);
   
		for (int x=0;x<shape->rightpix ;x++ )
		{
			shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
		}	  
	}

	free(pageLengths);
	pageLengths = NULL;	
    free(pageOffsets);
	pageOffsets = NULL;		
	free(Chunks);
	Chunks = NULL;		
    //fclose(file);
}

void PM_Shutdown()
{
    free(PMPages);
	PMPages = NULL;	
    free(PMPageData);
	PMPageData = NULL;	
}
