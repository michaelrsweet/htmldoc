<?php
//
// "$Id: login.php,v 1.6 2005/02/17 19:45:54 mike Exp $"
//
// Login/registration form...
//


//
// Include necessary headers...
//

include_once "phplib/html.php";

$usererror = "";

if (array_key_exists("PAGE", $_GET))
  $page = $_GET["PAGE"];
else if (array_key_exists("PAGE", $_POST))
  $page = $_POST["PAGE"];
else
  $page = "account.php";

if ($REQUEST_METHOD == "POST")
{
  if (array_key_exists("USERNAME", $_POST))
    $username = $_POST["USERNAME"];
  else
    $username = "";

  if (array_key_exists("REALNAME", $_POST))
    $realname = $_POST["REALNAME"];
  else
    $realname = "";

  if (array_key_exists("PASSWORD", $_POST))
    $password = $_POST["PASSWORD"];
  else
    $password = "";

  if (array_key_exists("PASSWORD2", $_POST))
    $password2 = $_POST["PASSWORD2"];
  else
    $password2 = "";

  if (array_key_exists("EMAIL", $_POST))
    $email = $_POST["EMAIL"];
  else
    $email = "";

  if (array_key_exists("EMAIL2", $_POST))
    $email2 = $_POST["EMAIL2"];
  else
    $email2 = "";

  if (array_key_exists("REGISTER", $_POST))
    $register = $_POST["REGISTER"];
  else
    $register = "";

  if ($username != "" && !eregi("^[-a-z0-9._]+\$", $username))
    $usererror = "Bad username - only letters, numbers, '.', '-', and '_' "
                ."are allowed!";
  else if ($argc == 1 && $argv[0] == "A" && $username != "" &&
           $realname != "" &&
	   $email != "" && validate_email($email) && $email == $email2)
  {
    // Good new account request so far; see if account already
    // exists...
    $dusername = db_escape($username);
    $demail    = db_escape($email);
    $result    = db_query("SELECT * FROM user WHERE "
                         ."name='$dusername' OR email LIKE '%<$demail>'");
    if (db_count($result) == 0)
    {
      // Nope, add unpublished user account and send registration email.
      db_free($result);

      // Create a random password until the user activates the account...
      for ($i = 0, $password = ""; $i < 20; $i ++)
	$password .= chr(rand(33,126));

      $hash   = crypt($password);
      $date   = time();
      $demail = db_escape("$realname <$email>");

      db_query("INSERT INTO user VALUES(NULL, 0, '$dusername', '$demail', "
              ."'$hash', 0, $date, '$dusername', $date, '$dusername')");

      $userid   = db_insert_id();
      $register = md5("$userid:$hash");

      mail($email, "$PROJECT_NAME User Registration",
           wordwrap("Thank you for requesting an account on the $PROJECT_NAME "
	           ."home page.  To complete your registration, go to the "
		   ."following URL:\n\n"
		   ."    $PHP_URL?E\n\n"
		   ."and enter your username ($username) and the "
		   ."following registration code:\n\n"
		   ."    $register\n\n"
//		   ."    md5('$userid:$hash')\n\n"
		   ."You will then be able to access your account.\n"),
	   "From: $PROJECT_EMAIL\r\n");

      html_header("Login Registration", "", "",
                  array("Login" => "$PHP_SELF",
		        "Enable Account" => "$PHP_SELF?E"));

      print("Thank you for requesting an account. You should receive an "
	   ."email from $PROJECT_EMAIL shortly with instructions on "
	   ."completing your registration.</p>\n");
      html_footer();
      exit();
    }

    // Account or email already exists...
    $row = db_next($result);

    if ($row["name"] == $username)
      $usererror = "Username already exists!";
    else
      $usererror = "Email address already in use for another account!";

    db_free($result);
  }
  else if ($argc == 1 && $argv[0] == "E" && $username != "" &&
           $password != "" && $password == $password2 &&
	   $register != "")
  {
    // Check that we have an existing user account...
    $dusername = db_escape($username);
    $result    = db_query("SELECT * FROM user WHERE name='$dusername'");
    if (db_count($result) == 1)
    {
      // Yes, now check the registration code...
      $row    = db_next($result);
      $userid = (int)$row["id"];
      $hash   = md5("$userid:$row[hash]");
      
      if ($hash == $register)
      {
        // Good code, enable the account and login...
	$hash = db_escape(crypt($password));
	db_query("UPDATE user SET is_published = 1, hash = '$hash' "
	        ."WHERE name='$dusername'");

	if (auth_login($username, $password) == "")
	{
	  db_query("UPDATE user SET is_published = 0 WHERE name='$dusername'");
	  $usererror = "Login failed!";
	}
        else if (!auth_write_file())
	  $usererror = "Authentication files not created!";
      }
      else
        $usererror = "Bad registration code!";
//        $usererror = "Bad registration code (expected '$hash' for "
//	            ."md5('$userid:$row[hash]'), got '$register')!";
    }
    else
      $usererror = "Username not found!";

    db_free($result);
  }
  else if ($argc == 1 && $argv[0] == "F" &&
           ($username != "" || ($email != "" && validate_email($email))))
  {
    // Good "forgot account/password" request so far; see if account already
    // exists...
    $dusername = db_escape($username);
    $demail    = db_escape($email);
    $result    = db_query("SELECT * FROM user WHERE "
                         ."name='$dusername' OR email LIKE '%<$demail>'");
    if (db_count($result) == 1)
    {
      // Found the account, send an email...
      $row  = db_next($result);
      $hash = md5("$row[id]:$row[hash]");

      db_free($result);

      mail($row["email"], "$PROJECT_NAME Password Reset Request",
           wordwrap("Some one, possibly you, requested that your password "
	           ."be reset on the $PROJECT_NAME home page.  To enter "
		   ."a new password, go to the following URL:\n\n"
		   ."    $PHP_URL?E\n\n"
		   ."and enter your username ($row[name]) and the "
		   ."following registration code:\n\n"
		   ."    $hash\n\n"
		   ."You will then be able to access your account.\n"),
	   "From: $PROJECT_EMAIL\r\n");

      html_header("Forgot Username or Password", "", "",
                  array("Login" => "$PHP_SELF",
		        "Reset Password" => "$PHP_SELF?E"));

      print("<p>You should receive an email from $PROJECT_EMAIL shortly "
           ."with instructions on resetting your password.</p>\n");
      html_footer();
      exit();
    }

    // Account and email not found...
    $usererror = "No matching username or email address was found!";

    db_free($result);
  }
  else if ($argc == 0 && $username != "" && $password != "")
    if (auth_login($username, $password) == "")
      $usererror = "Login failed!";
}
else
{
  $username  = "";
  $realname  = "";
  $password  = "";
  $password2 = "";
  $email     = "";
  $email2    = "";
  $register  = "";
}

if ($LOGIN_USER != "")
  header("Location: $page");
else if ($argc == 0 || $argv[0] != "E")
{
  // Header + start of table...
  html_header("Login", "", "",
              array("Login" => "$PHP_SELF", "Enable Account" => "$PHP_SELF?E"));

  print("<p><table width='100%' height='100%' border='0' cellpadding='0' "
       ."cellspacing='0'>\n"
       ."<tr><td valign='top'>\n");

  // Existing user...
  print("<h2>Current Users</h2>\n");

  if ($argc == 0 && $usererror != "")
    print("<p><b>$usererror</b></p>\n");

  $page = htmlspecialchars($page, ENT_QUOTES);

  print("<p>If you are a registered $PROJECT_NAME user or developer, "
       ."please enter your username and password to login:</p>\n"
       ."<form method='POST' action='$PHP_SELF'>"
       ."<input type='hidden' name='PAGE' value='$page'/>"
       ."<p><table width='100%'>\n"
       ."<tr><th align='right'>Username:</th>"
       ."<td><input type='text' name='USERNAME' size='16' maxsize='255'");

  if (array_key_exists("USERNAME", $_POST))
    print(" value='" . htmlspecialchars($_POST["USERNAME"], ENT_QUOTES) . "'");

  print("/></td></tr>\n"
       ."<tr><th align='right'>Password:</th>"
       ."<td><input type='password' name='PASSWORD' size='16' maxsize='255'/>"
       ."</td></tr>\n"
       ."<tr><th></th><td><input type='submit' value='Login'/></td></tr>\n"
       ."</table></p></form>\n");

  // Separator...
  print("</td>"
       ."<td>&nbsp;&nbsp;&nbsp;&nbsp;"
       ."<img src='images/black.gif' width='1' height='80%' alt=''/>"
       ."&nbsp;&nbsp;&nbsp;&nbsp;</td>"
       ."<td valign='top'>\n");

  // New user...
  print("<h2>New Users</h2>\n");

  if ($argc == 1 && $argv[0] == "A" && $usererror != "")
    print("<p><b>$usererror</b></p>\n");

  $username = htmlspecialchars($username, ENT_QUOTES);
  $realname = htmlspecialchars($realname, ENT_QUOTES);
  $email    = htmlspecialchars($email, ENT_QUOTES);
  $email2   = htmlspecialchars($email2, ENT_QUOTES);

  print("<p>If you are a not registered $PROJECT_NAME user or developer, "
       ."please fill in the form below to register. An email will be sent "
       ."to the address you supply to confirm the registration:</p>\n"
       ."<form method='POST' action='$PHP_SELF?A'>"
       ."<p><table width='100%'>\n"
       ."<tr><th align='right'>Username:</th>"
       ."<td><input type='text' name='USERNAME' size='16' maxsize='255' "
       ." value='$username'/></td></tr>\n"
       ."<tr><th align='right'>Real Name:</th>"
       ."<td><input type='text' name='REALNAME' size='16' maxsize='255' "
       ." value='$realname'/></td></tr>\n"
       ."<tr><th align='right'>EMail:</th>"
       ."<td><input type='text' name='EMAIL' size='16' maxsize='255' "
       ." value='$email'/></td></tr>\n"
       ."<tr><th align='right'>EMail Again:</th>"
       ."<td><input type='text' name='EMAIL2' size='16' maxsize='255' "
       ." value='$email2'/></td></tr>\n"
       ."<tr><th></th><td><input type='submit' value='Request Account'/></td></tr>\n"
       ."</table></p></form>\n");

  // Separator...
  print("</td>"
       ."<td>&nbsp;&nbsp;&nbsp;&nbsp;"
       ."<img src='images/black.gif' width='1' height='80%' alt=''/>"
       ."&nbsp;&nbsp;&nbsp;&nbsp;</td>"
       ."<td valign='top'>\n");

  // Forgot password...
  print("<h2>Did You Forget Your Username or Password?</h2>\n");

  if ($argc == 1 && $argv[0] == "F" && $usererror != "")
    print("<p><b>$usererror</b></p>\n");

  $username = htmlspecialchars($username, ENT_QUOTES);
  $realname = htmlspecialchars($realname, ENT_QUOTES);
  $email    = htmlspecialchars($email, ENT_QUOTES);

  print("<p>If you are a registered $PROJECT_NAME user or developer "
       ."but have forgotten your username or password, please fill in "
       ."the form below to reset your password. An email will be sent to the "
       ."address you supply with instructions:</p>\n"
       ."<form method='POST' action='$PHP_SELF?F'>"
       ."<p><table width='100%'>\n"
       ."<tr><th align='right'>Username:</th>"
       ."<td><input type='text' name='USERNAME' size='16' maxsize='255' "
       ." value='$username'/></td></tr>\n"
       ."<tr><th align='right'>EMail:</th>"
       ."<td><input type='text' name='EMAIL' size='16' maxsize='255' "
       ." value='$email'/></td></tr>\n"
       ."<tr><th></th><td><input type='submit' value='Forgot Username or Password'/></td></tr>\n"
       ."</table></p></form>\n");

  // End table
  print("</td></tr>\n"
       ."</table></p>\n");

  html_footer();
}
else
{
  html_header("Enable Account", "", "",
              array("Login" => "$PHP_SELF", "Enable Account" => "$PHP_SELF?E"));

  if ($usererror != NULL)
    print("<p><b>$usererror</b></p>\n");

  $username = htmlspecialchars($username, ENT_QUOTES);
  $register = htmlspecialchars($register, ENT_QUOTES);

  print("<p>Please enter the registration code that was emailed to you "
       ."with your username and password to enable your account and login:</p>\n"
       ."<form method='POST' action='$PHP_SELF?E'>"
       ."<center><table width='100%'>\n"
       ."<tr><th align='right'>Registration Code:</th>"
       ."<td><input type='text' name='REGISTER' size='32' maxsize='32' "
       ."value = '$register'/>"
       ."</td></tr>\n"
       ."<tr><th align='right'>Username:</th>"
       ."<td><input type='text' name='USERNAME' size='16' maxsize='255' "
       ."value='$username'/></td></tr>\n"
       ."<tr><th align='right'>Password:</th>"
       ."<td><input type='password' name='PASSWORD' size='16' maxsize='255'/>"
       ."</td></tr>\n"
       ."<tr><th align='right'>Password Again:</th>"
       ."<td><input type='password' name='PASSWORD2' size='16' maxsize='255'/>"
       ."</td></tr>\n"
       ."<tr><th></th><td><input type='submit' value='Enable Account'/></td></tr>\n"
       ."</table></center></form>\n");

  html_footer();
}


//
// End of "$Id: login.php,v 1.6 2005/02/17 19:45:54 mike Exp $".
//
?>
