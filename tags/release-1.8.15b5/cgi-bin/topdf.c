#include <stdio.h>
#include <stdlib.h>


/* topdf() - convert a HTML file to PDF */
FILE *topdf(const char *filename)	/* HTML file to convert */
{
  char	command[1024];			/* Command to execute */


  puts("Content-Type: application/pdf\n");

  sprintf(command, "htmldoc -t pdf --webpage %s", filename);

  return (popen(command, "w"));
}


/* topdf2() - pipe HTML output to HTMLDOC for conversion to PDF */
FILE *topdf2(void)
{
  puts("Content-Type: application/pdf\n");
  return (popen("htmldoc -t pdf --webpage -", "w"));
}
