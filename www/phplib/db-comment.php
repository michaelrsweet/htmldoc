<?php
//
// "$Id: db-comment.php,v 1.6 2006/07/11 13:55:14 mike Exp $"
//
// Class for the comment table.
//
// Contents:
//
//   comment::comment()		- Create a comment object.
//   comment::clear()		- Initialize a new a comment object.
//   comment::delete()		- Delete a comment object.
//   comment::edit()		- Display a form for a comment object.
//   comment::load()		- Load a comment object.
//   comment::loadform()	- Load a comment object from form data.
//   comment::save()		- Save a comment object.
//   comment::search()		- Get a list of comment IDs.
//   comment::validate()	- Validate the current comment object values.
//   comment::view()		- View the current comment object.
//

include_once "html.php";


class comment
{
  //
  // Instance variables...
  //

  var $id;
  var $parent_id, $parent_id_valid;
  var $status, $status_valid;
  var $url, $url_valid;
  var $contents, $contents_valid;
  var $create_date;
  var $create_user;
  var $display_name;


  //
  // 'comment::comment()' - Create a comment object.
  //

  function				// O - New Comment object
  comment($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'comment::clear()' - Initialize a new a comment object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->parent_id = 0;
    $this->status = 3;
    $this->url = "";
    $this->contents = "";
    $this->create_date = 0;
    $this->create_user = "";
    $this->display_name = "";
  }


  //
  // 'comment::delete()' - Delete a comment object.
  //

  function
  delete()
  {
    db_query("DELETE FROM comment WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'comment::edit()' - Display a form for a comment object.
  //

  function
  edit($options = "")			// I - Options
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF, $_POST, $REQUEST_METHOD;


    if ($REQUEST_METHOD == "POST")
    {
      if (!$this->id)
	$this->create_date = time();

      $this->view();
    }

    if ($this->id <= 0)
      $action = "Submit Comment";
    else
      $action = "Modify Comment #$this->id";

    print("<form action='$PHP_SELF?U$this->id$options' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // display_name
    print("<tr><th class='valid' align='right' valign='top' nowrap>From:</th><td>");
    print("$LOGIN_USER <input type='checkbox' name='anonymous'");
    if ($this->display_name == "Anonymous")
      print(" checked");
    print("/>Post Anonymously</td></tr>\n");
        
    // contents
    $html = htmlspecialchars($this->contents, ENT_QUOTES);
    if ($this->contents_valid || $REQUEST_METHOD == "GET")
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Comment:</th><td>");
    print("<textarea name='contents' cols='72' rows='10' wrap='virtual'>$html</textarea>\n"
         ."<p>May contain the following HTML elements: "
         ."<tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, "
         ."<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, "
         ."<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, "
         ."<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, "
         ."<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p>\n");
    print("</td></tr>\n");

    // status
    $html = htmlspecialchars($this->status, ENT_QUOTES);
    if ($this->status_valid || $REQUEST_METHOD == "GET")
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Moderation Score:</th><td>");

    if ($LOGIN_LEVEL >= AUTH_DEVEL)
      print("<input type='text' name='status' value='$html' size='1'/>");
    else
      print("$html");

    print("</td></tr>\n");

    // url
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      $html = htmlspecialchars($this->url, ENT_QUOTES);
      if ($this->url_valid || $REQUEST_METHOD == "GET")
	$hclass = "valid";
      else
	$hclass = "invalid";
      print("<tr><th class='$hclass' align='right' valign='top' nowrap>URL:</th><td>");
      print("<input type='text' name='url' "
           ."value='$html' size='72'/>");
      print("</td></tr>\n");
    }

    // Submit
    print("<tr><td></td><td>"
         ."<input type='submit' name='preview' value='Preview Comment'/> "
         ."<input type='submit' value='$action'/>"
         ."</td></tr>\n"
         ."</table></p>\n"
         ."</form>\n");
  }


  //
  // 'comment::load()' - Load a comment object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM comment WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->parent_id = $row["parent_id"];
    $this->status = $row["status"];
    $this->url = $row["url"];
    $this->contents = $row["contents"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];
    $this->display_name = $row["display_name"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'comment::loadform()' - Load a comment object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_USER, $LOGIN_LEVEL;


    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      if (array_key_exists("status", $_GET))
	$this->status = $_GET["status"];
      else if (array_key_exists("status", $_POST))
	$this->status = $_POST["status"];

      if (array_key_exists("url", $_GET))
	$this->url = $_GET["url"];
      else if (array_key_exists("url", $_POST))
	$this->url = $_POST["url"];
    }

    if (array_key_exists("contents", $_GET))
      $this->contents = $_GET["contents"];
    else if (array_key_exists("contents", $_POST))
      $this->contents = $_POST["contents"];

    if (array_key_exists("anonymous", $_GET) ||
        array_key_exists("anonymous", $_POST))
      $this->display_name = "Anonymous";
    else
      $this->display_name = $LOGIN_USER;

    return ($this->validate());
  }


  //
  // 'comment::save()' - Save a comment object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    if ($this->id > 0)
    {
      return (db_query("UPDATE comment "
                      ." SET parent_id = $this->parent_id"
                      .", status = $this->status"
                      .", display_name = '" . db_escape($this->display_name) . "'"
                      .", url = '" . db_escape($this->url) . "'"
                      .", contents = '" . db_escape($this->contents) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO comment VALUES"
                  ."(NULL"
                  .", $this->parent_id"
                  .", $this->status"
                  .", '" . db_escape($this->url) . "'"
                  .", '" . db_escape($this->contents) . "'"
                  .", $this->create_date"
                  .", '" . db_escape($this->create_user) . "'"
                  .", '" . db_escape($this->display_name) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'comment::search()' - Get a list of comment objects.
  //

  function				// O - Array of Comment objects
  search($search = "",			// I - Search string
         $order = "",			// I - Order fields
	 $status = -1)			// I - Comment status
  {
    $query  = "";
    $prefix = " WHERE ";

    if ($status >= 0)
    {
      $query .= "${prefix}status = $status";
      $prefix = " AND ";
    }

    if ($search != "")
    {
      // Convert the search string to an array of words...
      $words = html_search_words($search);

      // Loop through the array of words, adding them to the query...
      $query .= "${prefix}(";
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

          $query .= "${subpre}url LIKE \"%$word%\"";
          $subpre = " OR ";
          $query .= "${subpre}contents LIKE \"%$word%\"";
          $query .= "${subpre}create_user LIKE \"%$word%\"";
          $query .= "${subpre}display_name LIKE \"%$word%\"";

          $query .= ")";
          $prefix = $next;
          $logic  = '';
        }
      }

      $query .= ")";
    }

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

//    print("<p>query='$query'</p>\n");

    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM comment$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'comment::validate()' - Validate the current comment object values.
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

    if ($this->contents == "")
    {
      $this->contents_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->contents_valid = TRUE;

    if ($this->status < 0 || $this->status > 5)
    {
      $this->status_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->status_valid = TRUE;

    return ($valid);
  }


  //
  // 'comment::view()' - View the current comment object.
  //

  function
  view()
  {
    $create_date  = date("H:i M d, Y", $this->create_date);
    $display_name = htmlspecialchars($this->display_name);
    $contents     = html_format($this->contents);
    $url          = str_replace("_", "?", $this->url);

    print("<h2><a name='_USER_COMMENT_$this->id'>From</a> "
         ."$display_name, $create_date (score=$this->status)</h2>\n"
	 ."$contents\n"
	 ."<p>[ <tt><a href='$url'>$url</a></tt> ]</p>\n");
  }
}


//
// End of "$Id: db-comment.php,v 1.6 2006/07/11 13:55:14 mike Exp $".
//
?>
