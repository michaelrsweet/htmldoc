<?php
//
// "$Id: articles.php,v 1.21 2005/06/01 18:33:13 mike Exp $"
//
// Web form for the article table...
//


//
// Include necessary header...
//

include_once "phplib/rdf.php";


//
// 'notify_users()' - Notify users of new/updated articles...
//

function
notify_users($id,			// I - Article #
             $what = "created")		// I - Reason for notification
{
  global $PHP_URL, $PROJECT_EMAIL, $PROJECT_NAME;


  $result = db_query("SELECT * FROM article WHERE id = $id");
  if (db_count($result) == 1)
  {
    $row = db_next($result);

    mail($PROJECT_EMAIL, "$PROJECT_NAME Article #$id $what",
	 wordwrap("$row[create_user] has $what an article titled, "
	         ."'$row[title]' with the following abstract:\n\n"
		 ."    $row[abstract]\n\n"
		 ."Please approve or delete this article via the following "
		 ."page:\n\n"
		 ."    $PHP_URL?L$id\n"),
	 "From: $PROJECT_EMAIL\r\n");
  }
}


// Get command-line options...
//
// Usage: article.php [operation] [options]
//
// Operations:
//
// B         = Batch update selected articles
// D#        = Delete article
// G         = Generate RSS file...
// L         = List all 
// L#        = List article #
// U         = Create new article
// U#        = Modify article #
//
// Options:
//
// P         = Parent link ID
// Qtext     = Set search text (must be last option!)
// Ttype     = Set search type

$search = "";
$stype  = "";
$index  = 0;

if ($argc)
{
  $op = $argv[0][0];
  $id = (int)substr($argv[0], 1);

  if ($op != 'B' && $op != 'D' && $op != 'G' && $op != 'L' && $op != 'U')
  {
    html_header("Article Error");
    print("<p>Bad command '$op'!\n");
    html_footer();
    exit();
  }

  if ($op == 'D' && !$id)
  {
    html_header("Article Error");
    print("<p>Command '$op' requires an ID!\n");
    html_footer();
    exit();
  }

  if (($op == 'B' || $op == 'G') && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    html_header("Article Error");
    print("<p>You don't have permission to use command '$op'!\n");
    html_footer();
    exit();
  }

  if (($op == 'D' || $op == 'U') && $id && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    $article = new article($id);
    if ($article->id != $id)
    {
      html_header("Article Error");
      print("<p>Article #$id does not exist!\n");
      html_footer();
      exit();
    }

    if ($article->create_user != $LOGIN_USER)
    {
      html_header("Article Error");
      print("<p>You don't have permission to use command '$op'!\n");
      html_footer();
      exit();
    }
  }

  for ($i = 1; $i < $argc; $i ++)
  {
    $option = substr($argv[$i], 1);

    switch ($argv[$i][0])
    {
      case 'I' : // Set first article (no longer used)
	  break;
      case 'M' : // Set max STRs per page (no longer used)
	  break;
      case 'Q' : // Set search text
	  $search = urldecode($option);
	  $i ++;
	  while ($i < $argc)
	  {
	    $search .= urldecode(" $argv[$i]");
	    $i ++;
	  }
	  break;
      case 'T' : // Set search type
	  $stype = $option;
	  break;
      default :
	  html_header("Article Error");
	  print("<p>Bad option '$argv[$i]'!</p>\n");
	  html_footer();
	  exit();
	  break;
    }
  }
}
else
{
  $op = 'L';
  $id = 0;
}

if ($REQUEST_METHOD == "POST")
{
  if (array_key_exists("SEARCH", $_POST))
    $search = $_POST["SEARCH"];
  if (array_key_exists("STYPE", $_POST))
    $stype = $_POST["STYPE"];
  if (array_key_exists("FPAGEMAX", $_POST))
  {
    $PAGE_MAX = (int)$_POST["FPAGEMAX"];
    setcookie("PAGE_MAX", $PAGE_MAX, time() + 365 * 86400);
  }
}

$options = "+T" . urlencode($stype) . "+Q" . urlencode($search);

switch ($op)
{
  case 'B' : // Batch update selected articles
      if ($REQUEST_METHOD != "POST")
      {
	header("Location: $PHP_SELF?L$options");
	break;
      }

      if (array_key_exists("BATCHOP", $_POST) &&
	  ($_POST["BATCHOP"] == "Delete Checked" ||
	   $_POST["BATCHOP"] == "Publish Checked"))
      {
	$modify_date  = time();
	$modify_user  = db_escape($LOGIN_USER);
	$batchop      = $_POST["BATCHOP"];

	db_query("BEGIN TRANSACTION");

	reset($_POST);
	while (list($key, $val) = each($_POST))
	  if (substr($key, 0, 3) == "ID_")
	  {
	    $id = substr($key, 3);

	    if ($batchop == "Delete Checked")
	      db_query("DELETE FROM article WHERE id = $id");
	    else
	      db_query("UPDATE article SET is_published = 1, "
		      ."modify_date = $modify_date, "
		      ."modify_user = '$modify_user' "
		      ."WHERE id = $id");
	  }

	db_query("COMMIT TRANSACTION");
      }

      make_rdf_file("index.rss", "http://www.htmldoc.org/", "HTMLDOC",
		    "HTMLDOC News");
      header("Location: $PHP_SELF?L$options");
      break;

  case 'D' : // Delete Article
      $article = new article($id);

      if ($article->id != $id)
      {
	html_header("Delete Article #$id");
	print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($REQUEST_METHOD == "POST")
      {
	$article->delete();

	make_rdf_file("index.rss", "http://www.htmldoc.org/",
		      "HTMLDOC", "HTMLDOC News");
	header("Location: $PHP_SELF?L$options");
      }
      else
      {
	html_header("Delete Article #$id", "",
	            array("Documentation" => "documentation.php",
		          "Return to Articles" => "$PHP_SELF?L$options",
		          "View Article #$id" => "$PHP_SELF?L$id$options",
			  "Modify Article #$id" => "$PHP_SELF?U$id$options"));

	print("<form method='post' action='$PHP_SELF?D$id'>");
	$article->view();

	print("<input type='submit' value='Confirm Delete Article'>\n"
	     ."</form>\n");

	html_footer();
      }
      break;

  case 'G' : // Generate RSS file...
      make_rdf_file("index.rss", "http://www.htmldoc.org/", "HTMLDOC",
		    "HTMLDOC News");

      html_header("Update RSS File", "$PHP_SELF?L$options",
                  array("Documentation" => "documentation.php",
			"Return to Articles" => "$PHP_SELF?L$options"));

      print("<p>RSS file updated!</p>\n");
      html_footer();
      break;

  case 'L' : // List (all) Article(s)
      if ($id)
      {
	$article = new article($id);

	if ($article->id != $id)
	{
	  html_header("Article Error");
	  print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	  html_footer();
	  exit();
	}

	$links = array("Documentation" => "documentation.php",
		       "Return to Articles" => "$PHP_SELF?L$options",
		       "Show Comments" => "#_USER_COMMENTS",
		       "Submit Comment" => "comment.php?U+Particles.php_L$id");

	if ($LOGIN_LEVEL >= AUTH_DEVEL ||
	    $article->create_user == $LOGIN_USER)
	{
	  $links["Modify Article"] = "$PHP_SELF?U$id$options";
	  $links["Delete Article"] = "$PHP_SELF?D$id$options";
	}

	html_header("Article #$id: $article->title", "", $links);

	$article->view();
      }
      else
      {
	$bookmark  = "$PHP_URL?L+T" . urlencode($stype) .
		     "+Q" . urlencode($search);

	$links = array("Documentation" => "documentation.php",
		       "New Article" => "$PHP_SELF?U$options",
	               "Link To Search Results" => $bookmark);
	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  $links["Update RSS File"] = "$PHP_SELF?G$options";

	html_header("Articles", "", $links);

	$htmlsearch = htmlspecialchars($search, ENT_QUOTES);

	print("<form method='POST' action='$PHP_SELF?L'><p align='center'>"
	     ."<input type='search' size='60' name='SEARCH' "
	     ."value='$htmlsearch' placeholder='Search Articles' "
	     ."autosave='org.cups.search' results='20'>"
	     ."<input type='submit' value='Search Articles'><br>\n"
	     ."<i>Search supports 'and', 'or', 'not', and parenthesis. "
	     ."<a href='search-help.php'>More info...</a></i>"
	     ."</p></form>\n"
	     ."<hr noshade>\n"
	     ."<p><a href='$PHP_SELF'>All Topics</a>");

	$result = db_query("SELECT distinct(type) as type FROM article "
			  ."ORDER BY type");
	while ($row = db_next($result))
	  print(" &middot; <a href='$PHP_SELF?L+T$row[type]'>$row[type]</a>");

	print("</p>\n");

	$article = new article();
	$matches = $article->search($search, "type,-modify_date", $stype, 0);
	$count   = sizeof($matches);

	if ($count == 0)
	{
	  print("<p>No $stype articles found.</p>\n");

	  html_footer();
	  exit();
	}

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  print("<form method='POST' action='$PHP_SELF?B$options'>\n"
	       ."<input type='submit' name='BATCHOP' "
	       ."value='Delete Checked'>\n"
	       ."<input type='submit' name='BATCHOP' "
	       ."value='Publish Checked'>\n");

	$baseoptions = "+Q" . urlencode($search);

	for ($i = 0, $type = "", $typecount = 0, $typetotal = 0;
	     $i < $count;
	     $i ++)
	{
	  if (!$article->load($matches[$i]))
	    continue;

	  if ($type != $article->type)
	  {
	    if ($type != "")
	      print("</div>\n");

	    $type        = $article->type;
	    $typecount   = 0;
	    $typearticle = new article();

	    for ($typetotal = 1, $j = $i + 1;
		 $j < $count;
		 $j ++, $typetotal ++)
	    {
	      if (!$typearticle->load($matches[$j]) ||
		  $typearticle->type != $type)
		break;
	    }

	    print("<h2><a href='$PHP_SELF?L+T$type$baseoptions'>$type "
		 ."($typetotal articles)</a></h2>\n"
		 ."<div style='margin-left: 2em;'>\n");
	  }

	  $typecount ++;
	  if ($stype == "" && $typecount > 5)
	  {
	    if ($typecount == 6)
	      print("<p><a href='$PHP_SELF?L+T$type$baseoptions'>"
		   ."View All $typetotal $type Articles</a></p>\n");
	    continue;
	  }

	  $id   = $article->id;
	  $link = "<a href='$PHP_SELF?L$id$options' title='Article #$id'>";

	  $title = htmlspecialchars($article->title);
	  if ($article->is_published == 0)
	    $title .= " <img src='images/private.gif' width='16' "
		     ."height='16' border='0' align='absmiddle' "
		     ."alt='Private'>";
	  $date     = date("M d, Y", $article->modify_date);
	  $ccount   = count_comments("articles.php_L$id");
	  $abstract = htmlspecialchars($article->abstract);

	  print("<h3>$link$title</a></h3>\n"
	       ."<p>");

	  if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    print("<input type='checkbox' name='ID_$id'>");

	  print("<i>By $article->create_user on $date, $ccount "
	       ."comment(s)</i><br>\n"
	       ."$abstract</p>\n");
	}

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  print("<input type='submit' name='BATCHOP' "
	       ."value='Delete Checked'>\n"
	       ."<input type='submit' name='BATCHOP' "
	       ."value='Publish Checked'>\n"
	       ."</form>\n"
	       ."<p><img src='images/private.gif' width='16' "
	       ."height='16' align='absmiddle' alt='private'> = hidden from "
	       ."public view</p>\n");
      }

      html_footer();
      break;

  case 'U' : // Submit/Modify Article
      if ($LOGIN_USER == "")
      {
	header("Location: login.php?PAGE=$PHP_SELF?U$id");
	return;
      }

      $article = new article($id);
      if ($article->id != $id)
      {
	html_header("Modify Article #$id");
	print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($LOGIN_LEVEL < AUTH_DEVEL &&
	  $id != 0 && $article->create_user != $LOGIN_USER)
      {
	html_header("Modify Article #$id");
	print("<p><b>Error:</b> You do not have permission to modify "
	     ."article #$id!</p>\n");
	html_footer();
	exit();
      }

      if ($REQUEST_METHOD == "POST")
	$havedata = $article->loadform();
      else
	$havedata = 0;

      if ($havedata && array_key_exists("SUBMIT", $_POST) &&
	  $_POST["SUBMIT"] != "Preview Article")
      {
	$article->save();

	if (!$article->is_published)
	{
	  if ($id == 0)
	    notify_users($article->id, "created");
	  else
	    notify_users($id, "modified");
	}

	make_rdf_file("index.rss", "http://www.htmldoc.org/",
		      "HTMLDOC", "HTMLDOC News");
	header("Location: $PHP_SELF?L$id$options");
      }
      else
      {
	$links = array("Documentation" => "documentation.php",
		       "Return to Articles" => "$PHP_SELF?L$options");
	if ($id)
	  $links["Article #$id"] = "$PHP_SELF?L$id$options";

	if ($id)
	  html_header("Modify Article #$id", "", $links);
	else
	  html_header("New Article", "", $links);

	if ($REQUEST_METHOD == "POST" && !$havedata)
	  print("<p><b>Error:</b> Please fill in the fields marked "
	       ."<b class='invalid'>as follows</b> below and resubmit "
	       ."your article.</p><hr noshade>\n");
	else if ($id == 0)
	  print("<p>Please use this form to post announcements, how-to's, "
	       ."examples, and case studies showing how you use $PROJECT_NAME. "
	       ."We will proofread your article, and if we determine it is "
	       ."appropriate for the site, we will make the article public "
	       ."on the site. <i>Thank you</i> for supporting $PROJECT_NAME!</p>\n"
	       ."<hr noshade>\n");

	$article->edit($options);

	if ($article->contents != "")
	  $article->view();

	html_footer();
      }
      break;
}


//
// End of "$Id: articles.php,v 1.21 2005/06/01 18:33:13 mike Exp $".
//
?>
