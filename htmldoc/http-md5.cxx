//
// "$Id: http-md5.cxx,v 1.4.2.2 2004/03/30 03:49:15 mike Exp $"
//
//   MD5 password support for HTMLDOC.
//
//   Copyright 1997-2004 by Easy Software Products, all rights reserved.
//
//   These coded instructions, statements, and computer programs are the
//   property of Easy Software Products and are protected by Federal
//   copyright law.  Distribution and use rights are outlined in the file
//   "COPYING.txt" which should have been included with this file.  If this
//   file is missing or damaged please contact Easy Software Products
//   at:
//
//       Attn: ESP Licensing Information
//       Easy Software Products
//       44141 Airport View Drive, Suite 204
//       Hollywood, Maryland 20636-3142 USA
//
//       Voice: (301) 373-9600
//       EMail: info@easysw.com
//         WWW: http://www.easysw.com
//
// Contents:
//
//   hdHTTP::md5()        - Compute the MD5 sum of the username:group:password.
//   hdHTTP::md5Nonce()   - Combine the MD5 sum of the username, group, and
//                          password with the server-supplied nonce value.
//   hdHTTP::md5_string() - Convert an MD5 sum to a character string.
//

/*
 * Include necessary headers...
 */

#include "http.h"
#include "hdstring.h"


/*
 * 'hdHTTP::md5()' - Compute the MD5 sum of the username:group:password.
 */

char *					/* O - MD5 sum */
hdHTTP::md5(const char *username,	/* I - User name */
            const char *realm,		/* I - Realm name */
            const char *passwd,		/* I - Password string */
	    char       *md5,		/* O - MD5 string */
	    int        md5len)		/* I - Size of MD5 string */
{
  hdMD5		state;			/* MD5 state info */
  hdByte	sum[16];		/* Sum data */
  char		line[256];		/* Line to sum */


 /*
  * Compute the MD5 sum of the user name, group name, and password.
  */

  snprintf(line, sizeof(line), "%s:%s:%s", username, realm, passwd);
  state.init();
  state.append((hdByte *)line, strlen(line));
  state.finish(sum);

 /*
  * Return the sum...
  */

  return (hdHTTP::md5_string(sum, md5, md5len));
}


/*
 * 'hdHTTP::md5_final()' - Combine the MD5 sum of the username, group, and password
 *                    with the server-supplied nonce value, method, and
 *                    request-uri.
 */

char *					/* O - New sum */
hdHTTP::md5_final(const char *nonce,	/* I - Server nonce value */
                  const char *method,	/* I - METHOD (GET, POST, etc.) */
	          const char *resource,	/* I - Resource path */
	          char       *md5,	/* O - MD5 string */
	          int        md5len)	/* I - Size of MD5 string */
{
  hdMD5		state;			/* MD5 state info */
  hdByte	sum[16];		/* Sum data */
  char		line[1024];		/* Line of data */
  char		a2[33];			/* Hash of method and resource */


 /*
  * First compute the MD5 sum of the method and resource...
  */

  snprintf(line, sizeof(line), "%s:%s", method, resource);
  state.init();
  state.append((hdByte *)line, strlen(line));
  state.finish(sum);
  hdHTTP::md5_string(sum, a2, sizeof(a2));

 /*
  * Then combine A1 (MD5 of username, realm, and password) with the nonce
  * and A2 (method + resource) values to get the final MD5 sum for the
  * request...
  */

  snprintf(line, sizeof(line), "%s%s:%s", md5, nonce, a2);

  state.init();
  state.append((hdByte *)line, strlen(line));
  state.finish(sum);

  return (md5_string(sum, md5, md5len));
}


/*
 * 'hdHTTP::md5_string()' - Convert an MD5 sum to a character string.
 */

char *						/* O - MD5 sum in hex */
hdHTTP::md5_string(const hdByte *sum,		/* I - MD5 sum data */
	           char         *md5,		/* O - MD5 sun in hex */
	           int          md5len)		/* I - Size of MD5 string */
{
  int		i;				/* Looping var */
  char		*md5ptr;			/* Pointer into MD5 string */
  static char	*hex = "0123456789abcdef";	/* Hex digits */


 /*
  * Convert the MD5 sum to hexadecimal...
  */

  for (i = 16, md5ptr = md5; i > 0 && md5ptr < (md5 + md5len - 2); i --, sum ++)
  {
    *md5ptr++ = hex[*sum >> 4];
    *md5ptr++ = hex[*sum & 15];
  }

  *md5ptr = '\0';

  return (md5);
}


/*
 * End of "$Id: http-md5.cxx,v 1.4.2.2 2004/03/30 03:49:15 mike Exp $".
 */
