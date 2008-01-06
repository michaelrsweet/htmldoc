<?
//
// "$Id: links.php,v 1.9 2005/03/13 04:39:34 mike Exp $"
//
// Hierarchical link interface.
//
// Contents:
//
//   select_category() - Get a list of all categories.
//


//
// Include necessary headers...
//

include_once "phplib/db-article.php";


//
// 'select_category()' - Get a list of all categories.
//

function
select_category($parent_id = 0,		// I - Parent ID
                $is_category = 0)	// I - Selecting for category?
{
  // Scan the table for categories... We add "C" to the ID to
  // avoid PHP thinking we want an actual index in the array.
  $result = db_query("SELECT name,id FROM link "
                    ."WHERE is_published != 0 AND is_category != 0 "
		    ."ORDER BY name");

  $cats = array();
  $link = new link();

  while ($row = db_next($result))
    $cats["C$row[id]"] = $link->get_category($row['id'], 0);

  db_free($result);

  // Add the Root category if we are adding or modifying a category.
  if ($is_category)
    $cats["C0"] = "Root";

  // Sort the category list...
  asort($cats);

  // List the categories for selection...
  print("<select name='PARENT_ID'>");

  reset($cats);
  while (list($cat_id, $cat_name) = each($cats))
  {
    $cat_id   = (int)substr($cat_id, 1);
    $cat_name = htmlspecialchars($cat_name);

    if ($cat_id == $parent_id)
      print("<option value='$cat_id' selected>$cat_name</option>");
    else
      print("<option value='$cat_id'>$cat_name</option>");
  }

  print("</select>");
}


// Set globals...
$id        = 0;
$parent_id = 0;
$query     = '';

if ($LOGIN_LEVEL >= AUTH_DEVEL)
{
  $op = 'Z';
}
else
{
  $op = 'L';
}

// Get command-line options...
//
// Usage: links.php [operation] [options]
//
// Operations:
//
// LA        = List all links
// LC        = List links by category
// LM        = List all links owned by login user
// LU        = List unpublished links
// R#        = Rate listing #
// SH#       = Show homepage for listing #
// SD#       = Show download for listing #
// UC        = Add new category
// UC#       = Modify category #
// UL        = Add new listing
// UL#       = Modify listing #
// V#        = View listing #
// X#        = Delete category or listing #
//
// Options:
//
// P#        = Set parent ID
// Qtext     = Set search text

$search   = "";
$op       = "L";
$listtype = "C";

for ($i = 0; $i < $argc; $i ++)
{
  switch ($argv[$i][0])
  {
    case 'L' : // List or search
        $op = 'L';

	if (strlen($argv[$i]) > 1)
	{
	  $listtype = $argv[$i][1];

	  if ($listtype != 'C')
	    $parent_id = -1;
          else if ($listtype == "M" && $LOGIN_USER == "")
	  {
	    header("Location: login.php?PAGE=links.php?LM");
	    return;
	  }
	}
	break;

    case 'P' : // Parent
	$parent_id = (int)substr($argv[$i], 1);
	break;

    case 'Q' : // Set search text
        $search = substr($argv[$i], 1);
	$i ++;
	while ($i < $argc)
	{
	  $search .= " $argv[$i]";
	  $i ++;
	}
	break;

    case 'R' : // Rate
        $op = $argv[$i][0];
	$id = (int)substr($argv[$i], 1);
	break;

    case 'S' : // Show web or download page
        $op   = $argv[$i][0];
	$type = $argv[$i][1];
	$id   = (int)substr($argv[$i], 2);
	break;

    case 'U' : // Update/add
        $op   = $argv[$i][0];
	$type = $argv[$i][1];
	$id   = (int)substr($argv[$i], 2);
	break;

    case 'V' : // View
        $op = 'V';
	$id = (int)substr($argv[$i], 1);
	break;

    case 'X' : // Delete
        $op = 'X';
	$id = (int)substr($argv[$i], 1);
	break;

    default :
        header("Location: $PHP_SELF");
	exit();
  }
}

if (array_key_exists("SEARCH", $_POST))
  $search = $_POST["SEARCH"];

// Encode the search parameters so they can be propagated...
$options = "+Q" . urlencode($search);

// Now do operation..
switch ($op)
{
  case 'L' : // List...
      $link  = new link();
      $links = array();

      $links["Show All Listings"] = "links.php?LA$options";
      $links["Show Listings By Category"] = "links.php?LC$options";
      if ($LOGIN_USER != "")
        $links["Show My Listings"] = "links.php?LM$options";
      if ($LOGIN_LEVEL >= AUTH_DEVEL)
        $links["Show Unpublished Listings"] = "links.php?LU$options";

      html_header("Links", "", "", $links);

      print("<form method='POST' action='$PHP_SELF?L$listtype'>\n"
	   ."<center>Search:&nbsp;"
	   ."<input type='text' name='SEARCH' size='40' value='$search'/>"
	   ."<input type='submit' value='Search Links'/>"
	   ."</center>\n"
	   ."</form>\n"
	   ."<hr noshade/>\n");

      if ($search != "")
      {
	// Construct a query...
	$search_words = html_search_words($search);

	// Loop through the array of words, adding them to the 
	$query  = "";
	$prefix = "";
	$next   = "OR";
	$logic  = "";

	reset($search_words);
	foreach ($search_words as $keyword)
	{
	  if ($keyword == "or")
	  {
	    $next = 'OR';
	    if ($prefix != '')
              $prefix = 'OR';
	  }
	  else if ($keyword == "and")
	  {
	    $next = 'AND';
	    if ($prefix != '')
              $prefix = 'AND';
	  }
          else if ($keyword == "not")
            $logic = ' NOT';
	  else
	  {
	    $query  .= " $prefix$logic (name LIKE '%$keyword%'"
	              ." OR author LIKE '%$keyword%'"
		      ." OR description LIKE '%$keyword%')";
	    $prefix = $next;
	  }
	}
      }

      if ($search == "")
        $category = $link->get_category($parent_id);
      else
        $category = "Search";

      if ($listtype == 'U')
        $is_published = "is_published = 0 AND ";
      else if ($LOGIN_LEVEL >= AUTH_DEVEL)
        $is_published = "";
      else
        $is_published = "is_published = 1 AND ";

      // Show the categories...
      if ($query != "")
        $result = db_query("SELECT id FROM link "
                          ."WHERE ${is_published}is_category = 1 AND "
			  ."($query) "
			  ."ORDER BY name");
      else if ($parent_id >= 0)
        $result = db_query("SELECT id FROM link "
                          ."WHERE ${is_published}is_category = 1 AND "
			  ."parent_id = $parent_id "
			  ."ORDER BY name");
      else if ($listtype == "M")
        $result = db_query("SELECT id FROM link "
                          ."WHERE create_user = '$LOGIN_USER' AND "
			  ."is_category = 1 "
			  ."ORDER BY name");
      else
        $result = db_query("SELECT id FROM link "
                          ."WHERE ${is_published}is_category = 1 "
			  ."ORDER BY name");

      if ($listtype == "M")
	print("<h2>My Categories</h2>\n");
      else if ($parent_id < 0)
	print("<h2>All Categories</h2>\n");
      else
	print("<h2>Categories in $category</h2>\n");

      html_start_links();
      html_link("Submit New Category", "$PHP_SELF?UC+P$parent_id$options");
      html_end_links();

      print("<ul>\n");

      while ($row = db_next($result))
      {
        $id = $row["id"];

	$link->load($id);

	$name = htmlspecialchars($link->name);

        print("<li><a href='$PHP_SELF?L$listtype+P$id$options'>$name</a>");

        if (!$link->is_published)
	  print(" <img src='images/private.gif' width='16' height='16' "
	       ."align='middle' alt='private'/>");

        if ($LOGIN_LEVEL >= AUTH_DEVEL || $LOGIN_USER == $link->create_user)
	{
	  print(" [&nbsp;<a href='$PHP_SELF?UC$id+P$parent_id$options'>Edit</a> |"
	       ." <a href='$PHP_SELF?X$id+P$parent_id$options'>Delete</a>&nbsp;]");
	}

	print("</li>\n");
      }

      print("</ul>\n");

      db_free($result);

      // Then show the listings...
      if ($parent_id != 0 || $query != "")
      {
	if ($query != "")
          $result = db_query("SELECT id FROM link "
                            ."WHERE ${is_published}is_category = 0 AND "
			    ."($query) "
			    ."ORDER BY name");
	else if ($parent_id >= 0)
          $result = db_query("SELECT id FROM link "
                            ."WHERE ${is_published}is_category = 0 AND "
			    ."parent_id = $parent_id "
			    ."ORDER BY name");
	else if ($listtype == "M")
          $result = db_query("SELECT id FROM link "
                            ."WHERE create_user = '$LOGIN_USER' AND "
			    ."is_category = 0 "
			    ."ORDER BY name");
	else
          $result = db_query("SELECT id FROM link "
                            ."WHERE ${is_published}is_category = 0 "
			    ."ORDER BY name");

        if ($listtype == "M")
	  print("<h2>My Listings</h2>\n");
	else if ($parent_id < 0)
	  print("<h2>All Listings</h2>\n");
	else
	  print("<h2>Listings in $category</h2>\n");

	html_start_links();
	html_link("Submit New Listing", "$PHP_SELF?UL+P$parent_id$options");
	html_end_links();

	print("<ul>\n");

	while ($row = db_next($result))
	{
          $id = $row["id"];
	  $link->load($id);

	  $name        = htmlspecialchars($link->name);
	  $description = html_format($link->description);
	  $version     = htmlspecialchars($link->version);
          $age         = (int)((time() - $link->modify_date) / 86400);

          print("<li><b><a href='$PHP_SELF?V$id$options'>$name $version</a></b>");

	  if ($search != "")
	  {
	    $category = $link->get_category($link->parent_id, 1);
	    print(" in $category");
	  }

          if (!$link->is_published)
	    print(" <img src='images/private.gif' width='16' height='16' "
		 ."align='middle' alt='private'/>");

          if ($age == 1)
            print(", <i>Updated 1 day ago</i>");
	  else if ($age < 30)
            print(", <i>Updated $age days ago</i>");

          if ($LOGIN_LEVEL >= AUTH_DEVEL || $LOGIN_USER == $link->create_user)
	  {
	    print(" [&nbsp;<a href='$PHP_SELF?UL$id+P$parent_id$options'>Edit</a>"
		 ." | <a href='$PHP_SELF?X$id+P$parent_id$options'>Delete</a>&nbsp;]\n");
	  }

          print("$description</li>\n");
	}

	print("</ul>\n");

        db_free($result);
      }

      html_footer();
      break;

  case 'U' : // Add or update category or listing...
      if ($LOGIN_USER == "")
      {
        if ($id)
          header("Location: login.php?PAGE=links.php?U$type$id");
	else
          header("Location: login.php?PAGE=links.php?U$type");
	return;
      }

      if ($id > 0)
      {
        // Get current link data from database...
        $result = db_query("SELECT * FROM link WHERE id = $id");
	if (db_count($result) != 1)
	{
	  // Link doesn't exist!
          db_free($result);
	  header("Location: $PHP_SELF");
	  exit();
	}

	$row = db_next($result);

        if ($LOGIN_LEVEL < AUTH_DEVEL && $LOGIN_USER != $row["create_user"])
	{
	  // No permission!
          db_free($result);
	  header("Location: $PHP_SELF");
	  exit();
	}

	$is_category  = $row['is_category'];
	$is_published = $row['is_published'];
	$parent_id    = $row['parent_id'];
	$name         = $row['name'];
	$version      = $row['version'];
	$license      = $row['license'];
	$author       = $row['author'];
	$email        = $row['email'];
	$homepage_url = $row['homepage_url'];
	$download_url = $row['download_url'];
	$description  = $row['description'];

        db_free($result);
      }
      else
      {
        // Use default information for type...
        if ($type == 'C')
	  $is_category = 1;
	else
	  $is_category = 0;

        if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  $is_published = 1;
	else
	  $is_published = 0;

	$name         = "";
	$version      = "";
	$license      = "";
	$author       = "";
	$email        = "";
	$homepage_url = "http://";
	$download_url = "ftp://";
	$description  = "";
      }

      $abstract     = "";
      $announcement = "";

      if ($REQUEST_METHOD == "POST")
      {
        if (array_key_exists("PARENT_ID", $_POST))
	  $parent_id = (int)$_POST["PARENT_ID"];

        if ($LOGIN_LEVEL >= AUTH_DEVEL &&
	    array_key_exists("IS_PUBLISHED", $_POST))
	  $is_published = (int)$_POST["IS_PUBLISHED"];

        if (array_key_exists("NAME", $_POST))
	  $name = $_POST["NAME"];

        if (array_key_exists("VERSION", $_POST))
	  $version = $_POST["VERSION"];

        if (array_key_exists("LICENSE", $_POST))
	  $license = $_POST["LICENSE"];

        if (array_key_exists("AUTHOR", $_POST))
	  $author = $_POST["AUTHOR"];

        if (array_key_exists("EMAIL", $_POST))
	  $email = $_POST["EMAIL"];

        if (array_key_exists("HOMEPAGE_URL", $_POST))
	  $homepage_url = $_POST["HOMEPAGE_URL"];

        if (array_key_exists("DOWNLOAD_URL", $_POST))
	  $download_url = $_POST["DOWNLOAD_URL"];

        if (array_key_exists("DESCRIPTION", $_POST))
	  $description = $_POST["DESCRIPTION"];

        if (array_key_exists("ABSTRACT", $_POST) && $type == 'L')
	  $abstract = $_POST["ABSTRACT"];

        if (array_key_exists("ANNOUNCEMENT", $_POST) && $type == 'L')
	  $announcement = $_POST["ANNOUNCEMENT"];

        if ($name != "" &&
	    ($abstract != "") == ($announcement != "") &&
	    ($is_category ||
	     ($version != "" && $license != "" &&
	      $author != "" && $description != "" &&
	      $homepage_url != "http://" && $download_url != "ftp://")))
	  $havedata = 1;
	else
	  $havedata = 0;
      }
      else
        $havedata = 0;

      if ($type == 'C')
        $typename = 'Category';
      else
        $typename = 'Listing';

      if ($id > 0)
        $opname = 'Update';
      else
        $opname = 'Create';

      if ($havedata)
	$heading = htmlspecialchars("${opname}d $typename $name");
      else
	$heading = htmlspecialchars("$opname $typename $name");

      $links = array();

      if ($id)
        $links["Back to $name"] = "links.php?V$id$options";
      $links["Show All Listings"] = "links.php?LA$options";
      $links["Show Listings By Category"] = "links.php?LC$options";
      if ($LOGIN_LEVEL >= AUTH_DEVEL)
        $links["Show Unpublished Listings"] = "links.php?LU$options";

      html_header($heading, "", "", $links);

      if ($havedata)
      {
        $name         = db_escape($name);
	$version      = db_escape($version);
	$license      = db_escape($license);
	$author       = db_escape($author);
	$email        = db_escape($email);
	$homepage_url = db_escape($homepage_url);
	$download_url = db_escape($download_url);
	$user         = db_escape($LOGIN_USER);
	$date         = time();
        $what         = strtolower("${opname}d");

	if ($id == 0)
	{
          // Insert a new record...
	  db_query("INSERT INTO link VALUES(NULL,$parent_id,"
	          ."$is_category,$is_published,"
		  ."'$name','$version','$license',"
		  ."'$author','$email','$homepage_url','$download_url',"
		  ."'$description',5,1,0,0,$date,'$user',$date,'$user')");

          $id = db_insert_id();
	}
	else
	{
          // Modify the existing record...
	  db_query("UPDATE link SET is_published=$is_published,"
	          ."parent_id=$parent_id,"
		  ."name='$name',version='$version',license='$license',"
		  ."author='$author',email='$email',"
		  ."homepage_url='$homepage_url',download_url='$download_url',"
		  ."description='$description',modify_date=$date,"
		  ."modify_user='$user' "
		  ."WHERE id=$id");
	}

	if ($announcement != "" && $abstract != "")
	{
	  $abstract     = db_escape($abstract);
          $announcement = db_escape($announcement);

	  db_query("INSERT INTO article VALUES(NULL,$id,0,"
	          ."'$name $version','$abstract','$announcement',$date,"
		  ."'$user',$date,'$user')");

          $article_id = db_insert_id();

          // Notify the admin about the new article...
	  mail($PROJECT_EMAIL, "$PROJECT_NAME Article #$article_id created",
	       wordwrap("$user has created an article titled, "
	               ."'$name $version' with the following abstract:\n\n"
		       ."    $abstract\n\n"
		       ."Please approve or delete this article via the following "
		       ."page:\n\n"
		       ."    http://www.htmldoc.org/articles.php?L$article_id\n"),
	       "From: $PROJECT_EMAIL\r\n");
	}

	if ($is_published == 0)
	{
          // Send email to moderators...
	  $message = wordwrap("'$name' has been $what on the $PROJECT_NAME "
	                     ."links page and requires your approval before "
			     ."it will be made visible on the $PROJECT_NAME "
	                     ."site.  Please go to the following link to "
			     ."process the submission:\n\n"
			     ."    $PHP_URL?U$type$id\n");

	  mail($PROJECT_EMAIL, "$PROJECT_NAME $typename ${opname}d",
	       $message, "From: $PROJECT_EMAIL\r\n");

          // Let the user know that the moderator must approve it...
          print("<p>Your submission will be made visible as soon as one of "
               ."moderators approves it.</p>\n");
	}
	else
	{
	  print("<p>Thank you, your submission is now visible on the site.</p>\n");

	  if ($announcement != "")
	    print("<p>Your news announcement will be made visible as soon as "
	         ."one of moderators approves it.</p>\n");
        }

	html_start_links();
	html_link("Return to Listing", "$PHP_SELF?L$id+P$parent_id");
	html_end_links();
      }
      else
      {
	if ($REQUEST_METHOD == "POST")
	{
	  $what = strtolower($typename);

	  print("<p><b>Error:</b> Please fill in the fields marked in "
	       ."<b><font color='red'>bold red</font></b> below and resubmit "
	       ."your $what.</p><hr noshade/>\n");

	  $hstart = "<font color='red'>";
	  $hend   = "</font>";
	}
	else
	{
	  $hstart = "";
	  $hend   = "";
	}

        $name         = htmlspecialchars($name, ENT_QUOTES);
	$version      = htmlspecialchars($version, ENT_QUOTES);
	$license      = htmlspecialchars($license, ENT_QUOTES);
	$author       = htmlspecialchars($author, ENT_QUOTES);
	$email        = htmlspecialchars($email, ENT_QUOTES);
	$homepage_url = htmlspecialchars($homepage_url, ENT_QUOTES);
	$download_url = htmlspecialchars($download_url, ENT_QUOTES);
	$abstract     = htmlspecialchars($abstract, ENT_QUOTES);
	$announcement = htmlspecialchars($announcement, ENT_QUOTES);

	print("<form method='POST' action='$PHP_SELF?U$type$id+P$parent_id'>\n"
             ."<center><table border='0'>\n");

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	{
	  print("<tr><th align='right'>Published:</th><td>");
	  select_is_published($is_published);
	  print("</td></tr>\n");
	}
	else
	{
          print("<input type='hidden' name='IS_PUBLISHED' "
	       ."value='$is_published'>\n");
	}

        if ($name == "")
	  print("<tr><th align='right'>${hstart}Name:${hend}</th>");
	else
	  print("<tr><th align='right'>Name:</th>");
	print("<td><input type='text' name='NAME' value='$name' size='40'></td>"
	     ."</tr>\n");

	print("<tr><th align='right'>Category:</th><td>");
	select_category($parent_id, $is_category);
	print("</td></tr>\n");

	if (!$is_category)
	{
	  if ($version == "")
	    print("<tr><th align='right'>${hstart}Version:${hend}</th>");
	  else
	    print("<tr><th align='right'>Version:</th>");
	  print("<td><input type='text' name='VERSION' value='$version' "
	       ."size='32'></td></tr>\n");

          if ($license == "")
	    print("<tr><th align='right'>${hstart}License:${hend}</th>");
	  else
	    print("<tr><th align='right'>License:</th>");
	  print("<td><input type='text' name='LICENSE' value='$license' "
	       ."size='32'></td></tr>\n");

          if ($author == "")
	    print("<tr><th align='right'>${hstart}Author:${hend}</th>");
	  else
	    print("<tr><th align='right'>Author:</th>");
	  print("<td><input type='text' name='AUTHOR' value='$author' "
	       ."size='32'></td></tr>\n");

          if (!validate_email($email) && $email != "")
	    print("<tr><th align='right'>${hstart}EMail:${hend}</th>");
	  else
	    print("<tr><th align='right'>EMail:</th>");
	  print("<td><input type='text' name='EMAIL' value='$email' "
	       ."size='40'></td></tr>\n");

          if ($homepage_url == "http://")
	    print("<tr><th align='right' nowrap>${hstart}Home Page URL:${hend}</th>");
	  else
	    print("<tr><th align='right' nowrap>Home Page URL:</th>");
	  print("<td><input type='text' name='HOMEPAGE_URL' "
	       ."value='$homepage_url' size='40'></td></tr>\n");

          if ($download_url == "ftp://")
	    print("<tr><th align='right' nowrap>${hstart}Download URL:${hend}</th>");
	  else
	    print("<tr><th align='right' nowrap>Download URL:</th>");
	  print("<td><input type='text' name='DOWNLOAD_URL' "
	       ."value='$download_url' size='40'></td></tr>\n");

          if ($description == "")
	    print("<tr><th align='right' valign='top'>${hstart}Description:${hend}</th>");
	  else
	    print("<tr><th align='right' valign='top'>Description:</th>");
	  print("<td><textarea name='DESCRIPTION' wrap='virtual' cols='72' "
	       ."rows='12'>$description</textarea>"
	       ."<p>The description may contain the following "
	       ."HTML elements: <tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, "
	       ."<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, "
	       ."<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, "
	       ."<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, "
	       ."<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p></td></tr>\n");

	  print("<tr><th></th>"
	       ."<td><input type='submit' value='$opname $typename'/></td>"
	       ."</tr>\n");

	  print("<tr><td></td><td><hr noshade/>\n"
	       ."<h3>News Announcement (leave blank for no announcement)</h3></td>"
	       ."</tr>\n");

	  print("<tr><th align='right' valign='top'>Abstract:</th>"
	       ."<td><textarea name='ABSTRACT' wrap='virtual' cols='72' "
	       ."rows='2'>$abstract</textarea></td></tr>\n");

	  print("<tr><th align='right' valign='top'>Text:</th>"
	       ."<td><textarea name='ANNOUNCEMENT' wrap='virtual' cols='72' "
	       ."rows='12'>$announcement</textarea>"
	       ."<p>The announcement may contain the following "
	       ."HTML elements: <tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, "
	       ."<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, "
	       ."<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, "
	       ."<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, "
	       ."<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p></td></tr>\n");
	}

	print("<tr><th></th>"
	     ."<td><input type='submit' value='$opname $typename'/></td>"
	     ."</tr>\n");
	print("</table></center>\n");
	print("</form>");
      }

      html_footer();
      break;

  case 'V' : // View a listing...
      $link = new link($id);
      if ($link->id != $id)
      {
	header("Location: $PHP_SELF");
	exit();
      }

      if ($link->is_published == 0 && $LOGIN_LEVEL < AUTH_DEVEL &&
          $LOGIN_USER != $link->create_user)
      {
	// No permission!
	header("Location: $PHP_SELF");
	exit();
      }

      $parent_id    = $link->parent_id;
      $name         = htmlspecialchars($link->name, ENT_QUOTES);
      $version      = htmlspecialchars($link->version, ENT_QUOTES);
      $license      = htmlspecialchars($link->license, ENT_QUOTES);
      $author       = htmlspecialchars($link->author, ENT_QUOTES);
      $email        = htmlspecialchars($link->email, ENT_QUOTES);
      $homepage_url = htmlspecialchars($link->homepage_url, ENT_QUOTES);
      $download_url = htmlspecialchars($link->download_url, ENT_QUOTES);
      $description  = html_format($link->description);
      $create_date  = date("M d, Y", $link->create_date);
      $modify_date  = date("M d, Y", $link->modify_date);
      $category     = $link->get_category($link->parent_id);
      $rating       = (int)(100 * $link->rating_total /
                                  $link->rating_count) * 0.01;
      $email        = sanitize_email($link->email);

      if (($link->homepage_visits + $link->download_visits) > 0)
      {
        $visits     = db_query("SELECT MAX(homepage_visits), "
	                      ."MAX(download_visits) FROM link");
	$visrow     = db_next($visits);

        $popularity = (int)(100 * ($link->homepage_visits +
			           $link->download_visits) /
	                          ($visrow['MAX(homepage_visits)'] +
				   $visrow['MAX(download_visits)']));

        if ($popularity < 0)
	  $popularity = 0;

	db_free($visits);
      }
      else
      {
        $popularity = "???";
      }

      $links = array();

      $links["Back To Listings"] = "links.php?L+P$parent_id$options";
      if ($LOGIN_LEVEL >= AUTH_DEVEL || $LOGIN_USER == $link->create_user)
      {
        $links["Delete Listing"] = "links.php?X$id$options";
        $links["Edit Listing"] = "links.php?UL$id$options";
      }
      $links["Show Articles"] = "#_ARTICLES";
      $links["Show Comments"] = "#_USER_COMMENTS";

      html_header("$name $version", "", "", $links);

      print("<table width='100%' border='0'>\n");
      print("<tr>"
           ."<th align='right'>Category:</th>"
	   ."<td>$category</td>"
           ."<th align='right'>Rating:</th>"
	   ."<td><form method='POST' action='$PHP_SELF?R$id$options'>$rating&nbsp;"
	   ."<select name='RATING'>"
	   ."<option value='0'>0 - Worst</option>"
	   ."<option value='1'>1</option>"
	   ."<option value='2'>2</option>"
	   ."<option value='3'>3</option>"
	   ."<option value='4'>4</option>"
	   ."<option value='5' SELECTED>5 - Average</option>"
	   ."<option value='6'>6</option>"
	   ."<option value='7'>7</option>"
	   ."<option value='8'>8</option>"
	   ."<option value='9'>9</option>"
	   ."<option value='10'>10 - Best</option>"
	   ."</select>"
	   ."<input type='submit' value='Rate It!'></form>"
	   ."</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right'>Name:</th>"
	   ."<td>$name</td>"
           ."<th align='right'>Popularity:</th>"
	   ."<td>$popularity%</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right'>Version:</th>"
	   ."<td>$version</td>"
           ."<th align='right'>License:</th>"
	   ."<td>$license</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right'>Author:</th>"
	   ."<td>$author</td>"
           ."<th align='right'>EMail:</th>"
	   ."<td>$email</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right'>Home Page:</th>"
	   ."<td colspan='3'><a href='$PHP_SELF?SH$id'>$homepage_url</a> "
	   ."($link->homepage_visits visits)</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right'>Download:</th>"
	   ."<td colspan='3'><a href='$PHP_SELF?SD$id'>$download_url</a> "
	   ."($link->download_visits visits)</td>"
	   ."</tr>\n");
      print("<tr>"
           ."<th align='right' valign='top'>Description:</th>"
	   ."<td colspan='3'>$description</td>"
	   ."</tr>\n");
      print("</table>\n");

      print("<hr noshade/>\n"
           ."<h2><a name='_ARTICLES'>Articles</a></h2>\n");
      html_start_links();
      html_link("Submit Article", "articles.php?U+P$id");
      html_link("View All", "articles.php?L+P$id");
      html_end_links();

      $article = new article();
      $matches = $article->search("", "-modify_date", "", 0, $id);
      $count   = sizeof($matches);

      if ($count == 0)
        print("<p>No articles found.</p>\n");
      else
      {
        if ($count > 5)
	  $count = 5;

	html_start_table(array("ID","Title","Last Modified", "Comment(s)"));

	for ($i = 0; $i < $count && $article->load($matches[$i]); $i ++)
	{
          html_start_row();

          $aid  = $article->id;
	  $link = "<a href='articles.php?L$aid+P$id' alt='Article #$aid'>";

          print("<td nowrap>$link$aid</a></td>");

          $temp = htmlspecialchars($article->title);
          if ($article->is_published == 0)
	    $temp .= " <img src='${path}images/private.gif' width='16' height='16' "
	            ."border='0' align='middle' alt='Private'/>";

          print("<td width='67%'>$link$temp</a></td>");

          $temp = date("M d, Y", $article->modify_date);
          print("<td align='center' nowrap>$link$temp</a></td>");

          $ccount = count_comments("articles.php_L$aid");
          print("<td align='center'>$link$ccount</a></td>");

          html_end_row();

          html_start_row();
          $temp = html_format($article->abstract);
          print("<td></td><td colspan='3'>$temp</td>");
          html_end_row();
	}

        html_end_table();
      }

      print("<hr noshade/>\n"
           ."<h2><a name='_USER_COMMENTS'>Comments</a></h2>\n");
      html_start_links();
      html_link("Submit Comment", "comment.php?U+Plinks.php_V$id");
      html_end_links();

      show_comments("links.php_V$id");
      html_footer();
      break;

  case 'X' : // Delete listing...
      $result = db_query("SELECT * FROM link WHERE id = $id");
      if (db_count($result) != 1)
      {
        db_free($result);
        header("Location: $PHP_SELF?L$options");
	exit();
      }

      $row = db_next($result);

      if ($LOGIN_LEVEL < AUTH_DEVEL && $LOGIN_USER != $row["create_user"])
      {
        db_free($result);
        header("Location: $PHP_SELF?L$options");
	exit();
      }

      $name = htmlspecialchars($row["name"], ENT_QUOTES);

      db_free($result);

      if ($REQUEST_METHOD == "POST")
      {
        // Already confirmed it...
        db_query("DELETE FROM link WHERE id = $id");
	html_header("$name Deleted", "", "",
	            array("Return To Listings" => "links.php?L+P$parent_id$options"));

	print("<p>The listing for '$name' has been deleted.</p>\n");

        html_footer();
      }
      else
      {
        // Confirm deletion...
	html_header("Delete $name", "", "",
	            array("Return To $name" => "links.php?V$id+P$parent_id$options",
	                  "Return To Listings" => "links.php?L+P$parent_id$options"));
	print("<form method='POST' action='$PHP_SELF?X$id+P$parent_id$options'>\n"
             ."<center><input type='submit' value='Confirm Delete $name'></center>"
	     ."</form>\n");

        html_footer();
      }
      break;

  case 'R' : // Rate this entry...
      if (array_key_exists("RATING", $_POST))
      {
        $rating = (int)$_POST["RATING"];

	if ($rating < 0)
	  $rating = 0;
	else if ($rating > 10)
	  $rating = 10;

	if (db_query("INSERT INTO vote VALUES('link_${id}_${REMOTE_ADDR}')"))
          db_query("UPDATE link SET rating_count = rating_count + 1, "
	          ."rating_total = rating_total + $rating WHERE id = $id");
      }

      header("Location: $PHP_SELF?V$id$options");
      break;
        
  case 'S' : // Show home or download page...
      $result = db_query("SELECT * FROM link WHERE id = $id");

      if (db_count($result) != 1)
      {
        db_free($result);
        header("Location: $PHP_SELF?L$options");
	exit();
      }
	
      $row = db_next($result);

      if ($type == 'H' && $row["homepage_url"] != "")
      {
	db_query("UPDATE link SET homepage_visits = homepage_visits + 1 "
	        ."WHERE id = $id");

        header("Location: $row[homepage_url]");
      }
      else if ($type == 'D' && $row["download_url"] != "")
      {
	db_query("UPDATE link SET download_visits = download_visits + 1 "
	        ."WHERE id = $id");

        header("Location: $row[download_url]");
      }
      else
        header("Location: $PHP_SELF?V$id$options");

      db_free($result);
      break;
}


//
// End of "$Id: links.php,v 1.9 2005/03/13 04:39:34 mike Exp $".
//
?>
