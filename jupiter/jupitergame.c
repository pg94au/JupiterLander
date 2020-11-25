/*
** jupitergame.c - code for the game part of Jupiter Lander
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

int game(void);

void probe_collision(struct VSprite *vsprite_a, struct VSprite *vsprite_b);

int do_far_view(struct ProbeInfo *probeinfo);
int do_pad_a(struct ProbeInfo *probeinfo);
int do_pad_b(struct ProbeInfo *probeinfo);
int do_pad_c(struct ProbeInfo *probeinfo);
int do_landed(struct ProbeInfo *probeinfo, int bonus);
int do_out_of_sky(void);
int do_game_over(struct ProbeInfo *probeinfo);
int do_game_setup(struct ProbeInfo *probeinfo);

int process_far_view_events(struct ProbeInfo *probeinfo,
   struct Bob *probebob, struct Bob *landbob);
int process_pad_a_events(struct ProbeInfo *probeinfo,
   struct Bob *probebob, struct Bob *landbob, struct Bob *padbob);
int process_pad_b_events(struct ProbeInfo *probeinfo,
   struct Bob *probebob, struct Bob *landbob, struct Bob *padbob);
int process_pad_c_events(struct ProbeInfo *probeinfo,
   struct Bob *probebob, struct Bob *landbob, struct Bob *padbob);

void put_small_flames(struct ProbeInfo *probeinfo, struct Bob *probebob,
   struct Bob *leftflamebob, struct Bob *rightflamebob,
   struct Bob *downflamebob[2]);
void put_big_flames(struct ProbeInfo *probeinfo, struct Bob *probebob,
   struct Bob *leftflamebob, struct Bob *rightflamebob,
   struct Bob *downflamebob[2]);

int process_input(struct ProbeInfo *probeinfo);

int do_far_view_crash(struct Bob *probebob, struct Bob *landbob,
   struct ProbeInfo *probeinfo);
int do_close_view_crash(struct Bob *probebob, struct Bob *landbob,
   struct Bob *padbob, struct ProbeInfo *probeinfo);

/* jupitermain.c */
extern void unclip_window(void);
extern int get_workbench_depth(void);

/* jl_gels.c */
extern int setup_gels(void);
extern void cleanup_gels(void);
extern struct Bob *create_bob(NEWBOB *nBob);
extern void free_bob(struct Bob *bob);
extern void redraw_bobs(void);

/* jl_draw.c */
extern int draw_game(LONG hiscore);

/* jl_gamestuff.c */
extern void update_velocity_scale(struct ProbeInfo *probeinfo);
extern void update_fuel_bar(struct ProbeInfo *probeinfo);
extern void update_score(LONG *score, LONG *hiscore);

/* jl_keyboard.c */
extern int setup_keyboard(void);
extern void cleanup_keyboard(void);
extern int read_keyboard(void);

/* jl_audio.c */
extern int setup_audio(void);
extern void cleanup_audio(void);
extern void play_start_music(void);
extern void play_crash_sound(void);
extern void end_crash_sound(void);
extern void play_pop_sound(void);
extern void end_pop_sound(void);
extern void begin_main_thruster_sound(void);
extern void end_main_thruster_sound(void);
extern void begin_left_thruster_sound(void);
extern void end_left_thruster_sound(void);
extern void begin_right_thruster_sound(void);
extern void end_right_thruster_sound(void);


/** GLOBAL VARIABLES **/

int collision_code;  /* to be set by collision handling functions */

LONG score;
static LONG hiscore = 0L;


/* jupitermain.c */
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *LayersBase;
extern struct Library *DOSBase;

extern struct Window *window;

extern int title_height;

/* jl_images.c */
extern UWORD chip smallprobe_image[1*2*21];
extern UWORD chip largeprobe_image[1*42*3];

extern UWORD chip farcrash_image[7][1*2*21];
extern UWORD chip closecrash_image[7][1*3*42];

extern UWORD chip farflame_down_image[2][1*5*1];
extern UWORD chip farflame_left_image[1*3*1];
extern UWORD chip farflame_right_image[1*3*1];
extern UWORD chip closeflame_down_image[2][1*10*2];
extern UWORD chip closeflame_left_image[1*6*1];
extern UWORD chip closeflame_right_image[1*6*1];

extern UWORD *farview_image;
extern UWORD *farview_mask;

extern UWORD *padaview_image;
extern UWORD *padaview_mask;
extern UWORD chip pada_image[1*8*6];
extern UWORD chip pada_mask[1*8*6];

extern UWORD *padbview_image;
extern UWORD *padbview_mask;
extern UWORD chip padb_image[1*8*6];
extern UWORD chip padb_mask[1*8*6];

extern UWORD *padcview_image;
extern UWORD *padcview_mask;
extern UWORD chip padc_image[1*8*5];
extern UWORD chip padc_mask[1*8*5];


/*-----------------------------------------------------------------------*/


/*
** game()
**
** This is going to be the main function that handles the game play.
**
** RETURNS:  INTRO or QUIT depending on how it exits.
*/
int game()
{
   int result;
   struct ProbeInfo probeinfo;
   
   /* draw everything that remains static throughout the game.
   ** Then draw everything else as it is at the beginning of a game.
   */
   result = draw_game(hiscore);
   
   if (result == FALSE)
   {
      printf("Could not draw game screen.\n");
      return QUIT;
   }
   /* */

   /* open keyboard.device for reading key matrix */
   result = setup_keyboard();
   
   if (result == FALSE)
   {
      printf("Error returned from setup_keyboard()\n");
      return QUIT;
   }
   /* */
   
   /* open audio.device if possible */
   result = setup_audio(); /* result doesn't matter, game play w/o sound */
   
   /* prepare for start of game */
   score = 0L;
   probeinfo.fuel = 219.0;
   result = SETUP;
   /* */
   
   /* run game */
   FOREVER
   {
      switch (result)
      {
         case GO_FAR:
               result = do_far_view(&probeinfo);
               break;
         case GO_PAD_A:
               result = do_pad_a(&probeinfo);
               break;
         case GO_PAD_B:
               result = do_pad_b(&probeinfo);
               break;
         case GO_PAD_C:
               result = do_pad_c(&probeinfo);
               break;
         case GAME_OVER:
               do_game_over(&probeinfo);
               cleanup_keyboard();
               cleanup_audio();
               return INTRO;
               break;
         case QUIT:
               /* cleanup first? */
               cleanup_keyboard();
               cleanup_audio();
               return QUIT;
               break;
         case FAIL:
               /* cleanup first? */
               cleanup_keyboard();
               cleanup_audio();
               printf("Program failure!\n");
               return QUIT;
               break;
         case SETUP:
               result=do_game_setup(&probeinfo);
               break;
      }
   }
   /* */
}


/*
** probe_collision()
**
** This function is called by DoCollision if the probe collides with
** the surface of the land or with a pad.
*/
void probe_collision(struct VSprite *vsprite_a, struct VSprite *vsprite_b)
{
   if ( ((vsprite_a->MeMask == PADMASK) || (vsprite_b->MeMask == PADMASK))
         && (collision_code != CRASHED))
   {
      collision_code = LANDED;
   }
   
   if ((vsprite_a->MeMask == LANDMASK) || (vsprite_b->MeMask == LANDMASK))
   {
      collision_code = CRASHED;
   }
}


/*
** do_game_setup()
**
** Setup for game.  Called when game first started and after
** probe crash, probe out of sky, or probe landed.
**
** RETURNS:  GO_FAR, if any fuel remains.
**           GAME_OVER, if probe is out of fuel.
*/
int do_game_setup(struct ProbeInfo *probeinfo)
{
   /* initialize collision code */
   collision_code = NULL;

   /* check if game is over */
   if (probeinfo->fuel <= 0.0)
   {
      return GAME_OVER;
   }
   /* */

   /* initial probeinfo values */
   probeinfo->x_pos = 20;
   probeinfo->y_pos = 8;
   probeinfo->x_vel = 0.25;
   probeinfo->y_vel = 0.0;
   /* */
   
   return GO_FAR;
}


/*
** do_far_view()
**
** Run the far view of the game.
**
** RETURNS:  GO_PAD_A
**           GO_PAD_B
**           GO_PAD_C if the probe comes within specified range of them
**           SETUP if the probe crashed or left the window
**           QUIT if the user decides to quit game
**           FAIL if an error occurs
*/
int do_far_view(struct ProbeInfo *probeinfo)
{
   int result;
   struct Bob *probebob;
   struct Bob *landbob;
   NEWBOB smallprobeNewBob;
   NEWBOB farviewNewBob;
   
   VOID (*routine)();
   
   smallprobeNewBob.nb_Image = smallprobe_image;
   smallprobeNewBob.nb_WordWidth = 2;
   smallprobeNewBob.nb_ImageDepth = 1;
   smallprobeNewBob.nb_LineHeight = 21;
   smallprobeNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   smallprobeNewBob.nb_X = probeinfo->x_pos+3;
   smallprobeNewBob.nb_Y = probeinfo->y_pos+title_height;
   smallprobeNewBob.nb_HitMask = LANDMASK;
   smallprobeNewBob.nb_MeMask = PROBEMASK;
   smallprobeNewBob.nb_PlanePick = 0x02;
   smallprobeNewBob.nb_CollMask = NULL;
   
   farviewNewBob.nb_Image = (UWORD *)farview_image;
   farviewNewBob.nb_WordWidth = 17;
   farviewNewBob.nb_ImageDepth = 2;
   farviewNewBob.nb_LineHeight = 179;
   farviewNewBob.nb_BFlags = NULL;
   farviewNewBob.nb_X = 3;
   farviewNewBob.nb_Y = 0+title_height-10;
   farviewNewBob.nb_HitMask = PROBEMASK;
   farviewNewBob.nb_MeMask = LANDMASK;
   farviewNewBob.nb_PlanePick = 0x03;
   farviewNewBob.nb_CollMask = (UWORD *)farview_mask;
   
   
   /* setup GELS system for Bobs */
   result = setup_gels();
   
   if (result != FALSE)
   {
      probebob = create_bob(&smallprobeNewBob);
      
      if (probebob != NULL)
      {
         landbob = create_bob(&farviewNewBob);
         
         if (landbob != NULL)
         {
            AddBob(landbob, window->RPort);
            AddBob(probebob, window->RPort);
            
            routine = probe_collision;
            SetCollision(LANDHIT, routine, window->RPort->GelsInfo);
            SetCollision(PROBEHIT, routine, window->RPort->GelsInfo);
            
            redraw_bobs();
            
            /* play starting song, only at the beginning of each game */
            if ((score == 0L) && (probeinfo->fuel == 219.0))
            {
               play_start_music();
            }
            /* */
            
            /* handle input events for this view of the game */
            result = process_far_view_events(probeinfo, probebob, landbob);
            
            if (result == CRASHED)
            {
               result = do_far_view_crash(probebob, landbob, probeinfo);
            }
            
            if (result == OUT_OF_SKY)
            {
               result = do_out_of_sky();
            }
            /* */
            
            RemBob(probebob);
            RemBob(landbob);
            
            free_bob(landbob);
            free_bob(probebob);
            
            cleanup_gels();
            
            return result;
         }
         else
         {
            printf("Could not create farview landbob\n");
         }
         free_bob(probebob);
      }
      else
      {
         printf("Could not create smallprobe probebob\n");
      }
      cleanup_gels();
   }
   else
   {
      printf("Could not set up gels system\n");
   }
   
   return FAIL;
}


/*
** process_far_view_events()
**
** Read the keyboard & closegadget and handle movement on the screen.
**
** RETURNS:  GO_PAD_A
**           GO_PAD_B
**           GO_PAD_C if the probe comes within specified range of them
**           CRASHED if probe & land bobs collide
**           OUT_OF_SKY if probe leave the screen (collides with boundary)
**           QUIT if the user selects closewindow gadget
**           FAIL if function cannot allocate necessary bobs
*/
int process_far_view_events(struct ProbeInfo *probeinfo,
   struct Bob *probebob, struct Bob *landbob)
{
   int result;
   struct Bob *leftflamebob;
   struct Bob *rightflamebob;
   struct Bob *downflamebob[2];
   NEWBOB flameNewBob;
   
   flameNewBob.nb_WordWidth = 1;
   flameNewBob.nb_ImageDepth = 1;
   flameNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   flameNewBob.nb_X = 0;
   flameNewBob.nb_Y = 0;
   flameNewBob.nb_HitMask = NULL;
   flameNewBob.nb_MeMask = NULL;
   flameNewBob.nb_PlanePick = 0x02;
   flameNewBob.nb_CollMask = NULL;
   
   flameNewBob.nb_Image = farflame_left_image;
   flameNewBob.nb_LineHeight = 3;
   leftflamebob = create_bob(&flameNewBob);
   
   if (leftflamebob != NULL)
   {
      flameNewBob.nb_Image = farflame_right_image;
      rightflamebob = create_bob(&flameNewBob);
      
      if (rightflamebob != NULL)
      {
         flameNewBob.nb_Image = farflame_down_image[0];
         flameNewBob.nb_LineHeight = 5;
         
         downflamebob[0] = create_bob(&flameNewBob);
         
         if (downflamebob[0] != NULL)
         {
            flameNewBob.nb_Image = farflame_down_image[1];
            
            downflamebob[1] = create_bob(&flameNewBob);
            
            if (downflamebob[1] != NULL)
            {
               FOREVER
               {
                  /* read the idcmp port of window (affects probeinfo values) */
                  result = process_input(probeinfo);
                  
                  if (result != QUIT)
                  {
                     /* gravity is always active */
                     if (probeinfo->y_vel < 1.975)
                     {
                        probeinfo->y_vel+=0.025;
                     }
               
                     /* update the velocity scale */
                     update_velocity_scale(probeinfo);
                     
                     /* work out new postions for probe */
                     probeinfo->x_pos+=probeinfo->x_vel;
                     probeinfo->y_pos+=probeinfo->y_vel;
                     /* */
               
                     /* check if we've left this view */
                     if (probeinfo->y_pos >= 90)
                     {
                        if (probeinfo->x_pos < 101)
                        {
                           result = GO_PAD_A;
                        }
                        if (probeinfo->x_pos > 180)
                        {
                           result = GO_PAD_C;
                        }
                     }
                     if ((probeinfo->y_pos >= 10)
                        && (probeinfo->x_pos >= 100)
                        && (probeinfo->x_pos <= 180))
                     {
                        result = GO_PAD_B;
                     }
                     /* */
                     
                     /* move probe */
                     probebob->BobVSprite->X = (SHORT)probeinfo->x_pos+3;
                     probebob->BobVSprite->Y = (SHORT)probeinfo->y_pos+title_height;
                     /* */
                     
                     WaitTOF();
                     
                     SortGList(window->RPort);
                     DoCollision(window->RPort);
                     
                     if (collision_code == CRASHED)
                     {
                        result = CRASHED;
                     }
                  }
                  
                  if (result != OK)
                  {
                     /* remove flame bobs */
                     probeinfo->user_input = NULL;
                     
                     put_small_flames(probeinfo, probebob, leftflamebob,
                        rightflamebob, downflamebob);
                     /* */
                     
                     /* must redraw gels before freeing bobs */
                     redraw_bobs();
                     
                     /* free them now */
                     free_bob(leftflamebob);
                     free_bob(rightflamebob);
                     free_bob(downflamebob[0]);
                     free_bob(downflamebob[1]);
                     /* */
                     
                     return result;
                  }
                  
                  /* add flame bobs to gels list */
                  put_small_flames(probeinfo, probebob, leftflamebob,
                     rightflamebob, downflamebob);
                  
                  /* move probe (remove other bobs while redrawing for speed) */
                  RemBob(landbob);
                  DrawGList(window->RPort, ViewPortAddress(window));
                  AddBob(landbob, window->RPort);
                  WaitTOF();
                  /* */
                  
                  /* this check is done after probe is already redrawn */
                  if (probeinfo->y_pos <= -21)
                  {
                     /* remove flame bobs */
                     probeinfo->user_input = NULL;
                     
                     put_small_flames(probeinfo, probebob, leftflamebob,
                        rightflamebob, downflamebob);
                     /* */
                     
                     /* free them now */
                     free_bob(leftflamebob);
                     free_bob(rightflamebob);
                     free_bob(downflamebob[0]);
                     free_bob(downflamebob[1]);
                     /* */
                     
                     return OUT_OF_SKY;
                  }
               }
            }
            else
            {
               printf("Could not create farview downflamebob[1] bob\n");
            }
            free_bob(downflamebob[0]);
         }
         else
         {
            printf("Could not create farview downflamebob[0] bob\n");
         }
         free_bob(rightflamebob);
      }
      else
      {
         printf("Could not create farview rightflamebob bob\n");
      }
      free_bob(leftflamebob);
   }
   else
   {
      printf("Could not create farview leftflamebob bob\n");
   }
   
   return FAIL;
}


/*
** do_pad_a()
**
** Run the pad a view of the game.
**
** RETURNS:  GO_FAR if the probe leaves this view
**           SETUP if the probe crashed
**           QUIT if the user decides to quit game
**           FAIL if an error occurs
*/
int do_pad_a(struct ProbeInfo *probeinfo)
{
   int result;
   struct Bob *probebob;
   struct Bob *landbob;
   struct Bob *padbob;
   NEWBOB largeprobeNewBob;
   NEWBOB padaviewNewBob;
   NEWBOB padaNewBob;
   
   VOID (*routine)();
   
   largeprobeNewBob.nb_Image = largeprobe_image;
   largeprobeNewBob.nb_WordWidth = 3;
   largeprobeNewBob.nb_ImageDepth = 1;
   largeprobeNewBob.nb_LineHeight = 42;
   largeprobeNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   largeprobeNewBob.nb_X = ((probeinfo->x_pos - 10) * 2) + 3;
   largeprobeNewBob.nb_Y = ((probeinfo->y_pos - 90) * 2) + title_height;
   largeprobeNewBob.nb_HitMask = LANDMASK | PADMASK;
   largeprobeNewBob.nb_MeMask = PROBEMASK;
   largeprobeNewBob.nb_PlanePick = 0x02;
   largeprobeNewBob.nb_CollMask = NULL;
   
   padaviewNewBob.nb_Image = (UWORD *)padaview_image;
   padaviewNewBob.nb_WordWidth = 17;
   padaviewNewBob.nb_ImageDepth = 2;
   padaviewNewBob.nb_LineHeight = 179;
   padaviewNewBob.nb_BFlags = NULL;
   padaviewNewBob.nb_X = 3;
   padaviewNewBob.nb_Y = 0+title_height-10;
   padaviewNewBob.nb_HitMask = PROBEMASK;
   padaviewNewBob.nb_MeMask = LANDMASK;
   padaviewNewBob.nb_PlanePick = 0x03;
   padaviewNewBob.nb_CollMask = (UWORD *)padaview_mask;
   
   padaNewBob.nb_Image = pada_image;
   padaNewBob.nb_WordWidth = 6;
   padaNewBob.nb_ImageDepth = 1;
   padaNewBob.nb_LineHeight = 8;
   padaNewBob.nb_BFlags = OVERLAY;
   padaNewBob.nb_X = 91;
   padaNewBob.nb_Y = 117+title_height;
   padaNewBob.nb_HitMask = PROBEMASK;
   padaNewBob.nb_MeMask = PADMASK;
   padaNewBob.nb_PlanePick = 0x03;
   padaNewBob.nb_CollMask = pada_mask;
   
   /* setup GELS system for Bobs */
   result = setup_gels();
   
   if (result != FALSE)
   {
      probebob = create_bob(&largeprobeNewBob);
      
      if (probebob != NULL)
      {
         landbob = create_bob(&padaviewNewBob);
         
         if (landbob != NULL)
         {
            padbob = create_bob(&padaNewBob);
            
            if (padbob != NULL)
            {
               AddBob(landbob, window->RPort);
               AddBob(padbob, window->RPort);
               AddBob(probebob, window->RPort);
               
               routine = probe_collision;
               SetCollision(LANDHIT, routine, window->RPort->GelsInfo);
               SetCollision(PADHIT, routine, window->RPort->GelsInfo);
               SetCollision(PROBEHIT, routine, window->RPort->GelsInfo);
               
               redraw_bobs();
               
               /* handle input events for this part of the game */
               result = process_pad_a_events(probeinfo, probebob, landbob,
                           padbob);
               
               /* the check for LANDED comes first because if you landed
               ** too hard it will return CRASHED.
               */
               if (result == LANDED)
               {
                  result = do_landed(probeinfo, 5);
               }
               if (result == CRASHED)
               {
                  result = do_close_view_crash(probebob, landbob, padbob,
                     probeinfo);
               }
               /* */
               
               RemBob(probebob);
               RemBob(landbob);
               RemBob(padbob);
               
               free_bob(probebob);
               free_bob(landbob);
               free_bob(padbob);
               
               cleanup_gels();
               
               return result;
            }
            else
            {
               printf("Could not create pada padbob\n");
            }
            free_bob(landbob);
         }
         else
         {
            printf("Could not create padaview landbob\n");
         }
         free_bob(probebob);
      }
      else
      {
         printf("Could not create largeprobe probebob\n");
      }
      cleanup_gels();
   }
   else
   {
      printf("Could not set up gels system\n");
   }
   
   return FAIL;
}


/*
** process_pad_a_events()
**
** Read the keyboard & closegadget and handle movement on the screen.
**
** RETURNS:  GO_FAR if the probe leaves the screen
**           CRASHED if probe & land collide or if probe lands too hard
**           LANDED if the probe lands on the pad
**           QUIT if the user selects closewindow gadget
**           FAIL if function cannot allocate necessary bobs
*/
int process_pad_a_events(struct ProbeInfo *probeinfo, struct Bob *probebob,
                           struct Bob *landbob, struct Bob *padbob)
{
   int result;
   struct Bob *leftflamebob;
   struct Bob *rightflamebob;
   struct Bob *downflamebob[2];
   NEWBOB flameNewBob;
   
   flameNewBob.nb_ImageDepth = 1;
   flameNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   flameNewBob.nb_X = 0;
   flameNewBob.nb_Y = 0;
   flameNewBob.nb_HitMask = NULL;
   flameNewBob.nb_MeMask = NULL;
   flameNewBob.nb_PlanePick = 0x02;
   flameNewBob.nb_CollMask = NULL;
   
   flameNewBob.nb_Image = closeflame_left_image;
   flameNewBob.nb_WordWidth = 1;
   flameNewBob.nb_LineHeight = 6;
   leftflamebob = create_bob(&flameNewBob);
   
   if (leftflamebob != NULL)
   {
      flameNewBob.nb_Image = closeflame_right_image;
      rightflamebob = create_bob(&flameNewBob);
      
      if (rightflamebob != NULL)
      {
         flameNewBob.nb_Image = closeflame_down_image[0];
         flameNewBob.nb_WordWidth = 2;
         flameNewBob.nb_LineHeight = 10;
         downflamebob[0] = create_bob(&flameNewBob);
         
         if (downflamebob[0] != NULL)
         {
            flameNewBob.nb_Image = closeflame_down_image[1];
            downflamebob[1] = create_bob(&flameNewBob);
            
            if (downflamebob[1] != NULL)
            {
               FOREVER
               {
                  /* read the idcmp port of window (affects probeinfo values) */
                  result = process_input(probeinfo);
                  
                  if (result != QUIT)
                  {
                     /* gravity is always active */
                     if (probeinfo->y_vel < 1.975)
                     {
                        probeinfo->y_vel += 0.025;
                     }
                     
                     /* update the velocity scale */
                     update_velocity_scale(probeinfo);
                     
                     /* work out new positions for probe */
                     probeinfo->x_pos += probeinfo->x_vel;
                     probeinfo->y_pos += probeinfo->y_vel;
                     /* */
                     
                     /* check if we've left this view before redrawing probe */
                     if (probeinfo->y_pos < 90)
                     {
                        result = GO_FAR;
                     }
                     /* */
                     
                     /* move probe */
                     probebob->BobVSprite->X = (SHORT)(((probeinfo->x_pos - 10) * 2) + 3);
                     probebob->BobVSprite->Y = (SHORT)(((probeinfo->y_pos - 90) * 2) + title_height);
                     /* */
                     
                     WaitTOF();
                     
                     /* check for collisions (sets collision_code) */
                     SortGList(window->RPort);
                     DoCollision(window->RPort);
                     /* */
                     
                     if (collision_code == CRASHED)
                     {
                        result = CRASHED;
                     }
                     if (collision_code == LANDED)
                     {
                        /* make sure that probe is not below surface of pad */
                        probebob->BobVSprite->Y = (118+title_height-42);
                        RemBob(landbob);
                        RemBob(padbob);
                        DrawGList(window->RPort, ViewPortAddress(window));
                        AddBob(landbob, window->RPort);
                        AddBob(padbob, window->RPort);
                        /* */
                        
                        result = LANDED;
                     }
                  }
                  
                  if (result != OK)
                  {
                     /* remove flame bobs */
                     probeinfo->user_input = NULL;
                     
                     put_big_flames(probeinfo, probebob, leftflamebob,
                        rightflamebob, downflamebob);
                     /* */
                     
                     /* must redraw gels before freeing bobs */
                     redraw_bobs();
                     
                     /* free them now */
                     free_bob(leftflamebob);
                     free_bob(rightflamebob);
                     free_bob(downflamebob[0]);
                     free_bob(downflamebob[1]);
                     /* */
                     
                     return result;
                  }
                  
                  /* add flame bobs to gels list */
                  put_big_flames(probeinfo, probebob, leftflamebob,
                     rightflamebob, downflamebob);
                  
                  /* move probe (remove other bobs while redrawing for speed) */
                  RemBob(landbob);
                  RemBob(padbob);
                  DrawGList(window->RPort, ViewPortAddress(window));
                  AddBob(landbob, window->RPort);
                  AddBob(padbob, window->RPort);
                  WaitTOF();
                  /* */
               }
            }
            else
            {
               printf("Could not create closeview downflamebob[1] bob\n");
            }
            free_bob(downflamebob[0]);
         }
         else
         {
            printf("Could not create farview downflamebob[0] bob\n");
         }
         free_bob(rightflamebob);
      }
      else
      {
         printf("Could not create farview rightflamebob bob\n");
      }
      free_bob(leftflamebob);
   }
   else
   {
      printf("Could not create farview leftflamebob bob\n");
   }
   
   return FAIL;
}


/*
** do_pad_b()
**
** Run the pad b view of the game.
**
** RETURNS:  GO_FAR if the probe leaves this view
**           SETUP if the probe crashed
**           QUIT if the user decides to quit game
**           FAIL if an error occurs
*/
int do_pad_b(struct ProbeInfo *probeinfo)
{
   int result;
   struct Bob *probebob;
   struct Bob *landbob;
   struct Bob *padbob;
   NEWBOB largeprobeNewBob;
   NEWBOB padbviewNewBob;
   NEWBOB padbNewBob;
   
   VOID (*routine)();
   
   largeprobeNewBob.nb_Image = largeprobe_image;
   largeprobeNewBob.nb_WordWidth = 3;
   largeprobeNewBob.nb_ImageDepth = 1;
   largeprobeNewBob.nb_LineHeight = 42;
   largeprobeNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   largeprobeNewBob.nb_X = ((probeinfo->x_pos - 85) * 2) + 3;
   largeprobeNewBob.nb_Y = ((probeinfo->y_pos - 10) * 2) + title_height;
   largeprobeNewBob.nb_HitMask = LANDMASK | PADMASK;
   largeprobeNewBob.nb_MeMask = PROBEMASK;
   largeprobeNewBob.nb_PlanePick = 0x02;
   largeprobeNewBob.nb_CollMask = NULL;
   
   padbviewNewBob.nb_Image = (UWORD *)padbview_image;
   padbviewNewBob.nb_WordWidth = 17;
   padbviewNewBob.nb_ImageDepth = 2;
   padbviewNewBob.nb_LineHeight = 179;
   padbviewNewBob.nb_BFlags = NULL;
   padbviewNewBob.nb_X = 3;
   padbviewNewBob.nb_Y = 0+title_height-10;
   padbviewNewBob.nb_HitMask = PROBEMASK;
   padbviewNewBob.nb_MeMask = LANDMASK;
   padbviewNewBob.nb_PlanePick = 0x03;
   padbviewNewBob.nb_CollMask = (UWORD *)padbview_mask;
   
   padbNewBob.nb_Image = padb_image;
   padbNewBob.nb_WordWidth = 6;
   padbNewBob.nb_ImageDepth = 1;
   padbNewBob.nb_LineHeight = 8;
   padbNewBob.nb_BFlags = OVERLAY;
   padbNewBob.nb_X = 97;
   padbNewBob.nb_Y = 74+title_height;
   padbNewBob.nb_HitMask = PROBEMASK;
   padbNewBob.nb_MeMask = PADMASK;
   padbNewBob.nb_PlanePick = 0x03;
   padbNewBob.nb_CollMask = padb_mask;
   
   /* setup GELS system for Bobs */
   result = setup_gels();
   
   if (result != FALSE)
   {
      probebob = create_bob(&largeprobeNewBob);
      
      if (probebob != NULL)
      {
         landbob = create_bob(&padbviewNewBob);
         
         if (landbob != NULL)
         {
            padbob = create_bob(&padbNewBob);
            
            if (padbob != NULL)
            {
               AddBob(landbob, window->RPort);
               AddBob(padbob, window->RPort);
               AddBob(probebob, window->RPort);
               
               routine = probe_collision;
               SetCollision(LANDHIT, routine, window->RPort->GelsInfo);
               SetCollision(PADHIT, routine, window->RPort->GelsInfo);
               SetCollision(PROBEHIT, routine, window->RPort->GelsInfo);
               
               redraw_bobs();
               
               /* handle input events for this part of the game */
               result = process_pad_b_events(probeinfo, probebob, landbob,
                           padbob);
               
               /* the check for LANDED comes first because if you landed
               ** too hard it will return CRASHED.
               */
               if (result == LANDED)
               {
                  /* check if both legs are completely on the pad */
                  if ((probebob->BobVSprite->X >= 97) &&
                     (probebob->BobVSprite->X <= 132))
                  {
                     result = do_landed(probeinfo, 2);
                  }
                  else
                  {
                     result = CRASHED;
                  }
               }
               if (result == CRASHED)
               {
                  result = do_close_view_crash(probebob, landbob, padbob,
                     probeinfo);
               }
               /* */
               
               RemBob(probebob);
               RemBob(landbob);
               RemBob(padbob);
               
               free_bob(probebob);
               free_bob(landbob);
               free_bob(padbob);
               
               cleanup_gels();
               
               return result;
            }
            else
            {
               printf("Could not create padb padbob\n");
            }
            free_bob(landbob);
         }
         else
         {
            printf("Could not create padbview landbob\n");
         }
         free_bob(probebob);
      }
      else
      {
         printf("Could not create largeprobe probebob\n");
      }
      cleanup_gels();
   }
   else
   {
      printf("Could not set up gels system\n");
   }
   
   return FAIL;
}


/*
** process_pad_b_events()
**
** Read the keyboard & closegadget and handle movement on the screen.
**
** RETURNS:  GO_FAR if the probe leaves the screen
**           CRASHED if probe & land collide or if probe lands too hard
**           LANDED if the probe lands on the pad
**           QUIT if the user selects closewindow gadget
**           FAIL if function cannot allocate necessary bobs
*/
int process_pad_b_events(struct ProbeInfo *probeinfo, struct Bob *probebob,
                           struct Bob *landbob, struct Bob *padbob)
{
   int result;
   struct Bob *leftflamebob;
   struct Bob *rightflamebob;
   struct Bob *downflamebob[2];
   NEWBOB flameNewBob;
   
   flameNewBob.nb_ImageDepth = 1;
   flameNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   flameNewBob.nb_X = 0;
   flameNewBob.nb_Y = 0;
   flameNewBob.nb_HitMask = NULL;
   flameNewBob.nb_MeMask = NULL;
   flameNewBob.nb_PlanePick = 0x02;
   flameNewBob.nb_CollMask = NULL;
   
   flameNewBob.nb_Image = closeflame_left_image;
   flameNewBob.nb_WordWidth = 1;
   flameNewBob.nb_LineHeight = 6;
   leftflamebob = create_bob(&flameNewBob);
   
   if (leftflamebob != NULL)
   {
      flameNewBob.nb_Image = closeflame_right_image;
      rightflamebob = create_bob(&flameNewBob);
      
      if (rightflamebob != NULL)
      {
         flameNewBob.nb_Image = closeflame_down_image[0];
         flameNewBob.nb_WordWidth = 2;
         flameNewBob.nb_LineHeight = 10;
         downflamebob[0] = create_bob(&flameNewBob);
         
         if (downflamebob[0] != NULL)
         {
            flameNewBob.nb_Image = closeflame_down_image[1];
            downflamebob[1] = create_bob(&flameNewBob);
            
            if (downflamebob[1] != NULL)
            {
               FOREVER
               {
                  /* read the idcmp port of window (affects probeinfo values) */
                  result = process_input(probeinfo);
                  
                  if (result != QUIT)
                  {
                     /* gravity is always active */
                     if (probeinfo->y_vel < 1.975)
                     {
                        probeinfo->y_vel += 0.025;
                     }
                     
                     /* update the velocity scale */
                     update_velocity_scale(probeinfo);
                     
                     /* work out new positions for probe */
                     probeinfo->x_pos += probeinfo->x_vel;
                     probeinfo->y_pos += probeinfo->y_vel;
                     /* */
                     
                     /* check if we've left this view before redrawing probe */
                     if ((probeinfo->y_pos < 10) || (probeinfo->x_pos < 100)
                        || (probeinfo->x_pos > 180))
                     {
                        result = GO_FAR;
                     }
                     /* */
                     
                     /* move probe */
                     probebob->BobVSprite->X = (SHORT)(((probeinfo->x_pos - 85) * 2) + 3);
                     probebob->BobVSprite->Y = (SHORT)(((probeinfo->y_pos - 10) * 2) + title_height);
                     /* */
                     
                     WaitTOF();

                     /* check for collisions (sets collision_code) */
                     SortGList(window->RPort);
                     DoCollision(window->RPort);
                     /* */
                     
                     if (collision_code == CRASHED)
                     {
                        result = CRASHED;
                     }
                     if (collision_code == LANDED)
                     {
                        /* make sure that probe is not below surface of pad */
                        probebob->BobVSprite->Y = (75+title_height-42);
                        RemBob(landbob);
                        RemBob(padbob);
                        DrawGList(window->RPort, ViewPortAddress(window));
                        AddBob(landbob, window->RPort);
                        AddBob(padbob, window->RPort);
                        /* */
                        
                        result = LANDED;
                     }
                  }
                  
                  if (result != OK)
                  {
                     /* remove flame bobs */
                     probeinfo->user_input = NULL;
                     
                     put_big_flames(probeinfo, probebob, leftflamebob,
                        rightflamebob, downflamebob);
                     /* */
                     
                     /* must redraw gels before freeing bobs */
                     redraw_bobs();
                     
                     /* free them now */
                     free_bob(leftflamebob);
                     free_bob(rightflamebob);
                     free_bob(downflamebob[0]);
                     free_bob(downflamebob[1]);
                     /* */
                     
                     return result;
                  }
                  
                  /* add flame bobs to gels list */
                  put_big_flames(probeinfo, probebob, leftflamebob,
                     rightflamebob, downflamebob);
                  
                  /* move probe (remove other bobs while redrawing for speed) */
                  RemBob(landbob);
                  RemBob(padbob);
                  DrawGList(window->RPort, ViewPortAddress(window));
                  AddBob(landbob, window->RPort);
                  AddBob(padbob, window->RPort);
                  WaitTOF();
                  /* */
               }
            }
            else
            {
               printf("Could not create closeview downflamebob[1] bob\n");
            }
            free_bob(downflamebob[0]);
         }
         else
         {
            printf("Could not create farview downflamebob[0] bob\n");
         }
         free_bob(rightflamebob);
      }
      else
      {
         printf("Could not create farview rightflamebob bob\n");
      }
      free_bob(leftflamebob);
   }
   else
   {
      printf("Could not create farview leftflamebob bob\n");
   }
   
   return FAIL;
}


/*
** do_pad_c()
**
** Run the pad c view of the game.
**
** RETURNS:  GO_FAR if the probe leaves this view
**           SETUP if the probe crashed
**           QUIT if the user decides to quit game
**           FAIL if an error occurs
*/
int do_pad_c(struct ProbeInfo *probeinfo)
{
   int result;
   struct Bob *probebob;
   struct Bob *landbob;
   struct Bob *padbob;
   NEWBOB largeprobeNewBob;
   NEWBOB padcviewNewBob;
   NEWBOB padcNewBob;
   
   VOID (*routine)();
   
   largeprobeNewBob.nb_Image = largeprobe_image;
   largeprobeNewBob.nb_WordWidth = 3;
   largeprobeNewBob.nb_ImageDepth = 1;
   largeprobeNewBob.nb_LineHeight = 42;
   largeprobeNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   largeprobeNewBob.nb_X = ((probeinfo->x_pos - 140) * 2) + 3;
   largeprobeNewBob.nb_Y = ((probeinfo->y_pos - 90) * 2) + title_height;
   largeprobeNewBob.nb_HitMask = LANDMASK | PADMASK;
   largeprobeNewBob.nb_MeMask = PROBEMASK;
   largeprobeNewBob.nb_PlanePick = 0x02;
   largeprobeNewBob.nb_CollMask = NULL;
   
   padcviewNewBob.nb_Image = (UWORD *)padcview_image;
   padcviewNewBob.nb_WordWidth = 17;
   padcviewNewBob.nb_ImageDepth = 2;
   padcviewNewBob.nb_LineHeight = 179;
   padcviewNewBob.nb_BFlags = NULL;
   padcviewNewBob.nb_X = 3;
   padcviewNewBob.nb_Y = 0+title_height-10;
   padcviewNewBob.nb_HitMask = PROBEMASK;
   padcviewNewBob.nb_MeMask = LANDMASK;
   padcviewNewBob.nb_PlanePick = 0x03;
   padcviewNewBob.nb_CollMask = (UWORD *)padcview_mask;
   
   padcNewBob.nb_Image = padc_image;
   padcNewBob.nb_WordWidth = 5;
   padcNewBob.nb_ImageDepth = 1;
   padcNewBob.nb_LineHeight = 8;
   padcNewBob.nb_BFlags = OVERLAY;
   padcNewBob.nb_X = 93;
   padcNewBob.nb_Y = 133+title_height;
   padcNewBob.nb_HitMask = PROBEMASK;
   padcNewBob.nb_MeMask = PADMASK;
   padcNewBob.nb_PlanePick = 0x03;
   padcNewBob.nb_CollMask = padc_mask;
   
   /* setup GELS system for Bobs */
   result = setup_gels();
   
   if (result != FALSE)
   {
      probebob = create_bob(&largeprobeNewBob);
      
      if (probebob != NULL)
      {
         landbob = create_bob(&padcviewNewBob);
         
         if (landbob != NULL)
         {
            padbob = create_bob(&padcNewBob);
            
            if (padbob != NULL)
            {
               AddBob(landbob, window->RPort);
               AddBob(padbob, window->RPort);
               AddBob(probebob, window->RPort);
               
               routine = probe_collision;
               SetCollision(LANDHIT, routine, window->RPort->GelsInfo);
               SetCollision(PADHIT, routine, window->RPort->GelsInfo);
               SetCollision(PROBEHIT, routine, window->RPort->GelsInfo);
               
               redraw_bobs();
               
               /* handle input events for this part of the game */
               result = process_pad_c_events(probeinfo, probebob, landbob,
                           padbob);
               
               /* the check for LANDED comes first because if you landed
               ** too hard it will return CRASHED.
               */
               if (result == LANDED)
               {
                  result = do_landed(probeinfo, 10);
               }
               if (result == CRASHED)
               {
                  result = do_close_view_crash(probebob, landbob, padbob,
                     probeinfo);
               }
               /* */
               
               RemBob(probebob);
               RemBob(landbob);
               RemBob(padbob);
               
               free_bob(probebob);
               free_bob(landbob);
               free_bob(padbob);
               
               cleanup_gels();
               
               return result;
            }
            else
            {
               printf("Could not create padc padbob\n");
            }
            free_bob(landbob);
         }
         else
         {
            printf("Could not create padcview landbob\n");
         }
         free_bob(probebob);
      }
      else
      {
         printf("Could not create largeprobe probebob\n");
      }
      cleanup_gels();
   }
   else
   {
      printf("Could not set up gels system\n");
   }
   
   return FAIL;
}


/*
** process_pad_c_events()
**
** Read the keyboard & closegadget and handle movement on the screen.
**
** RETURNS:  GO_FAR if the probe leaves the screen
**           CRASHED if probe & land collide or if probe lands too hard
**           LANDED if the probe lands on the pad
**           QUIT if the user selects closewindow gadget
**           FAIL if function cannot allocate necessary bobs
*/
int process_pad_c_events(struct ProbeInfo *probeinfo, struct Bob *probebob,
                           struct Bob *landbob, struct Bob *padbob)
{
   int result;
   struct Bob *leftflamebob;
   struct Bob *rightflamebob;
   struct Bob *downflamebob[2];
   NEWBOB flameNewBob;
   
   flameNewBob.nb_ImageDepth = 1;
   flameNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   flameNewBob.nb_X = 0;
   flameNewBob.nb_Y = 0;
   flameNewBob.nb_HitMask = NULL;
   flameNewBob.nb_MeMask = NULL;
   flameNewBob.nb_PlanePick = 0x02;
   flameNewBob.nb_CollMask = NULL;
   
   flameNewBob.nb_Image = closeflame_left_image;
   flameNewBob.nb_WordWidth = 1;
   flameNewBob.nb_LineHeight = 6;
   leftflamebob = create_bob(&flameNewBob);
   
   if (leftflamebob != NULL)
   {
      flameNewBob.nb_Image = closeflame_right_image;
      rightflamebob = create_bob(&flameNewBob);
      
      if (rightflamebob != NULL)
      {
         flameNewBob.nb_Image = closeflame_down_image[0];
         flameNewBob.nb_WordWidth = 2;
         flameNewBob.nb_LineHeight = 10;
         downflamebob[0] = create_bob(&flameNewBob);
         
         if (downflamebob[0] != NULL)
         {
            flameNewBob.nb_Image = closeflame_down_image[1];
            downflamebob[1] = create_bob(&flameNewBob);
            
            if (downflamebob[1] != NULL)
            {
               FOREVER
               {
                  /* read the idcmp port of window (affects probeinfo values) */
                  result = process_input(probeinfo);
                  
                  if (result != QUIT)
                  {
                     /* gravity is always active */
                     if (probeinfo->y_vel < 1.975)
                     {
                        probeinfo->y_vel += 0.025;
                     }
                     
                     /* update the velocity scale */
                     update_velocity_scale(probeinfo);
                     
                     /* work out new positions for probe */
                     probeinfo->x_pos += probeinfo->x_vel;
                     probeinfo->y_pos += probeinfo->y_vel;
                     /* */
                     
                     /* check if we've left this view before redrawing probe */
                     if (probeinfo->y_pos < 90)
                     {
                        result = GO_FAR;
                     }
                     /* */
                     
                     /* move probe */
                     probebob->BobVSprite->X = (SHORT)(((probeinfo->x_pos - 140) * 2) + 3);
                     probebob->BobVSprite->Y = (SHORT)(((probeinfo->y_pos - 90) * 2) + title_height);
                     /* */
                     
                     WaitTOF();
                     
                     /* check for collisions (sets collision_code) */
                     SortGList(window->RPort);
                     DoCollision(window->RPort);
                     /* */
                     
                     if (collision_code == CRASHED)
                     {
                        result = CRASHED;
                     }
                     if (collision_code == LANDED)
                     {
                        /* make sure that probe is not below surface of pad */
                        probebob->BobVSprite->Y = (134+title_height-42);
                        RemBob(landbob);
                        RemBob(padbob);
                        DrawGList(window->RPort, ViewPortAddress(window));
                        AddBob(landbob, window->RPort);
                        AddBob(padbob, window->RPort);
                        /* */
                        
                        result = LANDED;
                     }
                  }
                  
                  if (result != OK)
                  {
                     /* remove flame bobs */
                     probeinfo->user_input = NULL;
                     
                     put_big_flames(probeinfo, probebob, leftflamebob,
                        rightflamebob, downflamebob);
                     /* */
                     
                     /* must redraw gels before freeing bobs */
                     redraw_bobs();
                     
                     /* free them now */
                     free_bob(leftflamebob);
                     free_bob(rightflamebob);
                     free_bob(downflamebob[0]);
                     free_bob(downflamebob[1]);
                     /* */
                     
                     return result;
                  }
                  
                  /* add flame bobs to gels list */
                  put_big_flames(probeinfo, probebob, leftflamebob,
                     rightflamebob, downflamebob);
                  
                  /* move probe (remove other bobs while redrawing for speed) */
                  RemBob(landbob);
                  RemBob(padbob);
                  DrawGList(window->RPort, ViewPortAddress(window));
                  AddBob(landbob, window->RPort);
                  AddBob(padbob, window->RPort);
                  WaitTOF();
                  /* */
               }
            }
            else
            {
               printf("Could not create closeview downflamebob[1] bob\n");
            }
            free_bob(downflamebob[0]);
         }
         else
         {
            printf("Could not create farview downflamebob[0] bob\n");
         }
         free_bob(rightflamebob);
      }
      else
      {
         printf("Could not create farview rightflamebob bob\n");
      }
      free_bob(leftflamebob);
   }
   else
   {
      printf("Could not create farview leftflamebob bob\n");
   }
   
   return FAIL;
}


/*
** process_input()
**
** Get a message from the window & handle input.  Change probeinfo values
** if a key is pressed, pause game if window becomes inactive.
** Update the fuel bar in the window if thrusters are used.
**
** RETURNS:  QUIT if user selected closewindow gadget, else returns OK.
*/
int process_input(struct ProbeInfo *probeinfo)
{
   struct IntuiMessage *imsg;
   ULONG class;
   USHORT code;
   
   /* check the idcmp port */
   if (NULL != (imsg = (struct IntuiMessage *)GetMsg(window->UserPort)))
   {
      class = imsg->Class;
      code = imsg->Code;
      ReplyMsg((struct Message *)imsg);
      
      switch(class)
      {
         case IDCMP_CLOSEWINDOW:
            return QUIT;
            break;
         case IDCMP_INACTIVEWINDOW:
            FOREVER
            {
               Wait (1 << window->UserPort->mp_SigBit);
               
               while (NULL != (imsg = (struct IntuiMessage *)GetMsg(window->UserPort)))
               {
                  class = imsg->Class;
                  ReplyMsg((struct Message *)imsg);
                  
                  if (class == IDCMP_ACTIVEWINDOW)
                  {
                     break;
                  }
               }
               if (class == IDCMP_ACTIVEWINDOW)
               {
                  break;
               }
            }
            break;
      }
   }
   /* */
   
   /* check keyboard */
   probeinfo->user_input = read_keyboard();
   
   if ((probeinfo->user_input & CURSOR_UP) && (probeinfo->fuel >= 0))
   {
      if (probeinfo->y_vel > -1.925)
      {
         probeinfo->y_vel-=0.075;
      }
      probeinfo->fuel-=0.5;
   }
   if ((probeinfo->user_input & CURSOR_RIGHT) && (probeinfo->fuel >= 0))
   {
      probeinfo->x_vel-=0.025;
      probeinfo->fuel-=0.25;
   }
   if ((probeinfo->user_input & CURSOR_LEFT) && (probeinfo->fuel >= 0))
   {
      probeinfo->x_vel+=0.025;
      probeinfo->fuel-=0.25;
   }
   /* */

   /* update the fuel bar in the window to reflect any change in fuel */
   update_fuel_bar(probeinfo);
   
   return OK;
}


/*
** do_landed()
**
** Check whether probe landed within allowed velocity.  If not then print
** "SORRY, NO BONUS" and return CRASHED.  If landing was successful,
** calculate increase in score and update score and fuel values (and update
** screen display of them).
**
** RETURNS:  SETUP if landed successfully after adding to score & fuel
**           CRASHED if landing was too hard
*/
int do_landed(struct ProbeInfo *probeinfo, int bonus)
{
   int points;    /* depends on how soft landing is */
   int total;     /* points x bonus */
   
   char output[16];  /* final output string "### x # = ####" */
   char s[7];        /* for conversion from INT->ASCII */
   int n;            /* for conversion from INT->ASCII */
   
   int x;   /* counter */
   
   
   /* did probe land too hard */
   if (probeinfo->y_vel >= 0.35)
   {
      /* print "SORRY, NO BONUS" */
      SetDrMd(window->RPort, JAM1);
      
      SetAPen(window->RPort, BLACK);
      RectFill(window->RPort, 66, 5+title_height, 202, 29+title_height);
      
      SetAPen(window->RPort, WHITE);
      Move(window->RPort, 74, 20+title_height);
      Text(window->RPort, "SORRY, NO BONUS", 15);
      /* */
      
      /* probe did land on pad so we'll add a little delay for suspense */
      Delay(100);

      return CRASHED;
   }
   /* probe landed within allowed impact level */
   else
   {
      /* points = ((0.35 - y_vel) / 0.35) * 4500.0 */
      points = (int)((0.35-probeinfo->y_vel)*1575.0);
      total = points * bonus;
      /* */

      /* print "### x ## = ####"  (points x bonus = total) */
      SetDrMd(window->RPort, JAM1);
      SetAPen(window->RPort, BLACK);
      RectFill(window->RPort, 66, 5+title_height, 202, 29+title_height);
      
         /* setup string to be printed */
         n = stci_d(s, points);
         for (x=0; x < (3-n); x++)
         {
            output[x] = ' ';
         }
         strcpy(&output[3-n], s);
         strcpy(&output[3], " x \0");
         n = stci_d(s, bonus);
         for (x=0; x < (2-n); x++)
         {
            output[x+6] = ' ';
         }
         strcpy(&output[8-n], s);
         strcpy(&output[8], " = \0");
         n = stci_d(s, total);
         for (x=0; x < (4-n); x++)
         {
            output[x+11] = ' ';
         }
         strcpy(&output[15-n], s);
         /* */
         
      SetAPen(window->RPort, WHITE);
      Move(window->RPort, 74, 20+title_height);
      Text(window->RPort, output, 15);
      /* */
      
      play_pop_sound();
      /* add to score and fuel and update screen */
      for (x=10; x <= total; x+=10)
      {
         score+=10;
         if (probeinfo->fuel < 219)
         {
            probeinfo->fuel+=0.25;
         }
         
         update_score(&score, &hiscore);
         update_fuel_bar(probeinfo);
         
         WaitTOF();
      }
      end_pop_sound();
      score+=(total-(x-10));           /* print final score (in case    */
      update_score(&score, &hiscore);  /* total is not evenly divisible */
      update_fuel_bar(probeinfo);      /* by ten                        */
      /* */
      
      Delay(100);
      
      return SETUP;
   }
}


/*
** do_out_of_sky()
**
** Print out OUT OF SKY in the window and delay for a few seconds.
**
** RETURNS:  SETUP, which begins play again.
*/
int do_out_of_sky()
{
   /* print "OUT OF SKY" */
   SetDrMd(window->RPort, JAM1);
   SetAPen(window->RPort, BLACK);
   RectFill(window->RPort, 86, 5+title_height, 182, 29+title_height);
   
   SetAPen(window->RPort, WHITE);
   Move(window->RPort, 94, 20+title_height);
   Text(window->RPort, "OUT OF SKY", 10);
   /* */
   
   Delay(150);
   
   return SETUP;
}


/*
** do_game_over()
**
** Print game over on the screen and delay before returning.
**
** RETURNS:  QUIT, which takes you back to title screen.
*/
int do_game_over(struct ProbeInfo *probeinfo)
{
   SetDrMd(window->RPort, JAM1);
   SetAPen(window->RPort, BLACK);
   RectFill(window->RPort, 66, 30+title_height, 202, 54+title_height);
   
   SetAPen(window->RPort, WHITE);
   Move(window->RPort, 98, 45+title_height);
   Text(window->RPort, "GAME OVER", 9);
   
   Delay(250);
   
   return QUIT;
}


/*
** do_far_view_crash()
**
** Display the destruction of the probe in the window.
** Decrease the amount of fuel remaining for probe.
**
** RETURNS:  SETUP if successful, FAIL if unsuccessful.
*/
int do_far_view_crash(struct Bob *probebob, struct Bob *landbob,
   struct ProbeInfo *probeinfo)
{
   struct Bob *farcrash_bob[7];
   NEWBOB farcrashNewBob;
   
   int x;
   
   farcrashNewBob.nb_Image = NULL;
   farcrashNewBob.nb_WordWidth = 2;
   farcrashNewBob.nb_ImageDepth = 1;
   farcrashNewBob.nb_LineHeight = 21;
   farcrashNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   farcrashNewBob.nb_X = probebob->BobVSprite->X;
   farcrashNewBob.nb_Y = probebob->BobVSprite->Y;
   farcrashNewBob.nb_HitMask = NULL;
   farcrashNewBob.nb_MeMask = NULL;
   farcrashNewBob.nb_PlanePick = 0x02;
   farcrashNewBob.nb_CollMask = NULL;
   
   farcrashNewBob.nb_Image = farcrash_image[0];
   farcrash_bob[0] = create_bob(&farcrashNewBob);
   if (farcrash_bob[0] != NULL)
   {
      farcrashNewBob.nb_Image = farcrash_image[1];
      farcrash_bob[1] = create_bob(&farcrashNewBob);
      if (farcrash_bob[1] != NULL)
      {
         farcrashNewBob.nb_Image = farcrash_image[2];
         farcrash_bob[2] = create_bob(&farcrashNewBob);
         if (farcrash_bob[2] != NULL)
         {
            farcrashNewBob.nb_Image = farcrash_image[3];
            farcrash_bob[3] = create_bob(&farcrashNewBob);
            if (farcrash_bob[3] != NULL)
            {
               farcrashNewBob.nb_Image = farcrash_image[4];
               farcrash_bob[4] = create_bob(&farcrashNewBob);
               if (farcrash_bob[4] != NULL)
               {
                  farcrashNewBob.nb_Image = farcrash_image[5];
                  farcrash_bob[5] = create_bob(&farcrashNewBob);
                  if (farcrash_bob[5] != NULL)
                  {
                     farcrashNewBob.nb_Image = farcrash_image[6];
                     farcrash_bob[6] = create_bob(&farcrashNewBob);
                     if (farcrash_bob[6] != NULL)
                     {
                        /* start the probe crash sound */
                        play_crash_sound();
                        
                        /* remove these to speed up bob redrawing */
                        RemBob(landbob);
                        RemBob(probebob);
                        /* */
                        
                        for (x=0; x<7; x++)
                        {
                           /* show each frame of the crash in order */
                           AddBob(farcrash_bob[x], window->RPort);
                           redraw_bobs();
                           WaitTOF();
                           WaitTOF();
                           WaitTOF();
                           WaitTOF();
                           RemBob(farcrash_bob[x]);
                        }
                        
                        /* clear the last bob in crash sequence */
                        redraw_bobs();
                        
                        /* free the farcrash bobs */
                        for (x=0; x<7; x++)
                        {
                           free_bob(farcrash_bob[x]);
                        }
                        /* */
                        
                        /* add these back before returning */
                        AddBob(landbob, window->RPort);
                        AddBob(probebob, window->RPort);
                        /* */
                        
                        /* wait for probe crash sound to finish */
                        end_crash_sound();
                        
                        /* decrease fuel (penalty for crashing) */
                        probeinfo->fuel-=30;
                        update_fuel_bar(probeinfo);
                        /* */
                        
                        Delay(100);
                        
                        return SETUP;
                     }
                     else
                     {
                        printf("Could not create farcrash_bob[6] bob\n");
                     }
                     free_bob(farcrash_bob[5]);
                  }
                  else
                  {
                     printf("Could not create farcrash_bob[5] bob\n");
                  }
                  free_bob(farcrash_bob[4]);
               }
               else
               {
                  printf("Could not create farcrash_bob[4] bob\n");
               }
               free_bob(farcrash_bob[3]);
            }
            else
            {
               printf("Could not create farcrash_bob[3] bob\n");
            }
            free_bob(farcrash_bob[2]);
         }
         else
         {
            printf("Could not create farcrash_bob[2] bob\n");
         }
         free_bob(farcrash_bob[1]);
      }
      else
      {
         printf("Could not create farcrash_bob[1] bob\n");
      }
      free_bob(farcrash_bob[0]);
   }
   else
   {
      printf("Could not create farcrash_bob[0] bob\n");
   }
   
   return FAIL;
}


/*
** do_close_view_crash()
**
** Display the destruction of the probe in the window.
** Decrease the amount of fuel remaining for probe.
**
** RETURNS:  SETUP if successful, FAIL if unsuccessful.
*/
int do_close_view_crash(struct Bob *probebob, struct Bob *landbob,
   struct Bob *padbob, struct ProbeInfo *probeinfo)
{
   struct Bob *closecrash_bob[7];
   NEWBOB closecrashNewBob;
   
   int x;
   
   closecrashNewBob.nb_Image = NULL;
   closecrashNewBob.nb_WordWidth = 3;
   closecrashNewBob.nb_ImageDepth = 1;
   closecrashNewBob.nb_LineHeight = 42;
   closecrashNewBob.nb_BFlags = SAVEBACK | OVERLAY;
   closecrashNewBob.nb_X = probebob->BobVSprite->X;
   closecrashNewBob.nb_Y = probebob->BobVSprite->Y;
   closecrashNewBob.nb_HitMask = NULL;
   closecrashNewBob.nb_MeMask = NULL;
   closecrashNewBob.nb_PlanePick = 0x02;
   closecrashNewBob.nb_CollMask = NULL;
   
   closecrashNewBob.nb_Image = closecrash_image[0];
   closecrash_bob[0] = create_bob(&closecrashNewBob);
   if (closecrash_bob[0] != NULL)
   {
      closecrashNewBob.nb_Image = closecrash_image[1];
      closecrash_bob[1] = create_bob(&closecrashNewBob);
      if (closecrash_bob[1] != NULL)
      {
         closecrashNewBob.nb_Image = closecrash_image[2];
         closecrash_bob[2] = create_bob(&closecrashNewBob);
         if (closecrash_bob[2] != NULL)
         {
            closecrashNewBob.nb_Image = closecrash_image[3];
            closecrash_bob[3] = create_bob(&closecrashNewBob);
            if (closecrash_bob[3] != NULL)
            {
               closecrashNewBob.nb_Image = closecrash_image[4];
               closecrash_bob[4] = create_bob(&closecrashNewBob);
               if (closecrash_bob[4] != NULL)
               {
                  closecrashNewBob.nb_Image = closecrash_image[5];
                  closecrash_bob[5] = create_bob(&closecrashNewBob);
                  if (closecrash_bob[5] != NULL)
                  {
                     closecrashNewBob.nb_Image = closecrash_image[6];
                     closecrash_bob[6] = create_bob(&closecrashNewBob);
                     if (closecrash_bob[6] != NULL)
                     {
                        /* start the probe crash sound */
                        play_crash_sound();
                        
                        /* remove these to speed up bob redrawing */
                        RemBob(landbob);
                        RemBob(probebob);
                        RemBob(padbob);
                        /* */
                        
                        for (x=0; x<7; x++)
                        {
                           /* show each frame of the crash in order */
                           AddBob(closecrash_bob[x], window->RPort);
                           redraw_bobs();
                           WaitTOF();
                           WaitTOF();
                           WaitTOF();
                           WaitTOF();
                           RemBob(closecrash_bob[x]);
                        }
                        
                        /* clear the last bob in crash sequence */
                        redraw_bobs();
                        
                        /* free the closecrash bobs */
                        for (x=0; x<7; x++)
                        {
                           free_bob(closecrash_bob[x]);
                        }
                        /* */
                        
                        /* add these back before returning */
                        AddBob(landbob, window->RPort);
                        AddBob(probebob, window->RPort);
                        AddBob(padbob, window->RPort);
                        /* */
                        
                        /* wait for probe crash sound to finish */
                        end_crash_sound();
                        
                        /* decrease fuel (penalty for crashing) */
                        probeinfo->fuel-=30;
                        update_fuel_bar(probeinfo);
                        /* */
                        
                        Delay(100);
                        
                        return SETUP;
                     }
                     else
                     {
                        printf("Could not create closecrash_bob[6] bob\n");
                     }
                     free_bob(closecrash_bob[5]);
                  }
                  else
                  {
                     printf("Could not create closecrash_bob[5] bob\n");
                  }
                  free_bob(closecrash_bob[4]);
               }
               else
               {
                  printf("Could not create closecrash_bob[4] bob\n");
               }
               free_bob(closecrash_bob[3]);
            }
            else
            {
               printf("Could not create closecrash_bob[3] bob\n");
            }
            free_bob(closecrash_bob[2]);
         }
         else
         {
            printf("Could not create closecrash_bob[2] bob\n");
         }
         free_bob(closecrash_bob[1]);
      }
      else
      {
         printf("Could not create closecrash_bob[1] bob\n");
      }
      free_bob(closecrash_bob[0]);
   }
   else
   {
      printf("Could not create closecrash_bob[0] bob\n");
   }
   
   return FAIL;
}


/*
** put_small_flames()
**
** Add the correct thruster flame bobs to the gels list depending on
** which keys the user is holding down, and position them correctly
** according to probe bob's position.  Remove any flame bobs no longer
** required.
*/
void put_small_flames(struct ProbeInfo *probeinfo, struct Bob *probebob,
   struct Bob *leftflamebob, struct Bob *rightflamebob,
   struct Bob *downflamebob[2])
{
   static int left=FALSE, right=FALSE, down=FALSE, x=0;
   
   
   /* check if left thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_LEFT) && (probeinfo->fuel > 0))
   {
      if (left == FALSE)
      {
         AddBob(leftflamebob, window->RPort);
         begin_left_thruster_sound();
         left = TRUE;
      }
      leftflamebob->BobVSprite->X = probebob->BobVSprite->X;
      leftflamebob->BobVSprite->Y = probebob->BobVSprite->Y + 5;
   }
   else
   {
      if (left == TRUE)
      {
         RemBob(leftflamebob);
         end_left_thruster_sound();
         left = FALSE;
      }
   }
   /* */
   
   /* check if right thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_RIGHT) && (probeinfo->fuel > 0))
   {
      if (right == FALSE)
      {
         AddBob(rightflamebob, window->RPort);
         begin_right_thruster_sound();
         right = TRUE;
      }
      rightflamebob->BobVSprite->X = probebob->BobVSprite->X + 20;
      rightflamebob->BobVSprite->Y = probebob->BobVSprite->Y + 5;
   }
   else
   {
      if (right == TRUE)
      {
         RemBob(rightflamebob);
         end_right_thruster_sound();
         right = FALSE;
      }
   }
   /* */
   
   /* check if main thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_UP) && (probeinfo->fuel > 0))
   {
      if (down == FALSE)
      {
         x ^= 1;  /* alternate x between 0 and 1 */
         AddBob(downflamebob[x], window->RPort);
         begin_main_thruster_sound();
         down = TRUE;
      }
      else
      {
         RemBob(downflamebob[x]);
         x ^= 1;
         AddBob(downflamebob[x], window->RPort);
      }
      downflamebob[x]->BobVSprite->X = probebob->BobVSprite->X + 7;
      downflamebob[x]->BobVSprite->Y = probebob->BobVSprite->Y + 16;
   }
   else
   {
      if (down == TRUE)
      {
         RemBob(downflamebob[x]);
         end_main_thruster_sound();
         down = FALSE;
      }
   }
   /* */
}


/*
** put_big_flames()
**
** Add the correct thruster flame bobs to the gels list depending on
** which keys the user is holding down, and position them correctly
** according to probe bob's position.  Remove any flame bobs no longer
** required.
*/
void put_big_flames(struct ProbeInfo *probeinfo, struct Bob *probebob,
   struct Bob *leftflamebob, struct Bob *rightflamebob,
   struct Bob *downflamebob[2])
{
   static int left=FALSE, right=FALSE, down=FALSE, x=0;
   
   
   /* check if left thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_LEFT) && (probeinfo->fuel > 0))
   {
      if (left == FALSE)
      {
         AddBob(leftflamebob, window->RPort);
         begin_left_thruster_sound();
         left = TRUE;
      }
      leftflamebob->BobVSprite->X = probebob->BobVSprite->X;
      leftflamebob->BobVSprite->Y = probebob->BobVSprite->Y + 10;
   }
   else
   {
      if (left == TRUE)
      {
         RemBob(leftflamebob);
         end_left_thruster_sound();
         left = FALSE;
      }
   }
   /* */
   
   /* check if right thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_RIGHT) && (probeinfo->fuel > 0))
   {
      if (right == FALSE)
      {
         AddBob(rightflamebob, window->RPort);
         begin_right_thruster_sound();
         right = TRUE;
      }
      rightflamebob->BobVSprite->X = probebob->BobVSprite->X + 40;
      rightflamebob->BobVSprite->Y = probebob->BobVSprite->Y + 10;
   }
   else
   {
      if (right == TRUE)
      {
         RemBob(rightflamebob);
         end_right_thruster_sound();
         right = FALSE;
      }
   }
   /* */
   
   /* check if main thruster is activated by user */
   if ((probeinfo->user_input & CURSOR_UP) && (probeinfo->fuel > 0))
   {
      if (down == FALSE)
      {
         x ^= 1;  /* alternate x between 0 and 1 */
         AddBob(downflamebob[x], window->RPort);
         begin_main_thruster_sound();
         down = TRUE;
      }
      else
      {
         RemBob(downflamebob[x]);
         x ^= 1;
         AddBob(downflamebob[x], window->RPort);
      }
      downflamebob[x]->BobVSprite->X = probebob->BobVSprite->X + 14;
      downflamebob[x]->BobVSprite->Y = probebob->BobVSprite->Y + 32;
   }
   else
   {
      if (down == TRUE)
      {
         RemBob(downflamebob[x]);
         end_main_thruster_sound();
         down = FALSE;
      }
   }
   /* */
}
