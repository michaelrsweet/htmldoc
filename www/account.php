<?php
//
// "$Id: account.php,v 1.9 2005/03/08 21:22:46 mike Exp $"
//
// Account management page...
//

//
// Include necessary headers...
//

include_once "phplib/db-article.php";
include_once "phplib/db-str.php";
include_once "phplib/db-user.php";


//
// Developer terms and work interests...
//

$DEVEL_JOBS = array(
  "ENG" => "Electrical engineer",
  "ART" => "Graphics artist",
  "GUI" => "GUI application programmer/engineer",
  "CMD" => "Non-GUI application programmer/engineer",
  "STU" => "Student",
  "ADM" => "System administrator",
  "SYS" => "Systems engineer/designer",
  "DOC" => "Technical writer",
  "WEB" => "Web site developer",
  "OTH" => "Other computer/technical professional"
);

$DEVEL_TERMS = array(
  "GPL" => "provide code/documentation under GNU licenses only",
  "OSS" => "provide code/documentation under any OSI-approved license",
  "LIC" => "license all code/documentation freely to Easy Software Products",
  "CPR" => "assign all code/documentation copyrights to Easy Software Products"
);

$DEVEL_WORK = array(
  "NO" => "not interested in working for Easy Software Products",
  "CONT" => "interested in working as a contractor for Easy Software Products",
  "PART" => "interested in working as a part-time employee for Easy Software Products",
  "FULL" => "interested in working as a full-time employee for Easy Software Products"
);

$DEVEL_YEARS = array("1 year or less", "2 to 5 years", "6 or more years");


//
// 'account_header()' - Show standard account page header...
//

function
account_header($title, $id = 0)
{
  global $PHP_SELF, $LOGIN_ID, $LOGIN_USER, $LOGIN_LEVEL, $options;

  $links = array();

  $links["Home"] = "account.php?L$LOGIN_ID$options";

  if ($LOGIN_LEVEL == AUTH_ADMIN)
    $links["Add"] = "account.php?U$options";

  $links["Change Information/Password"] = "account.php?U$LOGIN_ID$options";

  if ($LOGIN_LEVEL == AUTH_ADMIN)
  {
    if ($id)
      $links["Delete #$id"] = "account.php?D$id$options";

    $links["Generate Files"] = "account.php?G$options";
    $links["Manage Accounts"] = "account.php?L$options";

    if ($id)
      $links["Modify #$id"] = "account.php?L$id$options";
  }

  if ($LOGIN_LEVEL > AUTH_USER)
    $links["New/Pending"] = "account.php?N$options";

  $links["Logout"] = "account.php?X";

  if ($LOGIN_LEVEL < AUTH_DEVEL)
    $links["Request Developer Status"] = "account.php?R$options";

  html_header($title, "", "", $links);
}


if ($argc == 1 && $argv[0] == "X")
  auth_logout();

if ($LOGIN_USER == "")
{
  header("Location: login.php");
  exit(0);
}

// Get command-line options...
//
// Usage: account.php [operation] [options]
//
// Operations:
//
// B         = Batch update selected users
// D#        = Delete user
// G         = Generate authentication files...
// L         = List all/current user
// L#        = List user #
// N         = Show new/pending stuff
// R         = Request developer status...
// U         = Create new user
// U#        = Modify user #
// X         = Logout
//
// Options:
//
// I#        = Set first user
// Qtext     = Set search text

$search = "";
$index  = 0;

if ($argc)
{
  $op = $argv[0][0];
  $id = (int)substr($argv[0], 1);

  if ($op != 'B' && $op != 'D' && $op != 'G' && $op != 'L' &&
      $op != 'N' && $op != 'R' && $op != 'U')
  {
    html_header("Account Error");
    print("<p>Bad command '$op'!\n");
    html_footer();
    exit();
  }

  if ($op == 'D' && !$id)
  {
    html_header("Account Error");
    print("<p>Command '$op' requires an ID!\n");
    html_footer();
    exit();
  }

  if (($op == 'L' || $op == 'U') && !$id && $LOGIN_LEVEL < AUTH_ADMIN)
    $id = $LOGIN_ID;

  if (($op == 'B' || $op == 'D' || $op == 'G') && $LOGIN_LEVEL < AUTH_ADMIN)
  {
    html_header("Account Error");
    print("<p>You don't have permission to use command '$op'!\n");
    html_footer();
    exit();
  }

  if ($op == 'N' && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    html_header("Account Error");
    print("<p>You don't have permission to use command '$op'!\n");
    html_footer();
    exit();
  }

  if (($op == 'L' || $op == 'U') && $id &&
      $id != $LOGIN_ID && $LOGIN_LEVEL < AUTH_ADMIN)
  {
    $user = new user($id);

    if ($user->id != $id)
    {
      html_header("Account Error");
      print("<p>Account #$id does not exist!\n");
      html_footer();
      exit();
    }

    if ($user->create_user != $LOGIN_USER)
    {
      html_header("Account Error");
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
      case 'I' : // Set first user
          $index = (int)$option;
	  if ($index < 0)
	    $index = 0;
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
      default :
	  html_header("Account Error");
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
  $id = $LOGIN_ID;
}

if ($REQUEST_METHOD == "POST")
{
  if (array_key_exists("SEARCH", $_POST))
    $search = $_POST["SEARCH"];
}

$options = "+I$index+Q" . urlencode($search);

switch ($op)
{
  case 'B' : // Batch update
      // Disable/enable/expire/etc. accounts...
      if ($REQUEST_METHOD == "POST" && array_key_exists("OP", $_POST))
      {
	$op = $_POST["OP"];

        db_query("BEGIN TRANSACTION");

        reset($_POST);
        while (list($key, $val) = each($_POST))
          if (substr($key, 0, 3) == "ID_")
	  {
	    $id = (int)substr($key, 3);

            if ($op == "disable")
              db_query("UPDATE user SET is_published = 0 WHERE id = $id");
            else if ($op == "enable")
              db_query("UPDATE user SET is_published = 1 WHERE id = $id");
            else if ($op == "delete")
              db_query("DELETE FROM user WHERE id = $id");
	  }

        db_query("COMMIT TRANSACTION");

        if (!auth_write_file())
	{
	  account_header("Batch Update Error");

          print("<p>Authentication files not created!</p>\n");

	  html_footer();
	  exit();
	}
      }

      header("Location: $PHP_SELF?L$options");
      break;

  case 'D' : // Delete
      break;

  case 'G' : // Generate auth files
      account_header("Generate Auth Files");

      if (auth_write_file())
	print("<p>Authentication files created!</p>\n");
      else
	print("<p>Authentication files not created!</p>\n");

      html_footer();
      break;

  case 'L' : // View/list
      if ($id)
      {
	// Show account info...
	$user = new user($id);

	if ($user->id != $id)
	{
	  html_header("Account Error");
	  print("<p>Account #$id does not exist!\n");
	  html_footer();
	  exit();
	}

	account_header($user->name);

        $user->view();

        print("<h1>Articles &amp; FAQs</h1>\n");

	$result = db_query("SELECT id FROM article WHERE "
	                  ."create_user = '$user->name' "
			  ."ORDER BY modify_date DESC, title");
	if (db_count($result) > 0)
	{
	  $count = db_count($result);

          if ($count > 5)
	  {
	    html_start_links();
	    html_link("Show All $count Articles", "articles.php?L+TMine");
	    html_end_links();
          }

          html_start_table(array("ID","Title","Last Modified", "Comment(s)"));

	  for ($i = 0; $i < 5 && $row = db_next($result); $i ++)
	  {
	    $article = new article($row["id"]);

            html_start_row();

            $id   = $article->id;
	    $link = "<a href='articles.php?L$id$options' alt='Article #$id'>";

            print("<td nowrap>$link$id</a></td>");

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

	  html_end_table();
        }
	else
	  print("<p>No articles or FAQs found.</p>\n");

        db_free($result);

        print("<h1>Bug and Feature Requests</h1>\n");

        if ($LOGIN_LEVEL >= AUTH_DEVEL && $LOGIN_ID == $id)
	  $manager = "manager_user = '' OR manager_user = '$user->name'";
	else
	  $manager = "manager_user = '$user->name'";

	$result = db_query("SELECT id FROM str WHERE "
	                  ."(create_user = '$user->name' OR $manager) AND "
			  ."status >= " . STR_STATUS_ACTIVE . " "
			  ."ORDER BY status DESC, priority DESC, "
			  ."modify_date DESC, id");
	if (db_count($result) > 0)
	{
	  $count = db_count($result);

          if ($count > 5)
	  {
	    html_start_links();
	    html_link("Show All $count STRs", "str.php?L+E1");
	    html_end_links();
          }

          html_start_table(array("Id", "Priority", "Status", "Scope",
	                	 "Summary", "Version", "Last Updated",
				 "Assigned To"));

	  for ($i = 0; $i < 5 && $row = db_next($result); $i ++)
	  {
	    $str = new str($row["id"]);

	    $date     = date("M d, Y", $str->modify_date);
            $summary  = htmlspecialchars($str->summary, ENT_QUOTES);
	    $summabbr = htmlspecialchars(abbreviate($str->summary, 80), ENT_QUOTES);
	    $prtext   = $STR_PRIORITY_SHORT[$str->priority];
            $sttext   = $STR_STATUS_SHORT[$str->status];
            $sctext   = $STR_SCOPE_SHORT[$str->scope];
	    $link     = "<a href='str.php?L$str->id' "
	               ."alt='STR #$str->id: $summary'>";

            html_start_row();

            if ($str->is_published == 0)
	      $summabbr .= " <img src='${path}images/private.gif' width='16' height='16' "
	                  ."border='0' align='middle' alt='Private'/>";

            print("<td nowrap>");
	    print("$link$str->id</a></td>"
		 ."<td align='center'>$link$prtext</a></td>"
		 ."<td align='center'>$link$sttext</a></td>"
		 ."<td align='center'>$link$sctext</a></td>"
		 ."<td>$link$summabbr</a></td>"
		 ."<td align='center'>$link$str->str_version</a></td>"
		 ."<td align='center' nowrap>$link$date</a></td>");

	    if ($str->manager_user != "")
	      $email = sanitize_email($str->manager_user);
	    else
	      $email = "<i>Unassigned</i>";

	    print("<td align='center'>$link$email</a></td>");

	    html_end_row();

            if ($str->status >= STR_STATUS_PENDING)
	    {
              $textresult = db_query("SELECT * FROM strtext "
	                            ."WHERE str_id = $str->id "
	                            ."ORDER BY id DESC LIMIT 1");
              if ($textresult && db_count($textresult) > 0)
	      {
		$textrow = db_next($textresult);

        	html_start_row();

		$email    = sanitize_email($textrow['create_user']);
		$contents = abbreviate(quote_text($textrow['contents']), 128);

		print("<td align='center' valign='top' colspan='3'>$email</td>"
		     ."<td valign='top' colspan='5' width='100%'>"
		     ."<tt>$contents</tt></td>");

		html_end_row();

		db_free($textresult);
	      }
            }
	  }

	  html_end_table();
        }
	else
	  print("<p>No STRs found.</p>\n");

        db_free($result);
        print("<h1>Links</h1>\n");

	$result = db_query("SELECT id FROM link WHERE "
	                  ."create_user = '$user->name' "
			  ."ORDER BY modify_date DESC, name");
	if (db_count($result) > 0)
	{
	  $count = db_count($result);
	  if ($count > 5)
	  {
	    html_start_links();
	    html_link("Show All $count Links", "links.php?LM");
	    html_end_links();
          }

	  print("<ul>\n");

	  for ($i = 0; $i < 5 && $row = db_next($result); $i ++)
	  {
            $id = $row["id"];
	    $link = new link($id);

	    $name        = htmlspecialchars($link->name);
	    $description = html_format($link->description);
	    $version     = htmlspecialchars($link->version);
            $age         = (int)((time() - $link->modify_date) / 86400);

            print("<li><b><a href='links.php?V$id$options'>$name $version</a></b>");

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

	    print(" [&nbsp;<a href='links.php?UL$id+P$link->parent_id'>Edit</a>"
		 ." | <a href='links.php?X$id+P$link->parent_id'>Delete</a>&nbsp;]\n");

            print("$description</li>\n");
	  }

	  print("</ul>\n");
	}
	else
	  print("<p>No listings found.</p>\n");

        db_free($result);

	html_footer();
      }
      else
      {
        // List accounts...
	account_header("Manage Accounts");

        print("<form method='POST' action='$PHP_SELF?L'><p align='center'>"
	     ."Search:&nbsp;"
	     ."<input type='text' size='60' name='SEARCH' value='$search'>"
	     ."<input type='submit' value='Search Accounts'>"
             ."</p></form>\n");

	print("<hr noshade/>\n");

        $user    = new user();
        $matches = $user->search($search, "name");
	$count   = sizeof($matches);

        if ($count == 0)
	{
	  print("<p>No accounts found.</p>\n");

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

        print("<p>$count user(s) found, showing $start to $end:</p>\n");

        print("<form method='POST' action='$PHP_SELF?B$options'>\n");

        if ($count > $PAGE_MAX)
	{
          print("<p><table border='0' cellspacing='0' cellpadding='0' "
	       ."width='100%'>\n");

          print("<tr><td>");
	  if ($index > 0)
	    print("[&nbsp;<a href='$PHP_SELF?L+I$prev+Q" . urlencode($search)
		 ."'>Previous&nbsp;$PAGE_MAX</a>&nbsp;]");
          print("</td><td align='right'>");
	  if ($end < $count)
	  {
	    $next_count = min($PAGE_MAX, $count - $end);
	    print("[&nbsp;<a href='$PHP_SELF?L+I$next+Q" . urlencode($search)
		 ."'>Next&nbsp;$next_count</a>&nbsp;]");
          }
          print("</td></tr>\n");
	  print("</table></p>\n");
        }

        html_start_table(array("Username", "EMail", "Level"));

	for ($i = $start - 1; $i < $end; $i ++)
	{
	  $user->load($matches[$i]);

	  if ($user->id != $matches[$i])
	    continue;

	  $name  = htmlspecialchars($user->name, ENT_QUOTES);
	  $email = htmlspecialchars($user->email, ENT_QUOTES);
	  $level = $AUTH_LEVELS[$user->level];

          if (!$user->is_published)
	    $email .= " <img src='images/private.gif' width='16' height='16' "
	             ."border='0' align='middle' alt='Private'/>";

	  html_start_row();
	  print("<td nowrap><input type='checkbox' name='ID_$user->id'/>"
	       ."<a href='$PHP_SELF?U$user->id$options'>$name</a>"
	       ." [&nbsp;<a href='$PHP_SELF?L$user->id$options'>View</a>&nbsp;]</td>"
	       ."<td align='center'><a href='$PHP_SELF?U$user->id$options'>"
	       ."$email</a></td>"
	       ."<td align='center'><a href='$PHP_SELF?U$user->id$options'>"
	       ."$level</a></td>");
	  html_end_row();
	}

        html_start_row("header");
	print("<td align='center' colspan='3'>&nbsp;<br /><select name='OP'>"
	     ."<option value=''>-- Choose --</option>"
	     ."<option value='delete'>Delete</option>"
	     ."<option value='disable'>Disable</option>"
	     ."<option value='enable'>Enable</option>"
	     ."</select>"
	     ."<input type='submit' value='Checked Accounts'/></td>");
	html_end_row();

	html_end_table();

        if ($count > $PAGE_MAX)
	{
          print("<p><table border='0' cellspacing='0' cellpadding='0' "
	       ."width='100%'>\n");

          print("<tr><td>");
	  if ($index > 0)
	    print("[&nbsp;<a href='$PHP_SELF?L+I$prev+Q" . urlencode($search)
		 ."'>Previous&nbsp;$PAGE_MAX</a>&nbsp;]");
          print("</td><td align='right'>");
	  if ($end < $count)
	  {
	    $next_count = min($PAGE_MAX, $count - $end);
	    print("[&nbsp;<a href='$PHP_SELF?L+I$next+Q" . urlencode($search)
		 ."'>Next&nbsp;$next_count</a>&nbsp;]");
          }
          print("</td></tr>\n");
	  print("</table></p>\n");
        }

        print("<p><img src='images/private.gif' width='16' height='16' "
	     ."align='middle' alt='private'/> = hidden from public view</p>\n");

        html_footer();
      }
      break;

  case 'N' : // New/pending
      // New/Pending
      account_header("New/Pending");

      print("<h2>New/Pending Articles:</h2>\n");

      $result = db_query("SELECT * FROM article WHERE is_published = 0 "
	                ."ORDER BY modify_date");
      $count  = db_count($result);

      if ($count == 0)
	print("<p>No new/pending articles found.</p>\n");
      else
      {
        html_start_table(array("Id", "Title", "Last Updated"));

	while ($row = db_next($result))
	{
	  $id       = $row['id'];
          $title    = htmlspecialchars($row['title'], ENT_QUOTES) .
	              " <img src='images/private.gif' width='16' height='16' "
	             ."border='0' align='middle' alt='Private'/>";
          $abstract = htmlspecialchars($row['abstract'], ENT_QUOTES);
	  $date     = date("M d, Y", $row['modify_date']);

          html_start_row();

          print("<td align='center' nowrap><a "
	       ."href='articles.php?L$id$options'>$id</a></td>"
	       ."<td width='67%' align='center'><a "
	       ."href='articles.php?L$id$options'>$title</a></td>"
	       ."<td align='center'><a "
	       ."href='articles.php?L$id$options'>$date</a></td>");

	  html_end_row();

          html_start_row();

	  print("<td></td><td colspan='2'>$abstract</td>");

	  html_end_row();
	}

        html_end_table();
      }

      db_free($result);

      print("<h2>New/Pending Links:</h2>\n");

      $result = db_query("SELECT * FROM link WHERE is_published = 0 "
	                ."ORDER BY modify_date");
      $count  = db_count($result);

      if ($count == 0)
	print("<p>No new/pending links found.</p>\n");
      else
      {
        html_start_table(array("Id", "Name/Version", "Last Updated"));

	while ($row = db_next($result))
	{
	  $id       = $row['id'];
          $title    = htmlspecialchars($row['name'], ENT_QUOTES) . " " .
	              htmlspecialchars($row['version'], ENT_QUOTES) .
	              " <img src='images/private.gif' width='16' height='16' "
	             ."border='0' align='middle' alt='Private'/>";
	  $date     = date("M d, Y", $row['modify_date']);

          if ($row["is_category"])
	    $link = "<a href='links.php?UC$id'>";
	  else
	    $link = "<a href='links.php?UL$id'>";

          html_start_row();

          print("<td align='center' nowrap>$link$id</a></td>"
	       ."<td width='67%' align='center'>$link$title</a></td>"
	       ."<td align='center'>$link$date</a></td>");

	  html_end_row();
	}

        html_end_table();
      }

      db_free($result);

      print("<h2>New/Pending STRs:</h2>\n");

      $result = db_query("SELECT * FROM str WHERE status >= " . STR_STATUS_PENDING
	                ." AND (manager_user == '' OR manager_user = '$LOGIN_USER') "
	                ."ORDER BY status DESC, priority DESC, scope DESC, "
			."modify_date");
      $count  = db_count($result);

      if ($count == 0)
	print("<p>No new/pending STRs found.</p>\n");
      else
      {
        html_start_table(array("Id", "Priority", "Status", "Scope",
	                       "Summary", "Version", "Last Updated",
			       "Assigned To"));

	while ($row = db_next($result))
	{
	  $date     = date("M d, Y", $row['modify_date']);
          $summary  = htmlspecialchars($row['summary'], ENT_QUOTES);
	  $summabbr = htmlspecialchars(abbreviate($row['summary'], 80), ENT_QUOTES);
	  $prtext   = $priority_text[$row['priority']];
          $sttext   = $status_text[$row['status']];
          $sctext   = $scope_text[$row['scope']];

          if ($row['is_published'] == 0)
	    $summabbr .= " <img src='images/private.gif' width='16' height='16' "
	                ."border='0' align='middle' alt='Private'/>";

          html_start_row();

          print("<td nowrap>"
	       ."<a href='str.php?L$row[id]$options' alt='STR #$row[id]: $summary'>"
	       ."$row[id]</a></td>"
	       ."<td align='center'>$prtext</td>"
	       ."<td align='center'>$sttext</td>"
	       ."<td align='center'>$sctext</td>"
	       ."<td align='center'><a href='str.php?L$row[id]$options' "
	       ."alt='STR #$row[id]: $summary'>$summabbr</a></td>"
	       ."<td align='center'>$row[str_version]</td>"
	       ."<td align='center'>$date</td>");

	  if ($row['manager_user'] != "")
	    $email = sanitize_email($row['manager_user']);
	  else
	    $email = "<i>Unassigned</i>";

	  print("<td align='center'>$email</td>");

	  html_end_row();
	}

        html_end_table();
      }

      db_free($result);

      // Show hidden comments...
      print("<h2>Hidden Comments:</h2>\n");

      $result = db_query("SELECT * FROM comment WHERE status = 0 ORDER BY id");

      if (db_count($result) == 0)
        print("<p>No hidden comments.</p>\n");
      else
      {
        print("<ul>\n");

        while ($row = db_next($result))
	{
	  $create_date  = date("M d, Y", $row['create_date']);
	  $create_user  = sanitize_email($row['create_user']);
	  $contents     = sanitize_text($row['contents']);
          $location     = str_replace("_", "?", $row['url']);

	  print("<li><a href='$location'>$row[url]</a> "
	       ." by $create_user on $create_date "
	       ."[&nbsp;<a href='comment.php?U$row[id]+P$row[url]'>Edit</a> "
	       ."| <a href='comment.php?D$row[id]+P$row[url]'>Delete</a>&nbsp;"
	       ."]<br /><tt>$contents</tt></li>\n");
	}

        print("</ul>\n");
      }

      db_free($result);

      html_footer();
      break;

  case 'R' : // Request Developer Status
      $skills    = "";
      $interests = "";
      $years     = "";
      $havedata  = 0;
      $terms     = "";
      $job       = "";
      $work      = "";

      if ($REQUEST_METHOD == "POST")
      {
        if (array_key_exists("skills", $_POST))
	  $skills = trim($_POST["skills"]);

        if (array_key_exists("years", $_POST))
	  $years = $_POST["years"];

        if (array_key_exists("terms", $_POST) &&
	    array_key_exists($_POST["terms"], $DEVEL_TERMS))
	  $terms = $_POST["terms"];

        if (array_key_exists("job", $_POST) &&
	    array_key_exists($_POST["job"], $DEVEL_JOBS))
	  $job = $_POST["job"];

        if (array_key_exists("work", $_POST) &&
	    array_key_exists($_POST["work"], $DEVEL_WORK))
	  $work = $_POST["work"];

        if (array_key_exists("interests", $_POST))
	  $interests = $_POST["interests"];

        if ($projects && $skills != "" && $years != "" &&
	    $terms != "" && $work != "" && $job != "" && $interests != "")
	  $havedata = 1;
      }

      account_header("Request Developer Status");

      if ($havedata)
      {
        $message = "$LOGIN_USER has requested developer status for HTMLDOC.\n\n"
	          ."$LOGIN_USER ($LOGIN_EMAIL) has $years of programming "
		  ."experience, will " . $DEVEL_TERMS[$terms]
		  .", is " . $DEVEL_WORK[$work]
		  .", is a " . $DEVEL_JOBS[$job]
		  .", and has the following skills:\n\n"
		   ."$skills\n\n"
		   ."$LOGIN_USER is interested in working on the following:\n\n"
		   ."$interests\n\n";

        mail("mike@easysw.com", "HTMLDOC Developer Status Request from $LOGIN_USER",
	     wordwrap($message), "From: $LOGIN_EMAIL\r\n");

        print("<p>Your request has been sent to Easy Software Products. "
	     ."You should receive a response via email within 2 weeks.</p>\n");
      }
      else
      {
	print("<p>We are always looking for talented developers to contribute "
             ."to HTMLDOC or a HTMLDOC-related project. If you are accepted as a "
	     ."HTMLDOC developer, you will have write access to the HTMLDOC "
	     ."repositories and will be able to provide new "
	     ."software and documentation for the HTMLDOC project directly. "
	     ."Please fill out the questionaire below to request developer "
	     ."status on your account.</p>\n");

	if ($REQUEST_METHOD == "POST")
          print("<p><b>Note:</b> Please answer the questions "
	       ."<span class='invalid'>marked like this</span>.</p>\n");

	$user  = htmlspecialchars($LOGIN_USER);
	$email = htmlspecialchars($LOGIN_EMAIL);

	print("<form action='$PHP_SELF?R$options' method='POST'>\n");

        // skills (1)
        $html = htmlspecialchars($skills);
	if ($skills != "" || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>1. What skills do you offer as a developer "
	     ."(e.g. languages, work experience, etc.)?</h3>\n"
	     ."<p><textarea name='skills' cols='80' rows='4' wrap='virtual'>"
	     ."$html</textarea></p>\n");

        // interests (2)
        $html = htmlspecialchars($interests);
	if ($interests != "" || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>2. What do you want to do, specifically, "
	     ."as a HTMLDOC developer?</h3>\n"
	     ."<p><textarea name='interests' cols='80' rows='4' wrap='virtual'>"
	     ."$html</textarea></p>\n");

        // job (3)
	if ($job != "" || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>3. What best describes your current "
	     ."job/field?</h3>\n<p>");

	reset($DEVEL_JOBS);
	while (list($name, $text) = each($DEVEL_JOBS))
	{
          print("<input type='radio' name='job' value='$name'");
	  if ($work == $name)
	    print(" checked");
	  print("/>$text<br />");
	}
	print("</p>\n");

        // years (4)
	if ($years >= 0 || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>4. How many years have you worked "
	     ."professionally in your current field?</h3>\n<p>");

        reset($DEVEL_YEARS);
	foreach ($DEVEL_YEARS as $val)
	{
	  print("<input type='radio' name='years' value='$val'");
	  if ($years == $val)
	    print(" checked");
          print("/>$val<br />");
	}
	print("</p>\n");

        // terms (5)
	if ($terms != "" || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>5. Easy Software Products licenses HTMLDOC "
	     ."under both open-source and commercial terms. What terms will "
	     ."you use for the code and/or documentation you contribute to "
	     ."the HTMLDOC project?</h3>\n<p>");

	reset($DEVEL_TERMS);
	while (list($name, $text) = each($DEVEL_TERMS))
	{
          print("<input type='radio' name='terms' value='$name'");
	  if ($terms == $name)
	    print(" checked");
	  print("/>I will $text<br />");
	}
	print("</p>\n");

        // work (6)
	if ($work != "" || $REQUEST_METHOD == "GET")
	  $hclass = "valid";
	else
	  $hclass = "invalid";
        print("<h3 class='$hclass'>6. Are you interested in working for "
	     ."Easy Software Products?</h3>\n<p>");

	reset($DEVEL_WORK);
	while (list($name, $text) = each($DEVEL_WORK))
	{
          print("<input type='radio' name='work' value='$name'");
	  if ($work == $name)
	    print(" checked");
	  print("/>I am $text<br />");
	}
	print("</p>\n");

	// Submit
	print("<p><input type='submit' value='Request Developer Status'/></p>\n"
             ."</form>\n");
      }

      html_footer();
      break;

  case 'U' : // Update/create
      $user = new user($id);

      if ($user->id != $id)
      {
	html_header("Account Error");
	print("<p>Account #$id does not exist!\n");
	html_footer();
	exit();
      }

      account_header($id ? "Modify User $user->name" : "Create User");

      if ($REQUEST_METHOD == "POST" && $user->loadform())
      {
        $user->save();

        if ($id == $LOGIN_ID && array_key_exists("password", $_POST))
	{
	  // Re-login using the new password...
	  auth_login($LOGIN_USER, $_POST["password"]);
	}

        if (auth_write_file())
	{
          if ($id)
	    print("<p>Account $user->name modified successfully!</p>\n");
	  else
	    print("<p>Account $user->name added successfully!</p>\n");
        }
	else
	  print("<p>Authentication files not created!</p>\n");
      }
      else
	$user->edit($options);

      html_footer();
      break;

  default :
      account_header("Unknown Operation");
      print("<p>The operation code '$op' is unknown or unimplemented...</p>\n");
      html_footer();
      break;
}


//
// End of "$Id: account.php,v 1.9 2005/03/08 21:22:46 mike Exp $".
//
?>
