/*
** jl_memory.c - handles decompression of program images
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** DEFINES **/

#define FOREVER for(;;)

#define IMAGESIZE (2*17*181)*2
#define MASKSIZE (1*17*181)*2
#define TITLESIZE (1*20*187)*2


/** PROTOTYPES **/

void decompress_images(void);
void free_images(void);
void decompress(UBYTE *source, UBYTE *dest);


/** GLOBAL VARIABLES **/

extern UWORD *title_image_plane_1;
extern UWORD *title_image_plane_2;
extern UWORD *farview_image;
extern UWORD *farview_mask;
extern UWORD *padaview_image;
extern UWORD *padaview_mask;
extern UWORD *padbview_image;
extern UWORD *padbview_mask;
extern UWORD *padcview_image;
extern UWORD *padcview_mask;

extern UBYTE title_image_plane_1_comp[3378];
extern UBYTE title_image_plane_2_comp[4416];
extern UBYTE farview_image_comp[2924];
extern UBYTE farview_mask_comp[2560];
extern UBYTE padaview_image_comp[2107];
extern UBYTE padaview_mask_comp[1887];
extern UBYTE padbview_image_comp[962];
extern UBYTE padbview_mask_comp[739];
extern UBYTE padcview_image_comp[2264];
extern UBYTE padcview_mask_comp[1113];


/*-----------------------------------------------------------------------*/


/*
** decompress_images()
**
** Decompress all compressed images used in game.  Compression method is
** RLE encoding.  Allocate CHIP memory for decompressed images.
**
** RETURNS:  Nothing if sucessful, exit(RETURN_WARN) if unsuccessful.
*/
void decompress_images()
{
   title_image_plane_1 = (UWORD *)AllocMem(TITLESIZE, MEMF_CHIP);
   if (title_image_plane_1 != NULL)
   {
      title_image_plane_2 = (UWORD *)AllocMem(TITLESIZE, MEMF_CHIP);
      if (title_image_plane_2 != NULL)
      {
         farview_image = (UWORD *)AllocMem(IMAGESIZE, MEMF_CHIP);
         if (farview_image != NULL)
         {
            farview_mask = (UWORD *)AllocMem(MASKSIZE, MEMF_CHIP);
            if (farview_mask != NULL)
            {
               padaview_image = (UWORD *)AllocMem(IMAGESIZE, MEMF_CHIP);
               if (padaview_image != NULL)
               {
                  padaview_mask = (UWORD *)AllocMem(MASKSIZE, MEMF_CHIP);
                  if (padaview_mask != NULL)
                  {
                     padbview_image = (UWORD *)AllocMem(IMAGESIZE, MEMF_CHIP);
                     if (padbview_image != NULL)
                     {
                        padbview_mask = (UWORD *)AllocMem(MASKSIZE, MEMF_CHIP);
                        if (padbview_mask != NULL)
                        {
                           padcview_image = (UWORD *)AllocMem(IMAGESIZE, MEMF_CHIP);
                           if (padcview_image != NULL)
                           {
                              padcview_mask = (UWORD *)AllocMem(MASKSIZE, MEMF_CHIP);
                              if (padcview_mask != NULL)
                              {
                                 decompress(title_image_plane_1_comp,
                                             (UBYTE *)title_image_plane_1);
                                 decompress(title_image_plane_2_comp,
                                             (UBYTE *)title_image_plane_2);
                                 decompress(farview_image_comp,
                                             (UBYTE *)farview_image);
                                 decompress(farview_mask_comp,
                                             (UBYTE *)farview_mask);
                                 decompress(padaview_image_comp,
                                             (UBYTE *)padaview_image);
                                 decompress(padaview_mask_comp,
                                             (UBYTE *)padaview_mask);
                                 decompress(padbview_image_comp,
                                             (UBYTE *)padbview_image);
                                 decompress(padbview_mask_comp,
                                             (UBYTE *)padbview_mask);
                                 decompress(padcview_image_comp,
                                             (UBYTE *)padcview_image);
                                 decompress(padcview_mask_comp,
                                             (UBYTE *)padcview_mask);
                                 return;
                              }
                              FreeMem(padcview_image, IMAGESIZE);
                           }
                           FreeMem(padbview_mask, MASKSIZE);
                        }
                        FreeMem(padbview_image, IMAGESIZE);
                     }
                     FreeMem(padaview_mask, MASKSIZE);
                  }
                  FreeMem(padaview_image, IMAGESIZE);
               }
               FreeMem(farview_mask, MASKSIZE);
            }
            FreeMem(farview_image, IMAGESIZE);
         }
         FreeMem(title_image_plane_2, TITLESIZE);
      }
      FreeMem(title_image_plane_1, TITLESIZE);
   }
   
   printf("Could not allocate enough CHIP ram for images!\n");
   
   exit(RETURN_WARN);
}


/*
** free_images()
**
** Free memory for images that had to be allocated for decompression.
*/
void free_images()
{
   FreeMem(title_image_plane_1, TITLESIZE);
   FreeMem(title_image_plane_2, TITLESIZE);
   FreeMem(farview_image, IMAGESIZE);
   FreeMem(farview_mask, MASKSIZE);
   FreeMem(padaview_image, IMAGESIZE);
   FreeMem(padaview_mask, MASKSIZE);
   FreeMem(padbview_image, IMAGESIZE);
   FreeMem(padbview_mask, MASKSIZE);
   FreeMem(padcview_image, IMAGESIZE);
   FreeMem(padcview_mask, MASKSIZE);
}


/*
** decompress()
**
** Decompresses code at source to destination.
*/
void decompress(UBYTE *source, UBYTE *dest)
{
   UBYTE code, x, a;
   
   FOREVER
   {
      code = *source;
      source++;
      
      if (code == NULL)
      {
         break;
      }
      
      if (code & 0x80)
      {
         code &= 0x7f;
         x = *source;
         source++;
         
         for (a=0; a<code; a++)
         {
            *dest = x;
            dest++;
         }
      }
      else
      {
         for (a=0; a<code; a++)
         {
            *dest = *source;
            source++;
            dest++;
         }
      }
   }
}

