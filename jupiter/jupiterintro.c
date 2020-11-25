/*
** jupiterintro.c  -  Code for intro to Jupiter Lander
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

int intro(void);

int game_title(void);
int put_game_title_gadgets(struct Gadget **glist);
void draw_game_title(void);
int process_game_title_events(void);

int instructions(void);
int put_instructions_gadgets(struct Gadget **glist);
void draw_instructions(void);
int process_instructions_events(void);

int clipping_to_borders(void);

/* jupitermain.c */
extern void unclip_window(void);


/** GLOBAL VARIABLES **/

/* jupitermain.c */
extern struct Library *IntuitionBase;
extern struct Library *GfxBase;
extern struct Library *GadToolsBase;
extern struct Library *LayersBase;

extern struct Window *window;

extern int title_height;

/* jl_images.c */
extern UWORD *title_image_plane_1;
extern UWORD *title_image_plane_2;


/*-----------------------------------------------------------------------*/


/*
** clipping_to_borders()
**
** Set up a clipping region in the window such that any drawing functions
** are clipped at the borders of the window's frame.  Dispose of any
** previously installed clipping regions.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
**/
int clipping_to_borders()
{
   struct Region *old_region;
   struct Region *new_region;
   struct Rectangle my_rectangle;
   
   /* set up the limits for the clip */
   my_rectangle.MinX = window->BorderLeft;
   my_rectangle.MinY = window->BorderTop;
   my_rectangle.MaxX = (window->Width)-(window->BorderRight)-1;
   my_rectangle.MaxY = (window->Height)-(window->BorderBottom)-1;
   /* */
   
   /* get a new region and OR in the limits */
   new_region = NewRegion();
   
   if (new_region == NULL)
   {
      printf("Could not install a clipping region in window\n");
      return FALSE;
   }
   if (FALSE == OrRectRegion(new_region, &my_rectangle))
   {
      DisposeRegion(new_region);
      printf("Could not install a rectangle in region\n");
      return FALSE;
   }
   /* */

   /* Dispose of previous region if one was previously installed. */   
   old_region = InstallClipRegion(window->WLayer, new_region);
   
   if (old_region != NULL)
   {
      DisposeRegion(old_region);
   }
   /* */
   
   return TRUE;
}


/*
** intro()
**
** Run the intro to the game diplaying the game title and the instructions.
** 
** RETURNS:  either PLAY or QUIT depending on user's choice or the
**           occurence of an error.
*/
int intro()
{
   int result;
   
   /* set up clipping region for window (to window's borders) */
   if ( clipping_to_borders() == FALSE)
   {
      return QUIT;
   }
   /* */
   
   /* Loop between game title & instructions according to user's inputs
   ** until either PLAY or QUIT is selected or a FAIL returns.
   */
   FOREVER
   {
      /* handle game title */
      result = game_title();
      
      switch (result)
      {
         case PLAY:
               return PLAY;
               break;
         case QUIT:
         case FAIL:
               return QUIT;
               break;
         /*case HELP:*/    /* not necessary */
      }
      /* */
      
      /* handle instructions */
      result = instructions();

      switch (result)
      {
         case QUIT:
         case FAIL:
               return QUIT;
               break;
         /*case OK:*/      /* not necessary */
      }
      /* */
   }
   /* */
}


/*
** game_title()
**
** Handle the displaying of the game title.  Read gadgets in window
** (play, help, quit & closegadget) and return result of user's choice
** to intro().
**
** RETURNS:  PLAY, HELP, QUIT or FAIL depending on user's selection
**           or the occurence of error.
*/
int game_title()
{
   int result;
   struct Gadget *glist;
   
   SetRast(window->RPort, GREY);
   
   /* draw the graphics in this window */
   draw_game_title();
   
   /* put the 3 gadgets (play, help, quit) in the window */
   result = put_game_title_gadgets(&glist);
   
   if (result == FALSE)
   {
      return FAIL;
   }
   /* */
   
   /* handle input events for this window */
   result = process_game_title_events();
   
   /* remove the gadgets from the window */
   RemoveGList(window, glist, -1);
   FreeGadgets(glist);
   /* */
   
   return result;
}


/*
** put_game_title_gadgets()
**
** Add the 3 gadgets (play, help, quit) to the window.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
*/
int put_game_title_gadgets(struct Gadget **glist)
{
   struct Gadget *gad;
   struct NewGadget newgad;
   void *vi;   /* pointer to VisualInfo structure */
   struct TextAttr Topaz80 = {"topaz.font",8,0,0,};
   
   *glist = NULL;
   gad = CreateContext(glist);
   
   /* get the visualinfo for Workbench screen for gadget rendering */
   vi = GetVisualInfo(window->WScreen, TAG_END);
   
   if (vi == NULL)
   {
      printf("Could not get VisualInfo\n");
      return FALSE;
   }
   /* */
   
   /* these values are the same for all 3 gadgets */
   newgad.ng_TopEdge = 168+title_height;
   newgad.ng_Width = 54;
   newgad.ng_Height = 12;
   newgad.ng_TextAttr = &Topaz80;
   newgad.ng_Flags = PLACETEXT_IN;
   newgad.ng_VisualInfo = vi;
   newgad.ng_UserData = NULL;
   /* */
   
   /* set up each individual newgadget structure & create gadget */
   newgad.ng_LeftEdge = 32;
   newgad.ng_GadgetText = "_PLAY";
   newgad.ng_GadgetID = PLAY_BUTTON;
   
   gad = CreateGadget(BUTTON_KIND, gad, &newgad, GT_Underscore, '_',
                        TAG_END);
   
   newgad.ng_LeftEdge = 133;
   newgad.ng_GadgetText = "_HELP";
   newgad.ng_GadgetID = HELP_BUTTON;
   
   gad = CreateGadget(BUTTON_KIND, gad, &newgad, GT_Underscore, '_',
                        TAG_END);
   
   newgad.ng_LeftEdge = 234;
   newgad.ng_GadgetText = "_QUIT";
   newgad.ng_GadgetID = QUIT_BUTTON;
   
   gad = CreateGadget(BUTTON_KIND, gad, &newgad, GT_Underscore, '_',
                        TAG_END);
   /* */

   /* free the visualinfo structure obtained for gadget rendering */
   FreeVisualInfo(vi);
   
   /* check result in gad to see if operation was successful */
   if (gad != NULL)
   {
      /* put these gadgets in the window */
      AddGList(window, *glist, ~0, -1, NULL);
      RefreshGList(*glist, window, NULL, -1);
      GT_RefreshWindow(window, NULL);
      /* */
   
      return TRUE;
   }
   else
   {
      /* failure, remove gadgets */
      FreeGadgets(*glist);
      
      return FALSE;
   }
   /* */
}


/*
** draw_game_title()
**
** Draw the title picture into the game window.
*/
VOID draw_game_title()
{
   struct BitMap titleBitMap;
   
   /* set up the bitmap structure for the bitplanes we already have */
   InitBitMap(&titleBitMap, 2, 313, 187);
   
   titleBitMap.Planes[0] = (PLANEPTR)title_image_plane_1;
   titleBitMap.Planes[1] = (PLANEPTR)title_image_plane_2;
   /* */
   
   /* blit the contents of the bitmap into the window */
   BltBitMapRastPort(&titleBitMap, 0, 0, window->RPort, 3, title_height,
                     313, 187, 0xc0);
}


/*
** process_game_title_events()
**
** Read the IDCMP port for the window and report a gadget selection, or a
** closewindow selection.  Take care of gadget refreshing when required.
**
** RETURNS:  Returns PLAY, HELP or QUIT depending on user selection.
*/
int process_game_title_events()
{
   struct IntuiMessage *imsg;
   struct Gadget *gad;
   ULONG class;
   USHORT code;
   
   FOREVER
   {
      Wait(1 << window->UserPort->mp_SigBit);
      
      while ( imsg = GT_GetIMsg(window->UserPort) )
      {
         /* get needed info from this message & then reply to it */
         class = imsg->Class;
         code = imsg->Code;
         GT_ReplyIMsg(imsg);
         /* */
         
         /* act on the received message */
         switch (class)
         {
            case IDCMP_GADGETUP:
                  /* determine which gadget was selected */
                  gad = (struct Gadget *)imsg->IAddress;
                  
                  switch (gad->GadgetID)
                  {
                     case PLAY_BUTTON:
                           return PLAY;
                           break;
                     case HELP_BUTTON:
                           return HELP;
                           break;
                     case QUIT_BUTTON:
                           return QUIT;
                           break;
                  }
                  /* */
                  break;
            case IDCMP_VANILLAKEY:
                  /* check if a keyboard equivalent was selected */
                  switch (code)
                  {
                     case 'p':
                     case 'P':
                           return PLAY;
                           break;
                     case 'h':
                     case 'H':
                           return HELP;
                           break;
                     case 'q':
                     case 'Q':
                           return QUIT;
                           break;
                  }
                  /* */
                  break;
            case IDCMP_CLOSEWINDOW:
                  return QUIT;
                  break;
            case IDCMP_REFRESHWINDOW:
                  /* refreshing of gadgets is necessary */
                  GT_BeginRefresh(window);
                  GT_EndRefresh(window, TRUE);
                  /* */
                  break;
         }
         /* */
      }
   }
}


/*
** instructions()
**
** Handle the displaying of the instructions.  Read gadgets in window
** (ok & closegadget) and return result of user's choice to intro().
**
** RETURNS:  OK, QUIT or FAIL depending on user's selection
**           or the occurence of error.
*/
int instructions()
{
   int result;
   struct Gadget *glist;
   
   SetRast(window->RPort, GREY);
   
   /* put the gadget (ok) in the window */
   result = put_instructions_gadgets(&glist);
   
   if (result == FALSE)
   {
      return FAIL;
   }
   /* */
   
   /* draw the graphics in this window */
   draw_instructions();
   
   /* handle input events for this window */
   result = process_instructions_events();
   
   /* remove the gadget from the window */
   RemoveGList(window, glist, -1);
   FreeGadgets(glist);
   /* */
   
   return result;
}


/*
** put_instructions_gadgets()
**
** Add the gadget (ok) to the window.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
*/
int put_instructions_gadgets(struct Gadget **glist)
{
   struct Gadget *gad;
   struct NewGadget newgad;
   void *vi;   /* pointer to VisualInfo structure */
   struct TextAttr Topaz80 = {"topaz.font",8,0,0,};
   
   *glist = NULL;
   gad = CreateContext(glist);
   
   /* get the visualinfo for Workbench screen for gadget rendering */
   vi = GetVisualInfo(window->WScreen, TAG_END);
   
   if (vi == NULL)
   {
      printf("Could not get VisualInfo\n");
      return FALSE;
   }
   /* */
   
   /* set up newgadget structure & create gadget */
   newgad.ng_TopEdge = 168+title_height;
   newgad.ng_Width = 54;
   newgad.ng_Height = 12;
   newgad.ng_TextAttr = &Topaz80;
   newgad.ng_Flags = PLACETEXT_IN;
   newgad.ng_VisualInfo = vi;
   newgad.ng_UserData = NULL;
   
   newgad.ng_LeftEdge = 133;
   newgad.ng_GadgetText = "_OK";
   newgad.ng_GadgetID = OK_BUTTON;
   
   gad = CreateGadget(BUTTON_KIND, gad, &newgad, GT_Underscore, '_',
                        TAG_END);
   /* */

   /* free the visualinfo structure obtained for gadget rendering */
   FreeVisualInfo(vi);
   
   /* check result in gad to see if operation was successful */
   if (gad != NULL)
   {
      /* put this gadget in the window */
      AddGList(window, *glist, ~0, -1, NULL);
      RefreshGList(*glist, window, NULL, -1);
      GT_RefreshWindow(window, NULL);
      /* */
   
      return TRUE;
   }
   else
   {
      /* failure, remove gadget */
      printf("Could not put create OK gadget\n");
      FreeGadgets(*glist);
      
      return FALSE;
   }
   /* */
}


/*
** draw_instructions()
**
** Draw all of the instructions text into the game window.
*/
VOID draw_instructions()
{
   struct IntuiText instrIText[12];
   struct TextAttr instrTextAttr;
   int x;
   
   /* create a TextAttr that matches our font with bold set */
   instrTextAttr.ta_Name = window->RPort->Font->tf_Message.mn_Node.ln_Name;
   instrTextAttr.ta_YSize = window->RPort->Font->tf_YSize;
   instrTextAttr.ta_Style = window->RPort->Font->tf_Style | FSF_BOLD;
   instrTextAttr.ta_Flags = window->RPort->Font->tf_Flags;
   /* */
   
   /* set up values similar in all IntuiText structures */
   for (x=0; x<12; x++)
   {
      instrIText[x].BackPen = GREY;
      instrIText[x].DrawMode = JAM1;
      instrIText[x].ITextFont = NULL;
   }
   for (x=0; x<11; x++)
   {
      instrIText[x].NextText = &instrIText[x+1];
   }
   instrIText[11].NextText = NULL;
   /* */
   
   /* set up remaining value in IntuiText structures */
   instrIText[0].LeftEdge = 86;
   instrIText[0].TopEdge = 8;
   instrIText[0].IText = "JUPITER LANDER 0.9";
   instrIText[0].ITextFont = &instrTextAttr;
   
   instrIText[1].LeftEdge = 87;
   instrIText[1].TopEdge = 24;
   instrIText[1].IText = "©1993 Paul Grebenc";
   
   instrIText[2].LeftEdge = 23;
   instrIText[2].TopEdge = 40;
   instrIText[2].IText = "The object of this game is to land";
   
   instrIText[3].LeftEdge = 47;
   instrIText[3].TopEdge = 48;
   instrIText[3].IText = "the probe on one of the pads";
   
   instrIText[4].LeftEdge = 27;
   instrIText[4].TopEdge = 56;
   instrIText[4].IText = "lightly enough not to destroy it.";
   
   instrIText[5].LeftEdge = 19;
   instrIText[5].TopEdge = 72;
   instrIText[5].IText = "Use the 'A' and 'D' keys to control";
   
   instrIText[6].LeftEdge = 27;
   instrIText[6].TopEdge = 80;
   instrIText[6].IText = "your side thrusters and the right";
   
   instrIText[7].LeftEdge = 39;
   instrIText[7].TopEdge = 88;
   instrIText[7].IText = "SHIFT key to control your main";
   
   instrIText[8].LeftEdge = 123;
   instrIText[8].TopEdge = 96;
   instrIText[8].IText = "thruster.";
   
   instrIText[9].LeftEdge = 23;
   instrIText[9].TopEdge = 112;
   instrIText[9].IText = "See the README file for more info.";
   
   instrIText[10].LeftEdge = 31;
   instrIText[10].TopEdge = 128;
   instrIText[10].IText = "Title graphics by Andrew Powell.";
   
   instrIText[11].LeftEdge = 15;
   instrIText[11].TopEdge = 144;
   instrIText[11].IText = "This game may be freely distributed.";
   /* */
   
   /* Print the background shadow of all the text */
   instrIText[0].FrontPen = BLACK;
   instrIText[1].FrontPen = BLACK;
   for (x=2; x<11; x++)
   {
      instrIText[x].FrontPen = GREY;
   }
   instrIText[11].FrontPen = BLACK;
      
   /* Draw the text strings relative to base position 0,title_height */
   PrintIText(window->RPort, instrIText, 1, title_height+5);
   /* */
   
   /* Now print the text in it's main color */
   instrIText[0].FrontPen = WHITE;
   instrIText[1].FrontPen = WHITE;
   for (x=2; x<11; x++)
   {
      instrIText[x].FrontPen = BLACK;
   }
   instrIText[11].FrontPen = WHITE;
   
   PrintIText(window->RPort, instrIText, 0, title_height+4);
   /* */
}


/*
** process_instructions_events()
**
** Read the IDCMP port for the window and report a gadget selection, or a
** closewindow selection.  Take care of gadget refreshing when required.
**
** RETURNS:  Returns OK or QUIT depending on user selection.
*/
int process_instructions_events()
{
   struct IntuiMessage *imsg;
   struct Gadget *gad;
   ULONG class;
   USHORT code;
   
   FOREVER
   {
      Wait(1 << window->UserPort->mp_SigBit);
      
      while ( imsg = GT_GetIMsg(window->UserPort) )
      {
         /* get needed info from this message & then reply to it */
         class = imsg->Class;
         code = imsg->Code;
         GT_ReplyIMsg(imsg);
         /* */
         
         /* act on the received message */
         switch (class)
         {
            case IDCMP_GADGETUP:
                  /* determine if OK gadget was selected */
                  gad = (struct Gadget *)imsg->IAddress;
                  
                  if (gad->GadgetID == OK_BUTTON)
                  {
                     return OK;
                  }
                  /* */
                  break;
            case IDCMP_VANILLAKEY:
                  /* check if keyboard equivalent was selected */
                  if ((code == 'o') || (code == 'O'))
                  {
                     return OK;
                  }
                  /* */
                  break;
            case IDCMP_CLOSEWINDOW:
                  return QUIT;
                  break;
            case IDCMP_REFRESHWINDOW:
                  /* refreshing of gadget is necessary */
                  GT_BeginRefresh(window);
                  GT_EndRefresh(window, TRUE);
                  /* */
                  break;
         }
         /* */
      }
   }
}
