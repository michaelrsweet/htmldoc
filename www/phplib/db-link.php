<?php
//
// "$Id: db-link.php,v 1.6 2006/07/11 13:55:14 mike Exp $"
//
// Class for the link table.
//
// Contents:
//
//   link::link()	  - Create an Link object.
//   link::clear()	  - Initialize a new an Link object.
//   link::delete()	  - Delete an Link object.
//   link::edit()	  - Display a form for an Link object.
//   link::get_category() - Get the category path.
//   link::load()	  - Load an Link object.
//   link::loadform()	  - Load an Link object from form data.
//   link::save()	  - Save an Link object.
//   link::search()	  - Get a list of Link IDs.
//   link::validate()	  - Validate the current Link object values.
//   link::view()	  - View the current Link object.
//

include_once "html.php";


class link
{
  //
  // Instance variables...
  //

  var $id;
  var $parent_id, $parent_id_valid;
  var $is_category, $is_category_valid;
  var $is_published;
  var $name, $name_valid;
  var $version, $version_valid;
  var $license, $license_valid;
  var $author, $author_valid;
  var $email, $email_valid;
  var $homepage_url, $homepage_url_valid;
  var $download_url, $download_url_valid;
  var $description, $description_valid;
  var $rating_total, $rating_total_valid;
  var $rating_count, $rating_count_valid;
  var $homepage_visits, $homepage_visits_valid;
  var $download_visits, $download_visits_valid;
  var $create_date;
  var $create_user;
  var $modify_date;
  var $modify_user;


  //
  // 'link::link()' - Create an Link object.
  //

  function				// O - New Link object
  link($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'link::clear()' - Initialize a new an Link object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->parent_id = 0;
    $this->is_category = 0;
    $this->is_published = 0;
    $this->name = "";
    $this->version = "";
    $this->license = "";
    $this->author = "";
    $this->email = "";
    $this->homepage_url = "";
    $this->download_url = "";
    $this->description = "";
    $this->rating_total = 0;
    $this->rating_count = 0;
    $this->homepage_visits = 0;
    $this->download_visits = 0;
    $this->create_date = 0;
    $this->create_user = "";
    $this->modify_date = 0;
    $this->modify_user = "";
  }


  //
  // 'link::delete()' - Delete an Link object.
  //

  function
  delete()
  {
    db_query("DELETE FROM link WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'link::edit()' - Display a form for an Link object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit Link";
    else
      $action = "Modify Link #$this->id";

    print("<h1>$action</h1>\n"
         ."<form action='$PHP_SELF?U$this->id' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // is_category
    $html = htmlspecialchars($this->is_category, ENT_QUOTES);
    if ($this->is_category_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Is Category:</th><td>");
    print("<input type='text' name='is_category' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // is_published
    $html = htmlspecialchars($this->is_published, ENT_QUOTES);
    print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
    html_select_is_published($this->is_published);
    print("</td></tr>\n");

    // name
    $html = htmlspecialchars($this->name, ENT_QUOTES);
    if ($this->name_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Name:</th><td>");
    print("<input type='text' name='name' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // version
    $html = htmlspecialchars($this->version, ENT_QUOTES);
    if ($this->version_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Version:</th><td>");
    print("<input type='text' name='version' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // license
    $html = htmlspecialchars($this->license, ENT_QUOTES);
    if ($this->license_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>License:</th><td>");
    print("<input type='text' name='license' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // author
    $html = htmlspecialchars($this->author, ENT_QUOTES);
    if ($this->author_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Author:</th><td>");
    print("<input type='text' name='author' "
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

    // homepage_url
    $html = htmlspecialchars($this->homepage_url, ENT_QUOTES);
    if ($this->homepage_url_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Homepage Url:</th><td>");
    print("<input type='text' name='homepage_url' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // download_url
    $html = htmlspecialchars($this->download_url, ENT_QUOTES);
    if ($this->download_url_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Download Url:</th><td>");
    print("<input type='text' name='download_url' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // description
    $html = htmlspecialchars($this->description, ENT_QUOTES);
    if ($this->description_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Description:</th><td>");
    print("<textarea name='description' cols='72' rows='10' wrap='virtual'>$html</textarea>\n"
         ."<p>May contain the following HTML elements: "
         ."<tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, "
         ."<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, "
         ."<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, "
         ."<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, "
         ."<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p>\n");
    print("</td></tr>\n");

    // rating_total
    $html = htmlspecialchars($this->rating_total, ENT_QUOTES);
    if ($this->rating_total_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Rating Total:</th><td>");
    print("<input type='text' name='rating_total' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // rating_count
    $html = htmlspecialchars($this->rating_count, ENT_QUOTES);
    if ($this->rating_count_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Rating Count:</th><td>");
    print("<input type='text' name='rating_count' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // homepage_visits
    $html = htmlspecialchars($this->homepage_visits, ENT_QUOTES);
    if ($this->homepage_visits_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Homepage Visits:</th><td>");
    print("<input type='text' name='homepage_visits' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // download_visits
    $html = htmlspecialchars($this->download_visits, ENT_QUOTES);
    if ($this->download_visits_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Download Visits:</th><td>");
    print("<input type='text' name='download_visits' "
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
  // 'link::get_category()' - Get the category path.
  //

  function				// O - Category path
  get_category($id,			// I - Category ID
               $with_links = 2)		// I - 0 = no links, 1 = all links,
					//     2 = all but current
  {
    global $PHP_SELF;


    if ($id == 0)
    {
      if ($with_links == 1)
	return "<a href='$PHP_SELF?L+P0'>Root</a>";
      else
	return "Root";
    }
    else if ($id < 0)
    {
      return "All";
    }

    $result   = db_query("SELECT name, id, parent_id FROM link WHERE id = $id");
    $category = "";
    $sublinks = $with_links ? 1 : 0;

    if ($result)
    {
      $row = db_next($result);

      if ($row)
      {
	if ($with_links || $row['parent_id'] > 0)
          $category = $this->get_category($row['parent_id'], $sublinks) . "/";

	if ($with_links == 1)
          $category .= "<a href='$PHP_SELF?L+P$row[id]'>"
	             . htmlspecialchars($row['name']) . "</a>";
	else
          $category .= htmlspecialchars($row['name']);
      }

      db_free($result);
    }

    return ($category);
  }


  //
  // 'link::load()' - Load an Link object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM link WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->parent_id = $row["parent_id"];
    $this->is_category = $row["is_category"];
    $this->is_published = $row["is_published"];
    $this->name = $row["name"];
    $this->version = $row["version"];
    $this->license = $row["license"];
    $this->author = $row["author"];
    $this->email = $row["email"];
    $this->homepage_url = $row["homepage_url"];
    $this->download_url = $row["download_url"];
    $this->description = $row["description"];
    $this->rating_total = $row["rating_total"];
    $this->rating_count = $row["rating_count"];
    $this->homepage_visits = $row["homepage_visits"];
    $this->download_visits = $row["download_visits"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];
    $this->modify_date = $row["modify_date"];
    $this->modify_user = $row["modify_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'link::loadform()' - Load an Link object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if (array_key_exists("parent_id", $_GET))
      $this->parent_id = $_GET["parent_id"];
    else if (array_key_exists("parent_id", $_POST))
      $this->parent_id = $_POST["parent_id"];

    if (array_key_exists("is_category", $_GET))
      $this->is_category = $_GET["is_category"];
    else if (array_key_exists("is_category", $_POST))
      $this->is_category = $_POST["is_category"];

    if ($LOGIN_LEVEL < AUTH_DEVEL)
      $this->is_published = 0;
    else if (array_key_exists("is_published", $_GET))
      $this->is_published = $_GET["is_published"];
    else if (array_key_exists("is_published", $_POST))
      $this->is_published = $_POST["is_published"];

    if (array_key_exists("name", $_GET))
      $this->name = $_GET["name"];
    else if (array_key_exists("name", $_POST))
      $this->name = $_POST["name"];

    if (array_key_exists("version", $_GET))
      $this->version = $_GET["version"];
    else if (array_key_exists("version", $_POST))
      $this->version = $_POST["version"];

    if (array_key_exists("license", $_GET))
      $this->license = $_GET["license"];
    else if (array_key_exists("license", $_POST))
      $this->license = $_POST["license"];

    if (array_key_exists("author", $_GET))
      $this->author = $_GET["author"];
    else if (array_key_exists("author", $_POST))
      $this->author = $_POST["author"];

    if (array_key_exists("email", $_GET))
      $this->email = $_GET["email"];
    else if (array_key_exists("email", $_POST))
      $this->email = $_POST["email"];

    if (array_key_exists("homepage_url", $_GET))
      $this->homepage_url = $_GET["homepage_url"];
    else if (array_key_exists("homepage_url", $_POST))
      $this->homepage_url = $_POST["homepage_url"];

    if (array_key_exists("download_url", $_GET))
      $this->download_url = $_GET["download_url"];
    else if (array_key_exists("download_url", $_POST))
      $this->download_url = $_POST["download_url"];

    if (array_key_exists("description", $_GET))
      $this->description = $_GET["description"];
    else if (array_key_exists("description", $_POST))
      $this->description = $_POST["description"];

    if (array_key_exists("rating_total", $_GET))
      $this->rating_total = $_GET["rating_total"];
    else if (array_key_exists("rating_total", $_POST))
      $this->rating_total = $_POST["rating_total"];

    if (array_key_exists("rating_count", $_GET))
      $this->rating_count = $_GET["rating_count"];
    else if (array_key_exists("rating_count", $_POST))
      $this->rating_count = $_POST["rating_count"];

    if (array_key_exists("homepage_visits", $_GET))
      $this->homepage_visits = $_GET["homepage_visits"];
    else if (array_key_exists("homepage_visits", $_POST))
      $this->homepage_visits = $_POST["homepage_visits"];

    if (array_key_exists("download_visits", $_GET))
      $this->download_visits = $_GET["download_visits"];
    else if (array_key_exists("download_visits", $_POST))
      $this->download_visits = $_POST["download_visits"];

    return ($this->validate());
  }


  //
  // 'link::save()' - Save an Link object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    $this->modify_date = time();
    $this->modify_user = $LOGIN_USER;

    if ($this->id > 0)
    {
      return (db_query("UPDATE link "
                      ." SET parent_id = $this->parent_id"
                      .", is_category = $this->is_category"
                      .", is_published = $this->is_published"
                      .", name = '" . db_escape($this->name) . "'"
                      .", version = '" . db_escape($this->version) . "'"
                      .", license = '" . db_escape($this->license) . "'"
                      .", author = '" . db_escape($this->author) . "'"
                      .", email = '" . db_escape($this->email) . "'"
                      .", homepage_url = '" . db_escape($this->homepage_url) . "'"
                      .", download_url = '" . db_escape($this->download_url) . "'"
                      .", description = '" . db_escape($this->description) . "'"
                      .", rating_total = $this->rating_total"
                      .", rating_count = $this->rating_count"
                      .", homepage_visits = $this->homepage_visits"
                      .", download_visits = $this->download_visits"
                      .", modify_date = $this->modify_date"
                      .", modify_user = '" . db_escape($this->modify_user) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO link VALUES"
                  ."(NULL"
                  .", $this->parent_id"
                  .", $this->is_category"
                  .", $this->is_published"
                  .", '" . db_escape($this->name) . "'"
                  .", '" . db_escape($this->version) . "'"
                  .", '" . db_escape($this->license) . "'"
                  .", '" . db_escape($this->author) . "'"
                  .", '" . db_escape($this->email) . "'"
                  .", '" . db_escape($this->homepage_url) . "'"
                  .", '" . db_escape($this->download_url) . "'"
                  .", '" . db_escape($this->description) . "'"
                  .", $this->rating_total"
                  .", $this->rating_count"
                  .", $this->homepage_visits"
                  .", $this->download_visits"
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
  // 'link::search()' - Get a list of Link objects.
  //

  function				// O - Array of Link objects
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

          $query .= "${subpre}name LIKE \"%$word%\"";
          $subpre = " OR ";
          $query .= "${subpre}version LIKE \"%$word%\"";
          $query .= "${subpre}license LIKE \"%$word%\"";
          $query .= "${subpre}author LIKE \"%$word%\"";
          $query .= "${subpre}email LIKE \"%$word%\"";
          $query .= "${subpre}homepage_url LIKE \"%$word%\"";
          $query .= "${subpre}download_url LIKE \"%$word%\"";
          $query .= "${subpre}description LIKE \"%$word%\"";

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
    $result  = db_query("SELECT id FROM link$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'link::validate()' - Validate the current Link object values.
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

    if ($this->version == "")
    {
      $this->version_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->version_valid = TRUE;

    if ($this->license == "")
    {
      $this->license_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->license_valid = TRUE;

    if ($this->author == "")
    {
      $this->author_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->author_valid = TRUE;

    if ($this->email == "")
    {
      $this->email_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->email_valid = TRUE;

    if ($this->homepage_url == "")
    {
      $this->homepage_url_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->homepage_url_valid = TRUE;

    if ($this->download_url == "")
    {
      $this->download_url_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->download_url_valid = TRUE;

    if ($this->description == "")
    {
      $this->description_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->description_valid = TRUE;

    return ($valid);
  }


  //
  // 'link::view()' - View the current Link object.
  //

  function
  view()
  {
    print("<p><table class='view'>\n");

    // parent_id
    $html = htmlspecialchars($this->parent_id, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Parent Id:</th><td>");
    print($html);
    print("</td></tr>\n");

    // is_category
    $html = htmlspecialchars($this->is_category, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Is Category:</th><td>");
    print($html);
    print("</td></tr>\n");

    // is_published
    print("<tr><th align='right' valign='top' nowrap>Published:</th><td>");
    if ($this->is_published)
      print("Yes");
    else
      print("No");
    print("</td></tr>\n");

    // name
    $html = htmlspecialchars($this->name, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Name:</th><td>");
    print($html);
    print("</td></tr>\n");

    // version
    $html = htmlspecialchars($this->version, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Version:</th><td>");
    print($html);
    print("</td></tr>\n");

    // license
    $html = htmlspecialchars($this->license, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>License:</th><td>");
    print($html);
    print("</td></tr>\n");

    // author
    $html = htmlspecialchars($this->author, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Author:</th><td>");
    print($html);
    print("</td></tr>\n");

    // email
    $html = htmlspecialchars($this->email, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Email:</th><td>");
    print($html);
    print("</td></tr>\n");

    // homepage_url
    $html = htmlspecialchars($this->homepage_url, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Homepage Url:</th><td>");
    print($html);
    print("</td></tr>\n");

    // download_url
    $html = htmlspecialchars($this->download_url, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Download Url:</th><td>");
    print($html);
    print("</td></tr>\n");

    // description
    $html = html_format($this->description);
    print("<tr><th align='right' valign='top' nowrap>Description:</th><td>");
    print($html);
    print("</td></tr>\n");

    // rating_total
    $html = htmlspecialchars($this->rating_total, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Rating Total:</th><td>");
    print($html);
    print("</td></tr>\n");

    // rating_count
    $html = htmlspecialchars($this->rating_count, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Rating Count:</th><td>");
    print($html);
    print("</td></tr>\n");

    // homepage_visits
    $html = htmlspecialchars($this->homepage_visits, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Homepage Visits:</th><td>");
    print($html);
    print("</td></tr>\n");

    // download_visits
    $html = htmlspecialchars($this->download_visits, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Download Visits:</th><td>");
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

    // modify_date
    $html = date("H:i M d, Y", $this->modify_date);
    print("<tr><th align='right' valign='top' nowrap>Modify Date:</th><td>");
    print($html);
    print("</td></tr>\n");

    // modify_user
    $html = htmlspecialchars($this->modify_user, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Modify User:</th><td>");
    print($html);
    print("</td></tr>\n");
    print("</table></p>\n");
  }
}


//
// End of "$Id: db-link.php,v 1.6 2006/07/11 13:55:14 mike Exp $".
//
?>
