void WriteWK1Header (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes);

void WriteWK1Tail   (FILE *fp);

void WriteWK1Labels (FILE *fp, UINT uRow, PPSZ ppszFields);

void WriteWK1String (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uFormat);

void WriteWK1Number (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uDecimals);


