/*
 * TLS support code for HTMLDOC using GNU TLS.
 *
 * Copyright © 2007-2019 by Apple Inc.
 * Copyright © 1997-2007 by Easy Software Products, all rights reserved.
 *
 * Licensed under Apache License v2.0.  See the file "LICENSE" for more
 * information.
 */

/**** This file is included from tls.c ****/

/*
 * Include necessary headers...
 */

#include <sys/stat.h>


/*
 * Local globals...
 */

//static int		tls_auto_create = 0;
					/* Auto-create self-signed certs? */
//static char		*tls_common_name = NULL;
					/* Default common name */
static gnutls_x509_crl_t tls_crl = NULL;/* Certificate revocation list */
static char		*tls_keypath = NULL;
					/* Server cert keychain path */
//static _cups_mutex_t	tls_mutex = _CUPS_MUTEX_INITIALIZER;
					/* Mutex for keychain/certs */
static int		tls_options = -1,/* Options for TLS connections */
			tls_min_version = _HTTP_TLS_1_0,
			tls_max_version = _HTTP_TLS_MAX;


/*
 * Local functions...
 */

static gnutls_x509_crt_t http_gnutls_create_credential(http_credential_t *credential);
static const char	*http_gnutls_default_path(char *buffer, size_t bufsize);
//static void		http_gnutls_load_crl(void);
static const char	*http_gnutls_make_path(char *buffer, size_t bufsize, const char *dirname, const char *filename, const char *ext);
static ssize_t		http_gnutls_read(gnutls_transport_ptr_t ptr, void *data, size_t length);
static ssize_t		http_gnutls_write(gnutls_transport_ptr_t ptr, const void *data, size_t length);


/*
 * 'httpCopyCredentials()' - Copy the credentials associated with the peer in
 *                           an encrypted connection.
 *
 * @since CUPS 1.5/macOS 10.7@
 */

int					/* O - Status of call (0 = success) */
httpCopyCredentials(
    http_t	 *http,			/* I - Connection to server */
    cups_array_t **credentials)		/* O - Array of credentials */
{
  unsigned		count;		/* Number of certificates */
  const gnutls_datum_t *certs;		/* Certificates */


  DEBUG_printf(("httpCopyCredentials(http=%p, credentials=%p)", http, credentials));

  if (credentials)
    *credentials = NULL;

  if (!http || !http->tls || !credentials)
    return (-1);

  *credentials = cupsArrayNew(NULL, NULL);
  certs        = gnutls_certificate_get_peers(http->tls, &count);

  DEBUG_printf(("1httpCopyCredentials: certs=%p, count=%u", certs, count));

  if (certs && count)
  {
    while (count > 0)
    {
      httpAddCredential(*credentials, certs->data, certs->size);
      certs ++;
      count --;
    }
  }

  return (0);
}


/*
 * '_httpCreateCredentials()' - Create credentials in the internal format.
 */

http_tls_credentials_t			/* O - Internal credentials */
_httpCreateCredentials(
    cups_array_t *credentials)		/* I - Array of credentials */
{
  (void)credentials;

  return (NULL);
}


/*
 * '_httpFreeCredentials()' - Free internal credentials.
 */

void
_httpFreeCredentials(
    http_tls_credentials_t credentials)	/* I - Internal credentials */
{
  (void)credentials;
}


/*
 * 'httpCredentialsAreValidForName()' - Return whether the credentials are valid for the given name.
 *
 * @since CUPS 2.0/OS 10.10@
 */

int					/* O - 1 if valid, 0 otherwise */
httpCredentialsAreValidForName(
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Name to check */
{
  gnutls_x509_crt_t	cert;		/* Certificate */
  int			result = 0;	/* Result */


  cert = http_gnutls_create_credential((http_credential_t *)cupsArrayFirst(credentials));
  if (cert)
  {
    result = gnutls_x509_crt_check_hostname(cert, common_name) != 0;

    if (result)
    {
      gnutls_x509_crl_iter_t iter = NULL;
					/* Iterator */
      unsigned char	cserial[1024],	/* Certificate serial number */
			rserial[1024];	/* Revoked serial number */
      size_t		cserial_size,	/* Size of cert serial number */
			rserial_size;	/* Size of revoked serial number */

//      _cupsMutexLock(&tls_mutex);

      if (gnutls_x509_crl_get_crt_count(tls_crl) > 0)
      {
        cserial_size = sizeof(cserial);
        gnutls_x509_crt_get_serial(cert, cserial, &cserial_size);

	rserial_size = sizeof(rserial);

        while (!gnutls_x509_crl_iter_crt_serial(tls_crl, &iter, rserial, &rserial_size, NULL))
        {
          if (cserial_size == rserial_size && !memcmp(cserial, rserial, rserial_size))
	  {
	    result = 0;
	    break;
	  }

	  rserial_size = sizeof(rserial);
	}
	gnutls_x509_crl_iter_deinit(iter);
      }

//      _cupsMutexUnlock(&tls_mutex);
    }

    gnutls_x509_crt_deinit(cert);
  }

  return (result);
}


/*
 * 'httpCredentialsGetTrust()' - Return the trust of credentials.
 *
 * @since CUPS 2.0/OS 10.10@
 */

http_trust_t				/* O - Level of trust */
httpCredentialsGetTrust(
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Common name for trust lookup */
{
  http_trust_t		trust = HTTP_TRUST_OK;
					/* Trusted? */
  gnutls_x509_crt_t	cert;		/* Certificate */
  cups_array_t		*tcreds = NULL;	/* Trusted credentials */
//  _cups_globals_t	*cg = _cupsGlobals();
					/* Per-thread globals */


  if (!common_name)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("No common name specified."), 1);
    return (HTTP_TRUST_UNKNOWN);
  }

  if ((cert = http_gnutls_create_credential((http_credential_t *)cupsArrayFirst(credentials))) == NULL)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Unable to create credentials from array."), 1);
    return (HTTP_TRUST_UNKNOWN);
  }

#if 0
  if (cg->any_root < 0)
  {
    _cupsSetDefaults();
    http_gnutls_load_crl();
  }
#endif // 0

 /*
  * Look this common name up in the default keychains...
  */

  httpLoadCredentials(NULL, &tcreds, common_name);

  if (tcreds)
  {
    char	credentials_str[1024],	/* String for incoming credentials */
		tcreds_str[1024];	/* String for saved credentials */

    httpCredentialsString(credentials, credentials_str, sizeof(credentials_str));
    httpCredentialsString(tcreds, tcreds_str, sizeof(tcreds_str));

    if (strcmp(credentials_str, tcreds_str))
    {
     /*
      * Credentials don't match, let's look at the expiration date of the new
      * credentials and allow if the new ones have a later expiration...
      */

#if 0
      if (!cg->trust_first)
      {
       /*
        * Do not trust certificates on first use...
	*/

        _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Trust on first use is disabled."), 1);

        trust = HTTP_TRUST_INVALID;
      }
      else
#endif // 0
      if (httpCredentialsGetExpiration(credentials) <= httpCredentialsGetExpiration(tcreds))
      {
       /*
        * The new credentials are not newly issued...
	*/

        _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("New credentials are older than stored credentials."), 1);

        trust = HTTP_TRUST_INVALID;
      }
      else if (!httpCredentialsAreValidForName(credentials, common_name))
      {
       /*
        * The common name does not match the issued certificate...
	*/

        _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("New credentials are not valid for name."), 1);

        trust = HTTP_TRUST_INVALID;
      }
      else if (httpCredentialsGetExpiration(tcreds) < time(NULL))
      {
       /*
        * Save the renewed credentials...
	*/

	trust = HTTP_TRUST_RENEWED;

        httpSaveCredentials(NULL, credentials, common_name);
      }
    }

    httpFreeCredentials(tcreds);
  }
#if 0
  else if (cg->validate_certs && !httpCredentialsAreValidForName(credentials, common_name))
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("No stored credentials, not valid for name."), 1);
    trust = HTTP_TRUST_INVALID;
  }
  else if (!cg->trust_first)
  {
   /*
    * See if we have a site CA certificate we can compare...
    */

    if (!httpLoadCredentials(NULL, &tcreds, "site"))
    {
      if (cupsArrayCount(credentials) != (cupsArrayCount(tcreds) + 1))
      {
       /*
        * Certificate isn't directly generated from the CA cert...
	*/

        trust = HTTP_TRUST_INVALID;
      }
      else
      {
       /*
        * Do a tail comparison of the two certificates...
	*/

        http_credential_t	*a, *b;		/* Certificates */

        for (a = (http_credential_t *)cupsArrayFirst(tcreds), b = (http_credential_t *)cupsArrayIndex(credentials, 1);
	     a && b;
	     a = (http_credential_t *)cupsArrayNext(tcreds), b = (http_credential_t *)cupsArrayNext(credentials))
	  if (a->datalen != b->datalen || memcmp(a->data, b->data, a->datalen))
	    break;

        if (a || b)
	  trust = HTTP_TRUST_INVALID;
      }

      if (trust != HTTP_TRUST_OK)
	_cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Credentials do not validate against site CA certificate."), 1);
    }
    else
    {
      _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Trust on first use is disabled."), 1);
      trust = HTTP_TRUST_INVALID;
    }
  }
#endif // 0

  if (trust == HTTP_TRUST_OK /*&& !cg->expired_certs*/)
  {
    time_t	curtime;		/* Current date/time */

    time(&curtime);
    if (curtime < gnutls_x509_crt_get_activation_time(cert) ||
        curtime > gnutls_x509_crt_get_expiration_time(cert))
    {
      _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Credentials have expired."), 1);
      trust = HTTP_TRUST_EXPIRED;
    }
  }

#if 0
  if (trust == HTTP_TRUST_OK && !cg->any_root && cupsArrayCount(credentials) == 1)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Self-signed credentials are blocked."), 1);
    trust = HTTP_TRUST_INVALID;
  }
#endif // 0

  gnutls_x509_crt_deinit(cert);

  return (trust);
}


/*
 * 'httpCredentialsGetExpiration()' - Return the expiration date of the credentials.
 *
 * @since CUPS 2.0/OS 10.10@
 */

time_t					/* O - Expiration date of credentials */
httpCredentialsGetExpiration(
    cups_array_t *credentials)		/* I - Credentials */
{
  gnutls_x509_crt_t	cert;		/* Certificate */
  time_t		result = 0;	/* Result */


  cert = http_gnutls_create_credential((http_credential_t *)cupsArrayFirst(credentials));
  if (cert)
  {
    result = gnutls_x509_crt_get_expiration_time(cert);
    gnutls_x509_crt_deinit(cert);
  }

  return (result);
}


/*
 * 'httpCredentialsString()' - Return a string representing the credentials.
 *
 * @since CUPS 2.0/OS 10.10@
 */

size_t					/* O - Total size of credentials string */
httpCredentialsString(
    cups_array_t *credentials,		/* I - Credentials */
    char         *buffer,		/* I - Buffer or @code NULL@ */
    size_t       bufsize)		/* I - Size of buffer */
{
  http_credential_t	*first;		/* First certificate */
  gnutls_x509_crt_t	cert;		/* Certificate */


  DEBUG_printf(("httpCredentialsString(credentials=%p, buffer=%p, bufsize=" CUPS_LLFMT ")", credentials, buffer, CUPS_LLCAST bufsize));

  if (!buffer)
    return (0);

  if (buffer && bufsize > 0)
    *buffer = '\0';

  if ((first = (http_credential_t *)cupsArrayFirst(credentials)) != NULL &&
      (cert = http_gnutls_create_credential(first)) != NULL)
  {
    char		name[256],	/* Common name associated with cert */
			issuer[256];	/* Issuer associated with cert */
    size_t		len;		/* Length of string */
    time_t		expiration;	/* Expiration date of cert */
    int			sigalg;		/* Signature algorithm */
    _cups_md5_state_t	md5_state;	/* MD5 state */
    unsigned char	md5_digest[16];	/* MD5 result */

    len = sizeof(name) - 1;
    if (gnutls_x509_crt_get_dn_by_oid(cert, GNUTLS_OID_X520_COMMON_NAME, 0, 0, name, &len) >= 0)
      name[len] = '\0';
    else
      strlcpy(name, "unknown", sizeof(name));

    len = sizeof(issuer) - 1;
    if (gnutls_x509_crt_get_issuer_dn_by_oid(cert, GNUTLS_OID_X520_ORGANIZATION_NAME, 0, 0, issuer, &len) >= 0)
      issuer[len] = '\0';
    else
      strlcpy(issuer, "unknown", sizeof(issuer));

    expiration = gnutls_x509_crt_get_expiration_time(cert);
    sigalg     = gnutls_x509_crt_get_signature_algorithm(cert);

    _cupsMD5Init(&md5_state);
    _cupsMD5Append(&md5_state, first->data, (int)first->datalen);
    _cupsMD5Finish(&md5_state, md5_digest);

    snprintf(buffer, bufsize, "%s (issued by %s) / %s / %s / %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", name, issuer, httpGetDateString(expiration), gnutls_sign_get_name((gnutls_sign_algorithm_t)sigalg), md5_digest[0], md5_digest[1], md5_digest[2], md5_digest[3], md5_digest[4], md5_digest[5], md5_digest[6], md5_digest[7], md5_digest[8], md5_digest[9], md5_digest[10], md5_digest[11], md5_digest[12], md5_digest[13], md5_digest[14], md5_digest[15]);

    gnutls_x509_crt_deinit(cert);
  }

  DEBUG_printf(("1httpCredentialsString: Returning \"%s\".", buffer));

  return (strlen(buffer));
}


/*
 * 'httpLoadCredentials()' - Load X.509 credentials from a keychain file.
 *
 * @since CUPS 2.0/OS 10.10@
 */

int					/* O - 0 on success, -1 on error */
httpLoadCredentials(
    const char   *path,			/* I  - Keychain/PKCS#12 path */
    cups_array_t **credentials,		/* IO - Credentials */
    const char   *common_name)		/* I  - Common name for credentials */
{
  FILE			*fp;		/* Certificate file */
  char			filename[1024],	/* filename.crt */
			temp[1024],	/* Temporary string */
			line[256],	/* Base64-encoded line */
			*lineptr;	/* Pointer into line */
  unsigned char		*data = NULL;	/* Buffer for cert data */
  size_t		alloc_data = 0,	/* Bytes allocated */
			num_data = 0;	/* Bytes used */
  int			decoded;	/* Bytes decoded */
  int			in_certificate = 0;
					/* In a certificate? */


  if (!credentials || !common_name)
    return (-1);

  if (!path)
    path = http_gnutls_default_path(temp, sizeof(temp));
  if (!path)
    return (-1);

  http_gnutls_make_path(filename, sizeof(filename), path, common_name, "crt");

  if ((fp = fopen(filename, "r")) == NULL)
    return (-1);

  while (fgets(line, sizeof(line), fp))
  {
    if ((lineptr = line + strlen(line) - 1) >= line && *lineptr == '\n')
      *lineptr = '\0';

    if (!strcmp(line, "-----BEGIN CERTIFICATE-----"))
    {
      if (in_certificate)
      {
       /*
	* Missing END CERTIFICATE...
	*/

        httpFreeCredentials(*credentials);
	*credentials = NULL;
        break;
      }

      in_certificate = 1;
    }
    else if (!strcmp(line, "-----END CERTIFICATE-----"))
    {
      if (!in_certificate || !num_data)
      {
       /*
	* Missing data...
	*/

        httpFreeCredentials(*credentials);
	*credentials = NULL;
        break;
      }

      if (!*credentials)
        *credentials = cupsArrayNew(NULL, NULL);

      if (httpAddCredential(*credentials, data, num_data))
      {
        httpFreeCredentials(*credentials);
	*credentials = NULL;
        break;
      }

      num_data       = 0;
      in_certificate = 0;
    }
    else if (in_certificate)
    {
      if (alloc_data == 0)
      {
        data       = malloc(2048);
	alloc_data = 2048;

        if (!data)
	  break;
      }
      else if ((num_data + strlen(line)) >= alloc_data)
      {
        unsigned char *tdata = realloc(data, alloc_data + 1024);
					/* Expanded buffer */

	if (!tdata)
	{
	  httpFreeCredentials(*credentials);
	  *credentials = NULL;
	  break;
	}

	data       = tdata;
        alloc_data += 1024;
      }

      decoded = alloc_data - num_data;
      httpDecode64_2((char *)data + num_data, &decoded, line);
      num_data += (size_t)decoded;
    }
  }

  fclose(fp);

  if (in_certificate)
  {
   /*
    * Missing END CERTIFICATE...
    */

    httpFreeCredentials(*credentials);
    *credentials = NULL;
  }

  if (data)
    free(data);

  return (*credentials ? 0 : -1);
}


/*
 * 'httpSaveCredentials()' - Save X.509 credentials to a keychain file.
 *
 * @since CUPS 2.0/OS 10.10@
 */

int					/* O - -1 on error, 0 on success */
httpSaveCredentials(
    const char   *path,			/* I - Keychain/PKCS#12 path */
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Common name for credentials */
{
  FILE			*fp;		/* Certificate file */
  char			filename[1024],	/* filename.crt */
			nfilename[1024],/* filename.crt.N */
			temp[1024],	/* Temporary string */
			line[256];	/* Base64-encoded line */
  const unsigned char	*ptr;		/* Pointer into certificate */
  ssize_t		remaining;	/* Bytes left */
  http_credential_t	*cred;		/* Current credential */


  if (!credentials || !common_name)
    return (-1);

  if (!path)
    path = http_gnutls_default_path(temp, sizeof(temp));
  if (!path)
    return (-1);

  http_gnutls_make_path(filename, sizeof(filename), path, common_name, "crt");
  snprintf(nfilename, sizeof(nfilename), "%s.N", filename);

  if ((fp = fopen(nfilename, "w")) == NULL)
    return (-1);

  fchmod(fileno(fp), 0600);

  for (cred = (http_credential_t *)cupsArrayFirst(credentials);
       cred;
       cred = (http_credential_t *)cupsArrayNext(credentials))
  {
    fputs("-----BEGIN CERTIFICATE-----\n", fp);
    for (ptr = cred->data, remaining = (ssize_t)cred->datalen; remaining > 0; remaining -= 45, ptr += 45)
    {
      httpEncode64_2(line, sizeof(line), (char *)ptr, remaining > 45 ? 45 : remaining);
      fprintf(fp, "%s\n", line);
    }
    fputs("-----END CERTIFICATE-----\n", fp);
  }

  fclose(fp);

  return (rename(nfilename, filename));
}


/*
 * 'http_gnutls_create_credential()' - Create a single credential in the internal format.
 */

static gnutls_x509_crt_t			/* O - Certificate */
http_gnutls_create_credential(
    http_credential_t *credential)		/* I - Credential */
{
  int			result;			/* Result from GNU TLS */
  gnutls_x509_crt_t	cert;			/* Certificate */
  gnutls_datum_t	datum;			/* Data record */


  DEBUG_printf(("3http_gnutls_create_credential(credential=%p)", credential));

  if (!credential)
    return (NULL);

  if ((result = gnutls_x509_crt_init(&cert)) < 0)
  {
    DEBUG_printf(("4http_gnutls_create_credential: init error: %s", gnutls_strerror(result)));
    return (NULL);
  }

  datum.data = credential->data;
  datum.size = credential->datalen;

  if ((result = gnutls_x509_crt_import(cert, &datum, GNUTLS_X509_FMT_DER)) < 0)
  {
    DEBUG_printf(("4http_gnutls_create_credential: import error: %s", gnutls_strerror(result)));

    gnutls_x509_crt_deinit(cert);
    return (NULL);
  }

  return (cert);
}


/*
 * 'http_gnutls_default_path()' - Get the default credential store path.
 */

static const char *			/* O - Path or NULL on error */
http_gnutls_default_path(char   *buffer,/* I - Path buffer */
                         size_t bufsize)/* I - Size of path buffer */
{
  const char *home = getenv("HOME");	/* HOME environment variable */
//  _cups_globals_t	*cg = _cupsGlobals();
					/* Pointer to library globals */


//  if (cg->home)
  if (getuid() && home)
  {
//    snprintf(buffer, bufsize, "%s/.cups", cg->home);
    snprintf(buffer, bufsize, "%s/.htmldoc", home);
    if (access(buffer, 0))
    {
      DEBUG_printf(("1http_gnutls_default_path: Making directory \"%s\".", buffer));
      if (mkdir(buffer, 0700))
      {
        DEBUG_printf(("1http_gnutls_default_path: Failed to make directory: %s", strerror(errno)));
        return (NULL);
      }
    }

//    snprintf(buffer, bufsize, "%s/.cups/ssl", cg->home);
    snprintf(buffer, bufsize, "%s/.htmldoc/ssl", home);
    if (access(buffer, 0))
    {
      DEBUG_printf(("1http_gnutls_default_path: Making directory \"%s\".", buffer));
      if (mkdir(buffer, 0700))
      {
        DEBUG_printf(("1http_gnutls_default_path: Failed to make directory: %s", strerror(errno)));
        return (NULL);
      }
    }
  }
  else
//    strlcpy(buffer, CUPS_SERVERROOT "/ssl", bufsize);
    strlcpy(buffer, "/etc/htmldoc/ssl", bufsize);

  DEBUG_printf(("1http_gnutls_default_path: Using default path \"%s\".", buffer));

  return (buffer);
}


#if 0
/*
 * 'http_gnutls_load_crl()' - Load the certificate revocation list, if any.
 */

static void
http_gnutls_load_crl(void)
{
  _cupsMutexLock(&tls_mutex);

  if (!gnutls_x509_crl_init(&tls_crl))
  {
    cups_file_t		*fp;		/* CRL file */
    char		filename[1024],	/* site.crl */
			line[256];	/* Base64-encoded line */
    unsigned char	*data = NULL;	/* Buffer for cert data */
    size_t		alloc_data = 0,	/* Bytes allocated */
			num_data = 0;	/* Bytes used */
    int			decoded;	/* Bytes decoded */
    gnutls_datum_t	datum;		/* Data record */


    http_gnutls_make_path(filename, sizeof(filename), CUPS_SERVERROOT, "site", "crl");

    if ((fp = cupsFileOpen(filename, "r")) != NULL)
    {
      while (cupsFileGets(fp, line, sizeof(line)))
      {
	if (!strcmp(line, "-----BEGIN X509 CRL-----"))
	{
	  if (num_data)
	  {
	   /*
	    * Missing END X509 CRL...
	    */

	    break;
	  }
	}
	else if (!strcmp(line, "-----END X509 CRL-----"))
	{
	  if (!num_data)
	  {
	   /*
	    * Missing data...
	    */

	    break;
	  }

          datum.data = data;
	  datum.size = num_data;

	  gnutls_x509_crl_import(tls_crl, &datum, GNUTLS_X509_FMT_PEM);

	  num_data = 0;
	}
	else
	{
	  if (alloc_data == 0)
	  {
	    data       = malloc(2048);
	    alloc_data = 2048;

	    if (!data)
	      break;
	  }
	  else if ((num_data + strlen(line)) >= alloc_data)
	  {
	    unsigned char *tdata = realloc(data, alloc_data + 1024);
					    /* Expanded buffer */

	    if (!tdata)
	      break;

	    data       = tdata;
	    alloc_data += 1024;
	  }

	  decoded = alloc_data - num_data;
	  httpDecode64_2((char *)data + num_data, &decoded, line);
	  num_data += (size_t)decoded;
	}
      }

      cupsFileClose(fp);

      if (data)
	free(data);
    }
  }

  _cupsMutexUnlock(&tls_mutex);
}
#endif // 0


/*
 * 'http_gnutls_make_path()' - Format a filename for a certificate or key file.
 */

static const char *			/* O - Filename */
http_gnutls_make_path(
    char       *buffer,			/* I - Filename buffer */
    size_t     bufsize,			/* I - Size of buffer */
    const char *dirname,		/* I - Directory */
    const char *filename,		/* I - Filename (usually hostname) */
    const char *ext)			/* I - Extension */
{
  char	*bufptr,			/* Pointer into buffer */
	*bufend = buffer + bufsize - 1;	/* End of buffer */


  snprintf(buffer, bufsize, "%s/", dirname);
  bufptr = buffer + strlen(buffer);

  while (*filename && bufptr < bufend)
  {
    if (_cups_isalnum(*filename) || *filename == '-' || *filename == '.')
      *bufptr++ = *filename;
    else
      *bufptr++ = '_';

    filename ++;
  }

  if (bufptr < bufend)
    *bufptr++ = '.';

  strlcpy(bufptr, ext, (size_t)(bufend - bufptr + 1));

  return (buffer);
}


/*
 * 'http_gnutls_read()' - Read function for the GNU TLS library.
 */

static ssize_t				/* O - Number of bytes read or -1 on error */
http_gnutls_read(
    gnutls_transport_ptr_t ptr,		/* I - Connection to server */
    void                   *data,	/* I - Buffer */
    size_t                 length)	/* I - Number of bytes to read */
{
  http_t	*http;			/* HTTP connection */
  ssize_t	bytes;			/* Bytes read */


  DEBUG_printf(("6http_gnutls_read(ptr=%p, data=%p, length=%d)", ptr, data, (int)length));

  http = (http_t *)ptr;

  if (!http->blocking || http->timeout_value > 0.0)
  {
   /*
    * Make sure we have data before we read...
    */

    while (!_httpWait(http, http->wait_value, 0))
    {
      if (http->timeout_cb && (*http->timeout_cb)(http, http->timeout_data))
	continue;

      http->error = ETIMEDOUT;
      return (-1);
    }
  }

  bytes = recv(http->fd, data, length, 0);
  DEBUG_printf(("6http_gnutls_read: bytes=%d", (int)bytes));
  return (bytes);
}


/*
 * 'http_gnutls_write()' - Write function for the GNU TLS library.
 */

static ssize_t				/* O - Number of bytes written or -1 on error */
http_gnutls_write(
    gnutls_transport_ptr_t ptr,		/* I - Connection to server */
    const void             *data,	/* I - Data buffer */
    size_t                 length)	/* I - Number of bytes to write */
{
  ssize_t bytes;			/* Bytes written */


  DEBUG_printf(("6http_gnutls_write(ptr=%p, data=%p, length=%d)", ptr, data,
                (int)length));
  bytes = send(((http_t *)ptr)->fd, data, length, 0);
  DEBUG_printf(("http_gnutls_write: bytes=%d", (int)bytes));

  return (bytes);
}


/*
 * '_httpTLSInitialize()' - Initialize the TLS stack.
 */

void
_httpTLSInitialize(void)
{
 /*
  * Initialize GNU TLS...
  */

  gnutls_global_init();
}


/*
 * '_httpTLSPending()' - Return the number of pending TLS-encrypted bytes.
 */

size_t					/* O - Bytes available */
_httpTLSPending(http_t *http)		/* I - HTTP connection */
{
  return (gnutls_record_check_pending(http->tls));
}


/*
 * '_httpTLSRead()' - Read from a SSL/TLS connection.
 */

int					/* O - Bytes read */
_httpTLSRead(http_t *http,		/* I - Connection to server */
	     char   *buf,		/* I - Buffer to store data */
	     int    len)		/* I - Length of buffer */
{
  ssize_t	result;			/* Return value */


  result = gnutls_record_recv(http->tls, buf, (size_t)len);

  if (result < 0 && !errno)
  {
   /*
    * Convert GNU TLS error to errno value...
    */

    switch (result)
    {
      case GNUTLS_E_INTERRUPTED :
	  errno = EINTR;
	  break;

      case GNUTLS_E_AGAIN :
          errno = EAGAIN;
          break;

      default :
          errno = EPIPE;
          break;
    }

    result = -1;
  }

  return ((int)result);
}


/*
 * '_httpTLSSetOptions()' - Set TLS protocol and cipher suite options.
 */

void
_httpTLSSetOptions(int options,		/* I - Options */
                   int min_version,	/* I - Minimum TLS version */
                   int max_version)	/* I - Maximum TLS version */
{
  if (!(options & _HTTP_TLS_SET_DEFAULT) || tls_options < 0)
  {
    tls_options     = options;
    tls_min_version = min_version;
    tls_max_version = max_version;
  }
}


/*
 * '_httpTLSStart()' - Set up SSL/TLS support on a connection.
 */

int					/* O - 0 on success, -1 on failure */
_httpTLSStart(http_t *http)		/* I - Connection to server */
{
  char			hostname[256],	/* Hostname */
			*hostptr;	/* Pointer into hostname */
  int			status;		/* Status of handshake */
  gnutls_certificate_credentials_t *credentials;
					/* TLS credentials */
  char			priority_string[2048];
					/* Priority string */
  int			version;	/* Current version */
  double		old_timeout;	/* Old timeout value */
  http_timeout_cb_t	old_cb;		/* Old timeout callback */
  void			*old_data;	/* Old timeout data */
  static const char * const versions[] =/* SSL/TLS versions */
  {
    "VERS-SSL3.0",
    "VERS-TLS1.0",
    "VERS-TLS1.1",
    "VERS-TLS1.2",
    "VERS-TLS1.3",
    "VERS-TLS-ALL"
  };


  DEBUG_printf(("3_httpTLSStart(http=%p)", http));

  if (tls_options < 0)
  {
    DEBUG_puts("4_httpTLSStart: Setting defaults.");
//    _cupsSetDefaults();
    tls_options = _HTTP_TLS_NONE;
    DEBUG_printf(("4_httpTLSStart: tls_options=%x", tls_options));
  }

  if (http->mode == _HTTP_MODE_SERVER && !tls_keypath)
  {
    DEBUG_puts("4_httpTLSStart: cupsSetServerCredentials not called.");
    http->error  = errno = EINVAL;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Server credentials not set."), 1);

    return (-1);
  }

  credentials = (gnutls_certificate_credentials_t *)
                    malloc(sizeof(gnutls_certificate_credentials_t));
  if (credentials == NULL)
  {
    DEBUG_printf(("8_httpStartTLS: Unable to allocate credentials: %s",
                  strerror(errno)));
    http->error  = errno;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetHTTPError(HTTP_STATUS_ERROR);

    return (-1);
  }

  gnutls_certificate_allocate_credentials(credentials);
  status = gnutls_init(&http->tls, http->mode == _HTTP_MODE_CLIENT ? GNUTLS_CLIENT : GNUTLS_SERVER);
  if (!status)
    status = gnutls_set_default_priority(http->tls);

  if (status)
  {
    http->error  = EIO;
    http->status = HTTP_STATUS_ERROR;

    DEBUG_printf(("4_httpTLSStart: Unable to initialize common TLS parameters: %s", gnutls_strerror(status)));
    _cupsSetError(IPP_STATUS_ERROR_CUPS_PKI, gnutls_strerror(status), 0);

    gnutls_deinit(http->tls);
    gnutls_certificate_free_credentials(*credentials);
    free(credentials);
    http->tls = NULL;

    return (-1);
  }

  if (http->mode == _HTTP_MODE_CLIENT)
  {
   /*
    * Client: get the hostname to use for TLS...
    */

    if (httpAddrLocalhost(http->hostaddr))
    {
      strlcpy(hostname, "localhost", sizeof(hostname));
    }
    else
    {
     /*
      * Otherwise make sure the hostname we have does not end in a trailing dot.
      */

      strlcpy(hostname, http->hostname, sizeof(hostname));
      if ((hostptr = hostname + strlen(hostname) - 1) >= hostname &&
	  *hostptr == '.')
	*hostptr = '\0';
    }

    status = gnutls_server_name_set(http->tls, GNUTLS_NAME_DNS, hostname, strlen(hostname));
  }
  else
  {
   /*
    * Server: not supported...
    */

    http->error  = errno = EINVAL;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Unable to create server credentials."), 1);
    return (-1);
  }

  if (!status)
    status = gnutls_credentials_set(http->tls, GNUTLS_CRD_CERTIFICATE, *credentials);

  if (status)
  {
    http->error  = EIO;
    http->status = HTTP_STATUS_ERROR;

    DEBUG_printf(("4_httpTLSStart: Unable to complete client/server setup: %s", gnutls_strerror(status)));
    _cupsSetError(IPP_STATUS_ERROR_CUPS_PKI, gnutls_strerror(status), 0);

    gnutls_deinit(http->tls);
    gnutls_certificate_free_credentials(*credentials);
    free(credentials);
    http->tls = NULL;

    return (-1);
  }

  strlcpy(priority_string, "NORMAL", sizeof(priority_string));

  if (tls_max_version < _HTTP_TLS_MAX)
  {
   /*
    * Require specific TLS versions...
    */

    strlcat(priority_string, ":-VERS-TLS-ALL", sizeof(priority_string));
    for (version = tls_min_version; version <= tls_max_version; version ++)
    {
      strlcat(priority_string, ":+", sizeof(priority_string));
      strlcat(priority_string, versions[version], sizeof(priority_string));
    }
  }
  else if (tls_min_version == _HTTP_TLS_SSL3)
  {
   /*
    * Allow all versions of TLS and SSL/3.0...
    */

    strlcat(priority_string, ":+VERS-TLS-ALL:+VERS-SSL3.0", sizeof(priority_string));
  }
  else
  {
   /*
    * Require a minimum version...
    */

    strlcat(priority_string, ":+VERS-TLS-ALL", sizeof(priority_string));
    for (version = 0; version < tls_min_version; version ++)
    {
      strlcat(priority_string, ":-", sizeof(priority_string));
      strlcat(priority_string, versions[version], sizeof(priority_string));
    }
  }

  if (tls_options & _HTTP_TLS_ALLOW_RC4)
    strlcat(priority_string, ":+ARCFOUR-128", sizeof(priority_string));
  else
    strlcat(priority_string, ":!ARCFOUR-128", sizeof(priority_string));

  strlcat(priority_string, ":!ANON-DH", sizeof(priority_string));

  if (tls_options & _HTTP_TLS_DENY_CBC)
    strlcat(priority_string, ":!AES-128-CBC:!AES-256-CBC:!CAMELLIA-128-CBC:!CAMELLIA-256-CBC:!3DES-CBC", sizeof(priority_string));

#ifdef HAVE_GNUTLS_PRIORITY_SET_DIRECT
  gnutls_priority_set_direct(http->tls, priority_string, NULL);

#else
  gnutls_priority_t priority;		/* Priority */

  gnutls_priority_init(&priority, priority_string, NULL);
  gnutls_priority_set(http->tls, priority);
  gnutls_priority_deinit(priority);
#endif /* HAVE_GNUTLS_PRIORITY_SET_DIRECT */

  gnutls_transport_set_ptr(http->tls, (gnutls_transport_ptr_t)http);
  gnutls_transport_set_pull_function(http->tls, http_gnutls_read);
#ifdef HAVE_GNUTLS_TRANSPORT_SET_PULL_TIMEOUT_FUNCTION
  gnutls_transport_set_pull_timeout_function(http->tls, (gnutls_pull_timeout_func)httpWait);
#endif /* HAVE_GNUTLS_TRANSPORT_SET_PULL_TIMEOUT_FUNCTION */
  gnutls_transport_set_push_function(http->tls, http_gnutls_write);

 /*
  * Enforce a minimum timeout of 10 seconds for the TLS handshake...
  */

  old_timeout  = http->timeout_value;
  old_cb       = http->timeout_cb;
  old_data     = http->timeout_data;

  if (!old_cb || old_timeout < 10.0)
  {
    DEBUG_puts("4_httpTLSStart: Setting timeout to 10 seconds.");
    httpSetTimeout(http, 10.0, NULL, NULL);
  }

 /*
  * Do the TLS handshake...
  */

  while ((status = gnutls_handshake(http->tls)) != GNUTLS_E_SUCCESS)
  {
    DEBUG_printf(("5_httpStartTLS: gnutls_handshake returned %d (%s)",
                  status, gnutls_strerror(status)));

    if (gnutls_error_is_fatal(status))
    {
      http->error  = EIO;
      http->status = HTTP_STATUS_ERROR;

      _cupsSetError(IPP_STATUS_ERROR_CUPS_PKI, gnutls_strerror(status), 0);

      gnutls_deinit(http->tls);
      gnutls_certificate_free_credentials(*credentials);
      free(credentials);
      http->tls = NULL;

      httpSetTimeout(http, old_timeout, old_cb, old_data);

      return (-1);
    }
  }

 /*
  * Restore the previous timeout settings...
  */

  httpSetTimeout(http, old_timeout, old_cb, old_data);

  http->tls_credentials = credentials;

  return (0);
}


/*
 * '_httpTLSStop()' - Shut down SSL/TLS on a connection.
 */

void
_httpTLSStop(http_t *http)		/* I - Connection to server */
{
  int	error;				/* Error code */


  error = gnutls_bye(http->tls, GNUTLS_SHUT_WR);
  if (error != GNUTLS_E_SUCCESS)
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, gnutls_strerror(errno), 0);

  gnutls_deinit(http->tls);
  http->tls = NULL;

  if (http->tls_credentials)
  {
    gnutls_certificate_free_credentials(*(http->tls_credentials));
    free(http->tls_credentials);
    http->tls_credentials = NULL;
  }
}


/*
 * '_httpTLSWrite()' - Write to a SSL/TLS connection.
 */

int					/* O - Bytes written */
_httpTLSWrite(http_t     *http,		/* I - Connection to server */
	      const char *buf,		/* I - Buffer holding data */
	      int        len)		/* I - Length of buffer */
{
  ssize_t	result;			/* Return value */


  DEBUG_printf(("2http_write_ssl(http=%p, buf=%p, len=%d)", http, buf, len));

  result = gnutls_record_send(http->tls, buf, (size_t)len);

  if (result < 0 && !errno)
  {
   /*
    * Convert GNU TLS error to errno value...
    */

    switch (result)
    {
      case GNUTLS_E_INTERRUPTED :
	  errno = EINTR;
	  break;

      case GNUTLS_E_AGAIN :
          errno = EAGAIN;
          break;

      default :
          errno = EPIPE;
          break;
    }

    result = -1;
  }

  DEBUG_printf(("3http_write_ssl: Returning %d.", (int)result));

  return ((int)result);
}
