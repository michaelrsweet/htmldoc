<?php
//
// "$Id: db-articlefile.php,v 1.5 2006/07/11 13:55:14 mike Exp $"
//
// Class for the articlefile table.
//
// Contents:
//
//   articlefile::articlefile()	- Create an Article File object.
//   articlefile::clear()	- Initialize a new an Article File object.
//   articlefile::delete()	- Delete an Article File object.
//   articlefile::edit()	- Display a form for an Article File object.
//   articlefile::load()	- Load an Article File object.
//   articlefile::loadform()	- Load an Article File object from form data.
//   articlefile::save()	- Save an Article File object.
//   articlefile::search()	- Get a list of Article File IDs.
//   articlefile::validate()	- Validate the current Article File object values.
//   articlefile::view()	- View the current Article File object.
//

include_once "html.php";


class articlefile
{
  //
  // Instance variables...
  //

  var $id;
  var $article_id, $article_id_valid;
  var $is_published;
  var $filename, $filename_valid;
  var $create_date;
  var $create_user;


  //
  // 'articlefile::articlefile()' - Create an Article File object.
  //

  function				// O - New Article File object
  articlefile($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'articlefile::clear()' - Initialize a new an Article File object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->article_id = 0;
    $this->is_published = 0;
    $this->filename = "";
    $this->create_date = 0;
    $this->create_user = "";
  }


  //
  // 'articlefile::delete()' - Delete an Article File object.
  //

  function
  delete()
  {
    db_query("DELETE FROM articlefile WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'articlefile::edit()' - Display a form for an Article File object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit Article File";
    else
      $action = "Modify Article File #$this->id";

    print("<h1>$action</h1>\n"
         ."<form action='$PHP_SELF?U$this->id' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // is_published
    $html = htmlspecialchars($this->is_published, ENT_QUOTES);
    print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
    html_select_is_published($this->is_published);
    print("</td></tr>\n");

    // filename
    $html = htmlspecialchars($this->filename, ENT_QUOTES);
    if ($this->filename_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Filename:</th><td>");
    print("<input type='text' name='filename' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // create_date
    $html = htmlspecialchars($this->create_date, ENT_QUOTES);
    if ($this->create_date_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Create Date:</th><td>");
    print("<input type='text' name='create_date' "
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
  // 'articlefile::load()' - Load an Article File object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM articlefile WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->article_id = $row["article_id"];
    $this->is_published = $row["is_published"];
    $this->filename = $row["filename"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'articlefile::loadform()' - Load an Article File object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if (array_key_exists("article_id", $_GET))
      $this->article_id = $_GET["article_id"];
    else if (array_key_exists("article_id", $_POST))
      $this->article_id = $_POST["article_id"];

    if ($LOGIN_LEVEL < AUTH_DEVEL)
      $this->is_published = 0;
    else if (array_key_exists("is_published", $_GET))
      $this->is_published = $_GET["is_published"];
    else if (array_key_exists("is_published", $_POST))
      $this->is_published = $_POST["is_published"];

    if (array_key_exists("filename", $_GET))
      $this->filename = $_GET["filename"];
    else if (array_key_exists("filename", $_POST))
      $this->filename = $_POST["filename"];

    return ($this->validate());
  }


  //
  // 'articlefile::save()' - Save an Article File object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    if ($this->id > 0)
    {
      return (db_query("UPDATE articlefile "
                      ." SET article_id = $this->article_id"
                      .", is_published = $this->is_published"
                      .", filename = '" . db_escape($this->filename) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO articlefile VALUES"
                  ."(NULL"
                  .", $this->article_id"
                  .", $this->is_published"
                  .", '" . db_escape($this->filename) . "'"
                  .", $this->create_date"
                  .", '" . db_escape($this->create_user) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'articlefile::search()' - Get a list of Article File objects.
  //

  function				// O - Array of Article File objects
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

          $query .= "${subpre}filename LIKE \"%$word%\"";
          $subpre = " OR ";

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
    $result  = db_query("SELECT id FROM articlefile$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'articlefile::validate()' - Validate the current Article File object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->filename == "")
    {
      $this->filename_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->filename_valid = TRUE;

    return ($valid);
  }


  //
  // 'articlefile::view()' - View the current Article File object.
  //

  function
  view()
  {
    print("<p><table class='view'>\n");

    // article_id
    $html = htmlspecialchars($this->article_id, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Article Id:</th><td>");
    print($html);
    print("</td></tr>\n");

    // is_published
    print("<tr><th align='right' valign='top' nowrap>Published:</th><td>");
    if ($this->is_published)
      print("Yes");
    else
      print("No");
    print("</td></tr>\n");

    // filename
    $html = htmlspecialchars($this->filename, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Filename:</th><td>");
    print($html);
    print("</td></tr>\n");

    // create_date
    $html = date("H:i M d, Y", $this->create_date);
    print("<tr><th align='right' valign='top' nowrap>Create Date:</th><td>");
    print($html);
    print("</td></tr>\n");

    // create_user
    $html = htmlspecialchars($this->create_user, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Create User:</th><td>");
    print($html);
    print("</td></tr>\n");
    print("</table></p>\n");
  }
}


//
// End of "$Id: db-articlefile.php,v 1.5 2006/07/11 13:55:14 mike Exp $".
//
?>
