<?php
//
// "$Id: db-strtext.php,v 1.5 2004/10/16 00:56:16 mike Exp $"
//
// Class for the strtext table.
//
// Contents:
//
//   strtext::strtext()		- Create an STR Text object.
//   strtext::clear()		- Initialize a new an STR Text object.
//   strtext::delete()		- Delete an STR Text object.
//   strtext::edit()		- Display a form for an STR Text object.
//   strtext::load()		- Load an STR Text object.
//   strtext::loadform()	- Load an STR Text object from form data.
//   strtext::save()		- Save an STR Text object.
//   strtext::search()		- Get a list of STR Text IDs.
//   strtext::validate()	- Validate the current STR Text object values.
//   strtext::view()		- View the current STR Text object.
//

include_once "html.php";


class strtext
{
  //
  // Instance variables...
  //

  var $id;
  var $str_id;
  var $is_published;
  var $contents, $contents_valid;
  var $create_date;
  var $create_user;


  //
  // 'strtext::strtext()' - Create an STR Text object.
  //

  function				// O - New STR Text object
  strtext($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'strtext::clear()' - Initialize a new an STR Text object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->str_id = 0;
    $this->is_published = 0;
    $this->contents = "";
    $this->create_date = 0;
    $this->create_user = "";
  }


  //
  // 'strtext::delete()' - Delete an STR Text object.
  //

  function
  delete()
  {
    db_query("DELETE FROM strtext WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'strtext::edit()' - Display a form for an STR Text object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit STR Text";
    else
      $action = "Modify STR Text #$this->id";

    print("<h1>$action</h1>\n"
         ."<form action='$PHP_SELF?U$this->id' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // is_published
    $html = htmlspecialchars($this->is_published, ENT_QUOTES);
    print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
    html_select_is_published($this->is_published);
    print("</td></tr>\n");

    // contents
    $html = htmlspecialchars($this->contents, ENT_QUOTES);
    if ($this->contents_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Contents:</th><td>");
    print("<textarea name='contents' cols='72' rows='10' wrap='virtual'>$html</textarea>\n"
         ."<p>May contain the following HTML elements: "
         ."<tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, "
         ."<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, "
         ."<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, "
         ."<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, "
         ."<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p>\n");
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
  // 'strtext::load()' - Load an STR Text object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM strtext WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->str_id = $row["str_id"];
    $this->is_published = $row["is_published"];
    $this->contents = $row["contents"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'strtext::save()' - Save an STR Text object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    if ($this->id > 0)
    {
      return (db_query("UPDATE strtext "
                      ." SET str_id = $this->str_id"
                      .", is_published = $this->is_published"
                      .", contents = '" . db_escape($this->contents) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO strtext VALUES"
                  ."(NULL"
                  .", $this->str_id"
                  .", $this->is_published"
                  .", '" . db_escape($this->contents) . "'"
                  .", $this->create_date"
                  .", '" . db_escape($this->create_user) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'strtext::search()' - Get a list of STR Text objects.
  //

  function				// O - Array of STR Text objects
  search($str_id)			// I - STR #
  {
    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM strtext WHERE str_id = $str_id "
                       ."ORDER BY create_date");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }

  //
  // 'strtext::validate()' - Validate the current STR Text object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->contents == "")
    {
      $this->contents_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->contents_valid = TRUE;

    return ($valid);
  }


  //
  // 'strtext::view()' - View the current STR Text object.
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

    // contents
    $html = html_format($this->contents);
    print("<tr><th align='right' valign='top' nowrap>Contents:</th><td>");
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
// End of "$Id: db-strtext.php,v 1.5 2004/10/16 00:56:16 mike Exp $".
//
?>
