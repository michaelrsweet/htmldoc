<?php
//
// "$Id: comment.php,v 1.5 2005/03/29 19:29:41 mike Exp $"
//
// Comment and moderation interface for PHP pages...
//

//
// Include necessary headers...
//

include_once "phplib/db-comment.php";
include_once "phplib/db-article.php";


// Require a login...
if ($LOGIN_USER == "")
{
  for ($i = 0, $url=$PHP_SELF, $prefix="?"; $i < $argc; $i ++)
  {
    $url    .= $prefix;
    $url    .= urlencode($argv[$i]);
    $prefix = "+";
  }

  header("Location: login.php?PAGE=$url");
  return;
}

// Get command-line options...
//
// Usage: comment.php [operation] [options]
//
// Operations:
//
// B         = Batch update selected comments
// D#        = Delete comment
// L         = List all 
// L#        = List comment #
// M#        = Moderate comment up
// m#        = Moderate comment down
// U         = Create new comment
// U#        = Modify commente #
//
// Options:
//
// I#        = Set first comment
// Ppath     = Set path
// Qtext     = Set search text (must be last option!)
// R#        = Reference ID
// Ttype     = Which comments to show

$search   = "";
$stype    = "";
$index    = 0;
$urlpath  = "";
$refer_id = 0;

if ($argc)
{
  $op = $argv[0][0];
  $id = (int)substr($argv[0], 1);

  if ($op != 'B' && $op != 'D' && $op != 'L' && $op != 'U' &&
      $op != 'M' && $op != 'm')
  {
    html_header("Comment Error");
    print("<p>Bad command '$op'!\n");
    html_footer();
    exit();
  }

  if (($op == 'D' || $op == 'M' || $op == 'm') && !$id)
  {
    html_header("Comment Error");
    print("<p>Command '$op' requires an ID!\n");
    html_footer();
    exit();
  }

  if ($op == 'B' && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    html_header("Comment Error");
    print("<p>You don't have permission to use command '$op'!\n");
    html_footer();
    exit();
  }

  if (($op == 'D' || $op == 'U') && $id && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    $comment = new comment($id);
    if ($comment->id != $id)
    {
      html_header("Comment Error");
      print("<p>Comment #$id does not exist!\n");
      html_footer();
      exit();
    }

    if ($comment->create_user != $LOGIN_USER)
    {
      html_header("Comment Error");
      print("<p>You don't have permission to use command '$op'!\n");
      html_footer();
      exit();
    }
  }

  for ($i = 1; $i < $argc; $i ++)
  {
    $option = urldecode(substr($argv[$i], 1));

    switch ($argv[$i][0])
    {
      case 'I' : // Set first article
          $index = (int)$option;
	  if ($index < 0)
	    $index = 0;
	  break;
      case 'P' : // Set search type
          $urlpath = $option;
	  break;
      case 'R' : // Set parent ID
          $refer_id = (int)$option;
	  break;
      case 'Q' : // Set search text
          $search = $option;
	  $i ++;
	  while ($i < $argc)
	  {
	    $search .= " $argv[$i]";
	    $i ++;
	  }
	  break;
      case 'T' : // Set search type
          $stype = $option;
	  break;
      default :
	  html_header("Comment Error");
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

$options = "+I$index+T" . urlencode($stype) . "+P" . urlencode($urlpath) .
           "+R$refer_id+Q" . urlencode($search);

switch ($op)
{
  case 'B' : // Batch update selected comments
      // Moderate/delete comments...
      if ($REQUEST_METHOD == "POST" && array_key_exists("OP", $_POST))
      {
	$op = $_POST["OP"];

        db_query("BEGIN TRANSACTION");

        reset($_POST);
        while (list($key, $val) = each($_POST))
          if (substr($key, 0, 3) == "ID_")
	  {
	    $id = (int)substr($key, 3);

            if ($op == "up")
	      db_query("UPDATE comment SET status = status + 1 "
		      ."WHERE id = $id AND status < 5");
            else if ($op == "down")
	      db_query("UPDATE comment SET status = status - 1 "
		      ."WHERE id = $id AND status > 1");
            else if ($op == "delete")
	      db_query("DELETE FROM comment WHERE id = $id");
	  }

        db_query("COMMIT TRANSACTION");
      }

      header("Location: $PHP_SELF?L$options");
      break;

  case 'D' : // Delete comment
      $comment = new comment($id);

      if ($comment->id != $id)
      {
        html_header("Delete Comment #$id");
	print("<p><b>Error:</b> Comment #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($REQUEST_METHOD == "POST")
      {
        $comment->delete();

        header("Location: $PHP_SELF?L$options");
      }
      else
      {
        html_header("Delete Comment #$id", "", "",
	            array("Return to Comments" => "$PHP_SELF?L$options",
			  "View Comment #$id" => "$PHP_SELF?L$id$options",
			  "Modify Comment #$id" => "$PHP_SELF?U$id$options"));

        print("<form method='post' action='$PHP_SELF?D$id'>");
	$comment->view();

	print("<input type='submit' value='Confirm Delete Comment'>\n"
	     ."</form>\n");

        html_footer();
      }
      break;

  case 'L' : // List (all) Comment(s)
      if ($id)
      {
        $comment = new comment($id);

	if ($comment->id != $id)
	{
          html_header("Comment Error");
	  print("<p><b>Error:</b> Comment #$id was not found!</p>\n");
	  html_footer();
	  exit();
	}

        $links = array();

	$links["Return to Comments"] = "$PHP_SELF?L$options";

	if ($LOGIN_LEVEL >= AUTH_DEVEL ||
	    $comment->create_user == $LOGIN_USER)
	{
	  $links["Modify Comment"] = "$PHP_SELF?U$id$options";
	  $links["Delete Comment"] = "$PHP_SELF?D$id$options";
	}

        html_header("Comment #$id: $comment->title", "", "", $links);

	$comment->view();
      }
      else
      {
        html_header("Comments");

        print("<form method='POST' action='$PHP_SELF?L'><p align='center'>"
	     ."Search:&nbsp;"
	     ."<select name='STYPE'>"
	     ."<option value=''>All</option>");

	print("<option value='hidden'");
	if ($stype == "hidden")
	  print(" selected");
	print(">Hidden</option>");

	print("</select>"
	     ."<input type='text' size='60' name='SEARCH' value='$search'>"
	     ."<input type='submit' value='Search Comments'>"
             ."</p></form>\n");

	print("<hr noshade/>\n");

        $comment = new comment();

        if ($stype == "hidden")
	  $matches = $comment->search($search, "-create_date", 0);
	else
	  $matches = $comment->search($search, "-create_date");

	$count = sizeof($matches);

        if ($count == 0)
	{
	  print("<p>No Comments found.</p>\n");

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

        print("<p>$count comment(s) found, showing $start to $end:</p>\n");

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

        html_start_table(array("ID","From","Posted", "Score"));

	for ($i = $start - 1; $i < $end && $comment->load($matches[$i]); $i ++)
	{
          html_start_row();

          $id   = $comment->id;
	  $link = "<a href='$PHP_SELF?L$id$options' alt='Comment #$id'>";

          print("<td nowrap>");
          if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    print("<input type='checkbox' name='ID_$id'>");
          print("$link$id</a></td>");

          $temp = htmlspecialchars($comment->display_name);

          print("<td align='center' width='50%'>$link$temp</a></td>");

          $temp = date("M d, Y", $comment->create_date);
          print("<td align='center'>$link$temp</a></td>"
	       ."<td align='center'>$comment->status</td>");

          html_end_row();

          html_start_row();
          $temp = html_format($comment->contents);
          print("<td></td><td colspan='3'>$temp</td>");
          html_end_row();
	}

        if ($LOGIN_LEVEL > 0)
	{
          html_start_row("header");
	  print("<td align='center' colspan='4'>&nbsp;<br /><select name='OP'>"
	       ."<option value=''>-- Choose --</option>"
	       ."<option value='delete'>Delete</option>"
	       ."<option value='up'>Moderate Up</option>"
	       ."<option value='down'>Moderate Down</option>"
	       ."</select>"
	       ."<input type='submit' value='Checked Comments'/></td>");
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
      }

      html_footer();
      break;

  case "M" :
  case "m" :
      if (array_key_exists("${PROJECT_MODULE}MODPOINTS", $_COOKIE))
  	$modpoints = $_COOKIE["${PROJECT_MODULE}MODPOINTS"];
      else
	$modpoints = 5;

      if ($modpoints > 0)
      {
	$modpoints --;

        setcookie("${PROJECT_MODULE}MODPOINTS", $modpoints, time() + 2 * 86400, "/");

        if ($op == "M")
	  db_query("UPDATE comment SET status = status + 1 WHERE "
	          ."id = $id AND status < 5");
        else
	  db_query("UPDATE comment SET status = status - 1 WHERE "
	          ."id = $id AND status > 0");
      }

      header("Location: " . str_replace("_", "?", $urlpath) .
	     "#_USER_COMMENT_$id");
      break;      

  case 'U' : // Submit/Modify Comment
      if ($LOGIN_USER == "")
      {
        header("Location: login.php?PAGE=comment.php?U$id");
	return;
      }

      $comment = new comment($id);
      if ($comment->id != $id)
      {
        html_header("Modify Comment #$id");
	print("<p><b>Error:</b> Comment #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($LOGIN_LEVEL < AUTH_DEVEL &&
          $id != 0 && $comment->create_user != $LOGIN_USER)
      {
        html_header("Modify Comment #$id");
	print("<p><b>Error:</b> You do not have permission to modify "
	     ."comment #$id!</p>\n");
	html_footer();
	exit();
      }

      if (!$id)
      {
	$comment->url = $urlpath;
	$comment->parent_id = $refer_id;
      }

      if ($REQUEST_METHOD == "POST")
        $havedata = $comment->loadform();
      else
        $havedata = 0;

      if ($havedata && !array_key_exists("preview", $_POST))
      {
        $comment->save();

	if (!$id || $urlpath != "")
	{
	  if (strpos($urlpath, "articles.php") !== FALSE)
	  {
	    $article_id = (int)substr($urlpath,
	                              strpos($urlpath, "articles.php") + 14);

            $article = new article($article_id);
	    $what    = "article #$article_id";
	    $email   = auth_user_email($article->create_user);
          }
	  else if (strpos($urlpath, "links.php") !== FALSE)
	  {
	    $link_id = (int)substr($urlpath,
	                           strpos($urlpath, "links.php") + 11);

            $link  = new link($link_id);
	    $what  = $link->name;
	    $email = auth_user_email($link->create_user);
          }
	  else if (ereg(".*\\.html", $urlpath))
	  {
	    $what  = $urlpath;
	    $email = "webmaster@easysw.com";
	  }
	  else
	    $email = "";

	  $refurl = str_replace("_", "?", $urlpath) .
	            "#_USER_COMMENT_$comment->id";

          if ($email != "")
	  {
	    mail($email, "Comment posted to $what",
		 "$comment->create_user posted the following comment:\n\n" .
		 wordwrap($comment->contents) .
		 "\n\nView/Reply At: http://www.htmldoc.org/$refurl",
		 "From: webmaster@easysw.com\r\n");
	  }

	  header("Location: $refurl");
        }
	else
	  header("Location: $PHP_SELF?L$id$options");
      }
      else
      {
        $links = array();

	$links["Return to Comments"] = "$PHP_SELF?L$options";
	if ($id)
	  $links["Comment #$id"] = "$PHP_SELF?L$id$options";

        if ($id)
          html_header("Modify Comment #$id", "", "", $links);
	else
          html_header("Submit Comment", "", "", $links);

	if ($REQUEST_METHOD == "POST" && !$havedata)
	  print("<p><b>Error:</b> Please fill in the fields marked "
	       ."<b class='invalid'>as follows</b> below and resubmit "
	       ."your comment.</p><hr noshade/>\n");

        $comment->edit($options);

        html_footer();
      }
      break;
}


//
// End of "$Id: comment.php,v 1.5 2005/03/29 19:29:41 mike Exp $".
//
?>
