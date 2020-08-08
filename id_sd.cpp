//
//      ID Engine
//      ID_SD.c - Sound Manager for Wolfenstein 3D
//      v1.2
//      By Jason Blochowiak
//

//
//      This module handles dealing with generating sound on the appropriate
//              hardware
//
//      Depends on: User Mgr (for parm checking)
//
//      Globals:
//              For User Mgr:
//                      SoundBlasterPresent - SoundBlaster card present?
//                      AdLibPresent - AdLib card present?
//                      SoundMode - What device is used for sound effects
//                              (Use SM_SetSoundMode() to set)
//                      MusicMode - What device is used for music
//                              (Use SM_SetMusicMode() to set)
//                      DigiMode - What device is used for digitized sound effects
//                              (Use SM_SetDigiDevice() to set)
//
//              For Cache Mgr:
//                      NeedsDigitized - load digitized sounds?
//                      NeedsMusic - load music?
//

#include "wl_def.h"
//#include <SDL_mixer.h>
#include "fmopl.h"
#include <string.h>

#pragma hdrstop

#define ORIGSAMPLERATE 7042
extern int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
extern PCM m_dat[4];
typedef struct
{
	char RIFF[4];
	longword filelenminus8;
	char WAVE[4];
	char fmt_[4];
	longword formatlen;
	word val0x0001;
	word channels;
	longword samplerate;
	longword bytespersec;
	word bytespersample;
	word bitspersample;
} headchunk;

typedef struct
{
	char chunkid[4];
	longword chunklength;
} wavechunk;

typedef struct
{
    uint32_t startpage;
    uint32_t length;
} digiinfo;

static Mix_Chunk *SoundChunks[ STARTMUSIC - STARTDIGISOUNDS];
static byte      *SoundBuffers[STARTMUSIC - STARTDIGISOUNDS];

globalsoundpos channelSoundPos[MIX_CHANNELS];

//      Global variables
        boolean         //AdLibPresent,
                        SoundBlasterPresent,//SBProPresent,
                        SoundPositioned;
        SDMode          SoundMode;
        SMMode          MusicMode;
        SDSMode         DigiMode;
static  byte          **SoundTable;
        int             DigiMap[LASTSOUND];
        int             DigiChannel[STARTMUSIC - STARTDIGISOUNDS];

//      Internal variables
static  boolean                 SD_Started;
static  boolean                 nextsoundpos;
static  soundnames              SoundNumber;
static  soundnames              DigiNumber;
static  word                    SoundPriority;
static  word                    DigiPriority;
static  int                     LeftPosition;
static  int                     RightPosition;

        word                    NumDigi;
static  digiinfo               *DigiList;
static  boolean                 DigiPlaying;

//      PC Sound variables
static  volatile byte           pcLastSample;
static  byte * volatile         pcSound;
static  longword                pcLengthLeft;

//      AdLib variables
//static  byte * volatile         alSound;
//static  byte                    alBlock;
//static  longword                alLengthLeft;
//static  longword                alTimeCount;
//static  Instrument              alZeroInst;

//      Sequencer variables
//static  volatile boolean        sqActive;
//static  word                   *sqHack;
//static  word                   *sqHackPtr;
//static  int                     sqHackLen;
//static  int                     sqHackSeqLen;
//static  longword                sqHackTime;

extern void	satPlayMusic( Uint8 track );
extern void	satStopMusic( void );

Uint8 *lowsound = (Uint8 *)0x002C0000;

void SD_PrepareSound(int which)
{
	//slPrint("SD_PrepareSound",slLocate(10,14));
	//slPrintHex(which,slLocate(1,15));

	Sint32 i, fileId;
	long fileSize;
	char filename[15];
	unsigned char *mem_buf;
	sprintf(filename,"%03d.PCM",which);
	//slPrint("                                    ",slLocate(2,21));
	//slPrint(filename,slLocate(10,21));
	
 	fileId = GFS_NameToId((Sint8*)filename);

	if(fileId>0)
	{
		fileSize = GetFileSize(fileId);

		if(which <16)
		{
			mem_buf = (unsigned char *)malloc(fileSize);
		}
		else
		{
			mem_buf = (unsigned char *)(lowsound);
			*lowsound = (*lowsound + fileSize);
		} 
		GFS_Load(fileId, 0, mem_buf, fileSize);
		SoundChunks[which] = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
		SoundChunks[which]->abuf = mem_buf;
		SoundChunks[which]->alen = fileSize;
		if (fileSize<0x900)
		SoundChunks[which]->alen = 0x900;  
	//slPrint("size std",slLocate(50,21));	
	}
	else
	{
		   SoundChunks[which]->alen = 0;
		   	//slPrint("size zero",slLocate(50,21));
	}
	//while(1);
	
	//slPrint("                                    ",slLocate(2,21));	
}


boolean
SD_PlaySound(soundnames sound)
{
	////slPrint("SD_PlaySound",slLocate(10,16));
	////slPrintHex(DigiMap[sound],slLocate(23,16));
    //Mix_Chunk *sample = SoundChunks[DigiMap[sound]];	 //DigiMap[sound]
	Mix_Chunk *sample = SoundChunks[DigiMap[sound]];	 //DigiMap[sound]
    if(Mix_PlayChannel(0, sample, 0) == -1)
    {
        ////slPrint("Unable to play sound:", slLocate(10,19));
        return false;
    }
	return true;
}
word
SD_SoundPlaying(void)
{
	unsigned int i;
	for(i=0;i<4;i++)
	{
		if(slPCMStat(&m_dat[i]))
		{
//			slSynch(); // vbt remis 26/05
			return true;
		}
	}
	////slPrintHex(SoundMode,slLocate(10,3));
	return false;
}
void
SD_SetDigiDevice(SDSMode mode)
{
	//slPrint("SD_SetDigiDevice empty",slLocate(2,19));	
}

void
SD_Startup(void)
{
//   AdLibPresent = false;
    SoundBlasterPresent = true;

//    alTimeCount = 0;

    SD_SetSoundMode(sdm_PC);
    //SD_SetMusicMode(sdm_PC);

  SDL_AudioSpec desired, obtained;
  desired.freq = ORIGSAMPLERATE;
  desired.format = AUDIO_U8;
  desired.channels = 1;
  desired.samples = 1024;
  desired.userdata = NULL;

  SDL_OpenAudio(&desired, &obtained);
}

void
SD_Shutdown(void)
{

}

int
SD_MusicOff(void)
{
	//satStopMusic();
	return 0;
}

void
SD_StopSound(void)
{

}

void
SD_StopDigitized(void)
{

}

void
SD_StartMusic(int chunk)
{
//	slPrint((char *)"SD_StartMusic",slLocate(10,8));
	satPlayMusic(chunk);
}

void
SD_MusicOn(void)
{

}

boolean
SD_SetMusicMode(SMMode mode)
{
//slPrint("SD_SetMusicMode empty",slLocate(10,18));
return true;
}

void
SD_ContinueMusic(int chunk, int startoffs)
{
	satPlayMusic(chunk);
}

boolean
SD_SetSoundMode(SDMode mode)
{
	//slPrint("SD_SetSoundMode start",slLocate(2,18));	
	
    boolean result = false;
    word    tableoffset;
	//slPrint("SD_StopSound       ",slLocate(2,18));	

    SD_StopSound();

//    if ((mode == sdm_AdLib) && !AdLibPresent)
//        mode = sdm_PC;

    switch (mode)
    {
        case sdm_Off:
            tableoffset = STARTADLIBSOUNDS;
            result = true;
            break;
        case sdm_PC:
            tableoffset = STARTPCSOUNDS;
            result = true;
            break;
//        case sdm_AdLib:
//            tableoffset = STARTADLIBSOUNDS;
//            if (AdLibPresent)
//                result = true;
//            break;
        default:
            Quit("SD_SetSoundMode: Invalid sound mode %i", mode);
            return false;
    }
//    SoundTable = &audiosegs[tableoffset];

    if (result && (mode != SoundMode))
    {
        //SDL_ShutDevice();
        SoundMode = mode;
        //SDL_StartDevice();
    }
	//slPrint("SD_SetSoundMode end",slLocate(2,18));	

    return(result);

}

void
SD_WaitSoundDone(void)
{

}

void SD_SetPosition(int channel, int leftpos, int rightpos)
{

}

void
SD_PositionSound(int leftvol,int rightvol)
{

}
