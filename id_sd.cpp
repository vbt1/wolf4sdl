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
  #include "pcmsys.h"

#define ORIGSAMPLERATE 7042
extern int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
#ifdef PONY	
	
#else
extern PCM m_dat[4];
#endif

static Mix_Chunk *SoundChunks[ STARTMUSIC - STARTDIGISOUNDS];

//      Global variables
        boolean         //AdLibPresent,
                        SoundBlasterPresent,//SBProPresent,
                        SoundPositioned;
        SDSMode         DigiMode;
        int             DigiMap[LASTSOUND];

extern void	satPlayMusic( Uint8 track );
extern void	satStopMusic( void );

uintptr_t lowsound = (uintptr_t)0x002C0000;

void SD_PrepareSound(int which)
{
//	slPrint("SD_PrepareSound",slLocate(10,14));
//	slPrintHex(which,slLocate(1,15));
	Sint32 fileId;
	long fileSize;
	char filename[15];
	unsigned char *mem_buf;
	sprintf(filename,"%03d.PCM",which);
 	fileId = GFS_NameToId((Sint8*)filename);
#ifdef PONY
	if(fileId>0)
	{
		fileSize = GetFileSize(fileId);

//		if(which <23)
//		if(fileSize>8192 && fileSize<20000)
		{
			load_8bit_pcm((Sint8*)filename, ORIGSAMPLERATE);			
		} 
	}
	else
	{
		SoundChunks[which]->alen = 0;
	}	
#else
	if(fileId>0)
	{
		fileSize = GetFileSize(fileId);

		if(which <23)
//		if(fileSize>8192 && fileSize<20000)
		{
			mem_buf = (unsigned char *)malloc(fileSize);
			CHECKMALLOCRESULT(mem_buf);
		}
		else
		{
			mem_buf = (unsigned char *)lowsound;
			lowsound += (size_t)fileSize;
			
			if (lowsound % 4 != 0)
				lowsound = (lowsound + (4 - 1)) & -4;			
		}
		
		GFS_Load(fileId, 0, mem_buf, fileSize);
		SoundChunks[which] = (Mix_Chunk*)malloc(sizeof(Mix_Chunk));
		
		SoundChunks[which]->abuf = mem_buf;
		SoundChunks[which]->alen = fileSize;
		
		if (fileSize<0x900)
			SoundChunks[which]->alen = 0x900;  
	}
	else
	{
	   SoundChunks[which]->alen = 0;
	}
#endif	
}


boolean
SD_PlaySound(int sound)
{
#ifdef PONY
    if(Mix_PlayChannel(DigiMap[sound], NULL, 0) == -1)
#else
	Mix_Chunk *sample = SoundChunks[DigiMap[sound]];	 //DigiMap[sound]
    if(Mix_PlayChannel(0, sample, 0) == -1)
#endif	
    {
        return false;
    }
	return true;
}
word
SD_SoundPlaying(void)
{
#ifdef PONY	
	
#else
	unsigned char i;
	for(i=0;i<4;i++)
	{
		if(slPCMStat(&m_dat[i]))
		{
//			slSndFlush() ;
			//slSynch(); // vbt remis 26/05 // necessaire sinon reste planré à la fin du niveau
			return true;
		}
	}
	////slPrintHex(SoundMode,slLocate(10,3));
#endif
	
	return false;
}

void
SD_Startup(void)
{
//   AdLibPresent = false;
    SoundBlasterPresent = true;

//    alTimeCount = 0;

    //SD_SetSoundMode(sdm_PC);
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
/*
void
SD_StopDigitized(void)
{

}
*/
void
SD_StartMusic(int chunk)
{
//		slPrint((char *)"SD_StartMusic",slLocate(10,8));
	satPlayMusic(chunk);
}

void
SD_MusicOn(void)
{

}

void
SD_ContinueMusic(int chunk, int startoffs)
{
	satPlayMusic(chunk);
}

void
SD_WaitSoundDone(void)
{

}

/*
void
SD_SetDigiDevice(SDSMode mode)
{
	//slPrint("SD_SetDigiDevice empty",slLocate(2,19));	
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

boolean
SD_SetMusicMode(SMMode mode)
{
//slPrint("SD_SetMusicMode empty",slLocate(10,18));
return true;
}

void SD_SetPosition(int channel, int leftpos, int rightpos)
{

}

void
SD_PositionSound(int leftvol,int rightvol)
{

}
*/
