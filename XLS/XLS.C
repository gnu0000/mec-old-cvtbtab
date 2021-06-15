/*
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
#include "xls.h"


typedef struct
   {
   UINT uFormatIdx;
   UINT uFlags;
   } XF;
typedef XF *PXF;


/*
 * These are the default format strings Excel uses
 * These cannot be changed.
 */
static PSZ ppszSTATICFORMATS[] = 
           {"General",                                 // 0
            "0",                                       // 1
            "0.00",                                    // 2
            "#,##0",                                   // 3
            "#,##0.00",                                // 4
            "\"$\"#,##0_);\\(\"$\"#,##0\\)",           // 5
            "\"$\"#,##0_);[Red]\\(\"$\"#,##0\\)",      // 6
            "\"$\"#,##0.00_);\\(\"$\"#,##0.00\\)",     // 7
            "\"$\"#,##0.00_);[Red]\\(\"$\"#,##0.00\\)",// 8
            "0%",                                      // 9
            "0.00%",                                   // 10
            "0.00E+00",                                // 11
            "m/d/yy",                                  // 12
            "d\\-mmm\\-yy",                            // 13
            "d\\-mmm",                                 // 14
            "mmm\\-yy",                                // 15
            "h:mm\\ AM/PM",                            // 16
            "h:mm:ss\\ AM/PM",                         // 17
            "h:mm",                                    // 18
            "h:mm:ss",                                 // 19
            "m/d/yy\\ h:mm",                           // 20
            NULL};


static XF xfSTATICXFRECS[] =
           {{0, 0},
            {(UINT)-1, (UINT)-1}};
            

PSZ *ppszFORMATS = ppszSTATICFORMATS;
PXF pxfXFRECS    = xfSTATICXFRECS;

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

UINT XLSAddFormat (PSZ pszFormat)
   {
   UINT uIdx;

   for (uIdx=0; ppszFORMATS[uIdx]; uIdx++)
      if (!strcmp (pszFormat, ppszFORMATS[uIdx]))
         return uIdx; // string already in list

   /*--- we need to copy static data before we can modify it ---*/
   if (ppszFORMATS == ppszSTATICFORMATS) 
      {
      ppszFORMATS = malloc (sizeof (PSZ) * 64);
      for (uIdx=0; ppszSTATICFORMATS[uIdx]; uIdx++)
         ppszFORMATS[uIdx] = ppszSTATICFORMATS[uIdx];
      }
   else
      {
      for (uIdx=0; ppszFORMATS[uIdx]; uIdx++)
         ;
      }
   if (uIdx >= 62) // a couple are reserved
      return 0;

   ppszFORMATS[uIdx]   = strdup (pszFormat);
   ppszFORMATS[uIdx+1] = NULL;
   return uIdx;
   }


UINT XLSAddXF (UINT uFormat, UINT uFlags)
   {
   UINT uIdx;

   for (uIdx=0; pxfXFRECS[uIdx].uFormatIdx != (UINT)-1; uIdx++)
      if ((pxfXFRECS[uIdx].uFormatIdx == uFormat) &&
          (pxfXFRECS[uIdx].uFlags     == uFlags))
         return uIdx; // xf already in list

   /*--- we need to copy static data before we can modify it ---*/
   if (pxfXFRECS == xfSTATICXFRECS) 
      {
      pxfXFRECS = malloc (sizeof (XF) * 64);
      for (uIdx=0; xfSTATICXFRECS[uIdx].uFormatIdx != (UINT)-1; uIdx++)
         pxfXFRECS[uIdx] = xfSTATICXFRECS[uIdx];
      }
   else
      {
      for (uIdx=0; pxfXFRECS[uIdx].uFormatIdx != (UINT)-1; uIdx++)
         ;
      }
   if (uIdx >= 62) // a couple are reserved
      return 0;

   pxfXFRECS[uIdx].uFormatIdx = uFormat;
   pxfXFRECS[uIdx].uFlags     = uFlags;
   pxfXFRECS[uIdx+1].uFormatIdx = (UINT)-1;
   return uIdx;
   }


UINT XLSWriteHeader (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes)
   {
   UINT  uLen, i;

   /*--- BOF Record ---*/
   fwrite ("\x09\x00\x04\x00\x00\x00\x10\x00", 1, 8, fp);

   /*--- Write General Format Records ---*/
   for (i=0; ppszFORMATS[i]; i++)
      {
      uLen = strlen (ppszFORMATS[i]);
      FilWriteUShort (fp, 0x1e);        // Format Tag
      FilWriteUShort (fp, uLen+1);      // Record len
      FilWriteByte   (fp, (BYTE)uLen);  // Format len
      fwrite (ppszFORMATS[i], 1, uLen, fp);      // format string
      }

   /*--- Write XF Records ---*/
   for (i=0; pxfXFRECS[i].uFormatIdx != (UINT)-1; i++)
      {
      FilWriteUShort (fp, 0x43);        // XF Tag
      FilWriteUShort (fp, 0x04);        // Length
      FilWriteUShort (fp, 0x00);        // Font/Gridline indicies
      FilWriteByte   (fp, (BYTE)pxfXFRECS[i].uFormatIdx ); // Format record index
      FilWriteByte   (fp, (BYTE)pxfXFRECS[i].uFlags     );
      }

   /*--- Dimension Record ---*/
   FilWriteUShort (fp, 0);          // DIM Tag
   FilWriteUShort (fp, 8);          // Length
   FilWriteUShort (fp, 0);          // Starting Col offset
   FilWriteUShort (fp, uColCount);  // Max Cols
   FilWriteUShort (fp, 0);          // Starting Row offset
   FilWriteUShort (fp, uRowCount);  // Max Rows

   /*--- Write Column size records ---*/
   for (i=0; i < uColCount; i++)
      {
      FilWriteUShort (fp, 0x24);          // COLSIZ Tag
      FilWriteUShort (fp, 0x04);          // Length
      FilWriteByte (fp, (BYTE)i);         // uCol
      FilWriteByte (fp, (BYTE)i);         // uCol
      FilWriteByte (fp, 0);               // 1/256's of CHAR
      FilWriteByte (fp, (BYTE)(puColSizes ? (puColSizes[i]) : 8)); // chars
      }
   return 0;
   }


UINT XLSWriteTail (FILE *fp)
   {
   fwrite ("\x0a\x00\x00\x00", 1, 4, fp); // EOF record
   return 0;
   }


UINT XLSWriteLabels (FILE *fp, UINT uRow, PPSZ ppszFields)
   {
   UINT i, uXFIdx;

   uXFIdx = XLSAddXF (0, XLS_ALIGNCENTER | XLS_BORDERBOTTOM);

   for (i=0; ppszFields[i]; i++)
      XLSWriteString (fp, ppszFields[i], uRow, i, uXFIdx);
   return 0;
   }


UINT XLSWriteString (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uXF)
   {
   UINT uLen;

   if (!pszString || !*pszString)
      return 0;

   uLen = strlen (pszString);
   FilWriteUShort (fp, 0x0004);       // LABEL tag
   FilWriteUShort (fp, uLen+8);       // Record Len
   FilWriteUShort (fp, uRow);         // cell row
   FilWriteUShort (fp, uCol);         // cell col

   FilWriteByte (fp, (BYTE)uXF);      // string len
   FilWriteUShort (fp, 0);            // cell col

   FilWriteByte (fp, (BYTE)uLen);     // string len
   fwrite (pszString, 1, uLen, fp);   // string
   return 0;
   }


UINT XLSWriteNumber (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uXF)
   {
   double d;

   if (!MthValid (bgNum))
      return 0;

   FilWriteUShort (fp, 0x03);         // NUMBER tag
   FilWriteUShort (fp, 0x0F);         // record len
   FilWriteUShort (fp, uRow);         // cell row
   FilWriteUShort (fp, uCol);         // cell col

   FilWriteByte (fp, (BYTE)uXF);      // string len);
   FilWriteUShort (fp, 0);            // 

   d = (double) bgNum;
   fwrite (&d, 8, 1, fp);             // the number
   return 0;
   }



UINT XLSWriteInt (FILE *fp, USHORT u, UINT uRow, UINT uCol, UINT uXF)
   {
   FilWriteUShort (fp, 0x02);         // INTEGER tag
   FilWriteUShort (fp, 0x09);         // record len
   FilWriteUShort (fp, uRow);         // cell row
   FilWriteUShort (fp, uCol);         // cell col
   FilWriteByte   (fp, (BYTE)uXF);    // string len);
   FilWriteUShort (fp, 0);            // 
   FilWriteUShort (fp, u);            // cell col
   return 0;
   }


/**************************************************************************/
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/


