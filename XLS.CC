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
#include "xls.h"


/*
 * These are the default format strings Excel uses
 * These cannot be changed.
 *
 */
static CHAR *XLSStaticFormats[] = 

           //------------------------------------------------
           // Fixed formats
           //------------------------------------------------
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

           //------------------------------------------------
           // User formats
           //------------------------------------------------
            "#,##0.000",                               // 21 0
            "#,##0.00000",                             // 22 1
            "\"$\"#,##0.00",                           // 23 2

            NULL};

UINT uFIRSTUSERFORMAT = 21;

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

UINT XLSAddFormat (PSZ pszFormat)
   {
   }















/**************************************************************************/


/*
 * writes the initial records for an excel file
 *
 */
void WriteXLSHeader (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes)
   {
   UINT  uLen, i;
   PPSZ  ppsz;

   /*--- BOF Record ---*/
   fwrite ("\x09\x00\x04\x00\x00\x00\x10\x00", 1, 8, fp);

   /*--- Write General Format Records ---*/
   for (ppsz = XLSStaticFormats; *ppsz; ppsz++)
      {
      uLen = strlen (*ppsz);
      FilWriteUShort (fp, 0x1e);        // Format Tag
      FilWriteUShort (fp, uLen+1);      // Record len
      FilWriteByte   (fp, (BYTE)uLen);  // Format len
      fwrite (*ppsz, 1, uLen, fp);      // format string
      }

   /*--- Write Quantity format record ---*/
   fwrite ("\x1e\x00\x0a\x00\x09#,##0.000", 1, 14, fp); // label entries

   /*--- Write UnitPrice Format Record ---*/
   fwrite ("\x1e\x00\x0f\x00\x0e\"$\"#,##0.00000", 1, 19, fp); // label entries

   /*--- Write XF Records (3) ---*/
   fwrite ("\x43\x00\x04\x00\x00\x00\x00\x42", 1, 8, fp); // label entries
   fwrite ("\x43\x00\x04\x00\x00\x00\x40\x00", 1, 8, fp); // normal entries

   fwrite ("\x43\x00\x04\x00\x00\x00", 1, 6, fp);  // quan format
   FilWriteUShort (fp, uFIRSTUSERFORMAT);          // Format Tag

   fwrite ("\x43\x00\x04\x00\x00\x00", 1, 6, fp);  // unitpric format
   FilWriteUShort (fp, uFIRSTUSERFORMAT+1);        // Format Tag

   fwrite ("\x43\x00\x04\x00\x00\x00", 1, 6, fp);  // ext format
   FilWriteUShort (fp, uFIRSTUSERFORMAT+2);        // Format Tag

   /*--- Dimension Record ---*/
   FilWriteUShort (fp, 0);          // BOF Tag
   FilWriteUShort (fp, 8);          // Length
   FilWriteUShort (fp, 0);          // Starting Col offset
   FilWriteUShort (fp, uColCount);  // Max Cols
   FilWriteUShort (fp, 0);          // Starting Row offset
   FilWriteUShort (fp, uRowCount);  // Max Rows

   /*--- Write Column size records ---*/
   for (i=0; i < uColCount; i++)
      {
      fwrite ("\x24\x00\x04\x00", 1, 4, fp);
      FilWriteByte (fp, (BYTE)i);                    // uCol
      FilWriteByte (fp, (BYTE)i);                    // uCol
      FilWriteByte (fp, 0);                          // 1/256's of CHAR

      FilWriteByte (fp, (BYTE)(puColSizes ? (puColSizes[i]) : 8)); // chars
      }
   }


/*
 * finishes up
 */
void WriteXLSTail (FILE *fp)
   {
   fwrite ("\x0a\x00\x00\x00", 1, 4, fp); // EOF record
   }


/*
 * writes the labels line
 *
 */
void WriteXLSLabels (FILE *fp, UINT uRow, PPSZ ppszFields)
   {
   UINT i, uSize;

   for (i=0; ppszFields[i]; i++)
      {
      uSize = strlen (ppszFields[i]);
      fwrite ("\x04\x00", 1, 2, fp);
      FilWriteUShort (fp, uSize+8);
      FilWriteUShort (fp, uRow);
      FilWriteUShort (fp, i);
      fwrite ("\x00\x00\x42", 1, 3, fp);
      FilWriteByte (fp, (BYTE)uSize);
      fwrite (ppszFields[i], 1, uSize, fp);
      }
   }


void WriteXLSString (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uFormat)
   {
   UINT uLen;

   if (!pszString || !*pszString)
      return;

   uLen = strlen (pszString);
   FilWriteUShort (fp, 0x0004);       // LABEL tag
   FilWriteUShort (fp, uLen+8);       // Record Len
   FilWriteUShort (fp, uRow);         // cell row
   FilWriteUShort (fp, uCol);         // cell col
   fwrite ("\x04\x00\x00", 1, 3, fp); // cell attributes
   FilWriteByte (fp, (BYTE)uLen);     // string len
   fwrite (pszString, 1, uLen, fp);   // string
   }


void WriteXLSNumber (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uFormat)
   {
   double d;

   if (!MthValid (bgNum))
      return;

   fwrite ("\x03\x00\x0F\x00", 1, 4, fp); // NUMBER tag and rec len
   FilWriteUShort (fp, uRow);             // cell row
   FilWriteUShort (fp, uCol);             // cell col

   /*--- decide which XF record to use ---*/
   if (uFormat >= uFIRSTUSERFORMAT)
      FilWriteByte (fp, (BYTE)(uFormat - uFIRSTUSERFORMAT + 2));
   else
      FilWriteByte (fp, 1);

   FilWriteUShort (fp, uFormat);

   d = (double) bgNum;
   fwrite (&d, 8, 1, fp);                 // the number
   }


/**************************************************************************/
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/


