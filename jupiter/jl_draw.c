/*
** jl_draw.c - contains functions to handle graphic drawing in game
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

int draw_game(LONG hiscore);

void draw_velocity_scale(void);
void draw_velocity_symbols(void);
extern update_velocity_scale(struct ProbeInfo *probeinfo);

void draw_fuel_bar(void);

void draw_scores(LONG hiscore);

/* jl_gamestuff.c */
extern update_fuel_bar(struct ProbeInfo *probeinfo);


/** GLOBAL VARIABLES **/

extern struct Library *IntuitionBase;
extern struct Library *GfxBase;

extern struct Window *window;

extern int title_height;

extern UWORD chip mps_image[1*10*2];
extern UWORD chip zero_image[1*8*1];
extern UWORD chip ten_image[1*8*1];


/*-----------------------------------------------------------------------*/


/*
** draw_game()
**
** Draw the all relevant images in the window for the game.  The land
** masses are not drawn here, they are bobs handled in the game code.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
*/
int draw_game(LONG hiscore)
{
   SetRast(window->RPort, BLACK);

   draw_velocity_scale();
   draw_fuel_bar();
   draw_scores(hiscore);
   
   return TRUE;
}


/*
** draw_velocity_scale()
**
** Draw the velocity scale at the right in the game window and initialize
** it to its beginning position.
*/
void draw_velocity_scale()
{
   struct ProbeInfo dummy_probeinfo;
   
   
   SetDrMd(window->RPort, JAM1);
   
   SetAPen(window->RPort, GREY);
   RectFill(window->RPort, 295, 9+title_height, 309, 168+title_height);
   
   SetAPen(window->RPort, BLUE);
   RectFill(window->RPort, 295, 89+title_height, 309, 102+title_height);
   
   /* initialize position on scale */
   dummy_probeinfo.y_vel = 0.0;
   
   update_velocity_scale(&dummy_probeinfo);
   /* */
   
   /* draw the symbols next to the velocity scale */
   draw_velocity_symbols();
}


/*
** draw_velocity_symbols()
**
** Draw the "m/s", "+-0" and "-10" symbols next to the velocity scale.
*/
void draw_velocity_symbols()
{
   SetDrMd(window->RPort, JAM1);
   SetAPen(window->RPort, WHITE);
   
   /* draw the "m/s" */
   BltPattern(window->RPort, (PLANEPTR)&mps_image, 275, 4+title_height,
      275+31, 4+title_height+9, 4);
   
   /* draw the "+-0" */
   BltPattern(window->RPort, (PLANEPTR)&zero_image, 280, 85+title_height,
      280+15, 85+title_height+7, 2);
   
   /* draw the "-10" */
   BltPattern(window->RPort, (PLANEPTR)&ten_image, 277, 164+title_height,
      277+15, 164+title_height+7, 2);
}


/*
** draw_fuel_bar()
**
** Draw the fuel bar at the bottom of the game window at it's initial
** size.
*/
void draw_fuel_bar()
{
   struct ProbeInfo dummy_probeinfo;
   

   /* print "FUEL:" in window */
   SetDrMd(window->RPort, JAM1);
   SetAPen(window->RPort, WHITE);
   Move(window->RPort, 10, 185+title_height);
   Text(window->RPort, "FUEL:", 5);
   /* */

   /* draw the rectangle for the fuel bar */
   SetAPen(window->RPort, BLUE);
   RectFill(window->RPort, 60, 178+title_height, 279, 185+title_height);
   /* */
   
   /* give update_fuel_bar an initial value for old_fuel */
   dummy_probeinfo.fuel = 219.0;
   update_fuel_bar(&dummy_probeinfo);
   /* */
}


/*
** draw_scores()
**
** Draw the headings for score and hiscore, print the score as 00000, and
** print the current hiscore which was passed.
*/
void draw_scores(LONG hiscore)
{
   char s[13];       /* dest. for conversion LONG->ASCII */
   char output[6];   /* output string to be printed (hiscore) */
   int n;   /* length value for conversion */
   int x;   /* counter */
   
   
   /* print SCORE: 00000 and HISCORE: */
   SetDrMd(window->RPort, JAM1);
   SetAPen(window->RPort, WHITE);
   
   Move(window->RPort, 10, 176+title_height);
   Text(window->RPort, "SCORE: 00000", 12);
   
   Move(window->RPort, 140, 176+title_height);
   Text(window->RPort, "HISCORE:", 8);
   /* */
   
   /* setup output string for hiscore and print */
   n = stcl_d(s, hiscore);
   for (x=0; x < (5-n); x++)
   {
      output[x] = '0';
   }
   strcpy(&output[5-n], s);
   
   Move(window->RPort, 212, 176+title_height);
   Text(window->RPort, output, 5);
   /* */
}
