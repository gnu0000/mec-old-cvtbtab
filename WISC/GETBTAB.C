/*
 *
 * getbtab.c
 * Monday, 10/7/1996.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <GnuType.h>
#include <GnuFile.h>
#include <GnuStr.h>
#include <GnuMisc.h>
#include <GnuArg.h>

#define MAX_BIDDERS 10

CHAR sz    [2048];
CHAR szTmp [256];
UINT uBidders;


void Usage (void)
   {
   printf ("Wisconsin Bit tab report to CSV file converter    %s\n\n", __DATE__);
   printf ("USAGE:  GETBTAB xtabfile\n\n");
   printf ("WHERE:  xtabfile... is the name of the bid tab report file.\n");
   printf ("\n");
   printf ("A csv file is produced for each proposal.\n");
   printf ("\n");
   exit (0);
   }


PSZ GetStr (PSZ pszDest, PSZ pszSrc, UINT uStart, UINT uLen)
   {
   if (strlen (pszSrc) < uStart)
      Error ("GetStr Messup: [%lu](%d,%d) %s", FilGetLine (), uStart, uLen, pszSrc);

   strncpy (pszDest, pszSrc+uStart, uLen);
   pszDest[uLen] = '\0';
   StrStrip (StrClip (pszDest, " \t"), " \t");
   return pszDest;
   }


PSZ ToCSV(PSZ pszSrc)
   {
   return StrMakeCSVField (szTmp, pszSrc);
   }



void Extract (FILE *fpOut, FILE *fpIn)
   {
   UINT uBid;
   CHAR szUnitPrice[MAX_BIDDERS][64];
   CHAR szAmount   [MAX_BIDDERS][64];
   CHAR szUnit     [256];
   CHAR szDesc     [256];
   CHAR szItemNum  [256];
   CHAR szQuan     [256];

   while (FilReadLine (fpIn, sz, ";", sizeof (sz)) != (UINT)-1)
      {
      if (!strncmp (sz, "========", 8))
         break;
      if (!isdigit (*sz))
         continue;
      if (!isdigit (sz[2]))
         continue; // date

      /*--- read item line 1 ---*/
      GetStr (szItemNum, sz, 0, 5);
      GetStr (szQuan   , sz, 5, 11);
      for (uBid=0; uBid<uBidders; uBid++)
         {
         GetStr (szUnitPrice[uBid], sz, 16 + uBid * 21, 10);
         GetStr (szAmount   [uBid], sz, 26 + uBid * 21, 11);
         }

      /*--- read item line 2 ---*/
      while (FilReadLine (fpIn, sz, ";", sizeof (sz)) != (UINT)-1)
         {
         /*--- second line may be on next page ---*/
         if (strlen (sz) < 6)
            continue;
         if (strncmp (sz, "     ", 5))
            continue;
         if (sz[5] != ' ')
            break;
         }
      GetStr (szUnit, sz, 5, 4  );
      GetStr (szDesc, sz, 11, 60);

      fprintf (fpOut, "%s, %s, %s, %s", szItemNum, ToCSV(szDesc), szQuan, szUnit);
      for (uBid=0; uBid<uBidders; uBid++)
         fprintf (fpOut, ", %s, %s", szUnitPrice[uBid], szAmount [uBid]);
      fprintf (fpOut, "\n");
      }
   }


BOOL GetBidders (FILE *fpOut, FILE *fpIn)
   {
   PSZ  psz;

   while (FilReadLine (fpIn, sz, ";", sizeof (sz)) != (UINT)-1)
      {
      if (!strnicmp (sz, " BIDDER NO ", 11))
         break;
      }
   if (feof (fpIn))
      return FALSE;

   for (uBidders=1; ; uBidders++)
      {
      psz = StrStrip (StrClip (sz+11, " \t"), " \t");
      psz = StrStrip (psz + 1, " \t"); // remove bidder #

      if (uBidders > 1)
         fprintf (fpOut, ",");
      fprintf (fpOut, "%d, %s", uBidders, ToCSV(psz));

      if (FilReadLine (fpIn, sz, ";", sizeof (sz)) == (UINT)-1)
         break;
      if (strnicmp (sz, " BIDDER NO ", 11))
         break;
      }
   fprintf (fpOut, "\n");
   return !!uBidders;
   }



FILE *OpenXtabFile (PSZ pszProp)
   {
   FILE *fp;

   sprintf (sz, "PROP%s.CSV", pszProp);
   if (!(fp = fopen (sz, "wt")))
      Error ("Cannot open file %s", sz);
   return fp;
   }


BOOL FindProposal (FILE *fp, PSZ pszProp)
   {
   PSZ psz;

   while (FilReadLine (fp, sz, ";", sizeof (sz)) != (UINT)-1)
      {
      if (strnicmp (sz, " PROP ", 6))
         continue;
      psz = StrStrip (StrClip (sz+6, " \t"), " \t");
      if (strlen (psz) > 3)
         continue; // bid irregular for some reason;
      strcpy (pszProp, psz);
      return TRUE;
      }
   return FALSE;
   }


void ExtractXTABs (FILE *fpIn)
   {
   FILE *fpOut;
   UINT uProposalCount;
   CHAR szProp [128];

   while (FindProposal (fpIn, szProp))
      {
      fpOut = OpenXtabFile (szProp);
      if (!GetBidders (fpOut, fpIn))
         Error ("No bidders found for proposal %s", szProp);
      Extract (fpOut, fpIn);
      fclose (fpOut);
      uProposalCount++;
      }
   }



int main (int argc, char *argv[])
   {
   PSZ pszInFile;
   FILE *fpIn;

   if (ArgBuildBlk ("? *^Help "))
      Error ("%s", ArgGetErr ());

   if (ArgFillBlk (argv))
      Error ("%s", ArgGetErr ());

   if (ArgIs ("?") || ArgIs ("Help") || !ArgIs (NULL))
      Usage ();

   pszInFile = ArgGet (NULL, 0);
   if (!(fpIn = fopen (pszInFile, "rt")))
      Error ("Unable to open input file: %s", pszInFile);
   FilSetLine (0);

   ExtractXTABs (fpIn);
   fclose (fpIn);
   return 0;
   }

