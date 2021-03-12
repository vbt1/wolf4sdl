// ID_CA.C

// this has been customized for WOLF

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/
/*
#include <sys/types.h>
#if defined _WIN32
	#include <io.h>
#elif !defined _arch_dreamcast
	#include <sys/uio.h>
	#include <unistd.h>
#endif
 */
#include "wl_def.h"

#pragma hdrstop
#define THREEBYTEGRSTARTS
//#define HEAP_WALK 1

#ifdef HEAP_WALK
extern Uint32  end;
extern Uint32  __malloc_free_list;

extern "C" {
extern Uint32  _sbrk(int size);
}

void heapWalk(void)
{
    Uint32 chunkNumber = 1;
    // The __end__ linker symbol points to the beginning of the heap.
    Uint32 chunkCurr = (Uint32)&end;
    // __malloc_free_list is the head pointer to newlib-nano's link list of free chunks.
    Uint32 freeCurr = __malloc_free_list;
    // Calling _sbrk() with 0 reserves no more memory but it returns the current top of heap.
    Uint32 heapEnd = _sbrk(0);
    
//    printf("Heap Size: %lu\n", heapEnd - chunkCurr);
    char msg[100];
	sprintf (msg,"Heap Size: %d  e%08x s%08x                  ", heapEnd - chunkCurr,heapEnd, chunkCurr) ;
//	FNT_Print256_2bpp((volatile Uint8 *)SS_FONT,(Uint8 *)toto,12,216);
	slPrint((char *)msg,slLocate(1,3));
	
    // Walk through the chunks until we hit the end of the heap.
    while (chunkCurr < heapEnd)
    {
        // Assume the chunk is in use.  Will update later.
        int      isChunkFree = 0;
        // The first 32-bit word in a chunk is the size of the allocation.  newlib-nano over allocates by 8 bytes.
        // 4 bytes for this 32-bit chunk size and another 4 bytes to allow for 8 byte-alignment of returned pointer.
        Uint32 chunkSize = *(Uint32*)chunkCurr;
        // The start of the next chunk is right after the end of this one.
        Uint32 chunkNext = chunkCurr + chunkSize;
        
        // The free list is sorted by address.
        // Check to see if we have found the next free chunk in the heap.
        if (chunkCurr == freeCurr)
        {
            // Chunk is free so flag it as such.
            isChunkFree = 1;
            // The second 32-bit word in a free chunk is a pointer to the next free chunk (again sorted by address).
            freeCurr = *(Uint32*)(freeCurr + 4);
        }
        
        // Skip past the 32-bit size field in the chunk header.
        chunkCurr += 4;
        // 8-byte align the data pointer.
        chunkCurr = (chunkCurr + 7) & ~7;
        // newlib-nano over allocates by 8 bytes, 4 bytes for the 32-bit chunk size and another 4 bytes to allow for 8
        // byte-alignment of the returned pointer.
        chunkSize -= 8;
//        printf("Chunk: %lu  Address: 0xlX  Size: %lu  %s\n", chunkNumber, chunkCurr, chunkSize, isChunkFree ? "CHUNK FREE" : "");
        
	sprintf (msg,"%d A%04x  S%04d %s", chunkNumber, chunkCurr, chunkSize, isChunkFree ? "CHUNK FREE" : "") ;
	if(chunkNumber<20)	
		slPrint((char *)msg,slLocate(0,chunkNumber*10+10));
//	if(chunkNumber>=200)
//	slPrint((char *)msg,slLocate(0,chunkNumber-200));
//	if(chunkNumber>=230)
//	slPrint((char *)msg,slLocate(20,chunkNumber-230));

		chunkCurr = chunkNext;
        chunkNumber++;
    }
}
#endif

/*
=============================================================================

                             LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
    word bit0,bit1;       // 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
    word RLEWtag;
    int32_t headeroffsets[100];
} mapfiletype;


/*
=============================================================================

                             GLOBAL VARIABLES

=============================================================================
*/

#define BUFFERSIZE 0x800
static int32_t bufferseg[BUFFERSIZE/4];

int     mapon;

word    *mapsegs[MAPPLANES];
static maptype* mapheaderseg[NUMMAPS];
//byte    *audiosegs[NUMSNDCHUNKS];
byte    *grsegs[NUMCHUNKS];

word    RLEWtag;

int     numEpisodesMissing = 0;

/*
=============================================================================

                             LOCAL VARIABLES

=============================================================================
*/

char extension[5]; // Need a string, not constant to change cache files
//char graphext[5];
//char audioext[5];
static const char gheadname[] = "vgahead.";
static const char gfilename[] = "vgagraph.";
static const char gdictname[] = "vgadict.";
static const char mheadname[] = "maphead.";
//static const char mfilename[] = "maptemp.";
//static const char aheadname[] = "audiohed.";
//static const char afilename[] = "audiot.";

//void CA_CannotOpen(const char *string);

static int32_t  grstarts[NUMCHUNKS + 1];
//static int32_t* audiostarts; // array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode *grhuffman;
#else
huffnode grhuffman[255];
#endif

int    grhandle = -1;               // handle to EGAGRAPH
int    maphandle = -1;              // handle to MAPTEMP / GAMEMAPS
//int    audiohandle = -1;            // handle to AUDIOT / AUDIO

static int32_t GRFILEPOS(const size_t idx)
{
#define assert8(x) if(!(x)) { slPrint((char *)"asset test failed8", slLocate(10,20));return;}
	return grstarts[idx];
}

/*
=============================================================================

                            LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

int32_t CAL_GetGrChunkLength (int chunk)
{
	int32_t   chunkexplen;
    //lseek(grhandle,GRFILEPOS(chunk),SEEK_SET);
    //read(grhandle,&chunkexplen,sizeof(chunkexplen));

	uint8_t *Chunks;
	uint16_t delta = (uint16_t)(GRFILEPOS(chunk)/2048);
	uint32_t delta2 = (GRFILEPOS(chunk) - delta*2048); 
	Chunks=(uint8_t*)SATURN_CHUNK_ADDR;
	
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, sizeof(chunkexplen)+delta2);
	memcpy(&chunkexplen,&Chunks[delta2],sizeof(chunkexplen));
	Chunks = NULL;
	chunkexplen = SWAP_BYTES_32(chunkexplen);
    return GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
}

/*
============================================================================

                COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

static void CAL_HuffExpand(byte *source, byte *dest, int32_t length, huffnode *hufftable)
{
    byte *end;
    huffnode *headptr, *huffptr;

    if(!length || !dest)
    {
        Quit("length or dest is null!");
        return;
    }
    headptr = hufftable+254;        // head node is always node 254

    int written = 0;

    end=dest+length;

    byte val = *source++;
    byte mask = 1;
    word nodeval;
    huffptr = headptr;
    while(1)
    {
        if(!(val & mask))
            nodeval = huffptr->bit0;
        else
            nodeval = huffptr->bit1;
        if(mask==0x80)
        {
            val = *source++;
            mask = 1;
        }
        else mask <<= 1;

        if(nodeval<256)
        {
            *dest++ = (byte) nodeval;
            written++;
            huffptr = headptr;

            if(dest>=end) break;
        }
        else
        {
            huffptr = hufftable + (nodeval - 256);
        }
    }
}

/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG 0xa7
#define FARTAG  0xa8

void CAL_CarmackExpand (byte *source, word *dest, int length)
{
    word ch,chhigh,count,offset;
    byte *inptr;
    word *copyptr, *outptr;
    length/=2;

    inptr = (byte *) source;
    outptr = dest;
//ch |= *inptr++;"     -> "ch |= (*inptr++) << 8;
    while (length>0)
    {
        ch = READWORD(inptr);
        chhigh = ch>>8;
        if (chhigh == NEARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr = ch;
				outptr++;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;

                if(length<0) return;
                while (count--)
				{
                    *outptr = *copyptr;
					outptr++;
					copyptr++;
				}
            }
        }
        else if (chhigh == FARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                 ch |= *inptr++;
                *outptr = ch;
				outptr++;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                copyptr = dest + offset;
                length -= count;
                if(length<0) return;
                while (count--)
				{
                    *outptr = *copyptr;
					outptr++;
					copyptr++;
				}
            }
        }
        else
        {
            *outptr = ch;
			outptr++;
            length --;
        }
    }
}

/*
======================
=
= CA_RLEWcompress
=
======================
*/
/*
int32_t CA_RLEWCompress (word *source, int32_t length, word *dest, word rlewtag)
{
    word value,count;
    unsigned i;
    word *start,*end;

    start = dest;

    end = source + (length+1)/2;

    //
    // compress it
    //
    do
    {
        count = 1;
        value = *source++;
        while (*source == value && source<end)
        {
            count++;
            source++;
        }
        if (count>3 || value == rlewtag)
        {
            //
            // send a tag / count / value string
            //
            *dest++ = rlewtag;
            *dest++ = count;
            *dest++ = value;
        }
        else
        {
            //
            // send word without compressing
            //
            for (i=1;i<=count;i++)
                *dest++ = value;
        }

    } while (source<end);

    return (int32_t)(2*(dest-start));
}

*/
/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (word *source, word *dest, int32_t length, word rlewtag)
{
    word value,count,i;
    word *end=dest+length/2;

//
// expand it
//
    do
    {
        value = *source++;
        if (value != rlewtag)
            //
            // uncompressed
            //
            *dest++=value;
        else
        {
            //
            // compressed string
            //
            count = *source++;
            value = *source++;
            for (i=1;i<=count;i++)
                *dest++ = value;
        }
    } while (dest<end);
}



/*
=============================================================================

                                         CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
    char fname[13];
    //int handle;
	unsigned int i=0;
    byte *compseg;
	long fileSize;
	Sint32 fileId;

#ifdef GRHEADERLINKED

    grhuffman = (huffnode *)&EGAdict;
    grstarts = (int32_t _seg *)FP_SEG(&EGAhead);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

    strcpy(fname,gdictname);
    strcat(fname,extension);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
//	i=0;

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	fileId = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(fileId);
	compseg=(Uint8 *)0x002C0000;
//	CHECKMALLOCRESULT(vbtHuff);

	GFS_Load(fileId, 0, (void *)compseg, sizeof(grhuffman));
	huffnode *grhuffmanptr = (huffnode *)grhuffman;
	
	for(unsigned char x=0;x<255;x++)
	{
		grhuffmanptr->bit0=compseg[0] | (compseg[1]<<8);
		grhuffmanptr->bit1=compseg[2] | (compseg[3]<<8);
		grhuffmanptr++;
		compseg+=4;
	}	
	i=0;

    //read(handle, grhuffman, sizeof(grhuffman));
    //close(handle);

    // load the data offsets from ???head.ext
    strcpy(fname,gheadname);
    strcat(fname,extension);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
	i=0;

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);
    long headersize = fileSize;//lseek(handle, 0, SEEK_END);
    //lseek(handle, 0, SEEK_SET);

    if(!param_ignorenumchunks && headersize / 3 != (long) (lengthof(grstarts) - numEpisodesMissing))
        Quit("Wolf4SDL was not compiled for these data files:\n"
            "        %s contains a wrong number of offsets                          (%i instead of %i)!\n\n"
            "                          Please check whether you are using the right executable!\n"
            "(For mod developers: perhaps you forgot to update NUMCHUNKS?)",
            fname, headersize / 3, lengthof(grstarts) - numEpisodesMissing);

    byte data[lengthof(grstarts) * 3];
	//GFS_Load(fileId, 0, (void *)data, fileSize);
	GFS_Load(fileId, 0, (void *)data, sizeof(data));
    //read(handle, data, sizeof(data));
    //close(handle);

    const byte* d = data;
    for (int32_t* i = grstarts; i != endof(grstarts); ++i)
    {
        const int32_t val = d[0] | d[1] << 8 | d[2] << 16;
        *i = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }

#endif

//
// Open the graphics file, leaving it open until the game is finished
//
    strcpy(fname,gfilename);
    strcat(fname,extension);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}
	i=0;

    //grhandle = open(fname, O_RDONLY | O_BINARY);
    //if (grhandle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	grhandle = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(grhandle);
//
// load the pic and sprite headers into the arrays in the data segment
//
    pictable=(pictabletype *) malloc(NUMPICS*sizeof(pictabletype));
    CHECKMALLOCRESULT(pictable);
    int32_t   chunkcomplen = CAL_GetGrChunkLength(STRUCTPIC);                // position file pointer
//	compseg =(byte*)malloc((chunkcomplen));
	compseg =(byte*)0x002C0000;
//	CHECKMALLOCRESULT(compseg);
	GFS_Load(grhandle, 0, (void *)compseg, (chunkcomplen));
    CAL_HuffExpand(&compseg[4], (byte*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);

	for(unsigned long j=0;j<NUMPICS;j++)
	{
		pictable[j].height=SWAP_BYTES_16(pictable[j].height);
		pictable[j].width=SWAP_BYTES_16(pictable[j].width);
	} 
	compseg = NULL;
	// VBT correct
}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
    int     i,j;
    int32_t length,pos;
    char fname[13];
	long fileSize;

//
// load maphead.ext (offsets and tileinfo for map file)
//
    strcpy(fname,mheadname);
    strcat(fname,extension);

	Sint32 fileId;
	i=0;
	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 
	i=0;
	fileId = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(fileId); // utile
    //handle = open(fname, O_RDONLY | O_BINARY);
//    if(stat(fname, NULL))
//        CA_CannotOpen(fname);
    length = NUMMAPS*4+2; // used to be "filelength(handle);"
//    mapfiletype *tinf=(mapfiletype *) malloc(sizeof(mapfiletype));
	mapfiletype *tinf=(mapfiletype *)0x002C0000;
//    CHECKMALLOCRESULT(tinf);
	GFS_Load(fileId, 0, (void *)tinf, length);
    //read(handle, tinf, length);
    //close(handle);
     //word RLEWtag;

   tinf->RLEWtag=SWAP_BYTES_16(tinf->RLEWtag);

   for(i=0;i<100;i++)
		tinf->headeroffsets[i]=SWAP_BYTES_32(tinf->headeroffsets[i]);

    RLEWtag=tinf->RLEWtag;
	i=0;
//
// open the data file
//
#ifdef CARMACIZED
    strcpy(fname, "gamemaps.");
    strcat(fname, extension);

	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 

//	i=0;
	maphandle = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(maphandle);

    //maphandle = open(fname, O_RDONLY | O_BINARY);
    //if (maphandle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);
#else
    strcpy(fname,mfilename);
    strcat(fname,extension);

    maphandle = open(fname, O_RDONLY | O_BINARY);
    if (maphandle == -1)
        CA_CannotOpen(fname);
#endif

//
// load all map header
//
	uint8_t *maphandleptr;
//	maphandleptr = (Uint8*)malloc(fileSize);
	maphandleptr = (uint8_t*)(0x002D0000);
//	CHECKMALLOCRESULT(maphandleptr);
	GFS_Load(maphandle, 0, (void *)maphandleptr, fileSize);

    for (i=0;i<NUMMAPS;i++)
    {
        pos = tinf->headeroffsets[i];
        if (pos<0)                          // $FFFFFFFF start is a sparse map
            continue;	   

        mapheaderseg[i]=(maptype *) malloc(sizeof(maptype));
        CHECKMALLOCRESULT(mapheaderseg[i]);
        //lseek(maphandle,pos,SEEK_SET);
        //read (maphandle,(memptr)mapheaderseg[i],sizeof(maptype));
		memcpy((memptr)mapheaderseg[i],&maphandleptr[pos],sizeof(maptype));
		  
	   for(j=0;j<3;j++)
		{
			mapheaderseg[i]->planestart[j]=SWAP_BYTES_32(mapheaderseg[i]->planestart[j]);
			mapheaderseg[i]->planelength[j]=SWAP_BYTES_16(mapheaderseg[i]->planelength[j]);
		}	 
		mapheaderseg[i]->width=SWAP_BYTES_16(mapheaderseg[i]->width);
		mapheaderseg[i]->height=SWAP_BYTES_16(mapheaderseg[i]->height);
    }

//	free(maphandleptr);
	maphandleptr = NULL;	
//    free(tinf);
	tinf = NULL;
//
// allocate space for 3 64*64 planes
//
    for (i=0;i<MAPPLANES;i++)
    {
        mapsegs[i]=(word *) malloc(maparea*2);
        CHECKMALLOCRESULT(mapsegs[i]);
    }
}


//==========================================================================

/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
    unlink ("PROFILE.TXT");
    profilehandle = open("PROFILE.TXT", O_CREAT | O_WRONLY | O_TEXT);
#endif
    CAL_SetupMapFile ();
    CAL_SetupGrFile ();
    //CAL_SetupAudioFile ();

    mapon = -1;
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
#ifdef VBT	
    int i,start;

    if(maphandle != -1)
        close(maphandle);
    if(grhandle != -1)
        close(grhandle);
    if(audiohandle != -1)
        close(audiohandle);

    for(i=0; i<NUMCHUNKS; i++)
        UNCACHEGRCHUNK(i);
    free(pictable);
	pictable = NULL;

    switch(oldsoundmode)
    {
        case sdm_Off:
            return;
        case sdm_PC:
            start = STARTPCSOUNDS;
            break;
        case sdm_AdLib:
            start = STARTADLIBSOUNDS;
            break;
    }

    for(i=0; i<NUMSOUNDS; i++,start++)
        UNCACHEAUDIOCHUNK(start);
#endif
}

//===========================================================================

/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (int chunk, int32_t *source)
{
    int32_t    expanded;
    if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
    {
        //
        // expanded sizes of tile8/16/32 are implicit
        //

#define BLOCK           64
#define MASKBLOCK       128
        if (chunk<STARTTILE8M)          // tile 8s are all in one chunk!
            expanded = BLOCK*NUMTILE8;
        else if (chunk<STARTTILE16)
            expanded = MASKBLOCK*NUMTILE8M;
        else if (chunk<STARTTILE16M)    // all other tiles are one/chunk
            expanded = BLOCK*4;
        else if (chunk<STARTTILE32)
            expanded = MASKBLOCK*4;
        else if (chunk<STARTTILE32M)
            expanded = BLOCK*16;
        else
            expanded = MASKBLOCK*16;
    }
    else
    {
        //
        // everything else has an explicit size longword
        //
        expanded = *source++;
	expanded=SWAP_BYTES_32(expanded); 
    }
    //
    // allocate final space, decompress it, and free bigbuffer
    // Sprites need to have shifts made and various other junk
    //
    grsegs[chunk]=(byte *) malloc(expanded);
    CHECKMALLOCRESULT(grsegs[chunk]);
	 
    CAL_HuffExpand((byte *) source, grsegs[chunk], expanded, grhuffman);
}


/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (int chunk)
{
    int32_t pos,compressed;
    int32_t *source;
    int  next;

    if (grsegs[chunk])
	{
//		slPrint((char *)"already in memory",slLocate(10,11));
        return;                             // already in memory
	}
//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
    pos = GRFILEPOS(chunk);

    if (pos<0)                              // $FFFFFFFF start is a sparse tile
        return;
    next = chunk +1;
    while (GRFILEPOS(next) == -1)           // skip past any sparse tiles
        next++;
    compressed = GRFILEPOS(next)-pos;
    //lseek(grhandle,pos,SEEK_SET);
		uint8_t *Chunks;
		uint16_t delta;
		uint32_t delta2;
		Uint16 i,j=0;
		delta = (uint16_t)(pos/2048);
		delta2 = (pos - delta*2048); 

	Chunks=(uint8_t*)SATURN_CHUNK_ADDR;  // déplacé pour pas écraser de son
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, compressed+delta2);
	Chunks+=delta2;

    if (compressed<=BUFFERSIZE)
    {
		for(i=0;i<compressed;i+=4)
		{
// evite des erreurs d'alignement de pointeur ?			
			bufferseg[j++]=Chunks[3] | (Chunks[2]<<8)|(Chunks[1]<<16) | (Chunks[0]<<24);
			Chunks+=4;
		}
        source = bufferseg;
    }
    else
    {
        source = (int32_t *) malloc(compressed);
        CHECKMALLOCRESULT(source);
        //read(grhandle,source,compressed);
	   
		for(i=0;i<compressed;i+=4)
		{
// evite des erreurs d'alignement de pointeur ?				
			source[j++]=Chunks[3] | (Chunks[2]<<8)|(Chunks[1]<<16) | (Chunks[0]<<24);
			Chunks+=4;
		}	  
		  
    }
	Chunks = NULL;
    CAL_ExpandGrChunk (chunk,source);

    if (compressed>BUFFERSIZE)
	{
        free(source);
		source = NULL;
	}
}



//==========================================================================

/*
======================
=
= CA_CacheScreen
=
= Decompresses a chunk from disk straight onto the screen
=
======================
*/
void CA_CacheScreen (int chunk)
{
    int32_t    pos,compressed,expanded;
    memptr  bigbufferseg;
    int32_t    *source;
    int             next;
//
// load the chunk into a buffer
//
    pos = GRFILEPOS(chunk);
    next = chunk +1;

    while (GRFILEPOS(next) == -1)           // skip past any sparse tiles
        next++;
    compressed = GRFILEPOS(next)-pos;
 //#ifdef VBT
//CA_CacheScreen (TITLEPIC);
    //lseek(grhandle,pos,SEEK_SET);
//    bigbufferseg=malloc(compressed);
//    CHECKMALLOCRESULT(bigbufferseg);
    //read(grhandle,bigbufferseg,compressed);

	uint8_t *Chunks;
	uint16_t delta;
	uint32_t delta2;
	delta = (uint16_t)(pos/2048);
	delta2 = (pos - delta*2048); 
	Chunks=(uint8_t*)0x002F0000;
	bigbufferseg=(uint8_t*)0x002E0000;
	GFS_Load(grhandle, delta, (void *)Chunks, compressed+delta2);
	memcpy(bigbufferseg,(const void *)&Chunks[delta2],compressed);
	Chunks = NULL;
	
    source = (int32_t *) bigbufferseg;

    expanded = *source++;
	expanded = SWAP_BYTES_32(expanded);

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
//    byte *pic = (byte *) malloc(64000);
	byte *pic = (byte *)0x002F0000;
    CAL_HuffExpand((byte *) source, (byte *)pic, expanded, grhuffman);
    byte *vbuf = LOCK();
//				byte *vbufptr= (byte *)vbuf;

    for(unsigned int y = 0; y < 200; y++)
    {
        for(unsigned int x = 0; x < 320; x++)
        {
            byte col = pic[(y * 80 + (x >> 2)) + (x & 3) * 80 * 200];
			byte *vbufptr= (byte *)(vbuf+(y* curPitch +x) *scaleFactor);

			for(unsigned i = 0; i < scaleFactor; i++)
			{
                for(unsigned j = 0; j < scaleFactor; j++)
					*vbufptr++=col;
				vbufptr+=curPitch;
			}
        }
    }
    UNLOCK();
	pic = NULL;
	bigbufferseg = NULL;
#ifdef HEAP_WALK	
heapWalk();
#endif	
//#endif
}

//==========================================================================

/*
======================
=
= CA_CacheMap
=
= WOLF: This is specialized for a 64*64 map size
=
======================
*/

void CA_CacheMap (int mapnum)
{
    int32_t   pos,compressed;
    int       plane;
    word     *dest;
    memptr    bigbufferseg;
    unsigned  size;
    byte     *source;
#ifdef CARMACIZED
    word     *buffer2seg;
    int32_t   expanded;
#endif

    mapon = mapnum;

//
// load the planes into the allready allocated buffers
//
    size = maparea*2;
	long fileSize = GetFileSize(maphandle);
	uint8_t *Chunks=(uint8_t*)SATURN_CHUNK_ADDR;   // écrase les sons

	GFS_Load(maphandle, 0, (void *)Chunks, fileSize);
	
    for (plane = 0; plane<MAPPLANES; plane++)
    {
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];
        dest = mapsegs[plane];

        //lseek(maphandle,pos,SEEK_SET);
        if (compressed<=BUFFERSIZE)
		{
            source = (byte *) bufferseg;
		}
        else
        {
            bigbufferseg=malloc(compressed);
            CHECKMALLOCRESULT(bigbufferseg);
            source = (byte *) bigbufferseg;
        }
		memcpy(source,&Chunks[pos],compressed);
        //read(maphandle,source,compressed);
#ifdef CARMACIZED


        //
        // unhuffman, then unRLEW
        // The huffman'd chunk has a two byte expanded length first
        // The resulting RLEW chunk also does, even though it's not really
        // needed
        //
        //expanded = *source;
		expanded = source[0] | (source[1] << 8);	
        source++;
		source++;
 //       buffer2seg = (word *) SATURN_CHUNK_ADDR-0X4000;
        buffer2seg = (word *) malloc(expanded);

        CHECKMALLOCRESULT(buffer2seg);
        CAL_CarmackExpand((byte *) source, buffer2seg,expanded);
		// VBT valeur perdue ?????
//	   RLEWtag=0xabcd;
        CA_RLEWexpand(buffer2seg+1,dest,size,RLEWtag);
       free(buffer2seg);
		buffer2seg = NULL;
#else
        //
        // unRLEW, skipping expanded length
        //
        CA_RLEWexpand (source+1,dest,size,RLEWtag);
#endif

        if (compressed>BUFFERSIZE)
		{
            free(bigbufferseg);
			bigbufferseg = NULL;
		}
    }
	Chunks = NULL;
}

//===========================================================================
/*
void CA_CannotOpen(const char *string)
{
    char str[30];

    strcpy(str,"Can't open ");
    strcat(str,string);
    strcat(str,"!\n");
    Quit (str);
}
*/
