/*
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuFile.h>
#include <GnuWin.h>
#include <GnuMath.h>
#include <GnuMisc.h>
#include "wk1.h"


void WriteWK1Header (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes)
   {
   UINT i;

   FilWriteUShort (fp, 0);      // BOF marker
   FilWriteUShort (fp, 2);      // Length
   FilWriteUShort (fp, 1028);   // LOTUS 123 worksheet

   /*--- Write Column Widths ---*/
   for (i=0; i < uColCount; i++)
      {
      FilWriteUShort (fp, 8);                      // COLW1 code
      FilWriteUShort (fp, 3);                      // Length
      FilWriteUShort (fp, i);                      // uCol
      FilWriteByte (fp, (BYTE)(puColSizes ? puColSizes[i] : 8));
      }
   }


void WriteWK1Tail (FILE *fp)
   {
   FilWriteUShort (fp, 1);
   FilWriteUShort (fp, 0);
   }


void WriteWK1Labels (FILE *fp, UINT uRow, PPSZ ppszFields)
   {
   UINT i;

   for (i=0; ppszFields[i]; i++)
      {
      FilWriteUShort (fp, 15);
      FilWriteUShort (fp, 7 + strlen (ppszFields[i]));
      FilWriteByte   (fp, 0xFF);
      FilWriteUShort (fp, i);
      FilWriteUShort (fp, 0);
      FilWriteByte   (fp, '\'');
      fputs (ppszFields[i], fp);
      FilWriteByte   (fp, '\0');
      }
   }


void WriteWK1String (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uFormat)
   {
   FilWriteUShort (fp, 15);
   FilWriteUShort (fp, 7 + (pszString ? strlen (pszString) : 1));

   FilWriteByte (fp, 0xFF);
   FilWriteUShort (fp, uCol);
   FilWriteUShort (fp, uRow);
   FilWriteByte (fp, '\'');
   fputs ((pszString ? pszString : ""), fp);
   FilWriteByte (fp, '\0');
   }


void WriteWK1Number (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uDecimals)
   {
   double d;

   if (!MthValid (bgNum))
      return;

   FilWriteUShort (fp, 14);
   FilWriteUShort (fp, 13);

   FilWriteByte (fp, (BYTE)(0x80 | (uDecimals & 0x0F)));
   FilWriteUShort (fp, uCol);
   FilWriteUShort (fp, uRow);
   d = (double) bgNum;
   fwrite (&d, sizeof (double), 1, fp);
   }






