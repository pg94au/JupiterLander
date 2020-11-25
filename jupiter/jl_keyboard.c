/*
** jl_keyboard.c - contains functions used with jupitergame.c to read
**                 keyboard matrix
**
** (c)1993 Paul Grebenc
*/


/** INCLUDES & DEFINES **/

#include "jupiterdefs.h"


/** DEFINES **/

#define MATRIX_SIZE 16L


/** PROTOTYPES **/

int setup_keyboard(void);
void cleanup_keyboard(void);
int read_keyboard(void);


/** GLOBAL VARIABLES **/

struct IOStdReq *KeyIO;
struct MsgPort *KeyMP;
UBYTE *keyMatrix;


/*-----------------------------------------------------------------------*/


/*
** setup_keyboard()
**
** Setup and open keyboard.device for reading of keyboard matrix.
**
** RETURNS:  TRUE if successful, FALSE if unsuccessful.
*/
int setup_keyboard()
{
   if (KeyMP = CreatePort(NULL, NULL))
   {
      if (KeyIO = (struct IOStdReq *)CreateExtIO(KeyMP, sizeof(struct IOStdReq)))
      {
         if (!OpenDevice("keyboard.device", NULL, (struct IORequest *)KeyIO, NULL))
         {
            if (keyMatrix = AllocMem(MATRIX_SIZE, MEMF_PUBLIC | MEMF_CLEAR))
            {
               return TRUE;
            }
            else
            {
               printf("Could not allocate keyMatrix\n");
            }
            CloseDevice((struct IORequest *)KeyIO);
         }
         else
         {
            printf("Could not open keyboard.device\n");
         }
         DeleteExtIO((struct IORequest *)KeyIO);
      }
      else
      {
         printf("Could not create I/O request for keyboard.device\n");
      }
      DeletePort(KeyMP);
   }
   else
   {
      printf("Could not create message port for keyboard.device\n");
   }
   return FALSE;
}


/*
** cleanup_keyboard()
**
** Free everything allocated by setup_keyboard, close keyboard.device.
*/
void cleanup_keyboard()
{
   FreeMem(keyMatrix, MATRIX_SIZE);
   CloseDevice((struct IORequest *)KeyIO);
   DeleteExtIO((struct IORequest *)KeyIO);
   DeletePort(KeyMP);
}


/*
** read_keyboard()
**
** Read keyboard matrix.
**
** RETURNS:  Returns a code containing a bit set for each key pressed.
*/
int read_keyboard()
{
   int result=0;
   
   /* read keyboard matrix */
   KeyIO->io_Command = KBD_READMATRIX;
   KeyIO->io_Data = (APTR)keyMatrix;
   KeyIO->io_Length = MATRIX_SIZE;
   
   DoIO((struct IORequest *)KeyIO);
   /* */
   
   /* check keyboard matrix and set corresponding bits in result */
   if (keyMatrix[4] & 0x01)
   {
      result |= CURSOR_LEFT;
   }
   if (keyMatrix[4] & 0x04)
   {
      result |= CURSOR_RIGHT;
   }
   if (keyMatrix[12] & 0x02)
   {
      result |= CURSOR_UP;
   }
   /* */
   
   return result;
}
