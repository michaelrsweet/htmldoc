<?php
//
// "$Id: db-user.php,v 1.5 2006/07/11 13:55:14 mike Exp $"
//
// Class for the user table.
//
// Contents:
//
//   user::user()	- Create a user object.
//   user::clear()	- Initialize a new a user object.
//   user::delete()	- Delete a user object.
//   user::edit()	- Display a form for a user object.
//   user::load()	- Load a user object.
//   user::loadform()	- Load a user object from form data.
//   user::password()	- Set the hash field using a password.
//   user::save()	- Save a user object.
//   user::search()	- Get a list of user IDs.
//   user::validate()	- Validate the current user object values.
//   user::view()	- View the current user object.
//

include_once "html.php";


class user
{
  //
  // Instance variables...
  //

  var $id;
  var $is_published;
  var $name, $name_valid;
  var $email, $email_valid;
  var $hash, $hash_valid;
  var $level, $level_valid;
  var $create_date;
  var $create_user;
  var $modify_date;
  var $modify_user;

  var $previous_name;


  //
  // 'user::user()' - Create a user object.
  //

  function				// O - New user object
  user($id = 0)				// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'user::clear()' - Initialize a new a user object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->is_published = 0;
    $this->name = "";
    $this->email = "";
    $this->hash = "";
    $this->level = AUTH_USER;
    $this->create_date = 0;
    $this->create_user = "";
    $this->modify_date = 0;
    $this->modify_user = "";
    $this->previous_name = "";
  }


  //
  // 'user::delete()' - Delete a user object.
  //

  function
  delete()
  {
    db_query("DELETE FROM user WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'user::edit()' - Display a form for a user object.
  //

  function
  edit($options = "")			// I - Page options
  {
    global $AUTH_LEVELS,
           $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;


    if ($this->id <= 0)
      $action = "Create User";
    else
      $action = "Modify User $this->name";

    print("<form action='$PHP_SELF?U$this->id$options' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // is_published
    if ($LOGIN_LEVEL < AUTH_ADMIN)
    {
      print("<input type='hidden' name='is_published' "
           ."value='$this->is_published'/>");
    }
    else
    {
      print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
      html_select_is_published($this->is_published);
      print("</td></tr>\n");
    }

    // name
    $html = htmlspecialchars($this->name, ENT_QUOTES);
    if ($this->name_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Username:</th><td>");
    if ($LOGIN_LEVEL < AUTH_ADMIN)
      print("<input type='hidden' name='name' value='$html'/>$html");
    else
      print("<input type='text' name='name' value='$html' size='72'/>");
    print("</td></tr>\n");

    // email
    $html = htmlspecialchars($this->email, ENT_QUOTES);
    if ($this->email_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Email:</th><td>");
    print("<input type='text' name='email' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // password
    if ($this->hash_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Password:</th><td>");
    print("<input type='password' name='password' size='20'/> (leave blank for no change)");
    print("</td></tr>\n");
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Password Again:</th><td>");
    print("<input type='password' name='password2' size='20'/>");
    print("</td></tr>\n");

    // level
    if ($this->level_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Access Level:</th><td>");

    if ($LOGIN_LEVEL < AUTH_ADMIN)
      print("<input type='hidden' name='level' value='$this->level'/>"
	   . $AUTH_LEVELS[$this->level]);
    else
    {
      print("<select name='level'>");

      reset($AUTH_LEVELS);
      while (list($key, $val) = each($AUTH_LEVELS))
      {
	if ($this->level == $key)
	  print("<option value='$key' selected>$val</option>");
	else
	  print("<option value='$key'>$val</option>");
      }

      print("</select>");
    }

    print("</td></tr>\n");

    // Submit
    print("<tr><td></td><td>"
         ."<input type='submit' value='$action'/>"
         ."</td></tr>\n"
         ."</table></p>\n"
         ."</form>\n");
  }


  //
  // 'user::load()' - Load a user object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM user WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->is_published = $row["is_published"];
    $this->name = $this->previous_name = $row["name"];
    $this->email = $row["email"];
    $this->hash = $row["hash"];
    $this->level = $row["level"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];
    $this->modify_date = $row["modify_date"];
    $this->modify_user = $row["modify_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'user::loadform()' - Load a user object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if ($LOGIN_LEVEL == AUTH_ADMIN)
    {
      if (array_key_exists("is_published", $_GET))
	$this->is_published = $_GET["is_published"];
      else if (array_key_exists("is_published", $_POST))
	$this->is_published = $_POST["is_published"];

      if (array_key_exists("name", $_GET))
	$this->name = $_GET["name"];
      else if (array_key_exists("name", $_POST))
	$this->name = $_POST["name"];
    }

    if (array_key_exists("email", $_GET))
      $this->email = $_GET["email"];
    else if (array_key_exists("email", $_POST))
      $this->email = $_POST["email"];

    if (array_key_exists("level", $_GET))
      $this->level = $_GET["level"];
    else if (array_key_exists("level", $_POST))
      $this->level = $_POST["level"];

    if (array_key_exists("password", $_GET))
      $password = $_GET["password"];
    else if (array_key_exists("password", $_POST))
      $password = $_POST["password"];
    else
      $password = "";

    if (array_key_exists("password2", $_GET))
      $password2 = $_GET["password2"];
    else if (array_key_exists("password2", $_POST))
      $password2 = $_POST["password2"];
    else
      $password2 = "";

    if ($password != "" && $password == $password2)
      $this->password($password);
    else if ($password != "")
      $this->hash = "";

    return ($this->validate());
  }


  //
  // 'user::password()' - Set the hash field using a password.
  //

  function
  password($pass = "")			// I - Password string
  {
    // Map blank passwords to 20 random characters...
    if ($pass == "")
    {
      for ($i = 0; $i < 20; $i ++)
	$pass .= chr(rand(33,126));
    }

    // Create the hash string that is stored in the database...
    $this->hash = crypt($pass);
  }


  //
  // 'user::save()' - Save a user object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    $this->modify_date = time();
    $this->modify_user = $LOGIN_USER;

    if ($this->id > 0)
    {
      if (db_query("UPDATE user "
                  ." SET is_published = $this->is_published"
                  .", name = '" . db_escape($this->name) . "'"
                  .", email = '" . db_escape($this->email) . "'"
                  .", hash = '" . db_escape($this->hash) . "'"
                  .", level = $this->level"
                  .", modify_date = $this->modify_date"
                  .", modify_user = '" . db_escape($this->modify_user) . "'"
                  ." WHERE id = $this->id") === FALSE)
        return (FALSE);

      if ($this->name != $this->previous_name)
      {
        // Rename the user in all of the tables...
        $dname = db_escape($this->name);
	$dprev = db_escape($this->previous_name);

        db_query("UPDATE article SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE article SET modify_user = '$dname' "
	        ."WHERE modify_user = '$dprev'");

        db_query("UPDATE articlefile SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE comment SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE link SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE link SET modify_user = '$dname' "
	        ."WHERE modify_user = '$dprev'");

        db_query("UPDATE poll SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE poll SET modify_user = '$dname' "
	        ."WHERE modify_user = '$dprev'");

        db_query("UPDATE str SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE str SET modify_user = '$dname' "
	        ."WHERE modify_user = '$dprev'");

        db_query("UPDATE strfile SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE strtext SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE user SET create_user = '$dname' "
	        ."WHERE create_user = '$dprev'");

        db_query("UPDATE user SET modify_user = '$dname' "
	        ."WHERE modify_user = '$dprev'");
      }
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO user VALUES"
                  ."(NULL"
                  .", $this->is_published"
                  .", '" . db_escape($this->name) . "'"
                  .", '" . db_escape($this->email) . "'"
                  .", '" . db_escape($this->hash) . "'"
                  .", $this->level"
                  .", $this->create_date"
                  .", '" . db_escape($this->create_user) . "'"
                  .", $this->modify_date"
                  .", '" . db_escape($this->modify_user) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'user::search()' - Get a list of user objects.
  //

  function				// O - Array of user objects
  search($search = "",			// I - Search string
         $order = "")			// I - Order fields
  {
    if ($search != "")
    {
      // Convert the search string to an array of words...
      $words = html_search_words($search);

      // Loop through the array of words, adding them to the query...
      $query  = " WHERE (";
      $prefix = "";
      $next   = " OR";
      $logic  = "";

      reset($words);
      foreach ($words as $word)
      {
        if ($word == "or")
        {
          $next = ' OR';
          if ($prefix != '')
            $prefix = ' OR';
        }
        else if ($word == "and")
        {
          $next = ' AND';
          if ($prefix != '')
            $prefix = ' AND';
        }
        else if ($word == "not")
          $logic = ' NOT';
        else
        {
          $query .= "$prefix$logic (";
          $subpre = "";

          if (preg_match("/^[0-9]+\$/", $word))
          {
            $query .= "${subpre}id = $word";
            $subpre = " OR ";
          }

          $query .= "${subpre}name LIKE \"%$word%\"";
          $subpre = " OR ";
          $query .= "${subpre}email LIKE \"%$word%\"";

          $query .= ")";
          $prefix = $next;
          $logic  = '';
        }
      }

      $query .= ")";
    }
    else
      $query = "";

    if ($order != "")
    {
      // Separate order into array...
      $fields = explode(" ", $order);
      $prefix = " ORDER BY ";

      // Add ORDER BY stuff...
      foreach ($fields as $field)
      {
        if ($field[0] == '+')
          $query .= "${prefix}" . substr($field, 1);
        else if ($field[0] == '-')
          $query .= "${prefix}" . substr($field, 1) . " DESC";
        else
          $query .= "${prefix}$field";

        $prefix = ", ";
      }
    }

    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM user$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'user::validate()' - Validate the current user object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->name == "")
    {
      $this->name_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->name_valid = TRUE;

    if ($this->hash == "")
    {
      $this->hash_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->hash_valid = TRUE;

    if (!validate_email($this->email))
    {
      $this->email_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->email_valid = TRUE;

    if ($this->level < AUTH_USER || $this->level > AUTH_ADMIN)
    {
      $this->level_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->level_valid = TRUE;

    return ($valid);
  }


  //
  // 'user::view()' - View the current user object.
  //

  function
  view()
  {
    global $AUTH_LEVELS;


    print("<p><table class='view'>\n");

    // is_published
    print("<tr><th align='right' valign='top' nowrap>Published:</th><td>");
    if ($this->is_published)
      print("Yes");
    else
      print("No");
    print("</td></tr>\n");

    // name
    $html = htmlspecialchars($this->name, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Username:</th><td>");
    print($html);
    print("</td></tr>\n");

    // email
    $html = htmlspecialchars($this->email, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Email:</th><td>");
    print($html);
    print("</td></tr>\n");

    // level
    print("<tr><th align='right' valign='top' nowrap>Access Level:</th><td>");
    print($AUTH_LEVELS[$this->level]);
    print("</td></tr>\n");

    // create_date
    $html = date("H:i M d, Y", $this->create_date);
    print("<tr><th align='right' valign='top' nowrap>User Since:</th><td>");
    print($html);
    print("</td></tr>\n");

    // modify_date
    $html = date("H:i M d, Y", $this->modify_date);
    print("<tr><th align='right' valign='top' nowrap>Last Changed:</th><td>");
    print($html);
    print("</td></tr>\n");

    print("</table></p>\n");
  }
}


//
// End of "$Id: db-user.php,v 1.5 2006/07/11 13:55:14 mike Exp $".
//
?>
