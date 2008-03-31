<?php
//
// "$Id: globals.php,v 1.6 2006/05/23 14:05:51 mike Exp $"
//
// Global PHP constants and variables...
//
// This file should be included using "include_once"...
//

//
// Global vars...
//

$PROJECT_NAME = "HTMLDOC";		// Title of project
$PROJECT_SITE = "www.htmldoc.org";	// Web site
$PROJECT_EMAIL = "htmldoc-support@easysw.com";	// Default notification address
$PROJECT_REGISTER = "htmldoc-register@easysw.com";
					// User registration email
$PROJECT_MODULE = "htmldoc";		// Subversion module
$PROJECT_URL = "http://www.htmldoc.org/";
$PAGE_MAX = 10; 			// Max items per page


define("PROJECT_LINK_ALL", 0);


//
// PHP transition stuff...
//

global $_COOKIE, $_FILES, $_GET, $_POST, $_SERVER;

foreach (array("argc", "argv", "REQUEST_METHOD", "SERVER_NAME", "SERVER_PORT", "REMOTE_ADDR") as $var)
{
  if (array_key_exists($var, $_SERVER))
    $$var = $_SERVER[$var];
  else
    $$var = "";
}

// Handle PHP_SELF differently - we need to quote it properly...
if (array_key_exists("PHP_SELF", $_SERVER))
  $PHP_SELF = htmlspecialchars($_SERVER["PHP_SELF"], ENT_QUOTES);
else
  $PHP_SELF = "";

if (array_key_exists("ISHTTPS", $_SERVER))
  $PHP_URL = "https://$SERVER_NAME:$SERVER_PORT$PHP_SELF";
else
  $PHP_URL = "http://$SERVER_NAME:$SERVER_PORT$PHP_SELF";

//
// End of "$Id: globals.php,v 1.6 2006/05/23 14:05:51 mike Exp $".
//
?>
