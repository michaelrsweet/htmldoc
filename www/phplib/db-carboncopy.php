<?php
//
// "$Id: db-carboncopy.php,v 1.5 2006/07/11 13:55:14 mike Exp $"
//
// Class for the carboncopy table.
//
// Contents:
//
//   carboncopy::carboncopy()	- Create an Carbon Copy object.
//   carboncopy::clear()	- Initialize a new an Carbon Copy object.
//   carboncopy::delete()	- Delete an Carbon Copy object.
//   carboncopy::edit()	- Display a form for an Carbon Copy object.
//   carboncopy::load()	- Load an Carbon Copy object.
//   carboncopy::loadform()	- Load an Carbon Copy object from form data.
//   carboncopy::save()	- Save an Carbon Copy object.
//   carboncopy::search()	- Get a list of Carbon Copy IDs.
//   carboncopy::validate()	- Validate the current Carbon Copy object values.
//   carboncopy::view()	- View the current Carbon Copy object.
//

include_once "html.php";


class carboncopy
{
  //
  // Instance variables...
  //

  var $id;
  var $url, $url_valid;
  var $email, $email_valid;


  //
  // 'carboncopy::carboncopy()' - Create an Carbon Copy object.
  //

  function				// O - New Carbon Copy object
  carboncopy($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'carboncopy::clear()' - Initialize a new an Carbon Copy object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->url = "";
    $this->email = "";
  }


  //
  // 'carboncopy::delete()' - Delete an Carbon Copy object.
  //

  function
  delete()
  {
    db_query("DELETE FROM carboncopy WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'carboncopy::edit()' - Display a form for an Carbon Copy object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit Carbon Copy";
    else
      $action = "Modify Carbon Copy #$this->id";

    print("<h1>$action</h1>\n"
         ."<form action='$PHP_SELF?U$this->id' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // url
    $html = htmlspecialchars($this->url, ENT_QUOTES);
    if ($this->url_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Url:</th><td>");
    print("<input type='text' name='url' "
         ."value='$html' size='72'/>");
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

    // Submit
    print("<tr><td></td><td>"
         ."<input type='submit' value='$action'/>"
         ."</td></tr>\n"
         ."</table></p>\n"
         ."</form>\n");
  }


  //
  // 'carboncopy::load()' - Load an Carbon Copy object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM carboncopy WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->url = $row["url"];
    $this->email = $row["email"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'carboncopy::loadform()' - Load an Carbon Copy object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if (array_key_exists("url", $_GET))
      $this->url = $_GET["url"];
    else if (array_key_exists("url", $_POST))
      $this->url = $_POST["url"];

    if (array_key_exists("email", $_GET))
      $this->email = $_GET["email"];
    else if (array_key_exists("email", $_POST))
      $this->email = $_POST["email"];

    return ($this->validate());
  }


  //
  // 'carboncopy::save()' - Save an Carbon Copy object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    if ($this->id > 0)
    {
      return (db_query("UPDATE carboncopy "
                      ." SET url = '" . db_escape($this->url) . "'"
                      .", email = '" . db_escape($this->email) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO carboncopy VALUES"
                  ."(NULL"
                  .", '" . db_escape($this->url) . "'"
                  .", '" . db_escape($this->email) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'carboncopy::search()' - Get a list of Carbon Copy objects.
  //

  function				// O - Array of Carbon Copy objects
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

          if (ereg("^[0-9]+\$", $word))
          {
            $query .= "${subpre}id = $word";
            $subpre = " OR ";
          }

          $query .= "${subpre}url LIKE \"%$word%\"";
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
    $result  = db_query("SELECT id FROM carboncopy$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'carboncopy::validate()' - Validate the current Carbon Copy object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->url == "")
    {
      $this->url_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->url_valid = TRUE;

    if ($this->email == "")
    {
      $this->email_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->email_valid = TRUE;

    return ($valid);
  }


  //
  // 'carboncopy::view()' - View the current Carbon Copy object.
  //

  function
  view()
  {
    print("<p><table class='view'>\n");

    // url
    $html = htmlspecialchars($this->url, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Url:</th><td>");
    print($html);
    print("</td></tr>\n");

    // email
    $html = htmlspecialchars($this->email, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Email:</th><td>");
    print($html);
    print("</td></tr>\n");
    print("</table></p>\n");
  }
}


//
// End of "$Id: db-carboncopy.php,v 1.5 2006/07/11 13:55:14 mike Exp $".
//
?>
