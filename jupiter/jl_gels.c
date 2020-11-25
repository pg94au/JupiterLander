/*
** gels.c - code for handling the bobs used in Jupiter Lander
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** PROTOTYPES **/

extern int get_workbench_depth(void);

int setup_gels(void);
void cleanup_gels(void);
struct Bob *create_bob(NEWBOB *nBob);
void free_bob(struct Bob *bob);
void redraw_bobs(void);

/** GLOBAL VARIABLES **/

extern struct Library *IntuitionBase;
extern struct Library *GfxBase;

extern struct Window *window;

/*-----------------------------------------------------------------------*/


/*
** setup_gels()
**
** Set up the GELS system for use in game.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
*/
int setup_gels()
{
   struct GelsInfo *gInfo;
   struct VSprite *vsHead;
   struct VSprite *vsTail;
   
   if (NULL != (gInfo = (struct GelsInfo *)AllocMem(sizeof(struct GelsInfo), MEMF_CLEAR)))
   {
      if (NULL != (gInfo->nextLine = (WORD *)AllocMem(sizeof(WORD)*8, MEMF_CLEAR)))
      {
         if (NULL != (gInfo->lastColor = (WORD **)AllocMem(sizeof(LONG)*8, MEMF_CLEAR)))
         {
            if (NULL != (gInfo->collHandler = (struct collTable *)
                  AllocMem(sizeof(struct collTable), MEMF_CLEAR)))
            {
               if (NULL != (vsHead = (struct VSprite *)
                     AllocMem((LONG)sizeof(struct VSprite), MEMF_CLEAR)))
               {
                  if (NULL != (vsTail = (struct VSprite *)
                        AllocMem((LONG)sizeof(struct VSprite), MEMF_CLEAR)))
                  {
                     gInfo->sprRsrvd = 0x00;
                     window->RPort->GelsInfo = gInfo;
                     InitGels(vsHead, vsTail, gInfo);
                     
                     return TRUE;
                  }
                  else
                  {
                     printf("Could not allocate vsTail\n");
                  }
                  FreeMem(vsHead,(LONG)sizeof(*vsHead));
               }
               else
               {
                  printf("Could not allocate vsHead\n");
               }
               FreeMem(gInfo->collHandler,(LONG)sizeof(struct collTable));
            }
            else
            {
               printf("Could not allocate GelsInfo->collHandler\n");
            }
            FreeMem(gInfo->lastColor, (LONG)sizeof(LONG)*8);
         }
         else
         {
            printf("Could not allocate GelsInfo->lastColor\n");
         }
         FreeMem(gInfo->nextLine, (LONG)sizeof(WORD)*8);
      }
      else
      {
         printf("Could not allocate GelsInfo->nextLine\n");
      }
      FreeMem(gInfo, (LONG)sizeof(struct GelsInfo));
   }
   else
   {
      printf("Could not allocate GelsInfo\n");
   }
   
   return FALSE;
}


/*
** cleanup_gels()
**
** Clean up the GELS system.
*/
void cleanup_gels()
{
   struct GelsInfo *gInfo;
   
   gInfo = window->RPort->GelsInfo;
   
   window->RPort->GelsInfo = NULL;
   FreeMem(gInfo->collHandler, (LONG)sizeof(struct collTable));
   FreeMem(gInfo->lastColor, (LONG)sizeof(LONG)*8);
   FreeMem(gInfo->nextLine, (LONG)sizeof(WORD)*8);
   FreeMem(gInfo->gelHead, (LONG)sizeof(struct VSprite));
   FreeMem(gInfo->gelTail, (LONG)sizeof(struct VSprite));
   FreeMem(gInfo, (LONG)sizeof(struct GelsInfo));
}


/*
** create_bob()
**
** Create a Bob from the information given in nBob.
** Use free_bob() to free this GEL.
**
** RETURNS:  Address of resulting Bob structure if successful,
**           NULL if unsuccessful.
*/
struct Bob *create_bob(NEWBOB *nBob)
{
   struct Bob *bob;
   struct VSprite *vsprite;
   LONG rassize;
   LONG line_size;
   LONG plane_size;
   
   /* calculate values */
   rassize = (LONG)sizeof(UWORD) * nBob->nb_WordWidth * 
      nBob->nb_LineHeight * (get_workbench_depth());
   line_size = sizeof(WORD) * nBob->nb_WordWidth;
   plane_size = line_size * nBob->nb_LineHeight;
   /* */
   
   if (NULL != (bob = (struct Bob *)AllocMem((LONG)sizeof(struct Bob), MEMF_CLEAR)))
   {
      if (NULL != (bob->SaveBuffer = (WORD *)AllocMem(rassize, MEMF_CHIP)))
      {
         if (NULL != (vsprite = (struct VSprite *)AllocMem((LONG)sizeof(struct VSprite), MEMF_CLEAR)))
         {
            if (NULL != (vsprite->BorderLine = (WORD *)AllocMem(line_size, MEMF_CHIP)))
            {
               if(NULL != (vsprite->CollMask = (WORD *)AllocMem(plane_size, MEMF_CHIP)))
               {
                  vsprite->Y = nBob->nb_Y;
                  vsprite->X = nBob->nb_X;
                  vsprite->Flags = nBob->nb_BFlags;
                  vsprite->Width = nBob->nb_WordWidth;
                  vsprite->Depth = nBob->nb_ImageDepth;
                  vsprite->Height = nBob->nb_LineHeight;
                  vsprite->MeMask = nBob->nb_MeMask;
                  vsprite->HitMask = nBob->nb_HitMask;
                  vsprite->ImageData = nBob->nb_Image;
                  vsprite->SprColors = NULL;
                  
                  InitMasks(vsprite);
                  
                  vsprite->PlanePick = nBob->nb_PlanePick;
                  vsprite->PlaneOnOff = 0x00;
                  vsprite->VSBob = bob;
                  
                  bob->BobVSprite = vsprite;
                  bob->ImageShadow = vsprite->CollMask;
                  bob->Flags = NULL;
                  bob->Before = NULL;
                  bob->After = NULL;
                  bob->BobComp = NULL;
                  bob->DBuffer = NULL;

                  if (nBob->nb_CollMask != NULL)
                  {
                     vsprite->CollMask = nBob->nb_CollMask;
                  }
                  
                  return (bob);
               }
               else
               {
                  printf("Could not allocate vsprite->CollMask\n");
               }
               FreeMem(vsprite->BorderLine, line_size);
            }
            else
            {
               printf("Could not allocate vsprite->BorderLine\n");
            }
            FreeMem(vsprite, (LONG)sizeof(struct VSprite));
         }
         else
         {
            printf("Could not allocate bob->SaveBuffer\n");
         }
         FreeMem(bob->SaveBuffer, rassize);
      }
      else
      {
         printf("Could not allocate bob\n");
      }
      FreeMem(bob, (LONG)sizeof(struct Bob));
   }
   
   return NULL;
}


/*
** free_bob()
**
** Free the Bob created by create_bob().
*/
void free_bob(struct Bob *bob)
{
   int rasdepth;
   LONG rassize;
   LONG line_size;
   LONG plane_size;
   
   /* Calculate size of Bob's savebuffer. */
   rasdepth = get_workbench_depth();
   rassize = sizeof(WORD) * bob->BobVSprite->Width *
      bob->BobVSprite->Height * (get_workbench_depth());
   /* */
   
   FreeMem(bob->SaveBuffer, rassize);
   
   line_size = (LONG)sizeof(WORD) * bob->BobVSprite->Width;
   plane_size = line_size * bob->BobVSprite->Height;
   
   FreeMem(bob->BobVSprite->BorderLine, line_size);
   FreeMem(bob->ImageShadow, plane_size);
   FreeMem(bob->BobVSprite, (LONG)sizeof(struct VSprite));
   FreeMem(bob, (LONG)sizeof(struct Bob));
}


/*
** redraw_bobs()
**
** Redraws bobs in the window.  For use after their x and y coordinates
** have been changed to move bobs to their new positions.
** Does a WaitTOF() after redrawing.
*/
void redraw_bobs()
{
   SortGList(window->RPort);
   DrawGList(window->RPort, ViewPortAddress(window));
   WaitTOF();
}
