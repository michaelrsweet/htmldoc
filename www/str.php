<?php
//
// "$Id: str.php,v 1.21 2006/07/11 20:33:21 mike Exp $"
//
// Software Trouble Report page...
//
// Contents:
//
//   add_file()     - Add a file to a STR...
//   add_text()     - Add text to a STR...
//   notify_users() - Notify users of STR changes...
//   str_history()  - Show previous files and text posts.
//

//
// Include necessary headers...
//

include_once "phplib/db-str.php";


//
// 'add_file()' - Add a file to a STR...
//

function				// O - FALSE on failure, filename on success
add_file($str)				// I - STR
{
  global $_FILES, $_GET, $_POST, $LOGIN_EMAIL;


  // Grab form data...
  if (array_key_exists("file", $_FILES))
    $file = $_FILES["file"];
  else
    return ("");

  $filename = $file['name'];
  if (strlen($filename) < 1 || $filename[0] == '.' ||
      $filename[0] == '/' || $filename == "")
    return ("");

  // Get the source and destination filenames...
  $tmpname = $file['tmp_name'];
  $strname = "strfiles/$str->id/$filename";

  if (file_exists($strname))
  {
    // Rename file to avoid conflicts...
    for ($i = 2; $i < 1000; $i ++)
    {
      if (preg_match("/.*\\..*/", $filename))
	$temp = preg_replace("/([^\\.]*)\\.(.*)/",
                             "\\1_v$i.\\2", $filename);
      else
        $temp = "${filename}_v$i";

      if (!file_exists("strfiles/$str->id/$temp"))
        break;
    }

    // Only support up to 1000 versions of the same file...
    if ($i >= 1000)
      return (FALSE);

    $filename = $temp;
    $strname  = "strfiles/$str->id/$filename";
  }

  // Open files...
  $infile = fopen($tmpname, "rb");

  if (!$infile)
    return (FALSE);

  // Make the attachment directory...
  if (!file_exists("strfiles/$str->id"))
    mkdir("strfiles/$str->id");

  $outfile = fopen($strname, "wb");

  if (!$outfile)
  {
    fclose($infile);
    return (FALSE);
  }

  // Copy the attachment...
  while ($data = fread($infile, 8192))
    fwrite($outfile, $data);

  // Close files...
  fclose($infile);
  fclose($outfile);

  // Create the file record...
  $file = new strfile();

  $file->str_id       = $str->id;
  $file->is_published = 1;
  $file->filename     = $filename;
  $file->create_date  = time();
  $file->create_user  = $str->modify_user;

  // Save it...
  if (!$file->save())
    return (FALSE);

  return ($filename);
}


//
// 'add_text()' - Add text to a STR...
//

function				// O - FALSE on failure, text on success
add_text($str)				// I - STR
{
  global $_GET, $_POST, $LOGIN_EMAIL, $STR_MESSAGES;


  // Get form data...
  $contents = "";

  if (array_key_exists("message", $_GET))
    $contents .= $STR_MESSAGES[$_GET["message"]];
  else if (array_key_exists("message", $_POST))
    $contents .= $STR_MESSAGES[$_POST["message"]];

  if (((array_key_exists("message", $_GET) && $_GET["message"] != "") ||
       (array_key_exists("message", $_POST) && $_POST["message"] != "")) &&
      (array_key_exists("contents", $_GET) ||
       array_key_exists("contents", $_POST)))
    $contents .= "\n\n";

  if (array_key_exists("contents", $_GET))
    $contents .= $_GET["contents"];
  else if (array_key_exists("contents", $_POST))
    $contents .= $_POST["contents"];

  $contents = trim(str_replace("\r\n", "\n", $contents));

  if ($contents == "")
    return ("");

  // Create a new text record...
  $text = new strtext();

  $text->str_id       = $str->id;
  $text->is_published = 1;
  $text->contents     = $contents;
  $text->create_date  = time();
  $text->create_user  = $str->modify_user;

  // Save it...
  if (!$text->save())
    return (FALSE);

  return ($contents);
}


//
// 'notify_users()' - Notify users of STR changes...
//

function
notify_users($str,			// I - STR
	     $contents = "",		// I - Notification message, if any
	     $file = "",		// I - Attached file, if any
	     $what = "Re: ")		// I - Reply or new message
{
  global $STR_PRIORITY_SHORT, $STR_STATUS_LONG;
  global $PROJECT_EMAIL, $PROJECT_URL, $PROJECT_MODULE;


  // Make sure contents is non-empty for file attachments...
  if ($contents == "" && $file != "")
    $contents = "Attached file \"$file\"...";

  // Set the primary recipient of the message...
  if ($str->manager_user != "")
  {
    // Send the email to either the manager user or create user, depending
    // on who modified the STR...
    if ($str->modify_user != $str->manager_user)
      $to = auth_user_email($str->manager_user);
    else
      $to = auth_user_email($str->create_user);
  }
  else
    $to = $PROJECT_EMAIL;

  $from = auth_user_email($str->modify_user);

  if ($str->status >= STR_STATUS_ACTIVE)
    $replyto = "noreply@easysw.com";
  else
    $replyto = $PROJECT_EMAIL;

  // Setup the message and headers...
  $subject  = "${what}[" . $STR_PRIORITY_SHORT["$str->priority"]
             ."] Bug #$str->id: $str->summary";
  $headers  = "From: $from\n"
             ."Reply-To: $replyto\n";
  if ($str->status >= STR_STATUS_ACTIVE)
    $message  = "DO NOT REPLY TO THIS MESSAGE.  INSTEAD, POST ANY RESPONSES TO "
               ."THE LINK BELOW.\n\n";
  else
    $message = "";

  $message .= "[Bug " . substr($STR_STATUS_LONG[$str->status], 4) . "]\n"
             ."\n"
             . wordwrap(trim($contents)) . "\n"
	     ."\n"
	     ."Link: ${PROJECT_URL}str.php?L$str->id\n"
	     ."Version: $str->str_version\n";

  if ($str->fix_version != "")
  {
    // Add fix version
    $message .= "Fix Version: $str->fix_version";
    if ($str->fix_revision != 0)
      $message .= " (r$str->fix_revision)";
    $message .= "\n";
  }

  // Carbon copy create user, devel/bug lists, and interested addressees...
  if ($str->modify_user != $str->create_user)
    $headers .= "Cc: " . auth_user_email($str->create_user) . "\n";

  if ($str->manager_user != "" && $str->status <= STR_STATUS_UNRESOLVED)
  {
    // Carbon copy the email to the project address...
    $headers .= "Cc: $PROJECT_EMAIL\n";
  }

  $ccresult = db_query("SELECT email FROM carboncopy WHERE "
                      ."url = 'str.php_L$str->id'");
  if ($ccresult)
  {
    while ($ccrow = db_next($ccresult))
      $headers .= "Cc: $ccrow[email]\n";

    db_free($ccresult);
  }

  // Check for file attachments...
  if ($file != "")
    $bytes = filesize("strfiles/$str->id/$file");
  else
    $bytes = 0;

  if ($bytes > 0 && $bytes <= 102400 && !preg_match("/\\.zip\$/i", $file))
  {
    // Attach the file to the message...
    if (preg_match("/\\.(c|cc|cpp|cxx|diff|diffs|h|hpp|htm|html|txt|patch|"
		  ."php)\$/i", $file))
      $content_type = "text/plain";
    else if (preg_match("/\\.bmp\$/i", $file))
      $content_type = "image/bmp";
    else if (preg_match("/\\.gif\$/i", $file))
      $content_type = "image/gif";
    else if (preg_match("/\\.png\$/i", $file))
      $content_type = "image/png";
    else if (preg_match("/\\.jpg\$/i", $file))
      $content_type = "image/jpeg";
    else if (preg_match("/\\.pdf\$/i", $file))
      $content_type = "application/pdf";
    else
      $content_type = "application/octet-stream";

    $headers .= "Mime-Version: 1.0\n"
               ."Content-Type: multipart/mixed; boundary=\"PART-BOUNDARY\"\n"
               ."Content-Transfer-Encoding: 8bit\n"
	       ."\n";
    $body    = "--PART-BOUNDARY\n"
              ."Content-Type: text/plain\n"
	      ."\n"
	      ."$message"
	      ."--PART-BOUNDARY\n"
	      ."Content-Type: $content_type\n"
	      ."Content-Disposition: attachment; filename=\"$file\"\n";

    if ($content_type == "text/plain")
    {
      $data = str_replace("\r\n", "\n",
                          file_get_contents("strfiles/$str->id/$file"));

      $body .= "\n"
	      ."$data\n";
    }
    else
    {
      $data = chunk_split(base64_encode(
                          file_get_contents("strfiles/$str->id/$file")),
			  76, "\n");

      $body .= "Content-Transfer-Encoding: BASE64\n"
	      ."Content-Length: $bytes\n"
	      ."\n"
	      ."$data\n";
    }

    $body .= "--PART-BOUNDARY--\n";
  }
  else
  {
    // Message without attachment...
    $headers .= "Mime-Version: 1.0\n"
               ."Content-Type: text/plain\n";
    $body    = $message;

    // Add URL to attachment file, since it is too big to email...
    if ($file != "")
      $body .= "Attachment: ${PROJECT_URL}strfiles/$str->id/$file\n";
  }

  // Send the email notification...
  mail($to, $subject, $body, $headers);
}


//
// 'str_history()' - Show previous files and text posts.
//

function
str_history($str,			// I - STR to show
            $allowpost = FALSE,		// I - Show "post text/file" links
            $options = "")		// I - Options to pass along
{
  global $PHP_SELF, $LOGIN_LEVEL;


  print("<hr noshade><p><b>Bug Report Files:</b></p>\n");
  if ($str->status >= STR_STATUS_ACTIVE && $allowpost)
  {
    html_start_links();
    html_link("Post File", "$PHP_SELF?F$str->id$options");
    html_end_links();
  }

  $strfile = new strfile();
  $matches = $strfile->search($str->id);

  if (sizeof($matches) == 0)
    print("<p><i>No files</i></p>\n");
  else
  {
    html_start_table(array("Name/Time/Date", "Filename/Size"));

    for ($i = 0; $i < sizeof($matches); $i ++)
    {
      $strfile->load($matches[$i]);

      $date     = date("M d, Y", $strfile->create_date);
      $time     = date("H:i", $strfile->create_date);
      $email    = sanitize_email($strfile->create_user);
      $filename = htmlspecialchars($strfile->filename, ENT_QUOTES);
      $filesize = filesize("strfiles/$str->id/$strfile->filename");

      if ($filesize < 262144)
        $filesize = sprintf("%.0fk", ($filesize + 1023) / 1024.0);
      else
        $filesize = sprintf("%.1fM", $filesize / 1024.0 / 1024.0);

      html_start_row();
      print("<td align='center' valign='top'>$email<br>$time $date");

      if ($LOGIN_LEVEL >= AUTH_DEVEL)
      {
        print("<form method='POST' action='$PHP_SELF?U$str->id$options'>"
	     ."<input type='hidden' name='FILE_ID' value='$strfile->id'>");

        if ($strfile->is_published)
	  print("<input type='hidden' name='is_published' value='0'>"
	       ."<input type='submit' value='Hide'>");
        else
	  print("<input type='hidden' name='is_published' value='1'>"
	       ."<input type='submit' value='Show'>");

	print("</form>");
      }

      print("</td>"
           ."<td align='center' valign='top' width='100%'>"
	   ."<a href='strfiles/$str->id/$filename'>$filename</a><br>"
	   ."$filesize</td>");
      html_end_row();
    }

    html_end_table();
  }

  print("<hr noshade><p><b>Bug Report Dialog:</b></p>\n");
  if ($str->status >= STR_STATUS_ACTIVE && $allowpost)
  {
    html_start_links();
    html_link("Post Text", "$PHP_SELF?T$str->id$options");
    html_end_links();
  }

  $strtext = new strtext();
  $matches = $strtext->search($str->id);

  if (sizeof($matches) == 0)
    print("<p><i>No text</i></p>\n");
  else
  {
    html_start_table(array("Name/Time/Date", "Text"));

    for ($i = 0; $i < sizeof($matches); $i ++)
    {
      $strtext->load($matches[$i]);

      $date     = date("M d, Y", $strtext->create_date);
      $time     = date("H:i", $strtext->create_date);
      $email    = sanitize_email($strtext->create_user);
      $contents = quote_text($strtext->contents);

      html_start_row();
      print("<td align='center' valign='top'>$email<br>$time $date");

      if ($LOGIN_LEVEL >= AUTH_DEVEL)
      {
        print("<form method='POST' action='$PHP_SELF?U$str->id$options'>"
	     ."<input type='hidden' name='TEXT_ID' value='$strtext->id'>");

        if ($strtext->is_published)
	  print("<input type='hidden' name='is_published' value='0'>"
	       ."<input type='submit' value='Hide'>");
        else
	  print("<input type='hidden' name='is_published' value='1'>"
	       ."<input type='submit' value='Show'>");

	print("</form>");
      }

      print("</td><td valign='top'><tt>$contents</tt></td>");
      html_end_row();
    }

    html_end_table();
  }
}


// Get command-line options...
//
// Usage: str.php [operation] [options]
//
// Operations:
//
// B         = Batch update selected STRs
// F#        = Post file for Bug #
// L         = List all STRs
// L#        = List Bug #
// N#        = Update notification for Bug #
// T#        = Post text for Bug #
// U         = Post new STR
// U#        = Modify Bug #
//
// Options:
//
// I#        = Set first STR
// P#        = Set priority filter
// S#        = Set status filter
// C#        = Set scope filter
// E#        = Set user filter
// M#        = Set maximum STRs per page
// Qtext     = Set search text

$priority = 0;
$status   = -2;
$scope    = 0;
$search   = "";
$index    = 0;
$femail   = 0;

if ($argc)
{
  $op = $argv[0][0];
  $id = (int)substr($argv[0], 1);

  if ($op != 'L' && $op != 'T' && $op != 'F' &&
      $op != 'N' && $op != 'U' && $op != 'B')
  {
    html_header("Bugs & Features Error");
    print("<p>Bad command '$op'!</p>\n");
    html_footer();
    exit();
  }

  if ((($op == 'U' && $id > 0) || $op == 'B') && $LOGIN_LEVEL < AUTH_DEVEL)
  {
    html_header("Bugs & Features Error");
    print("<p>The '$op' command is not available to you!</p>\n");
    html_footer();
    exit();
  }

  if (($op == 'N' || $op == 'T' || $op == 'F') && $id <= 0)
  {
    html_header("Bugs & Features Error");
    print("<p>Command '$op' requires an STR number!</p>\n");
    html_footer();
    exit();
  }

  for ($i = 1; $i < $argc; $i ++)
  {
    $option = substr($argv[$i], 1);

    switch ($argv[$i][0])
    {
      case 'C' : // Set scope filter
	  $scope = (int)$option;
	  break;
      case 'E' : // Show only problem reports matching the current user
	  $femail = (int)$option;
	  break;
      case 'I' : // Set first STR
	  $index = (int)$option;
	  if ($index < 0)
	    $index = 0;
	  break;
      case 'M' : // Set max STRs per page
	  $PAGE_MAX = (int)$option;
	  break;
      case 'P' : // Set priority filter
	  $priority = (int)$option;
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
      case 'S' : // Set status filter
	  $status = (int)$option;
	  break;
      default :
	  html_header("Bugs & Features Error");
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
  if (array_key_exists("FPRIORITY", $_POST))
    $priority = (int)$_POST["FPRIORITY"];
  if (array_key_exists("FSTATUS", $_POST))
    $status = (int)$_POST["FSTATUS"];
  if (array_key_exists("FSCOPE", $_POST))
    $scope = (int)$_POST["FSCOPE"];
  if (array_key_exists("FEMAIL", $_POST))
    $femail = (int)$_POST["FEMAIL"];
  if (array_key_exists("FPAGEMAX", $_POST))
  {
    $PAGE_MAX = (int)$_POST["FPAGEMAX"];
    setcookie("${PROJECT_MODULE}PAGEMAX", $PAGE_MAX, time() + 365 * 86400);
  }
  if (array_key_exists("SEARCH", $_POST))
    $search = $_POST["SEARCH"];
}

$options = "+P$priority+S$status+C$scope+I$index+E$femail+M$PAGE_MAX+Q" .
	   urlencode($search);

// B         = Batch update selected STRs
// F#        = Post file for Bug #
// L         = List all STRs
// L#        = List Bug #
// N#        = Update notification for Bug #
// T#        = Post text for Bug #
// U         = Post new STR
// U#        = Modify Bug #

if ($op == "L" || ($op == "U" && $id != 0))
{
  // Set $strlinks to point to the previous and next STRs in the
  // current search, respectively...
  $str     = new str();
  $matches = $str->search($search, "is_published -status -priority id",
			  $priority, $status, $scope, $femail);
  $count   = sizeof($matches);

  if ($id != 0 && $count > 1 && $search != "")
  {
    if (($me = array_search($id, $matches)) === FALSE)
      $me = -1;

    $strlinks = "<span style='float: right;'>";

    if ($me > 0)
    {
      $previd   = $matches[$me - 1];
      $strlinks .= "<a href='$PHP_SELF?$op$previd$options'>Prev</a>";
    }
    else
      $strlinks .= "<span style='color: #cccccc;'>Prev</span>";

    $strlinks .= " &middot; ";

    if (($me + 1) < $count)
    {
      $nextid   = $matches[$me + 1];
      $strlinks .= "<a href='$PHP_SELF?$op$nextid$options'>Next</a>";
    }
    else
      $strlinks .= "<span style='color: #cccccc;'>Next</span>";

    $strlinks .= "</span>";
  }
  else
    $strlinks = "";
}

switch ($op)
{
  case 'B' : // Batch update selected STRs
      if ($REQUEST_METHOD != "POST")
      {
        header("Location: $PHP_SELF?L$options");
        break;
      }

      if (array_key_exists("status", $_POST) &&
          ($_POST["status"] != "" ||
	   $_POST["subsystem"] != "" ||
	   $_POST["fix_version"] != "" ||
	   $_POST["priority"] != "" ||
	   $_POST["manager_user"] != "" ||
	   $_POST["message"] != ""))
      {
        reset($_POST);
        while (list($key, $val) = each($_POST))
	{
          if (preg_match("/^ID_[0-9]+\$/", $key))
	  {
	    $id  = (int)substr($key, 3);
	    $str = new str($id);

	    if ($str->id != $id)
	      continue;

	    if ($_POST["status"] != "")
	      $str->status = (int)$_POST["status"];
	    if ($_POST["subsystem"] != "")
	      $str->subsystem = $_POST["subsystem"];
	    if ($_POST["fix_version"] != "")
	      $str->fix_version = $_POST["fix_version"];
	    if ($_POST["priority"] != "")
	      $str->priority = (int)$_POST["priority"];
	    if ($_POST["manager_user"] != "")
	      $str->manager_user = $_POST["manager_user"];

            if ($str->validate())
	    {
              $str->save();

              if ($_POST["message"] != "")
		$contents = add_text($str);
	      else
	        $contents = "";

	      if ($contents !== FALSE)
		notify_users($str, $contents);
	    }
	  }
        }
      }

      header("Location: $PHP_SELF?L$options");
      break;

  case 'L' : // List (all) STR(s)
      if ($id)
      {
        $str = new str($id);
	if ($str->id != $id)
	{
          html_header("Bugs & Features Error");
	  print("<p><b>Error:</b> Bug #$id was not found!</p>\n");
	  html_footer();
	  exit();
	}

        $links = array(
	  "Roadmap" => "roadmap.php",
	  "List" => "$PHP_SELF?L$options"
	);

        if ($row['status'] >= STR_STATUS_ACTIVE)
	{
	  $links["Post Text"] = "$PHP_SELF?T$id$options";
	  $links["Post File"] = "$PHP_SELF?F$id$options";
	}

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  $links["Modify STR"] = "$PHP_SELF?U$id$options";

        html_header("Bug #$id: $str->summary", "", "", $links);
	print($strlinks);

        $str->view($options);

        str_history($str, TRUE, $options);
      }
      else
      {
	$bookmark  = "$PHP_URL?L+P$priority+S$status+C$scope+E$femail+"
		    ."M$PAGE_MAX+Q" . urlencode($search);

        html_header("Bugs & Features", "",
	            array("Roadmap" => "roadmap.php",
		          "New Bug Report" => "$PHP_SELF?U$options'",
		          "Link To Search Results" => "$bookmark"));

	$htmlsearch = htmlspecialchars($search, ENT_QUOTES);

        print("<form method='POST' action='$PHP_SELF'><p align='center'>"
	     ."<input type='search' size='60' name='SEARCH' "
	     ."value='$htmlsearch' placeholder='Search Requests' "
	     ."autosave='org.htmldoc.search' results='20'>"
	     ."<input type='submit' value='Search Requests'><br>\n");

	print("Priority:&nbsp;<select name='FPRIORITY'>");
	print("<option value='0'>Don't Care</option>");
        for ($i = 1; $i <= 5; $i ++)
	{
	  print("<option value='$i'");
	  if ($priority == $i)
	    print(" selected");
	  print(">$STR_PRIORITY_SHORT[$i]</option>");
	}
        print("</select>\n");

	print("Status:&nbsp;<select name='FSTATUS'>");
	print("<option value='0'>Don't Care</option>");
	if ($status == -1)
	  print("<option value='-1' selected>Closed</option>");
	else
	  print("<option value='-1'>Closed</option>");
	if ($status == -2)
	  print("<option value='-2' selected>Open</option>");
	else
	  print("<option value='-2'>Open</option>");
	for ($i = 1; $i <= 5; $i ++)
	{
	  print("<option value='$i'");
	  if ($status == $i)
	    print(" selected");
	  print(">$STR_STATUS_SHORT[$i]</option>");
	}
        print("</select>\n");

	print("Scope:&nbsp;<select name='FSCOPE'>");
	print("<option value='0'>Don't Care</option>");
	for ($i = 1; $i <= 3; $i ++)
	{
	  print("<option value='$i'");
	  if ($scope == $i)
	    print(" selected");
	  print(">$STR_SCOPE_SHORT[$i]</option>");
	}
        print("</select>\n");

        if ($LOGIN_USER != "")
	{
	  print("Show:&nbsp;<select name='FEMAIL'>");
          print("<option value='0'>All</option>");
          print("<option value='1'");
	  if ($femail)
            print(" selected");
          if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    print(">Mine + Unassigned</option>");
	  else
	    print(">Only Mine</option>");
          print("</select>\n");
        }

	$values = array(10, 20, 50, 100, 1000);
	print("STRs/Page:&nbsp;<select name='FPAGEMAX'>");
	for ($i = 0; $i < sizeof($values); $i ++)
	{
	  if ($values[$i] == $PAGE_MAX)
	    print("<option value='$values[$i]' selected>$values[$i]</option>");
	  else
	    print("<option value='$values[$i]'>$values[$i]</option>");
	}
	print("</select>\n");

	print("<br><i>Search supports 'and', 'or', 'not', and parenthesis. "
	     ."<a href='search-help.php'>More info...</a></i>"
	     ."</p></form>\n");

	print("<hr noshade>\n");

        $str     = new str();
        $matches = $str->search($search, "-status -priority -scope",
	                        $priority, $status, $scope, $femail);
        $count   = sizeof($matches);

        if ($count == 0)
	{
	  print("<p>No requests found.</p>\n");

	  if (($priority || $status || $scope) && $search != "")
	    print("<p>[ <a href='$PHP_SELF?L+S0+Q" . urlencode($search)
	         ."'>Search for \"<i>$htmlsearch</i>\" in all requests</a> ]</p>\n");

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

        if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  print("<form method='POST' action='$PHP_SELF?B$options'>\n");


        if ($count > $PAGE_MAX)
	{
          $nav = "<p align='center'>";

	  if ($index > 0)
	    $nav .= "<a href='$PHP_SELF?L+P$priority+S$status+C$scope+I$prev+"
		   ."E$femail+Q"
		   . urlencode($search)
		 ."'>&larr; Previous&nbsp;$PAGE_MAX</a>";

          $nav .= " &middot; Showing $start to $end of $count &middot; ";

	  if ($end < $count)
	  {
	    $next_count = min($PAGE_MAX, $count - $end);
	    $nav .= "<a href='$PHP_SELF?L+P$priority+S$status+C$scope+I$next+"
		   ."E$femail+Q"
		   . urlencode($search)
		   ."'>Next&nbsp;$next_count &rarr;</a>";
          }

	  $nav .= "</p>\n";
        }
	else
	  $nav = "";

        print($nav);

        html_start_table(array("Id", "Priority", "Status", "Scope",
	                       "Summary", "Version", "Last Updated",
			       "Assigned To"));

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  $linkop = "U";
	else
	  $linkop = "L";

	for ($i = $index; $i < $end; $i ++)
	{
	  $str->load($matches[$i]);

	  $date     = date("M d, Y", $str->modify_date);
          $summary  = htmlspecialchars($str->summary, ENT_QUOTES);
	  $summabbr = abbreviate($str->summary, 80);
	  $prtext   = $STR_PRIORITY_SHORT[$str->priority];
          $sttext   = $STR_STATUS_SHORT[$str->status];
          $sctext   = $STR_SCOPE_SHORT[$str->scope];
	  $link     = "<a href='$PHP_SELF?$linkop$str->id$options' "
	             ."title='Bug #$str->id: $summary'>";

          html_start_row();

          if ($str->is_published == 0)
	    $summabbr .= " <img src='images/private.gif' width='16' height='16' "
	                ."border='0' align='middle' alt='Private'>";

          print("<td nowrap>");
          if ($LOGIN_LEVEL >= AUTH_DEVEL)
	    print("<input type='checkbox' name='ID_$str->id'>");
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
	                          ."WHERE str_id = $str->id AND "
				  ."is_published = 1 "
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

        if ($LOGIN_LEVEL >= AUTH_DEVEL)
	{
	  html_start_row("footer");
	  print("<th colspan='8'>");

          print("Status:&nbsp;<select name='status'>"
	       ."<option value=''>No Change</option>");
	  for ($i = 1; $i <= 5; $i ++)
	    print("<option value='$i'>$STR_STATUS_SHORT[$i]</option>");
          print("</select>\n");

	  print("Priority:&nbsp;<select name='priority'>"
	       ."<option value=''>No Change</option>");
          for ($i = 1; $i <= 5; $i ++)
	    print("<option value='$i'>$STR_PRIORITY_SHORT[$i]</option>");
          print("</select>\n");

	  print("Fix Version:&nbsp;<select name='fix_version'>"
	       ."<option value=''>No Change</option>"
	       ."<option>Not Applicable</option>");

	  reset($STR_VERSIONS);
	  while (list($key, $val) = each($STR_VERSIONS))
	  {
	    if ($val[0] == '+')
	      $val = substr($val, 1);

	    print("<option value='$val'>$val</option>");
	  }
          print("</select>\n");

	  print("Subsystem:&nbsp;<select name='subsystem'>"
	       ."<option value=''>No Change</option>");

	  reset($STR_SUBSYSTEMS);
	  while (list($key, $val) = each($STR_SUBSYSTEMS))
	    print("<option value='$val'>$val</option>");
          print("</select>\n");

	  print("<br>Assigned To:&nbsp;<select name='manager_user'>"
	       ."<option value=''>No Change</option>");
	  reset($STR_MANAGERS);
	  while (list($key, $val) = each($STR_MANAGERS))
	  {
	    $temp = sanitize_email($val);
	    print("<option value='$key'>$temp</option>");
	  }
          print("</select>\n");

	  print("Text:&nbsp;<select name='message'>"
	       ."<option value=''>No Message</option>");
	  reset($STR_MESSAGES);
	  while (list($key, $val) = each($STR_MESSAGES))
	  {
	    $temp = abbreviate($val);
	    print("<option value='$key'>$temp</option>");
	  }
          print("</select>\n");

	  print("<input type='submit' value='Modify Selected Requests'>");
	  print("</th>\n");
	  html_end_row();
        }

        html_end_table();

        print($nav);

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
	  print("</form>");

	print("<p>"
	     ."MACH = Machine, "
	     ."OS = Operating System, "
	     ."STR = Software Trouble Report");

	if ($LOGIN_LEVEL >= AUTH_DEVEL)
 	  print(", <img src='images/private.gif' width='16' height='16' "
	       ."align='absmiddle' alt='private'> = hidden from public view");

        print("</p>\n");
      }

      html_footer();
      break;

  case 'F' : // Post file for Bug #
      if ($LOGIN_USER == "")
      {
        header("Location: login.php?PAGE=str.php?F$id$options");
	return;
      }

      $action = "Post File For Bug #$id";
      $str    = new str($id);

      if ($str->id != $id)
      {
        html_header($action);
	print("<p><b>Error:</b> Bug #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($REQUEST_METHOD == "POST")
        $havedata = array_key_exists("file", $_FILES);
      else
        $havedata = 0;

      if ($havedata)
      {
        // Update STR status and modify user/date...
        if ($str->status < STR_STATUS_PENDING)
	  $str->status = STR_STATUS_PENDING;

        if (!$str->save())
	{
	  html_header($action);
	  print("<p>Unable to save STR!</p>\n");
	  html_footer();
	  exit();
	}

	// Add file...
	if (($file = add_file($str)) === FALSE)
	{
	  html_header($action);
	  print("<p>Unable to save file to STR!</p>\n");
	  html_footer();
	  exit();
	}
	else
	  notify_users($str, "", $file);

	header("Location: $PHP_SELF?L$id$options");
      }
      else
      {
        html_header($action, "",
	            array("Roadmap" => "roadmap.php",
			  "List" => "$PHP_SELF?L$options",
		          "Bug #$id" => "$PHP_SELF?L$id$options"));

	print("<form action='$PHP_SELF?F$id' method='POST' "
             ."enctype='multipart/form-data'>\n"
             ."<input type='hidden' name='MAX_FILE_SIZE' value='10485760'>"
	     ."<p align='center'><table class='edit'>\n");

	// File...
	print("<tr><th align='right' valign='top'>Post File:</th><td>"
             ."<input name='file' type='file'></td></tr>\n");

	// Submit
	print("<tr><td></td><td>"
             ."<input type='submit' value='$action'>"
             ."</td></tr>\n"
             ."</table>\n"
             ."</form>\n");

        html_footer();
      }
      break;

  case 'T' : // Post text for Bug #
      if ($LOGIN_USER == "")
      {
        header("Location: login.php?PAGE=str.php?T$id$options");
	return;
      }

      $action = "Post Text For Bug #$id";
      $str    = new str($id);

      if ($str->id != $id)
      {
        html_header($action);
	print("<p><b>Error:</b> Bug #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if (array_key_exists("contents", $_GET))
	$contents = $_GET["contents"];
      else if (array_key_exists("contents", $_POST))
	$contents = $_POST["contents"];
      else
	$contents = "";

      if ($contents != "")
      {
        // Update STR status and modify user/date...
        if ($str->status < STR_STATUS_PENDING)
	  $str->status = STR_STATUS_PENDING;

        if (!$str->save())
	{
	  html_header($action);
	  print("<p>Unable to save STR!</p>\n");
	  html_footer();
	  exit();
	}

	// Add file...
	if (($contents = add_text($str)) === FALSE)
	{
	  html_header($action);
	  print("<p>Unable to save text to STR!</p>\n");
	  html_footer();
	  exit();
	}
	else
	  notify_users($str, $contents);

	header("Location: $PHP_SELF?L$id$options");
      }
      else
      {
        html_header($action, "",
	            array("Roadmap" => "roadmap.php",
		          "List" => "$PHP_SELF?L$options",
		          "Bug #$id" => "$PHP_SELF?L$id$options"));

	print("<form action='$PHP_SELF?T$id' method='POST'>\n"
	     ."<p align='center'><table class='edit'>\n");

	if ($contents == "" && $REQUEST_METHOD == "POST")
	  $hclass = "invalid";
	else
	  $hclass = "valid";

	print("<tr><th class='$hclass' align='right' valign='top'>"
	     ."Additional Text:</th><td>");

	$html = htmlspecialchars($contents);
	print("<textarea name='contents' cols='72' rows='12' wrap='virtual'>"
             ."$html</textarea></td></tr>\n");

	// Submit
	print("<tr><td></td><td>"
             ."<input type='submit' value='$action'>"
             ."</td></tr>\n"
             ."</table>\n"
             ."</form>\n");

        html_footer();
      }
      break;

  case 'U' : // Post new/modify existing STR
      if ($LOGIN_USER == "")
      {
        header("Location: login.php?PAGE=str.php?U$id$options");
	return;
      }

      $str = new str($id);

      if ($id <= 0)
      {
	$action = "New Bug Report";
	$title  = "New Bug Report";
      }
      else
      {
	$action = "Modify Bug #$id";
	$title  = "$action: $str->summary";
      }

      if ($str->id != $id)
      {
        html_header($action);
	print("<p><b>Error:</b> Bug #$id was not found!</p>\n");
	html_footer();
	exit();
      }

      if ($LOGIN_LEVEL < AUTH_DEVEL && $id != 0)
      {
        html_header($title);
	print("<p><b>Error:</b> You do not have permission to modify "
	     ."Bug #$id!</p>\n");
	html_footer();
	exit();
      }

      if ($REQUEST_METHOD == "POST")
      {
	if (array_key_exists("FILE_ID", $_POST) &&
	    (int)$_POST["FILE_ID"] > 0 &&
	    array_key_exists("is_published", $_POST))
	{
	  $file_id = (int)$_POST["FILE_ID"];
	  $strfile = new strfile($file_id);

	  if ($strfile->id == $file_id)
	  {
	    $strfile->is_published = (int)$_POST["is_published"];
	    $strfile->save();
	  }

	  header("Location: $PHP_SELF?L$str->id$options");
	  exit();
	}

	if (array_key_exists("TEXT_ID", $_POST) &&
	    (int)$_POST["TEXT_ID"] > 0 &&
	    array_key_exists("is_published", $_POST))
	{
	  $text_id = (int)$_POST["TEXT_ID"];
	  $strtext = new strtext($text_id);

	  if ($strtext->id == $text_id)
	  {
	    $strtext->is_published = (int)$_POST["is_published"];
	    $strtext->save();
	  }

	  header("Location: $PHP_SELF?L$str->id$options");
	  exit();
	}

        $havedata = $str->loadform();

	if ($id == 0 && !array_key_exists("contents", $_POST))
	  $havedata = 0;
      }
      else
      {
        $str->validate();

        $havedata = 0;
      }

      if ($havedata)
      {
        if (!$str->save())
	{
	  html_header($title);
	  print("<p>Unable to save STR!</p>\n");
	  html_footer();
	  exit();
	}

	if (array_key_exists("contents", $_GET) ||
	    array_key_exists("contents", $_POST) ||
	    array_key_exists("message", $_GET) ||
	    array_key_exists("message", $_POST))
	{
	  // Add text...
	  if (($contents = add_text($str)) === FALSE)
	  {
	    html_header($title);
	    print("<p>Unable to save text to STR!</p>\n");
	    html_footer();
	    exit();
	  }
	}
	else
	  $contents = "";

        if (array_key_exists("file", $_FILES))
	{
	  // Add file...
	  if (($file = add_file($str)) === FALSE)
	  {
	    html_header($title);
	    print("<p>Unable to save file to STR!</p>\n");
	    html_footer();
	    exit();
	  }
	}
	else
	  $file = "";

	if ($id <= 0)
	  notify_users($str, $contents, $file, "");
	else
	  notify_users($str, $contents, $file);

	header("Location: $PHP_SELF?L$str->id$options");
      }
      else
      {
        $links = array("Roadmap" => "roadmap.php",
		       "List" => "$PHP_SELF?L$options");
	if ($id > 0)
	  $links["Bug #$id"] = "$PHP_SELF?L$id$options";

        html_header($title, "", $links);
	print($strlinks);

	if ($REQUEST_METHOD == "POST")
	  print("<p><b>Error:</b> Please fill in the fields as "
	       ."<span class='invalid'>marked</span> and resubmit.</p>\n"
	       ."<hr noshade>\n");
	else if ($id <= 0)
	  print("<p>Please use this form to report all bugs and request "
	       ."features in the $PROJECT_NAME software. General usage "
	       ."and compilation questions should be directed to the "
	       ."<a href='newsgroups.php'>$PROJECT_NAME forums</a>.</p>\n"
	       ."<p>When reporting bugs, please be sure to include "
	       ."the operating system, compiler, sample programs and/or "
	       ."files, and any other information you can about your "
	       ."problem. <i>Thank you</i> for helping us to improve "
	       ."$PROJECT_NAME!</p><hr noshade>\n");

        $str->edit($action, $options);

        str_history($str, FALSE, $options);

	html_footer();
      }
      break;

  case 'N' : // Update notification status
      // EMAIL and NOTIFICATION variables hold status; add/delete from strcc...
      $havedata = 0;

      if ($REQUEST_METHOD != "POST")
      {
	html_header("Bugs & Features Error");
	print("<p>The '$op' command requires a POST request!\n");
	html_footer();
	exit();
      }

      $notification = $_POST["NOTIFICATION"];
      $email        = $_POST["EMAIL"];

      if (($notification != "ON" && $notification != "OFF") || $email == "" ||
          !validate_email($email))
      {
	html_header("Bugs & Features Error");
	print("<p>Please press your browsers back button and enter a valid "
	     ."EMail address and choose whether to receive notification "
	     ."messages.</p>\n");
	html_footer();
	exit();
      }

      setcookie("${PROJECT_MODULE}FROM", "$email", time() + 90 * 86400, "/");

      $result = db_query("SELECT * FROM carboncopy WHERE "
                        ."url = 'str.php_L$id' AND email = '$email'");

      html_header("Bug #$id Notifications", "",
                  array("Roadmap" => "roadmap.php",
			"List" => "$PHP_SELF?L$options",
		        "Bug #$id" => "$PHP_SELF?L$id$options"));

      if ($notification == "ON")
      {
        if ($result && db_count($result) > 0)
	  print("<p>Your email address has already been added to the "
	       ."notification list for Bug #$id!</p>\n");
        else
	{
          db_query("INSERT INTO carboncopy VALUES(NULL,'str.php?L$id','$email')");

	  print("<p>Your email address has been added to the notification list "
               ."for Bug #$id.</p>\n");
        }
      }
      else if ($result && db_count($result) > 0)
      {
        db_query("DELETE FROM carboncopy WHERE "
	        ."url = 'str.php?L$id' AND email = '$email'");

	print("<p>Your email address has been removed from the notification list "
             ."for Bug #$id.</p>\n");
      }
      else
      {
	print("<p>Your email address is not on the notification list for "
	     ."Bug #$id!</p>\n");
      }

      if ($result)
        db_free($result);

      html_footer();
      break;
}

//
// End of "$Id: str.php,v 1.21 2006/07/11 20:33:21 mike Exp $".
//
?>
