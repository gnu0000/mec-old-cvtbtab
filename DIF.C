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
#include "dif.h"


void WriteDIFHeader (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes)
   {
   UINT i;

   fputs ("TABLE\n0,1\n\"\"\nVECTORS\n0,", fp);
   fprintf (fp, "%d\n\"\"\nTUPLES\n0,%d\n\"\"\n", uColCount, uRowCount);
   fputs ("DATA\n0,0\n\"\"\n-1,0\n", fp);
   }

void WriteDIFTail (FILE *fp)
   {
   fputs ("EOD\n", fp);
   }

void WriteDIFLabels (FILE *fp, UINT uRow, PPSZ ppszFields)
   {
   // nothing
   }

void WriteDIFString (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uFormat)
   {
   fputs ("1,0\n\"", fp);
   for (; *pszString; pszString++)
      {
      if (*pszString == '"')
         fputs ("''", fp);
      else
         fputc (*pszString, fp);
      }
   fputs ("\"\n", fp);
   }

void WriteDIFNumber (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uDecimals)
   {
   CHAR szDest [128];

   fprintf (fp, "0,%s\nV\n", MthFmat (szDest, bgNum, 16, uDecimals, FALSE, FALSE));
   }


void WriteDIFStartOfLine (FILE *fp)
   {
   fputs ("BOT\n", fp); // SOL token
   }


void WriteDIFEndOfLine (FILE *fp)
   {
   fputs ("-1,0\n", fp); // EOL token
   }

