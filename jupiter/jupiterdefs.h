/*
** jupiterdefs.h - defines to be included in Jupiter Lander source
**
** (c)1993 Paul Grebenc
*/


#define INTUI_V36_NAMES_ONLY


/** INCLUDES **/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include <devices/audio.h>
#include <devices/keyboard.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <graphics/collide.h>
#include <graphics/displayinfo.h>
#include <graphics/gels.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>


/** DEFINES **/

/* window stuff */
#define WINDOW_LEFT     (320)
#define WINDOW_TOP      (0)
#define WINDOW_WIDTH    (320)
#define WINDOW_HEIGHT   (189)

/* return codes */
#define QUIT         (0)
#define FAIL         (1)
#define PLAY         (2)
#define INTRO        (3)
#define HELP         (4)
#define OK           (5)
#define GO_FAR       (6)
#define GO_PAD_A     (7)
#define GO_PAD_B     (8)
#define GO_PAD_C     (9)
#define LANDED       (10)
#define CRASHED      (11)
#define OUT_OF_SKY   (12)
#define SETUP        (13)
#define GAME_OVER    (14)

/* colors */
#define GREY   (0)
#define BLACK  (1)
#define WHITE  (2)
#define BLUE   (3)

/* gadgets */
#define PLAY_BUTTON  (1)
#define HELP_BUTTON  (2)
#define QUIT_BUTTON  (3)
#define OK_BUTTON    (4)

/* rawkey codes */
#define CURSOR_UP    (1)
#define CURSOR_RIGHT (1<<1)
#define CURSOR_LEFT  (1<<2)

/* collision codes */
#define LANDHIT      (1)
#define PROBEHIT     (2)
#define PADHIT       (3)
#define LANDMASK     (1<<LANDHIT)
#define PROBEMASK    (1<<PROBEHIT)
#define PADMASK      (1<<PADHIT)



/* data structure to hold information for a new Bob */
typedef struct newBob {
   UWORD *nb_Image;
   SHORT  nb_WordWidth;
   SHORT  nb_ImageDepth;
   SHORT  nb_LineHeight;
   SHORT  nb_BFlags;
   SHORT  nb_X;
   SHORT  nb_Y;
   SHORT  nb_HitMask;
   SHORT  nb_MeMask;
   SHORT  nb_PlanePick;
   UWORD *nb_CollMask;
} NEWBOB;


/* contains info relevant for controlling probe */
struct ProbeInfo {
   float x_pos;
   float y_pos;
   float x_vel;
   float y_vel;
   float fuel;
   int user_input;
};
