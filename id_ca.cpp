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
#ifndef EMBEDDED
static maptype* mapheaderseg[NUMMAPS];
#else
static maptype mapheaderseg;
#endif
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
static const char gheadname[] = "VGAHEAD.";
static const char gfilename[] = "VGAGRAPH.";
static const char gdictname[] = "VGADICT.";
static const char mheadname[] = "MAPHEAD.";
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

static long   grhandle = -1;               // handle to EGAGRAPH
static long   maphandle = -1;              // handle to MAPTEMP / GAMEMAPS
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
	Chunks=(uint8_t*)saturnChunk;
	
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, sizeof(chunkexplen)+delta2); // lecture partielle ok
	memcpy(&chunkexplen,&Chunks[delta2],sizeof(chunkexplen));
//	Chunks = NULL;
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
                *outptr++ = ch;
                length--;
            }
            else
            {
                offset = *inptr++;
                copyptr = outptr - offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else if (chhigh == FARTAG)
        {
            count = ch&0xff;
            if (!count)
            {                               // have to insert a word containing the tag byte
                ch |= *inptr++;
                *outptr++ = ch;
                length --;
            }
            else
            {
                offset = READWORD(inptr);
                copyptr = dest + offset;
                length -= count;
                if(length<0) return;
                while (count--)
                    *outptr++ = *copyptr++;
            }
        }
        else
        {
            *outptr++ = ch;
            length --;
        }
    }
}

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
//	unsigned int j=0;
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
/*
	while (fname[j])
	{
		fname[j]= toupper(fname[j]);
		j++;
	}*/
//	i=0;

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	fileId = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(fileId);
	compseg=(Uint8 *)saturnChunk+0x8000;
//	CHECKMALLOCRESULT(vbtHuff);
//slPrint((char *)"CAL_SetupGrFile1     ",slLocate(10,12));
	GFS_Load(fileId, 0, (void *)compseg, sizeof(grhuffman)); // lecture VGADICT full
	huffnode *grhuffmanptr = (huffnode *)grhuffman;
	
	for(unsigned char x=0;x<255;x++)
	{
//	slPrint((char *)"CAL_SetupGrFile2     ",slLocate(10,12));	
		grhuffmanptr->bit0=compseg[0] | (compseg[1]<<8);
		grhuffmanptr->bit1=compseg[2] | (compseg[3]<<8);
		grhuffmanptr++;
		compseg+=4;
	}	
//	j=0;

    //read(handle, grhuffman, sizeof(grhuffman));
    //close(handle);

    // load the data offsets from ???head.ext
    strcpy(fname,gheadname);
    strcat(fname,extension);
/*
	while (fname[j])
	{
		fname[j]= toupper(fname[j]);
		j++;
	}
	j=0;*/

    //handle = open(fname, O_RDONLY | O_BINARY);
    //if (handle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);
//slSynch();
//slPrint((char *)"CAL_SetupGrFile3     ",slLocate(10,12));
	fileId = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(fileId);
    long headersize = fileSize;//lseek(handle, 0, SEEK_END);
    //lseek(handle, 0, SEEK_SET);
//headersize= 157*3;
#if 0
    if(headersize / 3 != (long) (lengthof(grstarts) - numEpisodesMissing))
	{
/*        Quit("Wolf4SDL was not compiled for these data files:\n"
            "        %s contains a wrong number of offsets                          (%i instead of %i)!\n\n"
            "                          Please check whether you are using the right executable!\n"
            "(For mod developers: perhaps you forgot to update NUMCHUNKS?)",
            fname, headersize / 3, lengthof(grstarts) - numEpisodesMissing);
*/
			char msg[200];
			sprintf(msg,"Wolf4SDL was not compiled for these data files:\n"
            "        %s contains a wrong number of offsets                          (%i instead of %i)!\n\n"
            "                          Please check whether you are using the right executable!\n"
            "(For mod developers: perhaps you forgot to update NUMCHUNKS?)",
            fname, headersize / 3, lengthof(grstarts) - numEpisodesMissing);
			
			slPrint((char *)msg,slLocate(1,3));
						while(1);
	}
#endif	
//slPrint((char *)"CAL_SetupGrFile4     ",slLocate(10,12));
    byte data[lengthof(grstarts) * 3];
	GFS_Load(fileId, 0, (void *)data, sizeof(data)); // lecture de VGAHEAD full
    //read(handle, data, sizeof(data));
    //close(handle);

    const byte* d = data;
    for (int32_t* i = grstarts; i != endof(grstarts); ++i)
    {
//	slPrint((char *)"CAL_SetupGrFile5     ",slLocate(10,12));	
        const int32_t val = d[0] | d[1] << 8 | d[2] << 16;
        *i = (val == 0x00FFFFFF ? -1 : val);
        d += 3;
    }

#endif
//slPrint((char *)"CAL_SetupGrFile6     ",slLocate(10,12));
//
// Open the graphics file, leaving it open until the game is finished
//
    strcpy(fname,gfilename);
    strcat(fname,extension);
/*
	while (fname[j])
	{
		fname[j]= toupper(fname[j]);
		j++;
	}
	j=0;*/

    //grhandle = open(fname, O_RDONLY | O_BINARY);
    //if (grhandle == -1)
//	if(stat(fname, NULL))
//        CA_CannotOpen(fname);

	grhandle = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(grhandle);
//
// load the pic and sprite headers into the arrays in the data segment
//
slPrint((char *)"CAL_SetupGrFile7     ",slLocate(10,12));
    pictable=(pictabletype *) malloc(NUMPICS*sizeof(pictabletype));
    CHECKMALLOCRESULT(pictable);
    int32_t   chunkcomplen = CAL_GetGrChunkLength(STRUCTPIC);                // position file pointer
//	compseg =(byte*)malloc((chunkcomplen));
	compseg =(byte*)saturnChunk;
//	CHECKMALLOCRESULT(compseg);
	GFS_Load(grhandle, 0, (void *)compseg, (chunkcomplen));
//    CAL_HuffExpand(&compseg[4], (byte*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);
slPrint((char *)"CAL_SetupGrFile8     ",slLocate(10,12));
    CAL_HuffExpand(&compseg[4], (byte*)pictable, NUMPICS * sizeof(pictabletype), grhuffman);
//slPrint((char *)"CAL_SetupGrFile9     ",slLocate(10,12));
	for(unsigned long k=0;k<NUMPICS;k++)
	{
slPrint((char *)"CAL_SetupGrFile10     ",slLocate(10,12));		
		pictable[k].height=SWAP_BYTES_16(pictable[k].height);
		pictable[k].width=SWAP_BYTES_16(pictable[k].width);
	} 
slPrint((char *)"CAL_SetupGrFile11     ",slLocate(10,12));	
	compseg = NULL;
	// VBT correct
}

//==========================================================================
long CAL_SetupMapFile (int mapnum)
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
/*	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}	 
	i=0;*/
	fileId = GFS_NameToId((Sint8*)fname);
//	fileSize = GetFileSize(fileId); // utile
    length = NUMMAPS*4+2; // used to be "filelength(handle);"
//    mapfiletype *tinf=(mapfiletype *) malloc(sizeof(mapfiletype));
	mapfiletype *tinf=(mapfiletype *)saturnChunk;
	GFS_Load(fileId, 0, (void *)tinf, length);
    //read(handle, tinf, length);

    tinf->RLEWtag=SWAP_BYTES_16(tinf->RLEWtag);
	tinf->headeroffsets[mapnum]=SWAP_BYTES_32(tinf->headeroffsets[mapnum]);
    RLEWtag=tinf->RLEWtag;
	i=0;
//
// open the data file
//
#ifdef CARMACIZED
    strcpy(fname, "GAMEMAPS.");
    strcat(fname, extension);

/*	while (fname[i])
	{
		fname[i]= toupper(fname[i]);
		i++;
	}*/
	maphandle = GFS_NameToId((Sint8*)fname);
	fileSize = GetFileSize(maphandle);
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
	uint8_t *maphandleptr = (uint8_t*)(((int)saturnChunk+sizeof(mapfiletype)+ (4 - 1)) & -4);
	GFS_Load(maphandle, 0, (void *)maphandleptr, fileSize); // lecture GAMEMAPS ou MAPHEAD

//slPrintHex(fileSize,slLocate(10,14));

	pos = tinf->headeroffsets[mapnum];
	if (pos<0)                          // $FFFFFFFF start is a sparse map
		return fileSize;
//	if(mapheaderseg[mapnum]==NULL)	mapheaderseg[mapnum]=(maptype *) malloc(sizeof(maptype));
#ifndef EMBEDDED
	mapheaderseg[mapnum]=(maptype *) ((saturnChunk+sizeof(mapfiletype)+fileSize + (8 - 1)) & -4);
//	CHECKMALLOCRESULT(mapheaderseg[mapnum]);
	//read (maphandle,(memptr)mapheaderseg[i],sizeof(maptype));
	memcpy((memptr)mapheaderseg[mapnum],&maphandleptr[pos],sizeof(maptype));

//
// allocate space for 3 64*64 planes
//
    for (i=0;i<MAPPLANES;i++)
    {
		mapheaderseg[mapnum]->planestart[i]=SWAP_BYTES_32(mapheaderseg[mapnum]->planestart[i]);
		mapheaderseg[mapnum]->planelength[i]=SWAP_BYTES_16(mapheaderseg[mapnum]->planelength[i]);		
		mapsegs[i]=(word *)SATURN_MAPSEG_ADDR+(0x2000*i);
//		mapsegs[i]=(word *) malloc(maparea*2);
    }
	mapheaderseg[mapnum]->width=SWAP_BYTES_16(mapheaderseg[mapnum]->width);
	mapheaderseg[mapnum]->height=SWAP_BYTES_16(mapheaderseg[mapnum]->height);	
#else
	memcpy((memptr)&mapheaderseg,&maphandleptr[pos],sizeof(maptype));
	
//
// allocate space for 3 64*64 planes
//
    for (i=0;i<MAPPLANES;i++)
    {
		mapheaderseg.planestart[i]=SWAP_BYTES_32(mapheaderseg.planestart[i]);
		mapheaderseg.planelength[i]=SWAP_BYTES_16(mapheaderseg.planelength[i]);		
//		mapsegs[i]=(word *)SATURN_MAPSEG_ADDR+(0x2000*i);
		if(mapsegs[i]==NULL) mapsegs[i]=(word *) malloc(maparea*2);
//		mapsegs[i]=(word *) malloc(maparea*2);
    }
	mapheaderseg.width=SWAP_BYTES_16(mapheaderseg.width);
	mapheaderseg.height=SWAP_BYTES_16(mapheaderseg.height);	
#endif


	maphandleptr = NULL;	
	tinf = NULL;
	
	return fileSize;
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

//    CAL_SetupMapFile ();
    CAL_SetupGrFile ();
    //CAL_SetupAudioFile ();

    mapon = -1;
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
	if(grsegs[chunk]==NULL)	grsegs[chunk]=(byte *) malloc(expanded);
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
		
	Chunks=(uint8_t*)saturnChunk;  // d?plac? pour pas ?craser de son
//	CHECKMALLOCRESULT(Chunks);
	GFS_Load(grhandle, delta, (void *)Chunks, compressed+delta2); // lecture partielle ok
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

	Chunks=(uint8_t*)saturnChunk+0x4000;
	bigbufferseg=(uint8_t*)screen->pixels;
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
	byte *pic = (byte *)saturnChunk;
    CAL_HuffExpand((byte *) source, (byte *)pic, expanded, grhuffman);

	byte *vbuf = LOCK();

    for(unsigned int y = 0; y < 200; y++)
    {
        for(unsigned int x = 0; x < 320; x++)
        {
            byte col = pic[(y * 80 + (x >> 2)) + (x & 3) * 80 * 200];
			byte *vbufptr= (byte *)(vbuf+(y* curPitch +x) *scaleFactor)+16;

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

	long fileSize = CAL_SetupMapFile(mapnum);
//
// load the planes into the allready allocated buffers
//
    size = maparea*2;
	uint8_t *Chunks=(uint8_t*)saturnChunk;   // ?crase les sons
//	uint8_t *Chunks=(uint8_t*)malloc(fileSize);   // ?crase les sons
//slPrintHex(maphandle,slLocate(10,19));
//slPrintHex(fileSize,slLocate(10,20));

	GFS_Load(maphandle, 0, (void *)Chunks, fileSize);
	
    for (plane = 0; plane<MAPPLANES; plane++)
    {
#ifndef EMBEDDED		
        pos = mapheaderseg[mapnum]->planestart[plane];
        compressed = mapheaderseg[mapnum]->planelength[plane];
#else
        pos = mapheaderseg.planestart[plane];
        compressed = mapheaderseg.planelength[plane];
#endif
        dest = mapsegs[plane];

        //lseek(maphandle,pos,SEEK_SET);
        if (compressed<=BUFFERSIZE)
		{
            source = (byte *) bufferseg;
		}
        else
        {
//            bigbufferseg=malloc(compressed);
//            CHECKMALLOCRESULT(bigbufferseg);
//            source = (byte *) bigbufferseg;
			  source = (byte *) saturnChunk+0X8000;
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
		source+=2;
//      source++;
//		source++;
//       buffer2seg = (word *) (SATURN_MAPSEG_ADDR-0X8000);
        buffer2seg = (word *) malloc(expanded);
		
//	int *val = (int *)buffer2seg;		
//slPrintHex((int)val,slLocate(10,21));
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

//        if (compressed>BUFFERSIZE)
		if(bigbufferseg!=NULL)
		{
            free(bigbufferseg);
			bigbufferseg = NULL;
		}
    }
//	free(mapheaderseg[mapnum]);
#ifndef EMBEDDED
	mapheaderseg[mapnum]=NULL;
#endif	
//	free(Chunks);
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
