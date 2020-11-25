/*
** jl_audio.c - contains functions to handle sound in Jupiter Lander
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** DEFINES **/

#define CLOCK 3579545


/** PROTOTYPES **/

int setup_audio(void);
void cleanup_audio(void);

void play_start_music(void);

void play_crash_sound(void);
void end_crash_sound(void);

void play_pop_sound(void);
void end_pop_sound(void);

void begin_main_thruster_sound(void);
void end_main_thruster_sound(void);

void begin_left_thruster_sound(void);
void end_left_thruster_sound(void);

void begin_right_thruster_sound(void);
void end_right_thruster_sound(void);


/** GLOBAL VARIABLES **/

extern struct Library *DOSBase;

struct IOAudio *AudioIO1;  /* left */
struct IOAudio *AudioIO2;  /* right */
struct IOAudio *AudioIO3;  /* start/stop */
struct IOAudio *AudioIO4;  /* left */
struct IOAudio *AudioIO5;  /* right */

struct MsgPort *AudioMP1;
struct MsgPort *AudioMP2;
struct MsgPort *AudioMP3;
struct MsgPort *AudioMP4;
struct MsgPort *AudioMP5;

int audio_flag = FALSE;

extern UWORD chip start_tune_a[11919];
extern UWORD chip start_tune_b[344];
extern UBYTE chip crash_sound[9716];
extern UBYTE chip tick_sound[760];
extern UBYTE chip main_thruster_sound[3028];
extern UBYTE chip side_thruster_sound[2958];



/*
** setup_audio()
**
** Setup and open audio.device for game sounds.
**
** RETURNS:  TRUE if successful (and sets audio_flag TRUE),
**           FALSE if unsuccessful (and sets audio_flag FALSE).
*/
int setup_audio()
{
   ULONG device;
   UBYTE channels[] = { 15, 14, 13 };
   
   
   AudioIO1 = (struct IOAudio *)AllocMem(sizeof(struct IOAudio),
               MEMF_PUBLIC | MEMF_CLEAR);
   if (AudioIO1 != NULL)
   {
      AudioIO2 = (struct IOAudio *)AllocMem(sizeof(struct IOAudio),
                  MEMF_PUBLIC | MEMF_CLEAR);
      if (AudioIO2 != NULL)
      {
         AudioIO3 = (struct IOAudio *)AllocMem(sizeof(struct IOAudio),
                     MEMF_PUBLIC | MEMF_CLEAR);
         if (AudioIO3 != NULL)
         {
            AudioIO4 = (struct IOAudio *)AllocMem(sizeof(struct IOAudio),
                        MEMF_PUBLIC | MEMF_CLEAR);
            if (AudioIO4 != NULL)
            {
               AudioIO5 = (struct IOAudio *)AllocMem(sizeof(struct IOAudio),
                           MEMF_PUBLIC | MEMF_CLEAR);
               if (AudioIO5 != NULL)
               {
                  AudioMP1 = CreatePort(0,0);
                  if (AudioMP1 != NULL)
                  {
                     AudioMP2 = CreatePort(0,0);
                     if (AudioMP2 != NULL)
                     {
                        AudioMP3 = CreatePort(0,0);
                        if (AudioMP3 != NULL)
                        {
                           AudioMP4 = CreatePort(0,0);
                           if (AudioMP4 != NULL)
                           {
                              AudioMP5 = CreatePort(0,0);
                              if (AudioMP5 != NULL)
                              {
                                 AudioIO1->ioa_Request.io_Message.mn_ReplyPort = AudioMP1;
                                 AudioIO1->ioa_Request.io_Message.mn_Node.ln_Pri = 100;
                                 AudioIO1->ioa_Request.io_Command = ADCMD_ALLOCATE;
                                 AudioIO1->ioa_Request.io_Flags = ADIOF_NOWAIT;
                                 AudioIO1->ioa_AllocKey = 0;
                                 AudioIO1->ioa_Data = channels;
                                 AudioIO1->ioa_Length = sizeof(channels);
                                 
                                 device = OpenDevice(AUDIONAME, 0L, (struct IORequest *)AudioIO1, 0L);
                                 
                                 if (device == NULL)
                                 {
                                    audio_flag = TRUE;   /* audio.device opened */
                                    
                                    /* create a 2nd - 5th IOAudio structure w/diff ports */
                                    *AudioIO2 = *AudioIO1;
                                    *AudioIO3 = *AudioIO1;
                                    *AudioIO4 = *AudioIO1;
                                    *AudioIO5 = *AudioIO1;
                                    AudioIO2->ioa_Request.io_Message.mn_ReplyPort = AudioMP2;
                                    AudioIO3->ioa_Request.io_Message.mn_ReplyPort = AudioMP3;
                                    AudioIO4->ioa_Request.io_Message.mn_ReplyPort = AudioMP4;
                                    AudioIO5->ioa_Request.io_Message.mn_ReplyPort = AudioMP5;
                                    /* */
                                    
                                    return TRUE;
                                 }
                                 DeletePort(AudioMP5);
                              }
                              DeletePort(AudioMP4);
                           }
                           DeletePort(AudioMP3);
                        }
                        DeletePort(AudioMP2);
                     }
                     DeletePort(AudioMP1);
                  }
                  FreeMem(AudioIO5, sizeof(struct IOAudio));
               }
               FreeMem(AudioIO4, sizeof(struct IOAudio));
            }
            FreeMem(AudioIO3, sizeof(struct IOAudio));
         }
         FreeMem(AudioIO2, sizeof(struct IOAudio));
      }
      FreeMem(AudioIO1, sizeof(struct IOAudio));
   }
   audio_flag = FALSE;
   return FALSE;
}


/*
** cleanup_audio()
**
** Free everything allocated by setup_audio & close audio.device if
** audio_flag = TRUE (if audio.device was opened).
*/
void cleanup_audio()
{
   
   if (audio_flag == TRUE)
   {
      /* free everything */
      CloseDevice((struct IORequest *)AudioIO1);
      DeletePort(AudioMP1);
      DeletePort(AudioMP2);
      DeletePort(AudioMP3);
      DeletePort(AudioMP4);
      DeletePort(AudioMP5);
      FreeMem(AudioIO1, sizeof(struct IOAudio));
      FreeMem(AudioIO2, sizeof(struct IOAudio));
      FreeMem(AudioIO3, sizeof(struct IOAudio));
      FreeMem(AudioIO4, sizeof(struct IOAudio));
      FreeMem(AudioIO5, sizeof(struct IOAudio));
      /* */
   }
}


/*
** play_start_music()
**
** If audio_flag = TRUE, play the tune for the beginning of each game,
** else delay for 3 seconds and return.
*/
void play_start_music()
{
   struct Message *AudioMSG;
   
   int x, pause[12] = { 7, 22, 18, 14, 11, 7, 3, 3, 3, 3, 3, 3 };
   
   
   if (audio_flag == TRUE)
   {
      /* play the first part of the starting tune */
      
      /* stop all sound so we can set up for tune */
      AudioIO3->ioa_Request.io_Command = CMD_STOP;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
      
      /* set up to play first part of tune (left channel) */
      AudioIO1->ioa_Request.io_Command = CMD_WRITE;
      AudioIO1->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO1->ioa_Request.io_Unit = (struct Unit *)0x01;
      AudioIO1->ioa_Data = (UBYTE *)&start_tune_a;
      AudioIO1->ioa_Length = sizeof(start_tune_a);
      AudioIO1->ioa_Period = CLOCK/8000;
      AudioIO1->ioa_Volume = 64;
      AudioIO1->ioa_Cycles = 2;
      
      BeginIO((struct IORequest *)AudioIO1);
      /* */
      
      /* set up to play first part of tune (right channel) */
      AudioIO2->ioa_Request.io_Command = CMD_WRITE;
      AudioIO2->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO2->ioa_Request.io_Unit = (struct Unit *)0x02;
      AudioIO2->ioa_Data = (UBYTE *)&start_tune_a;
      AudioIO2->ioa_Length = sizeof(start_tune_a);
      AudioIO2->ioa_Period = CLOCK/8000;
      AudioIO2->ioa_Volume = 64;
      AudioIO2->ioa_Cycles = 2;
      
      BeginIO((struct IORequest *)AudioIO2);
      /* */
      
      /* start sound again so we can hear this */
      AudioIO3->ioa_Request.io_Command = CMD_START;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
      
      /* wait for first part of tune to finish */
      WaitPort(AudioMP1);
      AudioMSG = GetMsg(AudioMP1);
      WaitPort(AudioMP2);
      AudioMSG = GetMsg(AudioMP2);
      /* */
      
      /* */
      
      /* play the end part of the tune */
      for (x=0; x<12; x++)
      {
         /* put delay in between each beep */
         Delay(pause[x]);
         
         /* stop all sound so we can set up for tune */
         AudioIO3->ioa_Request.io_Command = CMD_STOP;
         AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
         AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
         
         BeginIO((struct IORequest *)AudioIO3);
         /* */
         
         /* set up to play first part of tune (left channel) */
         AudioIO1->ioa_Request.io_Command = CMD_WRITE;
         AudioIO1->ioa_Request.io_Flags = ADIOF_PERVOL;
         AudioIO1->ioa_Request.io_Unit = (struct Unit *)0x01;
         AudioIO1->ioa_Data = (UBYTE *)&start_tune_b;
         AudioIO1->ioa_Length = sizeof(start_tune_b);
         AudioIO1->ioa_Period = CLOCK/8000;
         AudioIO1->ioa_Volume = 64;
         AudioIO1->ioa_Cycles = 1;
         
         BeginIO((struct IORequest *)AudioIO1);
         /* */
         
         /* set up to play first part of tune (right channel) */
         AudioIO2->ioa_Request.io_Command = CMD_WRITE;
         AudioIO2->ioa_Request.io_Flags = ADIOF_PERVOL;
         AudioIO2->ioa_Request.io_Unit = (struct Unit *)0x02;
         AudioIO2->ioa_Data = (UBYTE *)&start_tune_b;
         AudioIO2->ioa_Length = sizeof(start_tune_b);
         AudioIO2->ioa_Period = CLOCK/8000;
         AudioIO2->ioa_Volume = 64;
         AudioIO2->ioa_Cycles = 1;
         
         BeginIO((struct IORequest *)AudioIO2);
         /* */
         
         /* start sound again so we can hear this */
         AudioIO3->ioa_Request.io_Command = CMD_START;
         AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
         AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
         
         BeginIO((struct IORequest *)AudioIO3);
         /* */
         
         /* wait for first part of tune to finish */
         WaitPort(AudioMP1);
         AudioMSG = GetMsg(AudioMP1);
         WaitPort(AudioMP2);
         AudioMSG = GetMsg(AudioMP2);
         /* */
      }
      /* */
   }
   else
   {
      Delay(150);
   }
}


/*
** play_crash_sound()
**
** If audio_flag = TRUE, play the probe crash sound, else return.
*/
void play_crash_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop all sound so we can set up for crash sound */
      AudioIO3->ioa_Request.io_Command = CMD_STOP;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
      
      /* set up to play crash on left channel */
      AudioIO1->ioa_Request.io_Command = CMD_WRITE;
      AudioIO1->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO1->ioa_Request.io_Unit = (struct Unit *)0x01;
      AudioIO1->ioa_Data = (UBYTE *)&crash_sound;
      AudioIO1->ioa_Length = sizeof(crash_sound);
      AudioIO1->ioa_Period = CLOCK/7000;
      AudioIO1->ioa_Volume = 64;
      AudioIO1->ioa_Cycles = 1;
      
      BeginIO((struct IORequest *)AudioIO1);
      /* */
      
      /* set up to play crash on right channel */
      AudioIO2->ioa_Request.io_Command = CMD_WRITE;
      AudioIO2->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO2->ioa_Request.io_Unit = (struct Unit *)0x02;
      AudioIO2->ioa_Data = (UBYTE *)&crash_sound;
      AudioIO2->ioa_Length = sizeof(crash_sound);
      AudioIO2->ioa_Period = CLOCK/7000;
      AudioIO2->ioa_Volume = 64;
      AudioIO2->ioa_Cycles = 1;
      
      BeginIO((struct IORequest *)AudioIO2);
      /* */
      
      /* start sound again so we can hear this */
      AudioIO3->ioa_Request.io_Command = CMD_START;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}


/*
** end_crash_sound()
**
** If audio_flag = TRUE, wait for probe crash sound to end, else return.
*/
void end_crash_sound()
{
   struct Message *AudioMSG;
   
   if (audio_flag == TRUE)
   {
      WaitPort(AudioMP1);
      AudioMSG = GetMsg(AudioMP1);
      WaitPort(AudioMP2);
      AudioMSG = GetMsg(AudioMP2);
   }
}


/*
** play_pop_sound()
**
** If audio_flag = TRUE, begin playing the pop sound of the score counter,
** else return.
*/
void play_pop_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop all sound so we can set up for pop */
      AudioIO3->ioa_Request.io_Command = CMD_STOP;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
      
      /* set up to play the pop (left channel) */
      AudioIO1->ioa_Request.io_Command = CMD_WRITE;
      AudioIO1->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO1->ioa_Request.io_Unit = (struct Unit *)0x01;
      AudioIO1->ioa_Data = (UBYTE *)&tick_sound;
      AudioIO1->ioa_Length = sizeof(tick_sound);
      AudioIO1->ioa_Period = CLOCK/8000;
      AudioIO1->ioa_Volume = 64;
      AudioIO1->ioa_Cycles = 0;
      
      BeginIO((struct IORequest *)AudioIO1);
      /* */
      
      /* set up to play the pop (right channel) */
      AudioIO2->ioa_Request.io_Command = CMD_WRITE;
      AudioIO2->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO2->ioa_Request.io_Unit = (struct Unit *)0x02;
      AudioIO2->ioa_Data = (UBYTE *)&tick_sound;
      AudioIO2->ioa_Length = sizeof(tick_sound);
      AudioIO2->ioa_Period = CLOCK/8000;
      AudioIO2->ioa_Volume = 64;
      AudioIO2->ioa_Cycles = 0;
      
      BeginIO((struct IORequest *)AudioIO2);
      /* */
      
      /* start sound again so we can hear this */
      AudioIO3->ioa_Request.io_Command = CMD_START;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}


/*
** end_pop_sound()
**
** Finish up with popping sounds.
*/
void end_pop_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop the counter ticking sound */
      AudioIO3->ioa_Request.io_Command = ADCMD_FINISH;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}


/*
** begin_main_thruster_sound()
**
** If audio_flag = TRUE, begin playing the main thruster sound of probe,
** else return.
*/
void begin_main_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop channels 1 & 2 to set up for sound */
      AudioIO3->ioa_Request.io_Command = CMD_STOP;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
      
      /* setup left channel */
      AudioIO1->ioa_Request.io_Command = CMD_WRITE;
      AudioIO1->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO1->ioa_Request.io_Unit = (struct Unit *)0x01;
      AudioIO1->ioa_Data = (UBYTE *)&main_thruster_sound;
      AudioIO1->ioa_Length = sizeof(main_thruster_sound);
      AudioIO1->ioa_Period = CLOCK/8000;
      AudioIO1->ioa_Volume = 64;
      AudioIO1->ioa_Cycles = 0;  /* repeat */
      
      BeginIO((struct IORequest *)AudioIO1);
      /* */
      
      /* setup right channel */
      AudioIO2->ioa_Request.io_Command = CMD_WRITE;
      AudioIO2->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO2->ioa_Request.io_Unit = (struct Unit *)0x02;
      AudioIO2->ioa_Data = (UBYTE *)&main_thruster_sound;
      AudioIO2->ioa_Length = sizeof(main_thruster_sound);
      AudioIO2->ioa_Period = CLOCK/8000;
      AudioIO2->ioa_Volume = 64;
      AudioIO2->ioa_Cycles = 0;  /* repeat */
      
      BeginIO((struct IORequest *)AudioIO2);
      /* */
      
      /* start channels 1 & 2 */
      AudioIO3->ioa_Request.io_Command = CMD_START;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}


/*
** end_main_thruster_sound()
**
** Finish main thruster sound.
*/
void end_main_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop the main thruster sound (channels 1 & 2) */
      AudioIO3->ioa_Request.io_Command = ADCMD_FINISH;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x03;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}


/*
** begin_left_thruster_sound()
**
** If audio_flag = TRUE, begin playing the side thruster sound of probe
** through the a left channel, else return.
*/
void begin_left_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* setup left channel */
      AudioIO4->ioa_Request.io_Command = CMD_WRITE;
      AudioIO4->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO4->ioa_Request.io_Unit = (struct Unit *)0x08;
      AudioIO4->ioa_Data = (UBYTE *)&side_thruster_sound;
      AudioIO4->ioa_Length = sizeof(side_thruster_sound);
      AudioIO4->ioa_Period = CLOCK/8000;
      AudioIO4->ioa_Volume = 64;
      AudioIO4->ioa_Cycles = 0;  /* repeat */
      
      BeginIO((struct IORequest *)AudioIO4);
      /* */
   }
}


/*
** end_left_thruster_sound()
**
** Finish side thruster sound in left channel.
*/
void end_left_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop the main thruster sound (channels 1 & 2) */
      AudioIO3->ioa_Request.io_Command = ADCMD_FINISH;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x08;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}



/*
** begin_right_thruster_sound()
**
** If audio_flag = TRUE, begin playing the side thruster sound of probe
** through the a right channel, else return.
*/
void begin_right_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* setup left channel */
      AudioIO5->ioa_Request.io_Command = CMD_WRITE;
      AudioIO5->ioa_Request.io_Flags = ADIOF_PERVOL;
      AudioIO5->ioa_Request.io_Unit = (struct Unit *)0x04;
      AudioIO5->ioa_Data = (UBYTE *)&side_thruster_sound;
      AudioIO5->ioa_Length = sizeof(side_thruster_sound);
      AudioIO5->ioa_Period = CLOCK/8000;
      AudioIO5->ioa_Volume = 64;
      AudioIO5->ioa_Cycles = 0;  /* repeat */
      
      BeginIO((struct IORequest *)AudioIO5);
      /* */
   }
}


/*
** end_right_thruster_sound()
**
** Finish side thruster sound in right channel.
*/
void end_right_thruster_sound()
{
   if (audio_flag == TRUE)
   {
      /* stop the main thruster sound (channels 1 & 2) */
      AudioIO3->ioa_Request.io_Command = ADCMD_FINISH;
      AudioIO3->ioa_Request.io_Unit = (struct Unit *)0x04;
      AudioIO3->ioa_Request.io_Flags = IOF_QUICK;
      
      BeginIO((struct IORequest *)AudioIO3);
      /* */
   }
}




