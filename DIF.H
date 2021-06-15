


void WriteDIFHeader (FILE *fp, UINT uRowCount, UINT uColCount, PUINT puColSizes);

void WriteDIFTail   (FILE *fp);

void WriteDIFLabels (FILE *fp, UINT uRow, PPSZ ppszFields);

void WriteDIFString (FILE *fp, PSZ pszString, UINT uRow, UINT uCol, UINT uFormat);

void WriteDIFNumber (FILE *fp, BIG bgNum, UINT uRow, UINT uCol, UINT uDecimals);

void WriteDIFStartOfLine (FILE *fp);

void WriteDIFEndOfLine (FILE *fp);

