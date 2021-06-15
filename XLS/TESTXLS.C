/*
 *
 * testxls.c
 * Friday, 5/31/1996.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include "xls.h"



/*
 * XLSAddFormat (pszFormat)
 * XLSAddXF (uFormat, uFlags)
 * XLSWriteHeader (*fp, uRowCount, uColCount, PpuColSizes)
 * XLSWriteTail (*fp)
 * XLSWriteLabels (*fp, uRow, ppszFields)
 * XLSWriteString (*fp, pszString, uRow, uCol, uXF, uFlags)
 * XLSWriteNumber (*fp, bgNum, uRow, uCol, uXF, uFlags)
 * 
 * XLS_SHADED      
 * XLS_BORDERBOTTOM
 * XLS_BORDERTOP   
 * XLS_BORDERRIGHT 
 * XLS_BORDERLEFT   
 * XLS_ALIGNGENERAL
 * XLS_ALIGNLEFT   
 * XLS_ALIGNCENTER 
 * XLS_ALIGNRIGHT  
 * XLS_ALIGNFILL   
 */

int main (int argc, char *argv[])
   {
   UINT uFormat1, uFormat2, uFormat3;
   UINT uXF1, uXF2, uXF3;
   FILE *fp;

   fp = fopen ("A.xls", "wb");

   uFormat1 = XLSAddFormat ("#,##0.000"    );
   uFormat2 = XLSAddFormat ("#,##0.00000"  );
   uFormat3 = XLSAddFormat ("\"$\"#,##0.00");

   uXF1 = XLSAddXF (uFormat1, 0);
   uXF2 = XLSAddXF (uFormat2, XLS_BORDERBOTTOM);
   uXF3 = XLSAddXF (uFormat3, XLS_SHADED);

   XLSWriteHeader (fp, 5, 5, NULL);

// XLSWriteLabels (fp, uRow, ppszFields);

// XLSWriteString (fp, pszString, uRow, uCol, uXF, uFlags);

   XLSWriteString (fp, "Test String", 1, 0, 0);

// XLSWriteNumber (fp, bgNum, uRow, uCol, uXF);

   XLSWriteNumber (fp, 1.2345, 1, 1, uXF1);
   XLSWriteNumber (fp, 1.2345, 1, 2, uXF2);
   XLSWriteNumber (fp, 1.2345, 1, 3, uXF3);

   XLSWriteTail (fp);

   fclose (fp);

   return 0;
   }

