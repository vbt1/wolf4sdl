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

	uint8_t *ptr = (uint8_t *)0x00202000;
	
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

		static byte bmpbuff[0x1000];
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

#if 0

uint8_t * PM_DecodeSprites2(unsigned int start,unsigned int endi,uint32_t* pageOffsets,word *pageLengths,Uint8 *Chunks)
{
	uint8_t *compressed_chunk = (uint8_t *)0x00202000;
//	unsigned int i = start;
   for(unsigned int i = start; i < endi; i++)
    {
        PMPages[i] = compressed_chunk;
	
        if(!pageOffsets[i])
            continue;               // sparse page
slPrintHex(i-start,slLocate(5,4));
char toto[100];
sprintf(toto,"po0 %04x po1 %04x pl %04x    ",pageOffsets[i],pageOffsets[i + 1],pageLengths[i-PMSpriteStart]);
slPrint(toto,slLocate(5,5));

        // Use specified page length, when next page is sparse page.
        // Otherwise, calculate size from the offset difference between this and the next page.
        uint32_t size;
        if(!pageOffsets[i + 1]) size = pageLengths[i-PMSpriteStart];
        else size = pageOffsets[i + 1] - pageOffsets[i];

		memcpy(compressed_chunk,&Chunks[pageOffsets[i]],size);
		

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memcpy(compressed_chunk,&Chunks[pageOffsets[i]],size);
		memset(&compressed_chunk[size],0x00,end-size);		
		
		
		
		word  first_column, last_column; // first and last column of the sprite with non-transparent texels (left->right)
		word column_offsets[64];            // sequence of offsets to the column instructions.
		static byte buffer[64 * 64];

		int count_00=63;
	/*
uint16_t *ptr16 = (uint16_t *)compressed_chunk;
		for (int x=0;x<size;x++ )
		{
			*ptr16=SWAP_BYTES_16(*ptr16);
			ptr16++;
		}
*/
		first_column = (word)compressed_chunk[0] | (word)(compressed_chunk[1])<<8;
		last_column  = (word)compressed_chunk[2] | (word)(compressed_chunk[3])<<8;

sprintf(toto,"fc %04x lc %04x    ",first_column,last_column);
slPrint(toto,slLocate(5,6));

		word *column_offset_reader = column_offsets; // read-head that will traverse the column offsets
		
		for (int l = 0; l <= last_column - first_column; ++l) 
		{
//			column_offsets[l] = (word)compressed_chunk[4+2*l] | (word)(compressed_chunk[4+2*l+1])<<8;
//			if (column_offsets[l]>size)
//				column_offsets[l]=SWAP_BYTES_16(column_offsets[l]);
			
			word *drawing_instructions = (word *)(compressed_chunk + *column_offset_reader);

			uint idx = 0;			
			while (SWAP_BYTES_16(drawing_instructions[idx]) != 0)
			{
				unsigned int min_y=SWAP_BYTES_16(drawing_instructions[idx+2])/2;		
//				if(min_y>0xff)
//					min_y>>=8;
				
				if(min_y<count_00)
					count_00=min_y;
				

//char toto[100];
sprintf(toto,"%d min_y %04d**%04d**%04x**%04x**%04x**%02d***    ",i-start,min_y,count_00,size,SWAP_BYTES_16(drawing_instructions[idx]),column_offsets[l],l);
slPrint(toto,slLocate(2,8));
			
//			drawing_instructions[idx+1]=SWAP_BYTES_16(drawing_instructions[idx+0]);
//			sprdata[1]=SWAP_BYTES_16(sprdata[1]);
//			drawing_instructions[idx+2]=SWAP_BYTES_16(drawing_instructions[idx+2]);
				
				idx += 3;
			}
			++column_offset_reader; // advance the read-head		
		}
#if 1				
sprintf(toto,"%d a***    ",i-start);
slPrint(toto,slLocate(2,9));

		byte *bmpptr = (byte *)buffer+first_column;
		memset(buffer,0x00,(64)<<6);
		
		column_offset_reader = column_offsets; // read-head that will traverse the column offsets
		
//		for (word column = first_column; column <= last_column; ++column) 
/*	
		for (word column = 0; column <= last_column-first_column; ++column) 	
		{
			column_offsets[column] = (word)compressed_chunk[4+2*column] | (word)(compressed_chunk[4+2*column+1])<<8;
		
			word *drawing_instructions = (word *)(compressed_chunk + *column_offset_reader);

sprintf(toto,"%d off%02x %04x swp %04x b***    ",i-start,*column_offset_reader,drawing_instructions[2],SWAP_BYTES_16(drawing_instructions[2]));
slPrint(toto,slLocate(2,10));
			
			uint idx = 0;
			while (SWAP_BYTES_16(drawing_instructions[idx]) != 0) 
			{
				if(drawing_instructions[idx+2]<size)
				{
					for (int row = SWAP_BYTES_16(drawing_instructions[idx+2])/2; row < SWAP_BYTES_16(drawing_instructions[idx])/2; ++row) 
					{
	sprintf(toto,"%02x %04x %02d sz %04x idx %04x c***    ",row-count_00,row,count_00,size,idx);
	slPrint(toto,slLocate(2,11));
						
						bmpptr[(row-count_00)<<6] = compressed_chunk[SWAP_BYTES_16(drawing_instructions[idx+1])+ row];
						if(bmpptr[(row-count_00)<<6]==0) bmpptr[(row-count_00)<<6]=0xa0;						
					}
					idx += 3;
				}
				else
				{
					for (int row = (drawing_instructions[idx+2])/2; row < (drawing_instructions[idx])/2; ++row) 
					{
	sprintf(toto,"%02x %04x %02d sz %04x idx %04x c***    ",row-count_00,row,count_00,size,idx);
	slPrint(toto,slLocate(2,11));
						
						bmpptr[(row-count_00)<<6] = compressed_chunk[(drawing_instructions[idx+1])+ row];
						if(bmpptr[(row-count_00)<<6]==0) bmpptr[(row-count_00)<<6]=0xa0;						
					}
					idx += 3;					
				}		
			}
			column_offset_reader++; // advance the read-head
			bmpptr++;
		}
	*/	
#endif	
sprintf(toto,"%d c***    ",i-start);
slPrint(toto,slLocate(2,12));
		memcpy((void *)compressed_chunk,buffer,(64-count_00)<<6);
sprintf(toto,"%d d***    ",i-start);
slPrint(toto,slLocate(2,13));		
		compressed_chunk+=((64-count_00)<<6);
	}
	return compressed_chunk;
}

uint8_t * PM_DecodeSprites2bad(unsigned int start,unsigned int endi,uint8_t *ptr,uint32_t* pageOffsets,word *pageLengths,Uint8 *Chunks)
{
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

		int end = size;
		if (size % 4 != 0)
		{
			end = ((size + (4 - 1)) & -4);
		}
		memcpy(ptr,&Chunks[pageOffsets[i]],size);
		memset(&ptr[size],0x00,end-size);
		
uint16_t *ptr16 = (uint16_t *)ptr;
/*		for (int x=0;x<size;x+=2 )
		{
			*ptr16=SWAP_BYTES_16(*ptr16);
			ptr16++;
		}
*/
		t_compshape   *shape = (t_compshape   *)ptr;
		shape->leftpix=SWAP_BYTES_16(shape->leftpix);
		shape->rightpix=SWAP_BYTES_16(shape->rightpix);

		for (int x=0;x<(shape->rightpix-shape->leftpix)+1;x++ )
		{
			shape->dataofs[x]=SWAP_BYTES_16(shape->dataofs[x]);
		}

		static byte bmpbuff[0x1000];
		static byte *bmpptr;
		unsigned short  *cmdptr, *sprdata;

		// setup a pointer to the column offsets	
		cmdptr = shape->dataofs;
		int count_00=63;

//if (i>=SPR_SS_PAIN_1 && i<=SPR_SS_SHOOT3)count_00=1;
		for (unsigned int x = (shape->leftpix); x <= (shape->rightpix); x++)
		{
			sprdata = (unsigned short *)((unsigned char  *)shape+*cmdptr);
//			sprdata[0]=SWAP_BYTES_16(sprdata[0]);
//			sprdata[1]=SWAP_BYTES_16(sprdata[1]);
//			sprdata[2]=SWAP_BYTES_16(sprdata[2]);

			while (SWAP_BYTES_16(*sprdata) != 0)
//			while (*sprdata != 0)	
			{
//				unsigned int min_y=sprdata[2]/2; //(SWAP_BYTES_16(sprdata[2])/2);
				unsigned int min_y=(SWAP_BYTES_16(sprdata[2])/2);
				if(min_y<count_00)
					count_00=min_y;
					
				sprdata += 3;
//			sprdata[0]=SWAP_BYTES_16(sprdata[0]);
//			sprdata[1]=SWAP_BYTES_16(sprdata[1]);
//			sprdata[2]=SWAP_BYTES_16(sprdata[2]);
				
			}
			cmdptr++;
		}
		memset(bmpbuff,0x00,(64-count_00)<<6);
		
		unsigned char  *sprptr = (unsigned char  *)shape+(((((shape->rightpix)-(shape->leftpix))+1)*2)+4);
		// setup a pointer to the column offsets
		cmdptr = shape->dataofs;		

char toto[100];
sprintf(toto,"%d %08x",i-PMSpriteStart,(int)ptr);
slPrint(toto,slLocate(5,7));

		for (int x = (shape->leftpix); x <= (shape->rightpix); x++)
		{
			sprdata = (unsigned short *)((unsigned char  *)shape+*cmdptr);
//			bmpptr = (byte *)bmpbuff+x;
			bmpptr = (byte *)bmpbuff;
		
			while (SWAP_BYTES_16(*sprdata) != 0)
//			while (*sprdata != 0)
			{
				for (int y = SWAP_BYTES_16(sprdata[2])/2; y < SWAP_BYTES_16(*sprdata)/2; y++)
//				for (int y = sprdata[2]/2; y < *sprdata/2; y++)
				{
					
//sprintf(toto,"%02d %02d          ",SWAP_BYTES_16(sprdata[2])/2,SWAP_BYTES_16(*sprdata)/2);
//sprintf(toto,"%02d %02d          ",sprdata[2]/2,*sprdata/2);
//slPrint(toto,slLocate(5,8));					
					
//	slPrint((char *)"e3               ",slLocate(10,12));				
					bmpptr[(y-count_00)<<6] = *sprptr++;
					if(bmpptr[(y-count_00)<<6]==0) bmpptr[(y-count_00)<<6]=0xa0;					
				}
				sprdata += 3;
			}
			cmdptr++;
		}
sprintf(toto,"%d ***    ",i-start);
slPrint(toto,slLocate(2,11));		
		memcpyl((void *)ptr,bmpbuff,(64-count_00)<<6);
	sprintf(toto,"%d ***    ",i-start);
slPrint(toto,slLocate(2,12));	
		ptr+=((64-count_00)<<6);	
		
	}
	return ptr;
}
#endif
/*
void PM_Shutdown()
{
    free(PMPages);
	PMPages = NULL;	
//    free(PMPageData);
//	PMPageData = NULL;	
}
*/