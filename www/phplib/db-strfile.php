<?php
//
// "$Id: db-strfile.php,v 1.5 2004/10/16 00:56:16 mike Exp $"
//
// Class for the strfile table.
//
// Contents:
//
//   strfile::strfile()		- Create an STR File object.
//   strfile::clear()		- Initialize a new an STR File object.
//   strfile::delete()		- Delete an STR File object.
//   strfile::edit()		- Display a form for an STR File object.
//   strfile::load()		- Load an STR File object.
//   strfile::loadform()	- Load an STR File object from form data.
//   strfile::save()		- Save an STR File object.
//   strfile::search()		- Get a list of STR File IDs.
//   strfile::validate()	- Validate the current STR File object values.
//   strfile::view()		- View the current STR File object.
//

include_once "html.php";


class strfile
{
  //
  // Instance variables...
  //

  var $id;
  var $str_id;
  var $is_published;
  var $filename, $filename_valid;
  var $create_date;
  var $create_user;


  //
  // 'strfile::strfile()' - Create an STR File object.
  //

  function				// O - New STR File object
  strfile($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'strfile::clear()' - Initialize a new an STR File object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->str_id = 0;
    $this->is_published = 0;
    $this->filename = "";
    $this->create_date = 0;
    $this->create_user = "";
  }


  //
  // 'strfile::delete()' - Delete an STR File object.
  //

  function
  delete()
  {
    db_query("DELETE FROM strfile WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'strfile::edit()' - Display a form for an STR File object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit STR File";
    else
      $action = "Modify STR File #$this->id";

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
  // 'strfile::load()' - Load an STR File object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM strfile WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->str_id = $row["str_id"];
    $this->is_published = $row["is_published"];
    $this->filename = $row["filename"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'strfile::save()' - Save an STR File object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    if ($this->id > 0)
    {
      return (db_query("UPDATE strfile "
                      ." SET str_id = $this->str_id"
                      .", is_published = $this->is_published"
                      .", filename = '" . db_escape($this->filename) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO strfile VALUES"
                  ."(NULL"
                  .", $this->str_id"
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
  // 'strfile::search()' - Get a list of STR File objects.
  //

  function				// O - Array of STR File objects
  search($str_id)			// I - STR #
  {
    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM strfile WHERE str_id = $str_id "
                       ."ORDER BY create_date");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'strfile::validate()' - Validate the current STR File object values.
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
  // 'strfile::view()' - View the current STR File object.
  //

  function
  view()
  {
    print("<p><table class='view'>\n");

    // str_id
    $html = htmlspecialchars($this->str_id, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Str Id:</th><td>");
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
// End of "$Id: db-strfile.php,v 1.5 2004/10/16 00:56:16 mike Exp $".
//
?>
