

UINT XLSAddFormat (PSZ pszFormat);

UINT XLSAddXF (UINT uFormat, UINT uFlags);

UINT XLSWriteHeader (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes);

UINT XLSWriteTail (FILE *fp);

UINT XLSWriteLabels (FILE *fp, UINT uRow, PPSZ ppszFields);

UINT XLSWriteString (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uXF, UINT uFlags);

UINT XLSWriteNumber (FILE *fp, BIG bgNum,     UINT uRow, UINT uCol, UINT uXF, UINT uFlags);


#define XLS_SHADED        0x80

#define XLS_BORDERBOTTOM  0x40
#define XLS_BORDERTOP     0x20
#define XLS_BORDERRIGHT   0x10
#define XLS_BORDERLEFT    0x08 

#define XLS_ALIGNGENERAL  0x00
#define XLS_ALIGNLEFT     0x01
#define XLS_ALIGNCENTER   0x02
#define XLS_ALIGNRIGHT    0x03
#define XLS_ALIGNFILL     0x04


