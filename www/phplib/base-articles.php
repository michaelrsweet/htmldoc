<?php
//
// "$Id: base-articles.php,v 1.1 2005/06/01 18:33:14 mike Exp $"
//
// Web form for the article table...
//
// Contents:
//
//   notify_users()  - Notify users of new/updated articles...
//   articles_main() - Show the articles page.
//


//
// Include necessary headers...
//

include_once "rdf.php";


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


//
// 'articles_main()' - Show the articles page.
//

function
articles_main($link_id = PROJECT_LINK_ALL,
					// I - Link/application ID
              $path = "",		// I - Path to document root
	      $links = "")		// I - Tab links
{
  global $argc, $argv, $_GET, $_POST, $_COOKIE, $LOGIN_LEVEL, $LOGIN_USER,
         $PAGE_MAX, $PHP_SELF, $REQUEST_METHOD, $PROJECT_NAME,
	 $PROJECT_MODULE, $PROJECT_SITE, $ARTICLE_TYPES;


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
  // I#        = Set first article
  // P#        = Parent link ID
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
      html_header("Article Error", $path, "", $links);
      print("<p>Bad command '$op'!\n");
      html_footer();
      exit();
    }

    if ($op == 'D' && !$id)
    {
      html_header("Article Error", $path, "", $links);
      print("<p>Command '$op' requires an ID!\n");
      html_footer();
      exit();
    }

    if (($op == 'B' || $op == 'G') && $LOGIN_LEVEL < AUTH_DEVEL)
    {
      html_header("Article Error", $path, "", $links);
      print("<p>You don't have permission to use command '$op'!\n");
      html_footer();
      exit();
    }

    if (($op == 'D' || $op == 'U') && $id && $LOGIN_LEVEL < AUTH_DEVEL)
    {
      $article = new article($id);
      if ($article->id != $id)
      {
	html_header("Article Error", $path, "", $links);
	print("<p>Article #$id does not exist!\n");
	html_footer();
	exit();
      }

      if ($article->create_user != $LOGIN_USER)
      {
	html_header("Article Error", $path, "", $links);
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
	case 'I' : // Set first article
            $index = (int)$option;
	    if ($index < 0)
	      $index = 0;
	    break;
	case 'P' : // Set link ID
	    $link_id = (int)$option;
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
	    html_header("Article Error", $path, "", $links);
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
  }

  $options = "+I$index+T" . urlencode($stype) . "+P$link_id+Q" . urlencode($search);

  switch ($op)
  {
    case 'B' : // Batch update selected articles
	if ($REQUEST_METHOD != "POST")
	{
          header("Location: $PHP_SELF?L$options");
          break;
	}

	if (array_key_exists("IS_PUBLISHED", $_POST) &&
            $_POST["IS_PUBLISHED"] != "")
	{
          $modify_date  = time();
          $modify_user  = db_escape($LOGIN_USER);
	  $is_published = (int)$_POST["IS_PUBLISHED"];

          $query = "is_published = $is_published, modify_date = $modify_date, "
	          ."modify_user = '$modify_user'";

          db_query("BEGIN TRANSACTION");

          reset($_POST);
          while (list($key, $val) = each($_POST))
            if (substr($key, 0, 3) == "ID_")
	    {
	      $id = (int)substr($key, 3);

              db_query("UPDATE article SET $query WHERE id = $id");
	    }

          db_query("COMMIT TRANSACTION");
	}

	make_rdf_file("${path}index.rss", "http://$PROJECT_SITE/", $PROJECT_NAME,
                      "$PROJECT_NAME News");
	header("Location: $PHP_SELF?L$options");
	break;

    case 'D' : // Delete Article
	$article = new article($id);

	if ($article->id != $id)
	{
          html_header("Delete Article #$id", $path, "", $links);
	  print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	  html_footer();
	  exit();
	}

	if ($REQUEST_METHOD == "POST")
	{
          $article->delete();

	  make_rdf_file("${path}index.rss", "http://$PROJECT_SITE/", $PROJECT_NAME,
                	"$PROJECT_NAME News");
          header("Location: $PHP_SELF?L$options");
	}
	else
	{
          html_header("Delete Article #$id", $path, "", $links);

	  html_start_links(1);
	  html_link("Return to Articles", "$PHP_SELF?L$options");
	  html_link("View Article #$id", "$PHP_SELF?L$id$options");
	  html_link("Modify Article #$id", "$PHP_SELF?U$id$options");
	  html_end_links();

          print("<form method='post' action='$PHP_SELF?D$id'>");
	  $article->view();

	  print("<input type='submit' value='Confirm Delete Article'>\n"
	       ."</form>\n");

          html_footer();
	}
	break;

    case 'G' : // Generate RSS file...
	make_rdf_file("${path}index.rss", "http://$PROJECT_SITE/", $PROJECT_NAME,
                      "$PROJECT_NAME News");

	html_header("Update RSS File", $path, "$PHP_SELF?L$options", "", $links);

	html_start_links(1);
	html_link("Return to Articles", "$PHP_SELF?L$options");
	html_end_links();

	print("<p>RSS file updated!</p>\n");
	html_footer();
	break;

    case 'L' : // List (all) Article(s)
	if ($id)
	{
          $article = new article($id);

	  if ($article->id != $id)
	  {
            html_header("Article Error", $path, "", $links);
	    print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	    html_footer();
	    exit();
	  }

          html_header("Article #$id: $article->title", $path, "", $links);

	  html_start_links(1);
	  html_link("Return to Articles", "$PHP_SELF?L$options");
	  html_link("Show Comments", "#_USER_COMMENTS");
	  html_link("Submit Comment", "comment.php?U+Particles.php_L$id");

	  if ($LOGIN_LEVEL >= AUTH_DEVEL ||
	      $article->create_user == $LOGIN_USER)
	  {
	    html_link("Modify Article", "$PHP_SELF?U$id$options");
	    html_link("Delete Article", "$PHP_SELF?D$id$options");
	  }
	  html_end_links();

	  $article->view();
	}
	else
	{
          html_header("Articles", $path, "", $links);

	  html_start_links(1);
	  html_link("Submit Article", "$PHP_SELF?U$options");
	  if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    html_link("Update RSS File", "$PHP_SELF?G$options");
	  html_end_links();

          print("<form method='POST' action='$PHP_SELF?L'><p align='center'>"
	       ."Search:&nbsp;"
	       ."<select name='STYPE'>"
	       ."<option value=''>All</option>");

	  reset($ARTICLE_TYPES);
	  while (list($key, $val) = each($ARTICLE_TYPES))
	  {
	    print("<option value='$val'");
	    if ($stype == $val)
	      print(" selected");
	    print(">$val</option>");
	  }

          if ($LOGIN_USER != "")
	  {
	    print("<option value='Mine'");
	    if ($stype == "Mine")
	      print(" selected");
	    print(">Mine</option>");
	  }

	  print("</select>"
	       ."<input type='text' size='60' name='SEARCH' value='$search'>"
	       ."<input type='submit' value='Search Articles'>"
               ."</p></form>\n");

	  print("<hr noshade/>\n");

          $article = new article();
	  $matches = $article->search($search, "-modify_date", $stype, 0,
	                              $link_id);
	  $count   = sizeof($matches);

          if ($count == 0)
	  {
	    print("<p>No articles found.</p>\n");

	    html_footer();
	    exit();
	  }

          if ($index >= $count)
	    $index = $count - ($count % $PAGE_MAX);
	  if ($index < 0)
	    $index = 0;

          $start = $index + 1;
          $end   = $index + $PAGE_MAX;
	  if ($end > $count)
	    $end = $count;

          $prev = $index - $PAGE_MAX;
	  if ($prev < 0)
	    $prev = 0;
	  $next = $index + $PAGE_MAX;

          print("<p>$count article(s) found, showing $start to $end:</p>\n");

          if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    print("<form method='POST' action='$PHP_SELF?B$options'>\n");

          if ($count > $PAGE_MAX)
	  {
            print("<p><table border='0' cellspacing='0' cellpadding='0' "
		 ."width='100%'>\n");

            print("<tr><td>");
	    if ($index > 0)
	      print("[&nbsp;<a href='$PHP_SELF?L+I$prev+T" . urlencode($stype) . "+Q" . urlencode($search)
		   ."'>Previous&nbsp;$PAGE_MAX</a>&nbsp;]");
            print("</td><td align='right'>");
	    if ($end < $count)
	    {
	      $next_count = min($PAGE_MAX, $count - $end);
	      print("[&nbsp;<a href='$PHP_SELF?L+I$next+T" . urlencode($stype) . "+Q" . urlencode($search)
		   ."'>Next&nbsp;$next_count</a>&nbsp;]");
            }
            print("</td></tr>\n");
	    print("</table></p>\n");
          }

          html_start_table(array("ID","Title","Last Modified", "Comment(s)"));

	  for ($i = $start - 1; $i < $end && $article->load($matches[$i]); $i ++)
	  {
            html_start_row();

            $id   = $article->id;
	    $link = "<a href='$PHP_SELF?L$id$options' alt='Article #$id'>";

            print("<td nowrap>");
            if ($LOGIN_LEVEL >= AUTH_DEVEL)
	      print("<input type='checkbox' name='ID_$id'>");
            print("$link$id</a>&nbsp;&nbsp;</td>");

            $temp = htmlspecialchars($article->title);
            if ($article->is_published == 0)
	      $temp .= " <img src='${path}images/private.gif' width='16' height='16' "
	              ."border='0' align='middle' alt='Private'/>";

            print("<td width='67%'>$link$temp</a></td>");

            $temp = date("M d, Y", $article->modify_date);
            print("<td align='center' nowrap>$link$temp</a></td>");

            $ccount = count_comments("articles.php_L$id");
            print("<td align='center'>$link$ccount</a></td>");

            html_end_row();

            html_start_row();
            $temp = html_format($article->abstract);
            print("<td></td><td colspan='3'>$temp</td>");
            html_end_row();
	  }

          if ($LOGIN_LEVEL > 0)
	  {
	    html_start_row("header");

	    print("<th colspan='4'>&nbsp;<br />Published:&nbsp;");
	    select_is_published();
	    print("<input type='submit' value='Modify Selected Articles'/></th>\n");

	    html_end_row();
          }

          html_end_table();

          if ($count > $PAGE_MAX)
	  {
            print("<p><table border='0' cellspacing='0' cellpadding='0' "
		 ."width='100%'>\n");

            print("<tr><td>");
	    if ($index > 0)
	      print("[&nbsp;<a href='$PHP_SELF?L+I$prev+T" . urlencode($stype) . "+Q" . urlencode($search)
		   ."'>Previous&nbsp;$PAGE_MAX</a>&nbsp;]");
            print("</td><td align='right'>");
	    if ($end < $count)
	    {
	      $next_count = min($PAGE_MAX, $count - $end);
	      print("[&nbsp;<a href='$PHP_SELF?L+I$next+T" . urlencode($stype) . "+Q" . urlencode($search)
		   ."'>Next&nbsp;$next_count</a>&nbsp;]");
            }
            print("</td></tr>\n");
	    print("</table></p>\n");
          }

          if ($LOGIN_LEVEL >= AUTH_DEVEL)
            print("<p><img src='${path}images/private.gif' width='16' height='16' "
		 ."align='middle' alt='private'/> = hidden from public view</p>\n");
	}

	html_footer();
	break;

    case 'U' : // Submit/Modify Article
	if ($LOGIN_USER == "")
	{
          header("Location: ${path}login.php?PAGE=$PHP_SELF?U$id");
	  return;
	}

	$article = new article($id);
	if ($article->id != $id)
	{
          html_header("Modify Article #$id", $path, "", $links);
	  print("<p><b>Error:</b> Article #$id was not found!</p>\n");
	  html_footer();
	  exit();
	}

	if ($LOGIN_LEVEL < AUTH_DEVEL &&
            $id != 0 && $article->create_user != $LOGIN_USER)
	{
          html_header("Modify Article #$id", $path, "", $links);
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
	  $article->link_id = $link_id;
          $article->save();

          if (!$article->is_published)
	  {
	    if ($id == 0)
	      notify_users($article->id, "created");
	    else
	      notify_users($id, "modified");
	  }

	  make_rdf_file("${path}index.rss", "http://$PROJECT_SITE/", $PROJECT_NAME,
                	"$PROJECT_NAME News");
	  header("Location: $PHP_SELF?L$id$options");
	}
	else
	{
          if ($id)
            html_header("Modify Article #$id", $path, "", $links);
	  else
            html_header("Submit Article", $path, "", $links);

	  html_start_links(1);
	  html_link("Return to Articles", "$PHP_SELF?L$options");
	  if ($id)
	    html_link("Article #$id", "$PHP_SELF?L$id$options");
	  html_end_links();

	  if ($REQUEST_METHOD == "POST" && !$havedata)
	    print("<p><b>Error:</b> Please fill in the fields marked "
		 ."<b class='invalid'>as follows</b> below and resubmit "
		 ."your article.</p><hr noshade/>\n");
	  else if ($id == 0)
	    print("<p>Please use this form to post announcements, how-to's, "
        	 ."examples, and case studies showing how you use $PROJECT_NAME. "
		 ."We will proofread your article, and if we determine it is "
		 ."appropriate for the site, we will make the article public "
		 ."on the site. <i>Thank you</i> for supporting $PROJECT_NAME!</p>\n"
		 ."<hr noshade/>\n");

          $article->edit($options);

	  if ($article->contents != "")
	    $article->view();

          html_footer();
	}
	break;
  }
}


//
// End of "$Id: base-articles.php,v 1.1 2005/06/01 18:33:14 mike Exp $".
//
?>
