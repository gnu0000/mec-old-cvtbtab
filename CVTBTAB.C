/*
 *
 * cvt.c
 * Wednesday, 5/29/1996.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuFile.h>
#include <GnuStr.h>
#include <GnuMath.h>
#include <GnuArg.h>
#include <GnuMem.h>
#include <GnuMisc.h>
#include "xls.h"
#include "wk1.h"
#include "dif.h"


/*
 *
 *
 */
typedef struct
   {
   BIG bgUP;
   BIG bgExt;
   } TAB;
typedef TAB *PTAB;


/*
 * linked list of items
 *
 */
typedef struct _itm
   {
   PSZ  pszItemNum; //
   PSZ  pszDesc;    //
   PSZ  pszUnit;    //
   BIG  bgQuan;     //

   PTAB ptab;       // array of up/ext

   struct _itm *next;
   } ITM;
typedef ITM *PITM;


/*
 * bidder struct
 *
 */
typedef struct
   {
   PSZ  pszID;   //
   PSZ  pszName; //
   } BDR;
typedef BDR *PBDR;


/*
 * top level struct
 *
 */
typedef struct
   {
   PBDR pbdr;  // array of bidders 
   PITM pitm;  // linked list of items

   UINT uBidders;
   UINT uItems;
   } BT;
typedef BT *PBT;




PSZ ppszLABELS[] = 
     {"Bidder ID",
      "Bidder Name",
      "Item Number",
      "Item Description",
      "Item Quantity",
      "Item Unit",
      "Item Unit Price",
      "Item Extension",
      NULL};

UINT puCOLSIZES[] =
      {8, 16, 16, 60, 16, 6, 16, 16};

BOOL bSTDOUT;

/************************************************************************/
/*                                                                      */
/* Read File Stuff                                                      */
/*                                                                      */
/************************************************************************/


/*
 *
 *
 */
PBT ReadBidders (FILE *fp, PBT pbt)
   {
   PPSZ ppsz;
   CHAR szLine[512];
   UINT uCols, i, j;

   /*--- Read 1st line which contains bidder info ---*/
   if (FilReadLine (fp, szLine, ";", sizeof szLine) == (UINT)-1)
      Error ("Unable to read input file");

   ppsz = StrMakePPSZ (szLine, ",", TRUE, TRUE, &uCols);

   pbt->uBidders = uCols / 2;

   if (uCols % 2 || !pbt->uBidders)
      Error ("Input file Error: no bidders present or bad # of columns");

   pbt->pbdr = calloc (sizeof (BDR), pbt->uBidders);

   for (i=j=0; i < pbt->uBidders; i++)
      {
      pbt->pbdr[i].pszID   = strdup (ppsz[j++]);
      pbt->pbdr[i].pszName = strdup (ppsz[j++]);
      }

   MemFreePPSZ (ppsz, uCols);
   return pbt;
   } 



/*
 *
 *
 */
PBT ReadCSVFile (PSZ pszFile)
   {
   PPSZ ppsz;
   FILE *fp;
   CHAR szLine[512];
   UINT uCols, i;
   PITM pitm, pitmPrev = NULL;
   PBT  pbt;

   if (!(fp = fopen (pszFile, "rt")))
      Error ("Unable to open input file %s", pszFile);

   pbt = calloc (sizeof (BT), 1);
   pbt->uItems = 0;
   ReadBidders (fp, pbt);

   /*--- Read item linse which contains item and bid tab info ---*/
   while (FilReadLine (fp, szLine, ";", sizeof szLine) != (UINT)-1)
      {
      ppsz = StrMakePPSZ (szLine, ",", TRUE, TRUE, &uCols);

      if (uCols < (UINT)(4 + pbt->uBidders * 2))
         continue; // bad line

      pitm = calloc (sizeof (ITM), 1);

      if (pitmPrev)
         pitmPrev->next = pitm; // add to chain
      else
         pbt->pitm = pitm;      // start of chain

      pitmPrev = pitm;

      pitm->pszItemNum = strdup (ppsz[0]) ;
      pitm->pszDesc    = strdup (ppsz[1]) ;
      pitm->pszUnit    = strdup (ppsz[3]) ;
      pitm->bgQuan     = AToBIG (ppsz[2]) ;
      pitm->ptab       = malloc (sizeof (TAB) * pbt->uBidders);
                                                
      for (i=0; i<pbt->uBidders; i++)
         {
         pitm->ptab[i].bgUP   = AToBIG (ppsz[4 + i*2]);
         pitm->ptab[i].bgExt  = AToBIG (ppsz[5 + i*2]);
         }

      MemFreePPSZ (ppsz, uCols);

      pbt->uItems++;
      }
   fclose (fp);
   return pbt;
   } 



/************************************************************************/
/*                                                                      */
/*  Write File Stuff                                                    */
/*                                                                      */
/************************************************************************/


/*
 *
 *
 */
FILE *OpenFile (PSZ pszFileBase, PSZ pszExt, BOOL bBinary)
   {
   FILE *fp;
   CHAR szFile [256];
   PSZ  psz;

   strcpy (szFile, pszFileBase);
   if (psz = strrchr (szFile, '.'))
      *psz = '\0';
   strcat (szFile, ".");
   strcat (szFile, pszExt);

   if (bSTDOUT)
      strcpy (szFile, "CON");

   if (!(fp = fopen (szFile, (bBinary ? "wb" : "wt"))))
      Error ("Unable to open output file %s", szFile);
      
   return fp;
   }


PSZ MakeCSV (PSZ pszOut, PSZ pszIn, BOOL bTrueCsv)
   {
   PSZ psz;

   if (bTrueCsv)
      return StrMakeCSVField (pszOut, pszIn);

   for (psz = pszOut; *pszIn; pszIn++)
      if (*pszIn != ',' && *pszIn != '"')
         *psz++ = *pszIn;
   *psz = '\0';

   return pszOut;
   }



PSZ MakeColStr (PSZ pszDest, PSZ pszSrc, UINT uLen)
   {
   UINT i;

   strcpy (pszDest, pszSrc);
   for (i=strlen(pszDest); i<uLen; i++)
      pszDest[i] = ' ';
   pszDest[uLen] = '\0';

   return pszDest;
   }

PSZ MakeColNum (PSZ pszDest, BIG bgNum, UINT uLen, UINT uDec)
   {
   UINT i;

   MthFmat (pszDest, bgNum, uLen, uDec, FALSE, FALSE);

   for (i=strlen(pszDest); i<uLen; i++)
      pszDest[i] = ' ';
   pszDest[uLen] = '\0';

   return pszDest;
   }

/************************************************************************/


/*
 * normalized format 
 *
 */
void WriteCSVFile (PBT pbt, PSZ pszFile, BOOL bTrueCSV)
   {
   FILE *fp;
   CHAR szBuff1 [256], szBuff2 [256];
   CHAR szBuff3 [256], szBuff4 [256];
   UINT i;
   PITM pitm;

   fp = OpenFile (pszFile, (bTrueCSV ? "CSV" : "CS2"), FALSE);

   /*--- bidder lines ---*/
   for (i=0; i<pbt->uBidders; i++)
      fprintf (fp, "BDR,%s,%s\n",  MakeCSV (szBuff1, pbt->pbdr[i].pszID, bTrueCSV),
         MakeCSV (szBuff2, pbt->pbdr[i].pszName, bTrueCSV));

   /*--- item lines ---*/
   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      fprintf (fp, "ITM,%s,%s,%s,%s\n", 
         MakeCSV (szBuff1, pitm->pszItemNum, bTrueCSV),
         MakeCSV (szBuff2, pitm->pszDesc   , bTrueCSV),
         MakeCSV (szBuff3, pitm->pszUnit   , bTrueCSV),
         MthFmat (szBuff4, pitm->bgQuan, 10, 3, FALSE, FALSE));

   /*--- now for the bid tabs ---*/
   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         fprintf (fp, "TAB,%s,%s,%s,%s\n", 
            MakeCSV (szBuff1, pbt->pbdr[i].pszID, bTrueCSV),
            MakeCSV (szBuff2, pitm->pszItemNum, bTrueCSV),
            MthFmat (szBuff3, pitm->ptab[i].bgUP,  10, 5, FALSE, FALSE),
            MthFmat (szBuff4, pitm->ptab[i].bgExt, 12, 2, FALSE, FALSE));
      }
   fclose (fp);
   }



/*
 *
 *
 */
void WriteXLSFile (PBT pbt, PSZ pszFile)
   {
   FILE *fp;
   UINT uX, uY, i;
   PITM pitm;

   fp = OpenFile (pszFile, "XLS", TRUE);

   WriteXLSHeader (fp, pbt->uItems, 8, puCOLSIZES);
   WriteXLSLabels (fp, 0, ppszLABELS);

   uY = 0;
   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         {
         uX = 0;
         WriteXLSString (fp, pbt->pbdr[i].pszID,   uY+1, uX++, 0);
         WriteXLSString (fp, pbt->pbdr[i].pszName, uY+1, uX++, 0);

         WriteXLSString (fp, pitm->pszItemNum, uY+1, uX++, 0);
         WriteXLSString (fp, pitm->pszDesc   , uY+1, uX++, 0);
         WriteXLSNumber (fp, pitm->bgQuan    , uY+1, uX++, uFIRSTUSERFORMAT+2);
         WriteXLSString (fp, pitm->pszUnit   , uY+1, uX++, 0);

         WriteXLSNumber (fp, pitm->ptab[i].bgUP,  uY+1, uX++, uFIRSTUSERFORMAT);
         WriteXLSNumber (fp, pitm->ptab[i].bgExt, uY+1, uX++, uFIRSTUSERFORMAT+1);
         uY++;
         }
      }
   WriteXLSTail   (fp);
   }



/*
 *
 *
 */
void WriteWK1File (PBT pbt, PSZ pszFile)
   {
   FILE *fp;
   UINT uX, uY, i;
   PITM pitm;

   fp = OpenFile (pszFile, "WK1", TRUE);

   WriteWK1Header (fp, pbt->uItems, 8, puCOLSIZES);
   WriteWK1Labels (fp, 0, ppszLABELS);

   uY = 0;
   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         {
         uX = 0;
         WriteWK1String (fp, pbt->pbdr[i].pszID,   uY+1, uX++, 0);
         WriteWK1String (fp, pbt->pbdr[i].pszName, uY+1, uX++, 0);

         WriteWK1String (fp, pitm->pszItemNum, uY+1, uX++, 0);
         WriteWK1String (fp, pitm->pszDesc   , uY+1, uX++, 0);
         WriteWK1Number (fp, pitm->bgQuan    , uY+1, uX++, 3);
         WriteWK1String (fp, pitm->pszUnit   , uY+1, uX++, 0);

         WriteWK1Number (fp, pitm->ptab[i].bgUP,  uY+1, uX++, 5);
         WriteWK1Number (fp, pitm->ptab[i].bgExt, uY+1, uX++, 2);
         uY++;
         }
      }
   WriteWK1Tail   (fp);
   }


/*
 *
 *
 */
void WriteDBFFile (PBT pbt, PSZ pszFile)
   {
   FILE *fp;

   fp = OpenFile (pszFile, "DBF", TRUE);
   fclose (fp);
   }


/*
 *
 *
 */
void WriteCOLFile (PBT pbt, PSZ pszFile)
   {
   FILE *fp;
   CHAR szBuff1 [256], szBuff3 [256];
   CHAR szBuff2 [256], szBuff4 [256];
   UINT i;
   PITM pitm;

   fp = OpenFile (pszFile, "COL", FALSE);

   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         {
         fprintf (fp, "%s,%s,", 
            MakeColStr (szBuff1, pbt->pbdr[i].pszID, 8), 
            MakeColStr (szBuff2, pbt->pbdr[i].pszName, 60));
         fprintf (fp, "%s,%s,%s,%s\n", 
            MakeColStr (szBuff1, pitm->pszItemNum, 16),
            MakeColStr (szBuff2, pitm->pszDesc, 60   ),
            MakeColNum (szBuff3, pitm->bgQuan, 16, 3));
            MakeColStr (szBuff4, pitm->pszUnit, 5),
         fprintf (fp, "%s,%s\n", 
            MakeColNum (szBuff1, pitm->ptab[i].bgUP,  16, 5),
            MakeColNum (szBuff2, pitm->ptab[i].bgExt, 16, 2));
         }
      }
   fclose (fp);
   }


/*
 *
 *
 */
void WriteTABFile (PBT pbt, PSZ pszFile)
   {
   FILE *fp;
   CHAR szBuff1 [256], szBuff3 [256];
   CHAR szBuff2 [256], szBuff4 [256];
   UINT i;
   PITM pitm;

   fp = OpenFile (pszFile, "TAB", FALSE);

   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         {
         fprintf (fp, "%s,%s,", 
            StrMakeCSVField (szBuff1, pbt->pbdr[i].pszID), 
            StrMakeCSVField (szBuff2, pbt->pbdr[i].pszName));
         fprintf (fp, "%s,%s,%s,%s\n", 
            StrMakeCSVField (szBuff1, pitm->pszItemNum),
            StrMakeCSVField (szBuff2, pitm->pszDesc   ),
            MthFmat (szBuff3, pitm->bgQuan, 10, 3, FALSE, FALSE));
            StrMakeCSVField (szBuff4, pitm->pszUnit   ),
         fprintf (fp, "%s,%s\n", 
            MthFmat (szBuff1, pitm->ptab[i].bgUP,  10, 5, FALSE, FALSE),
            MthFmat (szBuff2, pitm->ptab[i].bgExt, 12, 2, FALSE, FALSE));
         }
      }
   fclose (fp);
   }


/*
 *
 *
 */
void WriteDIFFile (PBT pbt, PSZ pszFile)
   {
   FILE *fp;
   UINT uX, uY, i;
   PITM pitm;

   fp = OpenFile (pszFile, "DIF", FALSE);

   WriteDIFHeader (fp, pbt->uItems, 8, puCOLSIZES);
   WriteDIFLabels (fp, 0, ppszLABELS);

   uY = 0;
   for (pitm = pbt->pitm; pitm; pitm = pitm->next)
      {
      for (i=0; i<pbt->uBidders; i++)
         {
         uX = 0;
         WriteDIFStartOfLine (fp);

         WriteDIFString (fp, pbt->pbdr[i].pszID,   uY+1, uX++, 0);
         WriteDIFString (fp, pbt->pbdr[i].pszName, uY+1, uX++, 0);

         WriteDIFString (fp, pitm->pszItemNum, uY+1, uX++, 0);
         WriteDIFString (fp, pitm->pszDesc   , uY+1, uX++, 0);
         WriteDIFNumber (fp, pitm->bgQuan    , uY+1, uX++, 3);
         WriteDIFString (fp, pitm->pszUnit   , uY+1, uX++, 0);

         WriteDIFNumber (fp, pitm->ptab[i].bgUP,  uY+1, uX++, 5);
         WriteDIFNumber (fp, pitm->ptab[i].bgExt, uY+1, uX++, 2);

         WriteDIFEndOfLine (fp);

         uY++;
         }
      }
   WriteDIFTail   (fp);
   }


/************************************************************************/
/*                                                                      */
/* Framework stuff                                                      */
/*                                                                      */
/************************************************************************/


/*
 *
 *
 */
void Usage ()
   {
   printf ("CVTBTAB    Bid Tab File conversion utility v0.01          %s\n\n", __DATE__);
   printf ("\n");
   printf ("USAGE: CVTBTAB [option] InFile [OutFile]\n");
   printf ("\n");
   printf ("WHERE: InFile .... The file to convert.  If No file extension is given,\n");
   printf ("                   .CSV is assumed.\n");
   printf ("       OutFile ... The Name of the output file.  If not specified, the \n");
   printf ("                   name of the input file is used.  Extension based on opt\n");
   printf ("                   alternately tou can use /STDOUT to send output to console\n");
   printf ("\n");
   printf ("       [option]  is one or more of:\n");
   printf ("          /CSV ... Convert to a normalized CSV file\n");
   printf ("          /CS2 ... Convert to a normalized CSV like file\n");
   printf ("          /XLS ... Convert to a Excel file\n");
   printf ("          /WK1 ... Convert to a Lotos file\n");
//   printf ("          /DBF ... Convert to a DBASE file\n");
   printf ("          /COL ... Convert to a Fixed Column file\n");
   printf ("          /TAB ... Convert to a Tab Delimeted file\n");
   printf ("          /DIF ... Convert to a DIF file\n");
   exit (0);         
   }



/*
 *
 *
 */
int main (int argc, char *argv[])
   {
   PBT  pbt;
   PSZ  psz;
   CHAR szInFile [256];
   CHAR szOutFile[256];

   if (ArgBuildBlk ("? *^Debug *^CSV *^CS2 *^XLS *^WK1 *^DBF"
                    " *^COL *^TAB *^DIF *^STDOUT *^CON"))
      Error ("%s", ArgGetErr ());

   if (ArgFillBlk (argv))
      Error ("%s", ArgGetErr ());

   if (ArgIs ("?") || (ArgIs (NULL)  < 1))
      Usage ();

   strcpy (szInFile, ArgGet (NULL, 0));
   if (!(psz = strchr (szInFile, '.')))
      strcat (szInFile, ".CSV");

   strcpy (szOutFile, ArgGet(NULL, (ArgIs (NULL)>1 ? 1:0)));
   if (psz = strchr (szOutFile, '.'))
      *psz = '\0';

   bSTDOUT = ArgIs ("STDOUT") || ArgIs ("CON");

   pbt = ReadCSVFile (szInFile);

   if (ArgIs ("CSV")) WriteCSVFile (pbt, szOutFile, TRUE);
   if (ArgIs ("CS2")) WriteCSVFile (pbt, szOutFile, FALSE);
   if (ArgIs ("XLS")) WriteXLSFile (pbt, szOutFile);
   if (ArgIs ("WK1")) WriteWK1File (pbt, szOutFile);
   if (ArgIs ("DBF")) WriteDBFFile (pbt, szOutFile);
   if (ArgIs ("COL")) WriteCOLFile (pbt, szOutFile);
   if (ArgIs ("TAB")) WriteTABFile (pbt, szOutFile);
   if (ArgIs ("DIF")) WriteDIFFile (pbt, szOutFile);

   if (!(ArgIs ("CSV") + ArgIs ("CS2") + ArgIs ("XLS") +
         ArgIs ("WK1") + ArgIs ("DBF") + ArgIs ("COL") +
         ArgIs ("TAB") + ArgIs ("DIF")))
      Error ("Nothing to do!");

   return 0;
   }

