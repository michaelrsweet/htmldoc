/*
 * "$Id: testhtml.cxx,v 1.7 2004/10/23 07:06:19 mike Exp $"
 *
 *   Test program for HTML parsing routines for HTMLDOC, an HTML document
 *   processing program.
 *
 *   Copyright 1997-2004 by Michael Sweet.
 *
 *   HTMLDOC is distributed under the terms of the Aladdin Free Public License
 *   which is described in the file "LICENSE.txt".
 *
 * Contents:
 *
 *   main() - Main entry for test program.
 */

/*
 * Include necessary headers.
 */

#define _HTMLDOC_CXX_
#include "htmldoc.h"


void	prefs_load(void) { }
void	prefs_save(void) { }


/*
 * 'main()' - Main entry for test program.
 */

int					// O - Exit status
main(int  argc,				// I - Number of command-line arguments
     char *argv[])			// I - Command-line arguments
{
  int		i;			// Looping var
  FILE		*fp;			// Input file
  hdTree	*t,			// HTML markup tree
		*doc,			// HTML document
		*toc;			// Table-of-contents
  hdBook	*book;			// Book
  char		base[1024];		// Base directory


#ifdef DEBUG
  printf("HD_ELEMENT_NONE=%d\n", HD_ELEMENT_NONE);
  printf("HD_ELEMENT_COMMENT=%d\n", HD_ELEMENT_COMMENT);
  printf("HD_ELEMENT_A=%d\n", HD_ELEMENT_A);
  printf("HD_ELEMENT_ADDRESS=%d\n", HD_ELEMENT_ADDRESS);
  printf("HD_ELEMENT_APPLET=%d\n", HD_ELEMENT_APPLET);
  printf("HD_ELEMENT_AREA=%d\n", HD_ELEMENT_AREA);
  printf("HD_ELEMENT_B=%d\n", HD_ELEMENT_B);
  printf("HD_ELEMENT_BASE=%d\n", HD_ELEMENT_BASE);
  printf("HD_ELEMENT_BASEFONT=%d\n", HD_ELEMENT_BASEFONT);
  printf("HD_ELEMENT_BIG=%d\n", HD_ELEMENT_BIG);
  printf("HD_ELEMENT_BLINK=%d\n", HD_ELEMENT_BLINK);
  printf("HD_ELEMENT_BLOCKQUOTE=%d\n", HD_ELEMENT_BLOCKQUOTE);
  printf("HD_ELEMENT_BODY=%d\n", HD_ELEMENT_BODY);
  printf("HD_ELEMENT_BR=%d\n", HD_ELEMENT_BR);
  printf("HD_ELEMENT_CAPTION=%d\n", HD_ELEMENT_CAPTION);
  printf("HD_ELEMENT_CENTER=%d\n", HD_ELEMENT_CENTER);
  printf("HD_ELEMENT_CITE=%d\n", HD_ELEMENT_CITE);
  printf("HD_ELEMENT_CODE=%d\n", HD_ELEMENT_CODE);
  printf("HD_ELEMENT_DD=%d\n", HD_ELEMENT_DD);
  printf("HD_ELEMENT_DFN=%d\n", HD_ELEMENT_DFN);
  printf("HD_ELEMENT_DIR=%d\n", HD_ELEMENT_DIR);
  printf("HD_ELEMENT_DIV=%d\n", HD_ELEMENT_DIV);
  printf("HD_ELEMENT_DL=%d\n", HD_ELEMENT_DL);
  printf("HD_ELEMENT_DT=%d\n", HD_ELEMENT_DT);
  printf("HD_ELEMENT_EM=%d\n", HD_ELEMENT_EM);
  printf("HD_ELEMENT_EMBED=%d\n", HD_ELEMENT_EMBED);
  printf("HD_ELEMENT_FONT=%d\n", HD_ELEMENT_FONT);
  printf("HD_ELEMENT_FORM=%d\n", HD_ELEMENT_FORM);
  printf("HD_ELEMENT_FRAME=%d\n", HD_ELEMENT_FRAME);
  printf("HD_ELEMENT_FRAMESET=%d\n", HD_ELEMENT_FRAMESET);
  printf("HD_ELEMENT_H1=%d\n", HD_ELEMENT_H1);
  printf("HD_ELEMENT_H2=%d\n", HD_ELEMENT_H2);
  printf("HD_ELEMENT_H3=%d\n", HD_ELEMENT_H3);
  printf("HD_ELEMENT_H4=%d\n", HD_ELEMENT_H4);
  printf("HD_ELEMENT_H5=%d\n", HD_ELEMENT_H5);
  printf("HD_ELEMENT_H6=%d\n", HD_ELEMENT_H6);
  printf("HD_ELEMENT_H7=%d\n", HD_ELEMENT_H7);
  printf("HD_ELEMENT_HEAD=%d\n", HD_ELEMENT_HEAD);
  printf("HD_ELEMENT_HR=%d\n", HD_ELEMENT_HR);
  printf("HD_ELEMENT_HTML=%d\n", HD_ELEMENT_HTML);
  printf("HD_ELEMENT_I=%d\n", HD_ELEMENT_I);
  printf("HD_ELEMENT_IMG=%d\n", HD_ELEMENT_IMG);
  printf("HD_ELEMENT_INPUT=%d\n", HD_ELEMENT_INPUT);
  printf("HD_ELEMENT_ISINDEX=%d\n", HD_ELEMENT_ISINDEX);
  printf("HD_ELEMENT_KBD=%d\n", HD_ELEMENT_KBD);
  printf("HD_ELEMENT_LI=%d\n", HD_ELEMENT_LI);
  printf("HD_ELEMENT_LINK=%d\n", HD_ELEMENT_LINK);
  printf("HD_ELEMENT_MAP=%d\n", HD_ELEMENT_MAP);
  printf("HD_ELEMENT_MENU=%d\n", HD_ELEMENT_MENU);
  printf("HD_ELEMENT_META=%d\n", HD_ELEMENT_META);
  printf("HD_ELEMENT_MULTICOL=%d\n", HD_ELEMENT_MULTICOL);
  printf("HD_ELEMENT_NOBR=%d\n", HD_ELEMENT_NOBR);
  printf("HD_ELEMENT_NOFRAMES=%d\n", HD_ELEMENT_NOFRAMES);
  printf("HD_ELEMENT_OL=%d\n", HD_ELEMENT_OL);
  printf("HD_ELEMENT_OPTION=%d\n", HD_ELEMENT_OPTION);
  printf("HD_ELEMENT_P=%d\n", HD_ELEMENT_P);
  printf("HD_ELEMENT_PRE=%d\n", HD_ELEMENT_PRE);
  printf("HD_ELEMENT_S=%d\n", HD_ELEMENT_S);
  printf("HD_ELEMENT_SAMP=%d\n", HD_ELEMENT_SAMP);
  printf("HD_ELEMENT_SCRIPT=%d\n", HD_ELEMENT_SCRIPT);
  printf("HD_ELEMENT_SELECT=%d\n", HD_ELEMENT_SELECT);
  printf("HD_ELEMENT_SMALL=%d\n", HD_ELEMENT_SMALL);
  printf("HD_ELEMENT_SPACER=%d\n", HD_ELEMENT_SPACER);
  printf("HD_ELEMENT_STRIKE=%d\n", HD_ELEMENT_STRIKE);
  printf("HD_ELEMENT_STRONG=%d\n", HD_ELEMENT_STRONG);
  printf("HD_ELEMENT_STYLE=%d\n", HD_ELEMENT_STYLE);
  printf("HD_ELEMENT_SUB=%d\n", HD_ELEMENT_SUB);
  printf("HD_ELEMENT_SUP=%d\n", HD_ELEMENT_SUP);
  printf("HD_ELEMENT_TABLE=%d\n", HD_ELEMENT_TABLE);
  printf("HD_ELEMENT_TD=%d\n", HD_ELEMENT_TD);
  printf("HD_ELEMENT_TEXTAREA=%d\n", HD_ELEMENT_TEXTAREA);
  printf("HD_ELEMENT_TH=%d\n", HD_ELEMENT_TH);
  printf("HD_ELEMENT_TITLE=%d\n", HD_ELEMENT_TITLE);
  printf("HD_ELEMENT_TR=%d\n", HD_ELEMENT_TR);
  printf("HD_ELEMENT_TT=%d\n", HD_ELEMENT_TT);
  printf("HD_ELEMENT_U=%d\n", HD_ELEMENT_U);
  printf("HD_ELEMENT_UL=%d\n", HD_ELEMENT_UL);
  printf("HD_ELEMENT_VAR=%d\n", HD_ELEMENT_VAR);
  printf("HD_ELEMENT_WBR=%d\n", HD_ELEMENT_WBR);
#endif /* DEBUG */

  if (argc < 2)
  {
    fputs("Usage: testhtml filename.html\n", stderr);
    return(1);
  };

  for (i = 1, doc = NULL; i < argc; i ++)
    if ((fp = fopen(file_find("", argv[i]), "r")) != NULL)
    {
      strlcpy(base, argv[i], sizeof(base));
      if (strrchr(base, '/') != NULL)
        *strrchr(base, '/') = '\0';
      else
        base[0] = '\0';

      t = htmlReadFile(NULL, fp, base);
      fclose(fp);

      if (t != NULL)
        if (doc == NULL)
          doc = t;
        else
        {
          doc->next = t;
          t->prev   = doc;
        };
    }
    else
      fprintf(stderr, "testhtml: Unable to open input file \'%s\'!\n", argv[i]);

  if (doc != NULL)
  {
    htmlWriteFile(doc, stdout);
    book = new hdBook();
    toc = book->toc_build(doc);
    puts("---- TABLE OF CONTENTS ----");
    htmlWriteFile(toc, stdout);
  }

  return (doc == NULL);
}


/*
 * End of "$Id: testhtml.cxx,v 1.7 2004/10/23 07:06:19 mike Exp $".
 */
