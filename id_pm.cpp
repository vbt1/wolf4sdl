#include "wl_def.h"

int ChunksInFile;
int PMSpriteStart;
int PMSoundStart;

//bool PMSoundInfoPagePadded = false;

// holds the whole VSWAP
uint16_t *PMPageData;
//size_t PMPageDataSize;

// ChunksInFile+1 pointers to page starts.
// The last pointer points one byte after the last page.
uint8_t **PMPages;

void PM_Startup()
{
    char fname[13] = "vswap.";
	Uint32 i=0,j=0;
	Sint16 *Chunks;
	long fileSize;
    strcat(fname,extension);
	
	Sint32 fileId;

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 

//    if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);

    ChunksInFile = 0;
//    fread(&ChunksInFile, sizeof(word), 1, file);
	Chunks=(Sint16*)0x00230000;
	CHECKMALLOCRESULT(Chunks);
	GFS_Load(fileId, 0, (void *)Chunks, fileSize); //(ChunksInFile*3)+3);
	ChunksInFile=Chunks[0];
	ChunksInFile = SWAP_BYTES_16(ChunksInFile);
    PMSpriteStart = 0;
    //fread(&PMSpriteStart, sizeof(word), 1, file);
	PMSpriteStart=Chunks[1];
	PMSpriteStart = SWAP_BYTES_16(PMSpriteStart);
    PMSoundStart = 0;
    //fread(&PMSoundStart, sizeof(word), 1, file);
	PMSoundStart=Chunks[2];
	PMSoundStart = SWAP_BYTES_16(PMSoundStart);
    uint32_t* pageOffsets = (uint32_t *) malloc((ChunksInFile + 1) * sizeof(int32_t));
    CHECKMALLOCRESULT(pageOffsets);
//	GFS_Load(fileId, 0, (void *)Chunks,(ChunksInFile*3)+3);
	
	//Chunks+=3;
	
	for(i=0;i<(ChunksInFile*2);i+=2,j++)
	{
		pageOffsets[j]=SWAP_BYTES_16(Chunks[i+3]) | (SWAP_BYTES_16(Chunks[i+1+3])<<16);
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

    size_t PMPageDataSize = (size_t) pageDataSize + alignPadding;
    PMPageData = (uint16_t *) 0x00202000;
    CHECKMALLOCRESULT(PMPageData);
    PMPages = (uint8_t **) malloc((ChunksInFile + 1) * sizeof(uint8_t *));
    CHECKMALLOCRESULT(PMPages);
    // Load pages and initialize PMPages pointers
    uint8_t *ptr = (uint8_t *) PMPageData;
//	uintptr_t ptr4 = (uintptr_t)PMPageData;
	
    for(i = 0; i < ChunksInFile; i++)
    {
        if(i >= PMSpriteStart && i < PMSoundStart || i == ChunksInFile - 1)
        {
#if 0
	if (ptr4 % 2 != 0)
			{
				ptr4 = (ptr4 + (2 - 1)) & -2;
			}			
#else			
            size_t offs = ptr - (uint8_t *) PMPageData;
            // pad with zeros to make it 2-byte aligned
            if(offs & 1)
            {
                *ptr++ = 0;
//                if(i == ChunksInFile - 1) PMSoundInfoPagePadded = true;
            }
#endif			
        }

//        PMPages[i] = (uint8_t *)ptr4;
        PMPages[i] = ptr;
		
		uint8_t *x=(uint8_t*)0x00230000;
	
        if(!pageOffsets[i])
            continue;               // sparse page

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i];
        else size = pageOffsets[i + 1] - pageOffsets[i];
//		memcpy((uint8_t *)ptr4,&x[pageOffsets[i]],size);
		memcpy(ptr,&x[pageOffsets[i]],size);
//        ptr4 += size;
        ptr += size;
    }

    // last page points after page buffer
//    PMPages[ChunksInFile] = (uint8_t *)ptr4;
    PMPages[ChunksInFile] = ptr;

    for (i = PMSpriteStart;i < PMSoundStart;i++)
	{
		t_compshape   *shape = (t_compshape   *)PMPages[i];
		
		if(shape->rightpix>63) // strange fix
		{
			shape->leftpix=SWAP_BYTES_16(shape->leftpix);
			shape->rightpix=SWAP_BYTES_16(shape->rightpix);
	   
//			for (int x=0;x<shape->rightpix ;x++ )
			for (int x=0;x<shape->rightpix ;x++ )
			{
				shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
			}


		}	
	
//		}
/*		
#ifdef USE_SPRITES
		unsigned short  *cmdptr, *sprdata;
		cmdptr = shape->dataofs;

		for (int x = shape->leftpix; x <= shape->rightpix; x++)
		{
			sprdata = (unsigned short *)(shape+*cmdptr);

			while ((SWAP_BYTES_16(*sprdata)) != 0)
			{
				sprdata[2]=SWAP_BYTES_16(sprdata[2]);
				*sprdata=SWAP_BYTES_16(*sprdata);
				sprdata += 3;
			}
			cmdptr++;
		}
#endif	*/		
	}

	
	
	
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
	PMPageData = NULL;	
}
