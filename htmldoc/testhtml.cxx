/*
 * "$Id: testhtml.cxx,v 1.3.2.4 2003/01/06 22:09:44 mike Exp $"
 *
 *   Test program for HTML parsing routines for HTMLDOC, an HTML document
 *   processing program.
 *
 *   Copyright 1997-2003 by Michael Sweet.
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

int				/* O - Exit status */
main(int  argc,			/* I - Number of command-line arguments */
     char *argv[])		/* I - Command-line arguments */
{
  int		i;		/* Looping var */
  FILE		*fp;		/* Input file */
  tree_t	*t,		/* HTML markup tree */
		*doc;		/* HTML document */
  char		base[1024];	/* Base directory */


#ifdef DEBUG
  printf("MARKUP_NONE=%d\n", MARKUP_NONE);
  printf("MARKUP_COMMENT=%d\n", MARKUP_COMMENT);
  printf("MARKUP_A=%d\n", MARKUP_A);
  printf("MARKUP_ADDRESS=%d\n", MARKUP_ADDRESS);
  printf("MARKUP_APPLET=%d\n", MARKUP_APPLET);
  printf("MARKUP_AREA=%d\n", MARKUP_AREA);
  printf("MARKUP_B=%d\n", MARKUP_B);
  printf("MARKUP_BASE=%d\n", MARKUP_BASE);
  printf("MARKUP_BASEFONT=%d\n", MARKUP_BASEFONT);
  printf("MARKUP_BIG=%d\n", MARKUP_BIG);
  printf("MARKUP_BLINK=%d\n", MARKUP_BLINK);
  printf("MARKUP_BLOCKQUOTE=%d\n", MARKUP_BLOCKQUOTE);
  printf("MARKUP_BODY=%d\n", MARKUP_BODY);
  printf("MARKUP_BR=%d\n", MARKUP_BR);
  printf("MARKUP_CAPTION=%d\n", MARKUP_CAPTION);
  printf("MARKUP_CENTER=%d\n", MARKUP_CENTER);
  printf("MARKUP_CITE=%d\n", MARKUP_CITE);
  printf("MARKUP_CODE=%d\n", MARKUP_CODE);
  printf("MARKUP_DD=%d\n", MARKUP_DD);
  printf("MARKUP_DFN=%d\n", MARKUP_DFN);
  printf("MARKUP_DIR=%d\n", MARKUP_DIR);
  printf("MARKUP_DIV=%d\n", MARKUP_DIV);
  printf("MARKUP_DL=%d\n", MARKUP_DL);
  printf("MARKUP_DT=%d\n", MARKUP_DT);
  printf("MARKUP_EM=%d\n", MARKUP_EM);
  printf("MARKUP_EMBED=%d\n", MARKUP_EMBED);
  printf("MARKUP_FONT=%d\n", MARKUP_FONT);
  printf("MARKUP_FORM=%d\n", MARKUP_FORM);
  printf("MARKUP_FRAME=%d\n", MARKUP_FRAME);
  printf("MARKUP_FRAMESET=%d\n", MARKUP_FRAMESET);
  printf("MARKUP_H1=%d\n", MARKUP_H1);
  printf("MARKUP_H2=%d\n", MARKUP_H2);
  printf("MARKUP_H3=%d\n", MARKUP_H3);
  printf("MARKUP_H4=%d\n", MARKUP_H4);
  printf("MARKUP_H5=%d\n", MARKUP_H5);
  printf("MARKUP_H6=%d\n", MARKUP_H6);
  printf("MARKUP_H7=%d\n", MARKUP_H7);
  printf("MARKUP_HEAD=%d\n", MARKUP_HEAD);
  printf("MARKUP_HR=%d\n", MARKUP_HR);
  printf("MARKUP_HTML=%d\n", MARKUP_HTML);
  printf("MARKUP_I=%d\n", MARKUP_I);
  printf("MARKUP_IMG=%d\n", MARKUP_IMG);
  printf("MARKUP_INPUT=%d\n", MARKUP_INPUT);
  printf("MARKUP_ISINDEX=%d\n", MARKUP_ISINDEX);
  printf("MARKUP_KBD=%d\n", MARKUP_KBD);
  printf("MARKUP_LI=%d\n", MARKUP_LI);
  printf("MARKUP_LINK=%d\n", MARKUP_LINK);
  printf("MARKUP_MAP=%d\n", MARKUP_MAP);
  printf("MARKUP_MENU=%d\n", MARKUP_MENU);
  printf("MARKUP_META=%d\n", MARKUP_META);
  printf("MARKUP_MULTICOL=%d\n", MARKUP_MULTICOL);
  printf("MARKUP_NOBR=%d\n", MARKUP_NOBR);
  printf("MARKUP_NOFRAMES=%d\n", MARKUP_NOFRAMES);
  printf("MARKUP_OL=%d\n", MARKUP_OL);
  printf("MARKUP_OPTION=%d\n", MARKUP_OPTION);
  printf("MARKUP_P=%d\n", MARKUP_P);
  printf("MARKUP_PRE=%d\n", MARKUP_PRE);
  printf("MARKUP_S=%d\n", MARKUP_S);
  printf("MARKUP_SAMP=%d\n", MARKUP_SAMP);
  printf("MARKUP_SCRIPT=%d\n", MARKUP_SCRIPT);
  printf("MARKUP_SELECT=%d\n", MARKUP_SELECT);
  printf("MARKUP_SMALL=%d\n", MARKUP_SMALL);
  printf("MARKUP_SPACER=%d\n", MARKUP_SPACER);
  printf("MARKUP_STRIKE=%d\n", MARKUP_STRIKE);
  printf("MARKUP_STRONG=%d\n", MARKUP_STRONG);
  printf("MARKUP_STYLE=%d\n", MARKUP_STYLE);
  printf("MARKUP_SUB=%d\n", MARKUP_SUB);
  printf("MARKUP_SUP=%d\n", MARKUP_SUP);
  printf("MARKUP_TABLE=%d\n", MARKUP_TABLE);
  printf("MARKUP_TD=%d\n", MARKUP_TD);
  printf("MARKUP_TEXTAREA=%d\n", MARKUP_TEXTAREA);
  printf("MARKUP_TH=%d\n", MARKUP_TH);
  printf("MARKUP_TITLE=%d\n", MARKUP_TITLE);
  printf("MARKUP_TR=%d\n", MARKUP_TR);
  printf("MARKUP_TT=%d\n", MARKUP_TT);
  printf("MARKUP_U=%d\n", MARKUP_U);
  printf("MARKUP_UL=%d\n", MARKUP_UL);
  printf("MARKUP_VAR=%d\n", MARKUP_VAR);
  printf("MARKUP_WBR=%d\n", MARKUP_WBR);
#endif /* DEBUG */

  if (argc < 2)
  {
    fputs("Usage: testhtml filename.html\n", stderr);
    return(1);
  };

  for (i = 1, doc = NULL; i < argc; i ++)
    if ((fp = fopen(file_find("", argv[i]), "r")) != NULL)
    {
      strcpy(base, argv[i]);
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
    htmlWriteFile(doc, stdout);

  return (doc == NULL);
}


/*
 * End of "$Id: testhtml.cxx,v 1.3.2.4 2003/01/06 22:09:44 mike Exp $".
 */
