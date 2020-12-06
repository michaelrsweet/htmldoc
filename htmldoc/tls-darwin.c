/*
 * TLS support code for HTMLDOC on macOS.
 *
 * Copyright © 2020 by Michael R Sweet
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

#include "tls-darwin.h"

/*
 * Constants, very secure stuff...
 */

#define _CUPS_CDSA_PASSWORD	"42"	/* CUPS keychain password */
#define _CUPS_CDSA_PASSLEN	2	/* Length of keychain password */


/*
 * Local globals...
 */

static int		tls_auto_create = 0;
					/* Auto-create self-signed certs? */
static char		*tls_common_name = NULL;
					/* Default common name */
#if TARGET_OS_OSX
static int		tls_cups_keychain = 0;
					/* Opened the CUPS keychain? */
static SecKeychainRef	tls_keychain = NULL;
					/* Server cert keychain */
#else
static SecIdentityRef	tls_selfsigned = NULL;
					/* Temporary self-signed cert */
#endif /* TARGET_OS_OSX */
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

static SecCertificateRef http_cdsa_create_credential(http_credential_t *credential);
#if TARGET_OS_OSX
static const char	*http_cdsa_default_path(char *buffer, size_t bufsize);
static SecKeychainRef	http_cdsa_open_keychain(const char *path, char *filename, size_t filesize);
static SecKeychainRef	http_cdsa_open_system_keychain(void);
#endif /* TARGET_OS_OSX */
static OSStatus		http_cdsa_read(SSLConnectionRef connection, void *data, size_t *dataLength);
static int		http_cdsa_set_credentials(http_t *http);
static OSStatus		http_cdsa_write(SSLConnectionRef connection, const void *data, size_t *dataLength);


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
  OSStatus		error;		/* Error code */
  SecTrustRef		peerTrust;	/* Peer trust reference */
  CFIndex		count;		/* Number of credentials */
  SecCertificateRef	secCert;	/* Certificate reference */
  CFDataRef		data;		/* Certificate data */
  int			i;		/* Looping var */


  DEBUG_printf(("httpCopyCredentials(http=%p, credentials=%p)", (void *)http, (void *)credentials));

  if (credentials)
    *credentials = NULL;

  if (!http || !http->tls || !credentials)
    return (-1);

  if (!(error = SSLCopyPeerTrust(http->tls, &peerTrust)) && peerTrust)
  {
    DEBUG_printf(("2httpCopyCredentials: Peer provided %d certificates.", (int)SecTrustGetCertificateCount(peerTrust)));

    if ((*credentials = cupsArrayNew(NULL, NULL)) != NULL)
    {
      count = SecTrustGetCertificateCount(peerTrust);

      for (i = 0; i < count; i ++)
      {
	secCert = SecTrustGetCertificateAtIndex(peerTrust, i);

#ifdef DEBUG
        CFStringRef cf_name = SecCertificateCopySubjectSummary(secCert);
	char name[1024];
	if (cf_name)
	  CFStringGetCString(cf_name, name, sizeof(name), kCFStringEncodingUTF8);
	else
	  strlcpy(name, "unknown", sizeof(name));

	DEBUG_printf(("2httpCopyCredentials: Certificate %d name is \"%s\".", i, name));
#endif /* DEBUG */

	if ((data = SecCertificateCopyData(secCert)) != NULL)
	{
	  DEBUG_printf(("2httpCopyCredentials: Adding %d byte certificate blob.", (int)CFDataGetLength(data)));

	  httpAddCredential(*credentials, CFDataGetBytePtr(data), (size_t)CFDataGetLength(data));
	  CFRelease(data);
	}
      }
    }

    CFRelease(peerTrust);
  }

  return (error);
}


/*
 * '_httpCreateCredentials()' - Create credentials in the internal format.
 */

http_tls_credentials_t			/* O - Internal credentials */
_httpCreateCredentials(
    cups_array_t *credentials)		/* I - Array of credentials */
{
  CFMutableArrayRef	peerCerts;	/* Peer credentials reference */
  SecCertificateRef	secCert;	/* Certificate reference */
  http_credential_t	*credential;	/* Credential data */


  if (!credentials)
    return (NULL);

  if ((peerCerts = CFArrayCreateMutable(kCFAllocatorDefault,
				        cupsArrayCount(credentials),
				        &kCFTypeArrayCallBacks)) == NULL)
    return (NULL);

  for (credential = (http_credential_t *)cupsArrayFirst(credentials);
       credential;
       credential = (http_credential_t *)cupsArrayNext(credentials))
  {
    if ((secCert = http_cdsa_create_credential(credential)) != NULL)
    {
      CFArrayAppendValue(peerCerts, secCert);
      CFRelease(secCert);
    }
  }

  return (peerCerts);
}


/*
 * 'httpCredentialsAreValidForName()' - Return whether the credentials are valid for the given name.
 *
 * @since CUPS 2.0/macOS 10.10@
 */

int					/* O - 1 if valid, 0 otherwise */
httpCredentialsAreValidForName(
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Name to check */
{
  SecCertificateRef	secCert;	/* Certificate reference */
  CFStringRef		cfcert_name = NULL;
					/* Certificate's common name (CF string) */
  char			cert_name[256];	/* Certificate's common name (C string) */
  int			valid = 1;	/* Valid name? */


  if ((secCert = http_cdsa_create_credential((http_credential_t *)cupsArrayFirst(credentials))) == NULL)
    return (0);

 /*
  * Compare the common names...
  */

  if ((cfcert_name = SecCertificateCopySubjectSummary(secCert)) == NULL)
  {
   /*
    * Can't get common name, cannot be valid...
    */

    valid = 0;
  }
  else if (CFStringGetCString(cfcert_name, cert_name, sizeof(cert_name), kCFStringEncodingUTF8) &&
           _cups_strcasecmp(common_name, cert_name))
  {
   /*
    * Not an exact match for the common name, check for wildcard certs...
    */

    const char	*domain = strchr(common_name, '.');
					/* Domain in common name */

    if (strncmp(cert_name, "*.", 2) || !domain || _cups_strcasecmp(domain, cert_name + 1))
    {
     /*
      * Not a wildcard match.
      */

      /* TODO: Check subject alternate names */
      valid = 0;
    }
  }

  if (cfcert_name)
    CFRelease(cfcert_name);

  CFRelease(secCert);

  return (valid);
}


/*
 * 'httpCredentialsGetTrust()' - Return the trust of credentials.
 *
 * @since CUPS 2.0/macOS 10.10@
 */

http_trust_t				/* O - Level of trust */
httpCredentialsGetTrust(
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Common name for trust lookup */
{
  SecCertificateRef	secCert;	/* Certificate reference */
  http_trust_t		trust = HTTP_TRUST_OK;
					/* Trusted? */
  cups_array_t		*tcreds = NULL;	/* Trusted credentials */
//  _cups_globals_t	*cg = _cupsGlobals();
					/* Per-thread globals */


  if (!common_name)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("No common name specified."), 1);
    return (HTTP_TRUST_UNKNOWN);
  }

  if ((secCert = http_cdsa_create_credential((http_credential_t *)cupsArrayFirst(credentials))) == NULL)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Unable to create credentials from array."), 1);
    return (HTTP_TRUST_UNKNOWN);
  }

#if 0
  if (cg->any_root < 0)
    _cupsSetDefaults();
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
  else if (/*cg->validate_certs &&*/ !httpCredentialsAreValidForName(credentials, common_name))
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("No stored credentials, not valid for name."), 1);
    trust = HTTP_TRUST_INVALID;
  }
#if 0
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

  if (trust == HTTP_TRUST_OK && /*!cg->expired_certs &&*/ !SecCertificateIsValid(secCert, CFAbsoluteTimeGetCurrent()))
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Credentials have expired."), 1);
    trust = HTTP_TRUST_EXPIRED;
  }

#if 0
  if (trust == HTTP_TRUST_OK && !cg->any_root && cupsArrayCount(credentials) == 1)
  {
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Self-signed credentials are blocked."), 1);
    trust = HTTP_TRUST_INVALID;
  }
#endif // 0

  CFRelease(secCert);

  return (trust);
}


/*
 * 'httpCredentialsGetExpiration()' - Return the expiration date of the credentials.
 *
 * @since CUPS 2.0/macOS 10.10@
 */

time_t					/* O - Expiration date of credentials */
httpCredentialsGetExpiration(
    cups_array_t *credentials)		/* I - Credentials */
{
  SecCertificateRef	secCert;	/* Certificate reference */
  time_t		expiration;	/* Expiration date */


  if ((secCert = http_cdsa_create_credential((http_credential_t *)cupsArrayFirst(credentials))) == NULL)
    return (0);

  expiration = (time_t)(SecCertificateNotValidAfter(secCert) + kCFAbsoluteTimeIntervalSince1970);

  CFRelease(secCert);

  return (expiration);
}


/*
 * 'httpCredentialsString()' - Return a string representing the credentials.
 *
 * @since CUPS 2.0/macOS 10.10@
 */

size_t					/* O - Total size of credentials string */
httpCredentialsString(
    cups_array_t *credentials,		/* I - Credentials */
    char         *buffer,		/* I - Buffer or @code NULL@ */
    size_t       bufsize)		/* I - Size of buffer */
{
  http_credential_t	*first;		/* First certificate */
  SecCertificateRef	secCert;	/* Certificate reference */


  DEBUG_printf(("httpCredentialsString(credentials=%p, buffer=%p, bufsize=" CUPS_LLFMT ")", (void *)credentials, (void *)buffer, CUPS_LLCAST bufsize));

  if (!buffer)
    return (0);

  if (buffer && bufsize > 0)
    *buffer = '\0';

  if ((first = (http_credential_t *)cupsArrayFirst(credentials)) != NULL &&
      (secCert = http_cdsa_create_credential(first)) != NULL)
  {
   /*
    * Copy certificate (string) values from the SecCertificateRef and produce
    * a one-line summary.  The API for accessing certificate values like the
    * issuer name is, um, "interesting"...
    */

#  if TARGET_OS_OSX
    CFDictionaryRef	cf_dict;	/* Dictionary for certificate */
#  endif /* TARGET_OS_OSX */
    CFStringRef		cf_string;	/* CF string */
    char		commonName[256],/* Common name associated with cert */
			issuer[256],	/* Issuer name */
			sigalg[256];	/* Signature algorithm */
    time_t		expiration;	/* Expiration date of cert */
    _cups_md5_state_t	md5_state;	/* MD5 state */
    unsigned char	md5_digest[16];	/* MD5 result */

    if (SecCertificateCopyCommonName(secCert, &cf_string) == noErr)
    {
      CFStringGetCString(cf_string, commonName, (CFIndex)sizeof(commonName), kCFStringEncodingUTF8);
      CFRelease(cf_string);
    }
    else
    {
      strlcpy(commonName, "unknown", sizeof(commonName));
    }

    strlcpy(issuer, "unknown", sizeof(issuer));
    strlcpy(sigalg, "UnknownSignature", sizeof(sigalg));

#  if TARGET_OS_OSX
    if ((cf_dict = SecCertificateCopyValues(secCert, NULL, NULL)) != NULL)
    {
      CFDictionaryRef cf_issuer = CFDictionaryGetValue(cf_dict, kSecOIDX509V1IssuerName);
      CFDictionaryRef cf_sigalg = CFDictionaryGetValue(cf_dict, kSecOIDX509V1SignatureAlgorithm);

      if (cf_issuer)
      {
        CFArrayRef cf_values = CFDictionaryGetValue(cf_issuer, kSecPropertyKeyValue);
        CFIndex i, count = CFArrayGetCount(cf_values);
        CFDictionaryRef cf_value;

        for (i = 0; i < count; i ++)
        {
          cf_value = CFArrayGetValueAtIndex(cf_values, i);

          if (!CFStringCompare(CFDictionaryGetValue(cf_value, kSecPropertyKeyLabel), kSecOIDOrganizationName, kCFCompareCaseInsensitive))
            CFStringGetCString(CFDictionaryGetValue(cf_value, kSecPropertyKeyValue), issuer, (CFIndex)sizeof(issuer), kCFStringEncodingUTF8);
        }
      }

      if (cf_sigalg)
      {
        CFArrayRef cf_values = CFDictionaryGetValue(cf_sigalg, kSecPropertyKeyValue);
        CFIndex i, count = CFArrayGetCount(cf_values);
        CFDictionaryRef cf_value;

        for (i = 0; i < count; i ++)
        {
          cf_value = CFArrayGetValueAtIndex(cf_values, i);

          if (!CFStringCompare(CFDictionaryGetValue(cf_value, kSecPropertyKeyLabel), CFSTR("Algorithm"), kCFCompareCaseInsensitive))
          {
            CFStringRef cf_algorithm = CFDictionaryGetValue(cf_value, kSecPropertyKeyValue);

            if (!CFStringCompare(cf_algorithm, CFSTR("1.2.840.113549.1.1.5"), kCFCompareCaseInsensitive))
              strlcpy(sigalg, "SHA1WithRSAEncryption", sizeof(sigalg));
	    else if (!CFStringCompare(cf_algorithm, CFSTR("1.2.840.113549.1.1.11"), kCFCompareCaseInsensitive))
              strlcpy(sigalg, "SHA256WithRSAEncryption", sizeof(sigalg));
	    else if (!CFStringCompare(cf_algorithm, CFSTR("1.2.840.113549.1.1.4"), kCFCompareCaseInsensitive))
              strlcpy(sigalg, "MD5WithRSAEncryption", sizeof(sigalg));
	  }
        }
      }

      CFRelease(cf_dict);
    }
#  endif /* TARGET_OS_OSX */

    expiration = (time_t)(SecCertificateNotValidAfter(secCert) + kCFAbsoluteTimeIntervalSince1970);

    _cupsMD5Init(&md5_state);
    _cupsMD5Append(&md5_state, first->data, (int)first->datalen);
    _cupsMD5Finish(&md5_state, md5_digest);

    snprintf(buffer, bufsize, "%s (issued by %s) / %s / %s / %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", commonName, issuer, httpGetDateString(expiration), sigalg, md5_digest[0], md5_digest[1], md5_digest[2], md5_digest[3], md5_digest[4], md5_digest[5], md5_digest[6], md5_digest[7], md5_digest[8], md5_digest[9], md5_digest[10], md5_digest[11], md5_digest[12], md5_digest[13], md5_digest[14], md5_digest[15]);

    CFRelease(secCert);
  }

  DEBUG_printf(("1httpCredentialsString: Returning \"%s\".", buffer));

  return (strlen(buffer));
}


/*
 * '_httpFreeCredentials()' - Free internal credentials.
 */

void
_httpFreeCredentials(
    http_tls_credentials_t credentials)	/* I - Internal credentials */
{
  if (!credentials)
    return;

  CFRelease(credentials);
}


/*
 * 'httpLoadCredentials()' - Load X.509 credentials from a keychain file.
 *
 * @since CUPS 2.0/OS 10.10@
 */

int					/* O - 0 on success, -1 on error */
httpLoadCredentials(
    const char   *path,			/* I  - Keychain path or @code NULL@ for default */
    cups_array_t **credentials,		/* IO - Credentials */
    const char   *common_name)		/* I  - Common name for credentials */
{
  OSStatus		err;		/* Error info */
#if TARGET_OS_OSX
  char			filename[1024];	/* Filename for keychain */
  SecKeychainRef	keychain = NULL,/* Keychain reference */
			syschain = NULL;/* System keychain */
  CFArrayRef		list;		/* Keychain list */
#endif /* TARGET_OS_OSX */
  SecCertificateRef	cert = NULL;	/* Certificate */
  CFDataRef		data;		/* Certificate data */
  SecPolicyRef		policy = NULL;	/* Policy ref */
  CFStringRef		cfcommon_name = NULL;
					/* Server name */
  CFMutableDictionaryRef query = NULL;	/* Query qualifiers */


  DEBUG_printf(("httpLoadCredentials(path=\"%s\", credentials=%p, common_name=\"%s\")", path, (void *)credentials, common_name));

  if (!credentials)
    return (-1);

  *credentials = NULL;

#if TARGET_OS_OSX
  keychain = http_cdsa_open_keychain(path, filename, sizeof(filename));

  if (!keychain)
    goto cleanup;

  syschain = http_cdsa_open_system_keychain();

#else
  if (path)
    return (-1);
#endif /* TARGET_OS_OSX */

  cfcommon_name = CFStringCreateWithCString(kCFAllocatorDefault, common_name, kCFStringEncodingUTF8);

  policy = SecPolicyCreateSSL(1, cfcommon_name);

  if (cfcommon_name)
    CFRelease(cfcommon_name);

  if (!policy)
    goto cleanup;

  if (!(query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks)))
    goto cleanup;

  CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
  CFDictionaryAddValue(query, kSecMatchPolicy, policy);
  CFDictionaryAddValue(query, kSecReturnRef, kCFBooleanTrue);
  CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);

#if TARGET_OS_OSX
  if (syschain)
  {
    const void *values[2] = { syschain, keychain };

    list = CFArrayCreate(kCFAllocatorDefault, (const void **)values, 2, &kCFTypeArrayCallBacks);
  }
  else
    list = CFArrayCreate(kCFAllocatorDefault, (const void **)&keychain, 1, &kCFTypeArrayCallBacks);
  CFDictionaryAddValue(query, kSecMatchSearchList, list);
  CFRelease(list);
#endif /* TARGET_OS_OSX */

  err = SecItemCopyMatching(query, (CFTypeRef *)&cert);

  if (err)
    goto cleanup;

  if (CFGetTypeID(cert) != SecCertificateGetTypeID())
    goto cleanup;

  if ((data = SecCertificateCopyData(cert)) != NULL)
  {
    DEBUG_printf(("1httpLoadCredentials: Adding %d byte certificate blob.", (int)CFDataGetLength(data)));

    *credentials = cupsArrayNew(NULL, NULL);
    httpAddCredential(*credentials, CFDataGetBytePtr(data), (size_t)CFDataGetLength(data));
    CFRelease(data);
  }

  cleanup :

#if TARGET_OS_OSX
  if (keychain)
    CFRelease(keychain);

  if (syschain)
    CFRelease(syschain);
#endif /* TARGET_OS_OSX */
  if (cert)
    CFRelease(cert);
  if (policy)
    CFRelease(policy);
  if (query)
    CFRelease(query);

  DEBUG_printf(("1httpLoadCredentials: Returning %d.", *credentials ? 0 : -1));

  return (*credentials ? 0 : -1);
}


/*
 * 'httpSaveCredentials()' - Save X.509 credentials to a keychain file.
 *
 * @since CUPS 2.0/OS 10.10@
 */

int					/* O - -1 on error, 0 on success */
httpSaveCredentials(
    const char   *path,			/* I - Keychain path or @code NULL@ for default */
    cups_array_t *credentials,		/* I - Credentials */
    const char   *common_name)		/* I - Common name for credentials */
{
  int			ret = -1;	/* Return value */
  OSStatus		err;		/* Error info */
#if TARGET_OS_OSX
  char			filename[1024];	/* Filename for keychain */
  SecKeychainRef	keychain = NULL;/* Keychain reference */
  CFArrayRef		list;		/* Keychain list */
#endif /* TARGET_OS_OSX */
  SecCertificateRef	cert = NULL;	/* Certificate */
  CFMutableDictionaryRef attrs = NULL;	/* Attributes for add */


  DEBUG_printf(("httpSaveCredentials(path=\"%s\", credentials=%p, common_name=\"%s\")", path, (void *)credentials, common_name));
  if (!credentials)
    goto cleanup;

  if (!httpCredentialsAreValidForName(credentials, common_name))
  {
    DEBUG_puts("1httpSaveCredentials: Common name does not match.");
    return (-1);
  }

  if ((cert = http_cdsa_create_credential((http_credential_t *)cupsArrayFirst(credentials))) == NULL)
  {
    DEBUG_puts("1httpSaveCredentials: Unable to create certificate.");
    goto cleanup;
  }

#if TARGET_OS_OSX
  keychain = http_cdsa_open_keychain(path, filename, sizeof(filename));

  if (!keychain)
    goto cleanup;

#else
  if (path)
    return (-1);
#endif /* TARGET_OS_OSX */

  if ((attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks)) == NULL)
  {
    DEBUG_puts("1httpSaveCredentials: Unable to create dictionary.");
    goto cleanup;
  }

  CFDictionaryAddValue(attrs, kSecClass, kSecClassCertificate);
  CFDictionaryAddValue(attrs, kSecValueRef, cert);

#if TARGET_OS_OSX
  if ((list = CFArrayCreate(kCFAllocatorDefault, (const void **)&keychain, 1, &kCFTypeArrayCallBacks)) == NULL)
  {
    DEBUG_puts("1httpSaveCredentials: Unable to create list of keychains.");
    goto cleanup;
  }
  CFDictionaryAddValue(attrs, kSecMatchSearchList, list);
  CFRelease(list);
#endif /* TARGET_OS_OSX */

  /* Note: SecItemAdd consumes "attrs"... */
  err = SecItemAdd(attrs, NULL);
  DEBUG_printf(("1httpSaveCredentials: SecItemAdd returned %d.", (int)err));

  cleanup :

#if TARGET_OS_OSX
  if (keychain)
    CFRelease(keychain);
#endif /* TARGET_OS_OSX */
  if (cert)
    CFRelease(cert);

  DEBUG_printf(("1httpSaveCredentials: Returning %d.", ret));

  return (ret);
}


/*
 * '_httpTLSInitialize()' - Initialize the TLS stack.
 */

void
_httpTLSInitialize(void)
{
 /*
  * Nothing to do...
  */
}


/*
 * '_httpTLSPending()' - Return the number of pending TLS-encrypted bytes.
 */

size_t
_httpTLSPending(http_t *http)		/* I - HTTP connection */
{
  size_t bytes;				/* Bytes that are available */


  if (!SSLGetBufferedReadSize(http->tls, &bytes))
    return (bytes);

  return (0);
}


/*
 * '_httpTLSRead()' - Read from a SSL/TLS connection.
 */

int					/* O - Bytes read */
_httpTLSRead(http_t *http,		/* I - HTTP connection */
	      char   *buf,		/* I - Buffer to store data */
	      int    len)		/* I - Length of buffer */
{
  int		result;			/* Return value */
  OSStatus	error;			/* Error info */
  size_t	processed;		/* Number of bytes processed */


  error = SSLRead(http->tls, buf, (size_t)len, &processed);
  DEBUG_printf(("6_httpTLSRead: error=%d, processed=%d", (int)error,
                (int)processed));
  switch (error)
  {
    case 0 :
	result = (int)processed;
	break;

    case errSSLWouldBlock :
	if (processed)
	  result = (int)processed;
	else
	{
	  result = -1;
	  errno  = EINTR;
	}
	break;

    case errSSLClosedGraceful :
    default :
	if (processed)
	  result = (int)processed;
	else
	{
	  result = -1;
	  errno  = EPIPE;
	}
	break;
  }

  return (result);
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
_httpTLSStart(http_t *http)		/* I - HTTP connection */
{
  char			hostname[256],	/* Hostname */
			*hostptr;	/* Pointer into hostname */
//  _cups_globals_t	*cg = _cupsGlobals();
					/* Pointer to library globals */
  OSStatus		error;		/* Error code */
  const char		*message = NULL;/* Error message */
  char			msgbuf[1024];	/* Error message buffer */
//  cups_array_t		*credentials;	/* Credentials array */
//  cups_array_t		*names;		/* CUPS distinguished names */
//  CFArrayRef		dn_array;	/* CF distinguished names array */
//  CFIndex		count;		/* Number of credentials */
//  CFDataRef		data;		/* Certificate data */
  int			i;		/* Looping var */
//  http_credential_t	*credential;	/* Credential data */


  DEBUG_printf(("3_httpTLSStart(http=%p)", (void *)http));

  if (tls_options < 0)
  {
    DEBUG_puts("4_httpTLSStart: Setting defaults.");
//    _cupsSetDefaults();
    tls_options = _HTTP_TLS_NONE;
    DEBUG_printf(("4_httpTLSStart: tls_options=%x, tls_min_version=%d, tls_max_version=%d", tls_options, tls_min_version, tls_max_version));
  }

#if TARGET_OS_OSX
  if (http->mode == _HTTP_MODE_SERVER && !tls_keychain)
  {
    DEBUG_puts("4_httpTLSStart: cupsSetServerCredentials not called.");
    http->error  = errno = EINVAL;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Server credentials not set."), 1);

    return (-1);
  }
#endif /* TARGET_OS_OSX */

  if ((http->tls = SSLCreateContext(kCFAllocatorDefault, http->mode == _HTTP_MODE_CLIENT ? kSSLClientSide : kSSLServerSide, kSSLStreamType)) == NULL)
  {
    DEBUG_puts("4_httpTLSStart: SSLCreateContext failed.");
    http->error  = errno = ENOMEM;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetHTTPError(HTTP_STATUS_ERROR);

    return (-1);
  }

  error = SSLSetConnection(http->tls, http);
  DEBUG_printf(("4_httpTLSStart: SSLSetConnection, error=%d", (int)error));

  if (!error)
  {
    error = SSLSetIOFuncs(http->tls, http_cdsa_read, http_cdsa_write);
    DEBUG_printf(("4_httpTLSStart: SSLSetIOFuncs, error=%d", (int)error));
  }

  if (!error)
  {
    error = SSLSetSessionOption(http->tls, kSSLSessionOptionBreakOnServerAuth,
                                true);
    DEBUG_printf(("4_httpTLSStart: SSLSetSessionOption, error=%d", (int)error));
  }

  if (!error)
  {
    static const SSLProtocol protocols[] =	/* Min/max protocol versions */
    {
      kSSLProtocol3,
      kTLSProtocol1,
      kTLSProtocol11,
      kTLSProtocol12,
      kTLSProtocol13
    };

    if (tls_min_version < _HTTP_TLS_MAX)
    {
      error = SSLSetProtocolVersionMin(http->tls, protocols[tls_min_version]);
      DEBUG_printf(("4_httpTLSStart: SSLSetProtocolVersionMin(%d), error=%d", protocols[tls_min_version], (int)error));
    }

    if (!error && tls_max_version < _HTTP_TLS_MAX)
    {
      error = SSLSetProtocolVersionMax(http->tls, protocols[tls_max_version]);
      DEBUG_printf(("4_httpTLSStart: SSLSetProtocolVersionMax(%d), error=%d", protocols[tls_max_version], (int)error));
    }
  }

  if (!error)
  {
    SSLCipherSuite	supported[100];	/* Supported cipher suites */
    size_t		num_supported;	/* Number of supported cipher suites */
    SSLCipherSuite	enabled[100];	/* Cipher suites to enable */
    size_t		num_enabled;	/* Number of cipher suites to enable */

    num_supported = sizeof(supported) / sizeof(supported[0]);
    error         = SSLGetSupportedCiphers(http->tls, supported, &num_supported);

    if (!error)
    {
      DEBUG_printf(("4_httpTLSStart: %d cipher suites supported.", (int)num_supported));

      for (i = 0, num_enabled = 0; i < (int)num_supported && num_enabled < (sizeof(enabled) / sizeof(enabled[0])); i ++)
      {
        switch (supported[i])
	{
	  /* Obviously insecure cipher suites that we never want to use */
	  case SSL_NULL_WITH_NULL_NULL :
	  case SSL_RSA_WITH_NULL_MD5 :
	  case SSL_RSA_WITH_NULL_SHA :
	  case SSL_RSA_EXPORT_WITH_RC4_40_MD5 :
	  case SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5 :
	  case SSL_RSA_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_RSA_WITH_DES_CBC_SHA :
	  case SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_DH_DSS_WITH_DES_CBC_SHA :
	  case SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_DH_RSA_WITH_DES_CBC_SHA :
	  case SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_DHE_DSS_WITH_DES_CBC_SHA :
	  case SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_DHE_RSA_WITH_DES_CBC_SHA :
	  case SSL_DH_anon_EXPORT_WITH_RC4_40_MD5 :
	  case SSL_DH_anon_WITH_RC4_128_MD5 :
	  case SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA :
	  case SSL_DH_anon_WITH_DES_CBC_SHA :
	  case SSL_DH_anon_WITH_3DES_EDE_CBC_SHA :
	  case SSL_FORTEZZA_DMS_WITH_NULL_SHA :
	  case TLS_DH_anon_WITH_AES_128_CBC_SHA :
	  case TLS_DH_anon_WITH_AES_256_CBC_SHA :
	  case TLS_ECDH_ECDSA_WITH_NULL_SHA :
	  case TLS_ECDHE_RSA_WITH_NULL_SHA :
	  case TLS_ECDH_anon_WITH_NULL_SHA :
	  case TLS_ECDH_anon_WITH_RC4_128_SHA :
	  case TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA :
	  case TLS_ECDH_anon_WITH_AES_128_CBC_SHA :
	  case TLS_ECDH_anon_WITH_AES_256_CBC_SHA :
	  case TLS_RSA_WITH_NULL_SHA256 :
	  case TLS_DH_anon_WITH_AES_128_CBC_SHA256 :
	  case TLS_DH_anon_WITH_AES_256_CBC_SHA256 :
	  case TLS_PSK_WITH_NULL_SHA :
	  case TLS_DHE_PSK_WITH_NULL_SHA :
	  case TLS_RSA_PSK_WITH_NULL_SHA :
	  case TLS_DH_anon_WITH_AES_128_GCM_SHA256 :
	  case TLS_DH_anon_WITH_AES_256_GCM_SHA384 :
	  case TLS_PSK_WITH_NULL_SHA256 :
	  case TLS_PSK_WITH_NULL_SHA384 :
	  case TLS_DHE_PSK_WITH_NULL_SHA256 :
	  case TLS_DHE_PSK_WITH_NULL_SHA384 :
	  case TLS_RSA_PSK_WITH_NULL_SHA256 :
	  case TLS_RSA_PSK_WITH_NULL_SHA384 :
	  case SSL_RSA_WITH_DES_CBC_MD5 :
	      DEBUG_printf(("4_httpTLSStart: Excluding insecure cipher suite %d", supported[i]));
	      break;

          /* RC4 cipher suites that should only be used as a last resort */
	  case SSL_RSA_WITH_RC4_128_MD5 :
	  case SSL_RSA_WITH_RC4_128_SHA :
	  case TLS_ECDH_ECDSA_WITH_RC4_128_SHA :
	  case TLS_ECDHE_ECDSA_WITH_RC4_128_SHA :
	  case TLS_ECDH_RSA_WITH_RC4_128_SHA :
	  case TLS_ECDHE_RSA_WITH_RC4_128_SHA :
	  case TLS_PSK_WITH_RC4_128_SHA :
	  case TLS_DHE_PSK_WITH_RC4_128_SHA :
	  case TLS_RSA_PSK_WITH_RC4_128_SHA :
	      if (tls_options & _HTTP_TLS_ALLOW_RC4)
	        enabled[num_enabled ++] = supported[i];
	      else
		DEBUG_printf(("4_httpTLSStart: Excluding RC4 cipher suite %d", supported[i]));
	      break;

          /* DH/DHE cipher suites that are problematic with parameters < 1024 bits */
          case TLS_DH_DSS_WITH_AES_128_CBC_SHA :
          case TLS_DH_RSA_WITH_AES_128_CBC_SHA :
          case TLS_DHE_DSS_WITH_AES_128_CBC_SHA :
          case TLS_DHE_RSA_WITH_AES_128_CBC_SHA :
          case TLS_DH_DSS_WITH_AES_256_CBC_SHA :
          case TLS_DH_RSA_WITH_AES_256_CBC_SHA :
          case TLS_DHE_DSS_WITH_AES_256_CBC_SHA :
          case TLS_DHE_RSA_WITH_AES_256_CBC_SHA :
          case TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA :
          case TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA :
          case TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA :
          case TLS_DH_DSS_WITH_AES_128_CBC_SHA256 :
          case TLS_DH_RSA_WITH_AES_128_CBC_SHA256 :
          case TLS_DHE_DSS_WITH_AES_128_CBC_SHA256 :
          case TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 :
          case TLS_DH_DSS_WITH_AES_256_CBC_SHA256 :
          case TLS_DH_RSA_WITH_AES_256_CBC_SHA256 :
          case TLS_DHE_DSS_WITH_AES_256_CBC_SHA256 :
          case TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 :
          case TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA :
          case TLS_DHE_PSK_WITH_AES_128_CBC_SHA :
          case TLS_DHE_PSK_WITH_AES_256_CBC_SHA :
          case TLS_DHE_PSK_WITH_AES_128_CBC_SHA256 :
          case TLS_DHE_PSK_WITH_AES_256_CBC_SHA384 :
	      if (tls_options & _HTTP_TLS_DENY_CBC)
	      {
	        DEBUG_printf(("4_httpTLSStart: Excluding CBC cipher suite %d", supported[i]));
	        break;
	      }

//          case TLS_DHE_RSA_WITH_AES_128_GCM_SHA256 :
//          case TLS_DHE_RSA_WITH_AES_256_GCM_SHA384 :
          case TLS_DH_RSA_WITH_AES_128_GCM_SHA256 :
          case TLS_DH_RSA_WITH_AES_256_GCM_SHA384 :
//          case TLS_DHE_DSS_WITH_AES_128_GCM_SHA256 :
//          case TLS_DHE_DSS_WITH_AES_256_GCM_SHA384 :
          case TLS_DH_DSS_WITH_AES_128_GCM_SHA256 :
          case TLS_DH_DSS_WITH_AES_256_GCM_SHA384 :
          case TLS_DHE_PSK_WITH_AES_128_GCM_SHA256 :
          case TLS_DHE_PSK_WITH_AES_256_GCM_SHA384 :
              if (tls_options & _HTTP_TLS_ALLOW_DH)
	        enabled[num_enabled ++] = supported[i];
	      else
		DEBUG_printf(("4_httpTLSStart: Excluding DH/DHE cipher suite %d", supported[i]));
              break;

          case TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA :
          case TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 :
          case TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 :
          case TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256 :
          case TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384 :
          case TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 :
          case TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 :
          case TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256 :
          case TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384 :
          case TLS_RSA_WITH_3DES_EDE_CBC_SHA :
          case TLS_RSA_WITH_AES_128_CBC_SHA :
          case TLS_RSA_WITH_AES_256_CBC_SHA :
              if (tls_options & _HTTP_TLS_DENY_CBC)
	      {
	        DEBUG_printf(("4_httpTLSStart: Excluding CBC cipher suite %d", supported[i]));
	        break;
	      }

          /* Anything else we'll assume is "secure" */
          default :
	      enabled[num_enabled ++] = supported[i];
	      break;
	}
      }

      DEBUG_printf(("4_httpTLSStart: %d cipher suites enabled.", (int)num_enabled));
      error = SSLSetEnabledCiphers(http->tls, enabled, num_enabled);
    }
  }

  if (!error && http->mode == _HTTP_MODE_CLIENT)
  {
   /*
    * Client: set client-side credentials, if any...
    */

#if 0
    if (cg->client_cert_cb)
    {
      error = SSLSetSessionOption(http->tls,
				  kSSLSessionOptionBreakOnCertRequested, true);
      DEBUG_printf(("4_httpTLSStart: kSSLSessionOptionBreakOnCertRequested, "
                    "error=%d", (int)error));
    }
    else
#endif // 0
    {
      error = http_cdsa_set_credentials(http);
      DEBUG_printf(("4_httpTLSStart: http_cdsa_set_credentials, error=%d",
                    (int)error));
    }
  }
  else if (!error)
  {
   /*
    * Server: not supported...
    */

    http->error  = errno = EINVAL;
    http->status = HTTP_STATUS_ERROR;
    _cupsSetError(IPP_STATUS_ERROR_INTERNAL, _("Unable to create server credentials."), 1);
    return (-1);
  }

  DEBUG_printf(("4_httpTLSStart: tls_credentials=%p", (void *)http->tls_credentials));

 /*
  * Let the server know which hostname/domain we are trying to connect to
  * in case it wants to serve up a certificate with a matching common name.
  */

  if (!error && http->mode == _HTTP_MODE_CLIENT)
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

    error = SSLSetPeerDomainName(http->tls, hostname, strlen(hostname));

    DEBUG_printf(("4_httpTLSStart: SSLSetPeerDomainName, error=%d", (int)error));
  }

  if (!error)
  {
    int			done = 0;	/* Are we done yet? */
    double		old_timeout;	/* Old timeout value */
    http_timeout_cb_t	old_cb;		/* Old timeout callback */
    void		*old_data;	/* Old timeout data */

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

    while (!error && !done)
    {
      error = SSLHandshake(http->tls);

      DEBUG_printf(("4_httpTLSStart: SSLHandshake returned %d.", (int)error));

      switch (error)
      {
	case noErr :
	    done = 1;
	    break;

	case errSSLWouldBlock :
	    error = noErr;		/* Force a retry */
	    usleep(1000);		/* in 1 millisecond */
	    break;

	case errSSLServerAuthCompleted :
	    error = 0;
#if 0
	    if (cg->server_cert_cb)
	    {
	      error = httpCopyCredentials(http, &credentials);
	      if (!error)
	      {
		error = (cg->server_cert_cb)(http, http->tls, credentials,
					     cg->server_cert_data);
		httpFreeCredentials(credentials);
	      }

	      DEBUG_printf(("4_httpTLSStart: Server certificate callback "
	                    "returned %d.", (int)error));
	    }
#endif // 0
	    break;

	case errSSLClientCertRequested :
	    error = 0;

#if 0
	    if (cg->client_cert_cb)
	    {
	      names = NULL;
	      if (!(error = SSLCopyDistinguishedNames(http->tls, &dn_array)) &&
		  dn_array)
	      {
		if ((names = cupsArrayNew(NULL, NULL)) != NULL)
		{
		  for (i = 0, count = CFArrayGetCount(dn_array); i < count; i++)
		  {
		    data = (CFDataRef)CFArrayGetValueAtIndex(dn_array, i);

		    if ((credential = malloc(sizeof(*credential))) != NULL)
		    {
		      credential->datalen = (size_t)CFDataGetLength(data);
		      if ((credential->data = malloc(credential->datalen)))
		      {
			memcpy((void *)credential->data, CFDataGetBytePtr(data),
			       credential->datalen);
			cupsArrayAdd(names, credential);
		      }
		      else
		        free(credential);
		    }
		  }
		}

		CFRelease(dn_array);
	      }

	      if (!error)
	      {
		error = (cg->client_cert_cb)(http, http->tls, names,
					     cg->client_cert_data);

		DEBUG_printf(("4_httpTLSStart: Client certificate callback "
		              "returned %d.", (int)error));
	      }

	      httpFreeCredentials(names);
	    }
#endif // 0
	    break;

	case errSSLUnknownRootCert :
	    message = _("Unable to establish a secure connection to host "
	                "(untrusted certificate).");
	    break;

	case errSSLNoRootCert :
	    message = _("Unable to establish a secure connection to host "
	                "(self-signed certificate).");
	    break;

	case errSSLCertExpired :
	    message = _("Unable to establish a secure connection to host "
	                "(expired certificate).");
	    break;

	case errSSLCertNotYetValid :
	    message = _("Unable to establish a secure connection to host "
	                "(certificate not yet valid).");
	    break;

	case errSSLHostNameMismatch :
	    message = _("Unable to establish a secure connection to host "
	                "(host name mismatch).");
	    break;

	case errSSLXCertChainInvalid :
	    message = _("Unable to establish a secure connection to host "
	                "(certificate chain invalid).");
	    break;

	case errSSLConnectionRefused :
	    message = _("Unable to establish a secure connection to host "
	                "(peer dropped connection before responding).");
	    break;

 	default :
	    break;
      }
    }

   /*
    * Restore the previous timeout settings...
    */

    httpSetTimeout(http, old_timeout, old_cb, old_data);
  }

  if (error)
  {
    http->error  = error;
    http->status = HTTP_STATUS_ERROR;
    errno        = ECONNREFUSED;

    CFRelease(http->tls);
    http->tls = NULL;

   /*
    * If an error string wasn't set by the callbacks use a generic one...
    */

    if (!message)
    {
#if 0
      if (!cg->lang_default)
        cg->lang_default = cupsLangDefault();

      snprintf(msgbuf, sizeof(msgbuf), _cupsLangString(cg->lang_default, _("Unable to establish a secure connection to host (%d).")), error);
#else
      snprintf(msgbuf, sizeof(msgbuf), "Unable to establish a secure connection to host (%d).", error);
#endif // 0

      message = msgbuf;
    }

    _cupsSetError(IPP_STATUS_ERROR_CUPS_PKI, message, 1);

    return (-1);
  }

  return (0);
}


/*
 * '_httpTLSStop()' - Shut down SSL/TLS on a connection.
 */

void
_httpTLSStop(http_t *http)		/* I - HTTP connection */
{
  while (SSLClose(http->tls) == errSSLWouldBlock)
    usleep(1000);

  CFRelease(http->tls);

  if (http->tls_credentials)
    CFRelease(http->tls_credentials);

  http->tls             = NULL;
  http->tls_credentials = NULL;
}


/*
 * '_httpTLSWrite()' - Write to a SSL/TLS connection.
 */

int					/* O - Bytes written */
_httpTLSWrite(http_t     *http,		/* I - HTTP connection */
	       const char *buf,		/* I - Buffer holding data */
	       int        len)		/* I - Length of buffer */
{
  ssize_t	result;			/* Return value */
  OSStatus	error;			/* Error info */
  size_t	processed;		/* Number of bytes processed */


  DEBUG_printf(("2_httpTLSWrite(http=%p, buf=%p, len=%d)", (void *)http, (void *)buf, len));

  error = SSLWrite(http->tls, buf, (size_t)len, &processed);

  switch (error)
  {
    case 0 :
	result = (int)processed;
	break;

    case errSSLWouldBlock :
	if (processed)
	{
	  result = (int)processed;
	}
	else
	{
	  result = -1;
	  errno  = EINTR;
	}
	break;

    case errSSLClosedGraceful :
    default :
	if (processed)
	{
	  result = (int)processed;
	}
	else
	{
	  result = -1;
	  errno  = EPIPE;
	}
	break;
  }

  DEBUG_printf(("3_httpTLSWrite: Returning %d.", (int)result));

  return ((int)result);
}


/*
 * 'http_cdsa_create_credential()' - Create a single credential in the internal format.
 */

static SecCertificateRef			/* O - Certificate */
http_cdsa_create_credential(
    http_credential_t *credential)		/* I - Credential */
{
  SecCertificateRef	cert;			/* Certificate */
  CFDataRef		data;			/* Data object */


  if (!credential)
    return (NULL);

  data = CFDataCreate(kCFAllocatorDefault, credential->data, (CFIndex)credential->datalen);
  cert = SecCertificateCreateWithData(kCFAllocatorDefault, data);
  CFRelease(data);

  return (cert);
}


#if TARGET_OS_OSX
/*
 * 'http_cdsa_default_path()' - Get the default keychain path.
 */

static const char *			/* O - Keychain path */
http_cdsa_default_path(char   *buffer,	/* I - Path buffer */
                       size_t bufsize)	/* I - Size of buffer */
{
  const char *home = getenv("HOME");	/* HOME environment variable */
//  _cups_globals_t	*cg = _cupsGlobals();
					/* Pointer to library globals */


 /*
  * Determine the default keychain path.  Note that the login and system
  * keychains are no longer accessible to user applications starting in macOS
  * 10.11.4 (!), so we need to create our own keychain just for CUPS.
  */

  if (home)
    snprintf(buffer, bufsize, "%s/.htmldoc/ssl.keychain", home);
  else
    strlcpy(buffer, "/etc/htmldoc/ssl.keychain", bufsize);

  DEBUG_printf(("1http_cdsa_default_path: Using default path \"%s\".", buffer));

  return (buffer);
}


/*
 * 'http_cdsa_open_keychain()' - Open (or create) a keychain.
 */

static SecKeychainRef			/* O - Keychain or NULL */
http_cdsa_open_keychain(
    const char *path,			/* I - Path to keychain */
    char       *filename,		/* I - Keychain filename */
    size_t     filesize)		/* I - Size of filename buffer */
{
  SecKeychainRef	keychain = NULL;/* Temporary keychain */
  OSStatus		err;		/* Error code */
  Boolean		interaction;	/* Interaction allowed? */
  SecKeychainStatus	status = 0;	/* Keychain status */


 /*
  * Get the keychain filename...
  */

  if (!path)
  {
    path = http_cdsa_default_path(filename, filesize);
    tls_cups_keychain = 1;
  }
  else
  {
    strlcpy(filename, path, filesize);
    tls_cups_keychain = 0;
  }

 /*
  * Save the interaction setting and disable while we open the keychain...
  */

  SecKeychainGetUserInteractionAllowed(&interaction);
  SecKeychainSetUserInteractionAllowed(FALSE);

  if (access(path, R_OK) && tls_cups_keychain)
  {
   /*
    * Create a new keychain at the given path...
    */

    err = SecKeychainCreate(path, _CUPS_CDSA_PASSLEN, _CUPS_CDSA_PASSWORD, FALSE, NULL, &keychain);
  }
  else
  {
   /*
    * Open the existing keychain and unlock as needed...
    */

    err = SecKeychainOpen(path, &keychain);

    if (err == noErr)
      err = SecKeychainGetStatus(keychain, &status);

    if (err == noErr && !(status & kSecUnlockStateStatus) && tls_cups_keychain)
      err = SecKeychainUnlock(keychain, _CUPS_CDSA_PASSLEN, _CUPS_CDSA_PASSWORD, TRUE);
  }

 /*
  * Restore interaction setting...
  */

  SecKeychainSetUserInteractionAllowed(interaction);

 /*
  * Release the keychain if we had any errors...
  */

  if (err != noErr)
  {
    /* TODO: Set cups last error string */
    DEBUG_printf(("4http_cdsa_open_keychain: Unable to open keychain (%d), returning NULL.", (int)err));

    if (keychain)
    {
      CFRelease(keychain);
      keychain = NULL;
    }
  }

 /*
  * Return the keychain or NULL...
  */

  return (keychain);
}


/*
 * 'http_cdsa_open_system_keychain()' - Open the System keychain.
 */

static SecKeychainRef
http_cdsa_open_system_keychain(void)
{
  SecKeychainRef	keychain = NULL;/* Temporary keychain */
  OSStatus		err;		/* Error code */
  Boolean		interaction;	/* Interaction allowed? */
  SecKeychainStatus	status = 0;	/* Keychain status */


 /*
  * Save the interaction setting and disable while we open the keychain...
  */

  SecKeychainGetUserInteractionAllowed(&interaction);
  SecKeychainSetUserInteractionAllowed(TRUE);

  err = SecKeychainOpen("/Library/Keychains/System.keychain", &keychain);

  if (err == noErr)
    err = SecKeychainGetStatus(keychain, &status);

  if (err == noErr && !(status & kSecUnlockStateStatus))
    err = errSecInteractionNotAllowed;

 /*
  * Restore interaction setting...
  */

  SecKeychainSetUserInteractionAllowed(interaction);

 /*
  * Release the keychain if we had any errors...
  */

  if (err != noErr)
  {
    /* TODO: Set cups last error string */
    DEBUG_printf(("4http_cdsa_open_system_keychain: Unable to open keychain (%d), returning NULL.", (int)err));

    if (keychain)
    {
      CFRelease(keychain);
      keychain = NULL;
    }
  }

 /*
  * Return the keychain or NULL...
  */

  return (keychain);
}
#endif /* TARGET_OS_OSX */


/*
 * 'http_cdsa_read()' - Read function for the CDSA library.
 */

static OSStatus				/* O  - -1 on error, 0 on success */
http_cdsa_read(
    SSLConnectionRef connection,	/* I  - SSL/TLS connection */
    void             *data,		/* I  - Data buffer */
    size_t           *dataLength)	/* IO - Number of bytes */
{
  OSStatus	result;			/* Return value */
  ssize_t	bytes;			/* Number of bytes read */
  http_t	*http;			/* HTTP connection */


  http = (http_t *)connection;

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

  do
  {
    bytes = recv(http->fd, data, *dataLength, 0);
  }
  while (bytes == -1 && (errno == EINTR || errno == EAGAIN));

  if ((size_t)bytes == *dataLength)
  {
    result = 0;
  }
  else if (bytes > 0)
  {
    *dataLength = (size_t)bytes;
    result = errSSLWouldBlock;
  }
  else
  {
    *dataLength = 0;

    if (bytes == 0)
      result = errSSLClosedGraceful;
    else if (errno == EAGAIN)
      result = errSSLWouldBlock;
    else
      result = errSSLClosedAbort;
  }

  return (result);
}


/*
 * 'http_cdsa_set_credentials()' - Set the TLS credentials.
 */

static int				/* O - Status of connection */
http_cdsa_set_credentials(http_t *http)	/* I - HTTP connection */
{
//  _cups_globals_t *cg = _cupsGlobals();	/* Pointer to library globals */
  OSStatus		error = 0;	/* Error code */
  http_tls_credentials_t credentials = NULL;
					/* TLS credentials */


  DEBUG_printf(("7http_tls_set_credentials(%p)", (void *)http));

 /*
  * Prefer connection specific credentials...
  */

#if 0
  if ((credentials = http->tls_credentials) == NULL)
    credentials = cg->tls_credentials;
#else
  credentials = http->tls_credentials;
#endif // 0

  if (credentials)
  {
    error = SSLSetCertificate(http->tls, credentials);
    DEBUG_printf(("4http_tls_set_credentials: SSLSetCertificate, error=%d",
		  (int)error));
  }
  else
    DEBUG_puts("4http_tls_set_credentials: No credentials to set.");

  return (error);
}


/*
 * 'http_cdsa_write()' - Write function for the CDSA library.
 */

static OSStatus				/* O  - -1 on error, 0 on success */
http_cdsa_write(
    SSLConnectionRef connection,	/* I  - SSL/TLS connection */
    const void       *data,		/* I  - Data buffer */
    size_t           *dataLength)	/* IO - Number of bytes */
{
  OSStatus	result;			/* Return value */
  ssize_t	bytes;			/* Number of bytes read */
  http_t	*http;			/* HTTP connection */


  http = (http_t *)connection;

  do
  {
    bytes = write(http->fd, data, *dataLength);
  }
  while (bytes == -1 && (errno == EINTR || errno == EAGAIN));

  if ((size_t)bytes == *dataLength)
  {
    result = 0;
  }
  else if (bytes >= 0)
  {
    *dataLength = (size_t)bytes;
    result = errSSLWouldBlock;
  }
  else
  {
    *dataLength = 0;

    if (errno == EAGAIN)
      result = errSSLWouldBlock;
    else
      result = errSSLClosedAbort;
  }

  return (result);
}
