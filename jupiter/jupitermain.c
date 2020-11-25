/*
** JUPITER LANDER v0.9
**
** (c)1993 Paul Grebenc
**
** Started:  05/26/93
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

/* jl_memory.c */
extern void decompress_images(void);
extern void free_images(void);

void open_libraries(void);
void close_libraries(void);

void open_window(void);
void close_window(void);
int check_workbench_depth(void);
int get_workbench_depth(void);
int get_window_title_height(void);

void unclip_window(void);

/* jupiterintro.c */
extern int intro(void);

/* jupitergame.c */
extern int game(void);


/** GLOBAL VARIABLES **/

static char version[] = "\0$VER: Jupiter_Lander 0.90e (25.8.93)";

struct Library *IntuitionBase;
struct Library *GfxBase;
struct Library *GadToolsBase;
struct Library *LayersBase;
struct Library *DOSBase;

struct Window *window;

int title_height;


/*-----------------------------------------------------------------------*/


main()
{
   decompress_images();
   
   open_libraries();
   
   open_window();
   
   /* set the priority of this program to 127 */
   SetTaskPri( FindTask(0), 127);
    
   FOREVER
   {
      if ( QUIT == intro() )
         break;
      
      if ( QUIT == game() )
         break;
   }
   
   close_window();
   
   close_libraries();
   
   free_images();
   
   exit(RETURN_OK);
}


/*
** open_libraries()
**
** Open all the libraries used in this program.  Exit on error.
*/
void open_libraries()
{
   IntuitionBase = OpenLibrary("intuition.library",36);
   if (IntuitionBase != NULL)
   {
      GfxBase = OpenLibrary("graphics.library",36);
      if (GfxBase != NULL)
      {
         GadToolsBase = OpenLibrary("gadtools.library",36);
         if (GadToolsBase != NULL)
         {
            LayersBase = OpenLibrary("layers.library",36);
            if (LayersBase != NULL)
            {
               DOSBase = OpenLibrary("dos.library",36);
               if (DOSBase != NULL)
               {
                  return;
               }
               printf("Could not open dos.library V36\n");
               CloseLibrary(LayersBase);
            }
            else
            {
               printf("Could not open layers.library V36\n");
            }
            CloseLibrary(GadToolsBase);
         }
         else
         {
            printf("Could not open gadtools.library V36\n");
         }
         CloseLibrary(GfxBase);
      }
      else
      {
         printf("Could not open graphics.library V36\n");
      }
      CloseLibrary(IntuitionBase);
   }
   else
   {
      printf("Could not open intuition.library V36\n");
   }
   
   free_images();
   
   exit(RETURN_FAIL);
}



/*
** close_libraries()
**
** Close all libraries opened by open_libraries().
*/
void close_libraries()
{
   CloseLibrary(DOSBase);
   CloseLibrary(LayersBase);
   CloseLibrary(GadToolsBase);
   CloseLibrary(GfxBase);
   CloseLibrary(IntuitionBase);
}


/*
** open_window()
**
** Open window on Workbench for program to use.  Exit on error.
** Also inform & exit if depth of Workbench is not at least 2 (4 colors).
*/
void open_window()
{
   struct TextAttr ta = {"topaz.font", 8, NULL, FPF_ROMFONT | FPF_DESIGNED};
   struct TextFont *windowfont;
   
   /* get the height of the font used in this window's title */
   title_height = get_window_title_height();
   
   if ( FALSE == check_workbench_depth() )
   {
      printf("Workbench screen must have at least 4 colors!\n");
      close_libraries();
      exit(RETURN_WARN);
   }
   
   window = OpenWindowTags(NULL,
               WA_Left,          WINDOW_LEFT,
               WA_Top,           WINDOW_TOP,
               WA_Width,         WINDOW_WIDTH,
               WA_Height,        WINDOW_HEIGHT + title_height,
               WA_DragBar,       TRUE,
               WA_DepthGadget,   TRUE,
               WA_CloseGadget,   TRUE,
               WA_Activate,      TRUE,
               WA_SmartRefresh,  TRUE,
               WA_IDCMP,         IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW |
                                 IDCMP_VANILLAKEY |
                                 IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW |
                                 BUTTONIDCMP,
               WA_Title,         "Jupiter Lander 0.90e",
               WA_ScreenTitle,   "JUPITER LANDER - (c)1993 Paul Grebenc",
               TAG_END);
   
   if (window == NULL)
   {
      printf("Could not open window\n");
      close_libraries();
      free_images();
      exit(RETURN_WARN);
   }
   
   /* open the font Topaz 8 for use in this window */
   if ((windowfont = OpenFont(&ta)) == NULL)
   {
      printf("Could not open font Topaz 8\n");
      close_libraries();
      free_images();
      exit(RETURN_WARN);
   }
   /* */
   
   /* set the font for this window to windowfont */
   SetFont(window->RPort, windowfont);
}


/*
** close_window()
**
** Close the window opened by open_window (remove any clipping regions).
*/
void close_window()
{
   /* remove any existing clipping regions */
   unclip_window();
   
   /* close the font we opened for this window */
   CloseFont(window->RPort->Font);
   
   CloseWindow(window);
}


/*
** get_window_title_height()
**
** RETURNS:  Height of the window's title bar in pixels, which varies
**           according to the default screen font selected in Preferences
*/
int get_window_title_height()
{
   struct Screen *wbscrn = NULL;
   struct DrawInfo *wbscrn_drawinfo = NULL;
   int height;
   
   /* get the height of the screen font used for window titles */
   wbscrn = LockPubScreen("Workbench");
   if (wbscrn != NULL)
   {
      wbscrn_drawinfo = GetScreenDrawInfo(wbscrn);
      if (wbscrn_drawinfo != NULL)
      {
         height = wbscrn_drawinfo->dri_Font->tf_YSize + 3;
         FreeScreenDrawInfo(wbscrn, wbscrn_drawinfo);
      }
      UnlockPubScreen("Workbench", wbscrn);
   }
   /* */
   
   return height;
}


/*
** get_workbench_depth()
**
** RETURNS:  Depth in bitplanes of Workbench screen.
*/
int get_workbench_depth()
{
   /* screen and drawinfo used to check if Workbench depth >= 2 */
   struct Screen *wbscrn = NULL;
   struct DrawInfo *wbscrn_drawinfo = NULL;
   UWORD depth = NULL;
   
   /* get the current depth of the Workbench screen */
   wbscrn = LockPubScreen("Workbench");
   if (wbscrn != NULL)
   {
      wbscrn_drawinfo = GetScreenDrawInfo(wbscrn);
      if (wbscrn_drawinfo != NULL)
      {
         depth = wbscrn_drawinfo->dri_Depth;
         FreeScreenDrawInfo(wbscrn,wbscrn_drawinfo);
      }
      UnlockPubScreen("Workbench",wbscrn);
   }
   /* */
   
   return depth;
}


/*
** check_workbench_depth()
**
** Check depth of Workbench screen to make sure it is >= 2.
**
** RETURNS:  TRUE if depth >=2, FALSE if depth < 2.
*/
int check_workbench_depth()
{
   int depth;
   
   depth = get_workbench_depth();
   
   /* check depth */
   if (depth >= 2)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
   /* */
}


/*
** unclip_window()
**
** Remove any existing clipping regions from the window by installing a
** NULL region, then dispose of the old region if one was installed.
*/
void unclip_window()
{
   struct Region *old_region;
   
   if (NULL != (old_region = InstallClipRegion(window->WLayer,NULL)))
   {
      DisposeRegion(old_region);
   }
}
