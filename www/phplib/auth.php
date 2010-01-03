<?
//
// "$Id: auth.php,v 1.5 2005/02/05 05:51:24 mike Exp $"
//
// Authentication functions for PHP pages...
//
// Contents:
//
//   auth_current()    - Return the currently logged in user...
//   auth_login()      - Log a user into the system.
//   auth_logout()     - Logout of the current user by clearing the session ID.
//   auth_user_email() - Return the email address of a user...
//   auth_write_file() - Write the authorization file for Apache.
//

//
// Include necessary headers...
//

include_once "globals.php";
include_once "db.php";


//
// Define authorization levels...
//

define("AUTH_USER", 0);
define("AUTH_DEVEL", 50);
define("AUTH_ADMIN", 100);

$AUTH_LEVELS = array(
  AUTH_USER => "User",
  AUTH_DEVEL => "Developer",
  AUTH_ADMIN => "Admin"
);


//
// Store the current user in the global variable LOGIN_USER...
//

$LOGIN_ID    = 0;
$LOGIN_LEVEL = 0;
$LOGIN_USER  = "";
$LOGIN_EMAIL = "";

// Check for a logout on the command-line...
if ($argc == 1 && $argv[0] == "logout")
{
  auth_logout();
  $argc = 0;
}
else
  auth_current();


//
// 'auth_current()' - Return the currently logged in user...
//

function				// O - Current username or ""
auth_current()
{
  global $_COOKIE, $_SERVER, $LOGIN_EMAIL, $LOGIN_LEVEL, $LOGIN_USER,
	 $PROJECT_MODULE, $LOGIN_ID;


  // See if the SID cookie is set; if not, the user is not logged in...
  if (!array_key_exists("${PROJECT_MODULE}SID", $_COOKIE))
    return ("");

  // Extract the "username:hash" from the SID string...
  $cookie = explode(':', $_COOKIE["${PROJECT_MODULE}SID"]);

  // Don't allow invalid values...
  if (count($cookie) != 2)
    return ("");

  // Lookup the username in the user table and compare...
  $result = db_query("SELECT * FROM user WHERE "
                    ."name='".db_escape($cookie[0])."' AND "
		    ."is_published = 1");
  if (db_count($result) == 1 && ($row = db_next($result)))
  {
    // Compute the session ID...
    $sid = md5("$_SERVER[REMOTE_ADDR]:$row[hash]");

    // See if it matches the cookie value...
    if ($cookie[1] == $sid)
    {
      // Refresh the cookies so they don't expire...
      setcookie("${PROJECT_MODULE}SID", "$cookie[0]:$sid", time() + 90 * 86400, "/");
      setcookie("${PROJECT_MODULE}FROM", $row['email'], time() + 90 * 86400, "/");

      // Set globals...
      $LOGIN_ID                         = $row["id"];
      $LOGIN_USER                       = $row["name"];
      $LOGIN_LEVEL                      = $row["level"];
      $LOGIN_EMAIL                      = $row["email"];
      $_COOKIE["${PROJECT_MODULE}FROM"] = $row["email"];

      // Return the current user...
      return ($cookie[0]);
    }
  }

  return ("");
}


//
// 'auth_login()' - Log a user into the system.
//

function				// O - Current username or ""
auth_login($name,			// I - Username
           $password)			// I - Password
{
  global $_COOKIE, $_SERVER, $LOGIN_EMAIL, $LOGIN_LEVEL, $LOGIN_USER,
	 $PROJECT_MODULE, $LOGIN_ID;


  // Reset the user...
  $LOGIN_USER = "";

  // Lookup the username in the database...
  $result = db_query("SELECT * FROM user WHERE "
                    ."name='".db_escape($name)."' AND "
		    ."is_published = 1");
  if (db_count($result) == 1 && ($row = db_next($result)))
  {
    // Encrypt the password...
    $hash = crypt($password, $row["hash"]);

    // See if they match...
    if ($row["hash"] == $hash)
    {
      // Update the username and email...
      $LOGIN_ID                         = $row["id"];
      $LOGIN_USER                       = $row["name"];
      $LOGIN_LEVEL                      = $row["level"];
      $LOGIN_EMAIL                      = $row["email"];
      $_COOKIE["${PROJECT_MODULE}FROM"] = $row["email"];

      // Compute the session ID...
      $sid = "$name:" . md5("$_SERVER[REMOTE_ADDR]:$hash");

      // Save the SID and email address cookies...
      setcookie("${PROJECT_MODULE}SID", $sid, time() + 90 * 86400, "/");
      setcookie("${PROJECT_MODULE}FROM", $row['email'], time() + 90 * 86400, "/");
    }
  }

  return ($LOGIN_USER);
}


//
// 'auth_logout()' - Logout of the current user by clearing the session ID.
//

function
auth_logout()
{
  global $LOGIN_EMAIL, $LOGIN_LEVEL, $LOGIN_USER,
	 $PROJECT_MODULE, $LOGIN_ID;


  $LOGIN_ID    = 0;
  $LOGIN_USER  = "";
  $LOGIN_EMAIL = "";
  $LOGIN_LEVEL = 0;

  setcookie("${PROJECT_MODULE}SID", "", time() + 90 * 86400, "/");
}


//
// 'auth_user_email()' - Return the email address of a user...
//

function				// O - Email address
auth_user_email($username)		// I - Username
{
  $result = db_query("SELECT * FROM user WHERE "
                    ."name = '" . db_escape($username) . "'");
  if (db_count($result) == 1)
  {
    $row = db_next($result);
    $email = $row["email"];
  }
  else
    $email = "";

  db_free($result);

  return ($email);
}


//
// 'auth_write_file()' - Write the authorization file for Apache.
//

function				// O - TRUE on success, FALSE on failure
auth_write_file()
{
  $result = db_query("SELECT * FROM user WHERE is_published = 1 AND "
                    ."level >= " . AUTH_DEVEL);

  if (db_count($result) > 0)
  {
    $fp = fopen("data/authfile", "w");
    if (!$fp)
    {
      db_free($result);
      return (FALSE);
    }

    while ($row = db_next($result))
      fwrite($fp, "$row[name]:$row[hash]\n");

    fclose($fp);
  }

  db_free($result);

  return (TRUE);
}


//
// End of "$Id: auth.php,v 1.5 2005/02/05 05:51:24 mike Exp $".
//
?>
