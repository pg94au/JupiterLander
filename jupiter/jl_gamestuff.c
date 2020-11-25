/*
** jl_gamestuff.c - contains functions used with jupitergame.c
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

void update_velocity_scale(struct ProbeInfo *probeinfo);
void update_fuel_bar(struct ProbeInfo *probeinfo);
void update_score(LONG *score, LONG *hiscore);


/** GLOBAL VARIABLES **/

extern struct Library *IntuitionBase;
extern struct Library *GfxBase;

extern struct Window *window;

extern int title_height;

/*-----------------------------------------------------------------------*/


/*
** update_velocity_scale()
**
** Move line on the vertical velocity scale at the right on the game
** window if velocity has changed.
*/
void update_velocity_scale(struct ProbeInfo *probeinfo)
{
   static int old_scale_pos = 0;
   static LONG old_color = BLUE;
   int new_scale_pos;
   
   /* y-coord of pos on the scale = 89+title_height+((y_vel/2.0)*80.0) */
   new_scale_pos = 89+title_height + (probeinfo->y_vel * 40.0);
   
   if (new_scale_pos != old_scale_pos)
   {
      SetDrMd(window->RPort, JAM1);
      
      /* erase old line */
      SetAPen(window->RPort, old_color);
      Move(window->RPort, 295, old_scale_pos);
      Draw(window->RPort, 309, old_scale_pos);
      /* */
      
      /* store the color below the new position of the line */
      old_color = ReadPixel(window->RPort, 295, new_scale_pos);
      
      /* draw new line */
      SetAPen(window->RPort, BLACK);
      Move(window->RPort, 295, new_scale_pos);
      Draw(window->RPort, 309, new_scale_pos);
      /* */
      
      /* store position of the new line */
      old_scale_pos = new_scale_pos;
   }
}


/*
** update_fuel_bar()
**
** Update the length of the fuel bar by lengthening or shortening the blue
** rectangle based on remaining amount of fuel.
*/
void update_fuel_bar(struct ProbeInfo *probeinfo)
{
   static int old_fuel = 219;
   int new_fuel;
   
   new_fuel = (int)probeinfo->fuel;
   
   if (old_fuel > new_fuel)   /* fuel level has been lowered */
   {
      SetDrMd(window->RPort, JAM1);
      
      /* check if probe is out of fuel */
      if (new_fuel <= 0)
      {
         new_fuel = 0;
         
         /* erase whatever was left of the fuel bar */
         SetAPen(window->RPort, BLACK);
         RectFill(window->RPort, 60, 178+title_height, 279, 185+title_height);
         /* */
         
         /* print "OUT OF FUEL" */
         SetAPen(window->RPort, BLUE);
         Move(window->RPort, 126, 185+title_height);
         Text(window->RPort, "OUT OF FUEL", 11);
         /* */
      }
      /* there's still some fuel */
      else
      {
         SetAPen(window->RPort, BLACK);
         RectFill(window->RPort, 60+new_fuel, 178+title_height, 60+old_fuel, 185+title_height);
      }
      /* */
      
      old_fuel = new_fuel;
   }
   else
   {
      if (old_fuel < new_fuel)   /* fuel level has been raised */
      {
         SetDrMd(window->RPort, JAM1);
         
         /* check if probe was previously out of fuel */
         if (old_fuel == 0)
         {
            SetAPen(window->RPort, BLACK);
            RectFill(window->RPort, 60, 178+title_height, 279, 185+title_height);
         }
         /* */
         
         SetAPen(window->RPort, BLUE);
         RectFill(window->RPort, 60+old_fuel, 178+title_height, 60+new_fuel, 185+title_height);
         
         old_fuel = new_fuel;
      }
   }
}


/*
** update_score()
**
** Update the current score, and also update the hi score if necessary.
** Take care of the score counters looping back to 00000 after 99999.
*/
void update_score(LONG *score, LONG *hiscore)
{
   char s[13];    /* dest. in LONG->ASCII conversion */
   char output[16];  /* final output string */
   int n;   /* counter for LONG->ASCII conversion */
   int x;   /* scratch */
   
   
   /* loop score counters if necessary */
   if (*score > *hiscore)
   {
      if (*score > 99999)
      {
         *score-=100000;
      }
      *hiscore = *score;
   }
   /* */
   
   /* setup string to be printed */
   n = stcl_d(s,*score);
   
   for (x=0; x < (5-n); x++)
   {
      output[x] = '0';
   }
   
   strcpy(&output[5-n], s);
   /* */
   
   /* print score */
   SetDrMd(window->RPort, JAM2);
   SetAPen(window->RPort, WHITE);
   SetBPen(window->RPort, BLACK);
   
   Move(window->RPort, 66, 176+title_height);
   Text(window->RPort, output, 5);
   /*
   
   /* check for new hiscore and print if new */
   if (*score == *hiscore)
   {
      Move(window->RPort, 212, 176+title_height);
      Text(window->RPort, output, 5);
   }
   /* */
}
