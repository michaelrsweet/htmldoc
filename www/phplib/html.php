<?php
//
// "$Id: html.php,v 1.23 2005/06/01 18:38:54 mike Exp $"
//
// PHP functions for standardized HTML output...
//
// This file should be included using "include_once"...
//
// Contents:
//
//   html_header()              - Show the standard page header and navbar...
//   html_footer()              - Show the standard footer for a page.
//   html_start_links()         - Start of series of hyperlinks.
//   html_end_links()           - End of series of hyperlinks.
//   html_link()                - Show a single hyperlink.
//   html_links()               - Show an array of links.
//   html_start_table()         - Start a rounded, shaded table.
//   html_end_table()           - End a rounded, shaded table.
//   html_start_row()           - Start a table row.
//   html_end_row()             - End a table row.
//   html_search_words()        - Generate an array of search words.
//   html_select_is_published() - Do a <select> for the "is published" field...
//   html_format()              - Convert plain text to HTML...
//


//
// Include necessary headers...
//

include_once "auth.php";
include_once "common.php";
include_once "phplib/db-str.php";

date_default_timezone_set("America/Los_Angeles");


//
// Search keywords...
//

$html_keywords = array(
  "conversion",
  "documentation",
  "functions",
  "html",
  "htmldoc",
  "index",
  "indexing",
  "linux",
  "mac os x",
  "pdf",
  "postscript",
  "ps",
  "software",
  "table of contents",
  "unix",
  "windows",
  "xhtml",
  "xml"
);


// Figure out the base path...
$html_path = dirname($PHP_SELF);

if (array_key_exists("PATH_INFO", $_SERVER))
{
  $i = -1;
  while (($i = strpos($_SERVER["PATH_INFO"], "/", $i + 1)) !== FALSE)
    $html_path = dirname($html_path);
}

if ($html_path == "/")
  $html_path = "";

//
// 'html_header()' - Show the standard page header and navbar...
//

function				// O - User information
html_header($title = "",		// I - Additional document title
	    $refresh = "",		// I - Refresh URL
	    $links = "",		// I - Array of links
	    $javascript = "")		// I - Javascript, if any
{
  global $argc, $argv, $html_keywords, $html_path, $_GET, $LOGIN_USER;
  global $LOGIN_LEVEL, $PHP_SELF, $_SERVER;


  // Specify HTML 4.0 so that the footer is rendered at the bottom of the
  // page; for some reason using HTML 4.01 makes browsers ignore the height
  // property of the interior row in our outer table...
  print("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
       ."\"http://www.w3.org/TR/html4/loose.dtd\">\n");
  print("<html>\n");
  print("<head>\n");

  // Title...
  if ($title != "")
    $html_title = "$title -";
  else
    $html_title = "";

  print("  <title>$html_title HTMLDOC</title>\n"
       ."  <meta http-equiv='Pragma' content='no-cache'>\n"
       ."  <meta http-equiv='Content-Type' content='text/html; "
       ."charset=utf-8'>\n"
       ."  <link rel='stylesheet' type='text/css' href='$html_path/style.css'>\n"
       ."  <link rel='alternate' title='HTMLDOC RSS' "
       ."type='application/rss+xml' href='$html_path/index.rss'>\n"
       ."  <link rel='shortcut icon' href='$html_path/images/htmldoc.png' "
       ."type='image/png'>\n");

  // If refresh URL is specified, add the META tag...
  if ($refresh != "")
    print("  <meta http-equiv='refresh' content='3; $refresh'>\n");

  // Search engine keywords...
  reset($html_keywords);

  list($key, $val) = each($html_keywords);
  print("  <meta name='keywords' content='$val");

  while (list($key, $val) = each($html_keywords))
    print(",$val");

  print("'>\n");

  if ($javascript != "")
    print("  <script type='text/javascript'>\n$javascript\n  </script>\n");

  print("</head>\n"
       ."<body>\n");

  // Standard navigation stuff...
  if (array_key_exists("Q", $_GET))
    $q = htmlspecialchars($_GET["Q"], ENT_QUOTES);
  else
    $q = "";

  if (stripos($_SERVER["HTTP_USER_AGENT"], "webkit") !== FALSE)
  {
    // Use Safari search box...
    $search = "<input type='search' name='Q' value='$q' size='25' "
	     ."autosave='org.htmldoc.search' results='5' "
             ."placeholder='Search'>";
  }
  else
  {
    // Use standard HTML text field...
    $search = "<input type='text' name='Q' value='$q' size='15' "
             ."title='Search'><input type='submit' value='Search'>";
  }

  if (strpos($PHP_SELF, "/documentation.php/") !== FALSE)
    $base = "documentation.php";
  else
    $base = basename($PHP_SELF);

  $pages = array(
    "Bugs &amp; Features" => array("roadmap.php", "str.php"),
    "Documentation" => array("documentation.php", "articles.php", "comment.php",
                             "htmldoc-cmp.php", "search-help.php"),
    "Download" => array("software.php"),
    "Forums" => array("newsgroups.php")
  );

  if ($base == "index.php")
    $class = "sel";
  else
    $class = "unsel";

  print("<table width='100%' style='height: 100%;' border='0' cellspacing='0' "
       ."cellpadding='0' summary=''>\n"
       ."<tr>"
       ."<td class='$class'><a href='index.php'><img "
       ."src='$html_path/images/htmldoc.jpg' width='32' height='32' border='0' "
       ."alt='&lt;HTML&gt;DOC'></a></td>");

  if ($base == "login.php" || $base == "account.php")
    print("<td class='sel'");
  else
    print("<td class='unsel'>");

  if ($LOGIN_USER)
  {
    print("<a href='$html_path/account.php'>$LOGIN_USER</a>"
         ."<a href='$html_path/account.php?X' style='padding-left: 0px'><img "
	 ."src='$html_path/images/logout.png' width='16' height='16' "
	 ."align='absmiddle' alt='Logout' title='Log Out'></a>");
  }
  else
  {
    // Show login link which redirects back to the current page...
    $url    = urlencode($PHP_SELF);
    $prefix = "?";
    for ($i = 0; $i < $argc; $i ++)
    {
      $url    .= $prefix . urlencode($argv[$i]);
      $prefix = "+";
    }

    print("<a href='$html_path/login.php?PAGE=$url'>Login</a>");
  }
  print("</td>");

  reset($pages);
  foreach ($pages as $label => $hrefs)
  {
    if (in_array($base, $hrefs))
      $class = "sel";
    else
      $class = "unsel";

    $url = $hrefs[0];
    if ($url == "roadmap.php" && $LOGIN_USER != "")
    {
      // See if there are pending bugs for the current user...
      $duser = db_escape($LOGIN_USER);

      if ($LOGIN_LEVEL == AUTH_USER)
        $result = db_query("SELECT id FROM str WHERE status = "
	                  . STR_STATUS_ACTIVE
			  ." AND create_user = '$duser'");
      else
        $result = db_query("SELECT id FROM str WHERE (status = "
	                  . STR_STATUS_PENDING
			  ." AND manager_user = '$duser')"
			  ." OR status = " . STR_STATUS_NEW);

      if (($count = db_count($result)) > 0)
      {
        $label .= " ($count)";
	$url   = "str.php?L+E1";
      }
    }

    print("<td class='$class' nowrap><a href='$html_path/$url'>"
	 ."$label</a></td>");
  }

  print("<td class='unsel' align='right' width='100%'>"
       ."<form action='$html_path/search.php' method='GET'>"
       ."$search</form></td>"
       ."</tr>\n"
       ."<tr>"
       ."<td class='page' colspan='7'>");

  if ($links != "")
  {
    html_start_links(TRUE);
    html_links($links);
    html_end_links();
  }

  if ($title != "")
    print("<h1>$title</h1>\n");
}


//
// 'html_footer()' - Show the standard footer for a page.
//

function
html_footer()
{
  print("</td></tr>\n"
       ."<tr><td class='footer' colspan='7'>"
       ."Copyright 2011 by Michael R Sweet. All rights reserved.</td></tr>\n"
       ."</table>\n"
       ."</body>\n"
       ."</html>\n");
}


//
// 'html_start_links()' - Start of series of hyperlinks.
//

function
html_start_links($center = 0)		// I - 1 for centered, 0 for in-line
{
  global $html_firstlink;

  $html_firstlink = 1;

  if ($center)
    print("<p class='links'>");
  else
    print("<p>");
}


//
// 'html_end_links()' - End of series of hyperlinks.
//

function
html_end_links()
{
  print("</p>\n");
}


//
// 'html_link()' - Show a single hyperlink.
//

function
html_link($text,			// I - Text for hyperlink
          $link,			// I - URL for hyperlink
	  $onclick = "")		// I - Javascript for "onclick"
{
  global $html_firstlink;

  if ($html_firstlink)
    $html_firstlink = 0;
  else
    print(" &middot; ");

  $safetext = str_replace(" ", "&nbsp;", htmlspecialchars($text));

  print("<a href='$link'");
  if ($onclick != "")
    print(" onclick='$onclick'");
  print(">$safetext</a>");
}


//
// 'html_links()' - Show an array of links.
//

function
html_links($links)			// I - Associated array of hyperlinks
{
  reset($links);
  while (list($key, $val) = each($links))
  {
    $data = explode(" ", $val);

    if (sizeof($data) == 2)
      html_link($key, $data[0], $data[1]);
    else
      html_link($key, $val);
  }
}


//
// 'html_start_table()' - Start a rounded, shaded table.
//

function
html_start_table($headings)		// I - Array of heading strings
{
  global $html_row;


  print("<table class='standard' summary=''>"
       ."<tr class='header'>");

  $html_row = 0;

  for ($i = 0; $i < count($headings); $i ++)
  {
    $header = $headings[$i];

    if (strlen($header))
      print("<th>$header</th>");
    else
      print("<th>&nbsp;</th>");
  }

  print("</tr>\n");
}


//
// 'html_end_table()' - End a rounded, shaded table.
//

function
html_end_table()
{
  print("</table>\n");
}


//
// 'html_start_row()' - Start a table row.
//

function
html_start_row($classname = "")		// I - HTML class to use
{
  global $html_row;

  if ($classname == "")
    $classname = "data$html_row";
  else
    $html_row = 1 - $html_row;

  print("<tr class='$classname'>");
}


//
// 'html_end_row()' - End a table row.
//

function
html_end_row()
{
  global $html_row;

  $html_row = 1 - $html_row;

  print("</tr>\n");
}


//
// 'html_search_words()' - Generate an array of search words.
//

function				// O - Array of words
html_search_words($search = "")		// I - Search string
{
  $words = array();
  $temp  = "";
  $len   = strlen($search);

  for ($i = 0; $i < $len; $i ++)
  {
    switch ($search[$i])
    {
      case "\"" :
          if ($temp != "")
	  {
	    $words[sizeof($words)] = strtolower($temp);
	    $temp = "";
	  }

	  $i ++;

	  while ($i < $len && $search[$i] != "\"")
	  {
	    $temp .= $search[$i];
	    $i ++;
	  }

	  $words[sizeof($words)] = strtolower($temp);
	  $temp = "";
          break;

      case " " :
      case "\t" :
      case "\n" :
          if ($temp != "")
	  {
	    $words[sizeof($words)] = strtolower($temp);
	    $temp = "";
	  }
	  break;

      default :
          $temp .= $search[$i];
	  break;
    }
  }

  if ($temp != "")
    $words[sizeof($words)] = strtolower($temp);

  return ($words);
}


//
// 'html_select_is_published()' - Do a <select> for the "is published" field...
//

function
html_select_is_published($is_published = 1)
					// I - Default state
{
  print("<select name='is_published'>");
  if ($is_published)
  {
    print("<option value='0'>No</option>");
    print("<option value='1' selected>Yes</option>");
  }
  else
  {
    print("<option value='0' selected>No</option>");
    print("<option value='1'>Yes</option>");
  }
  print("</select>");
}


//
// 'html_format()' - Convert plain text to HTML...
//

function				// O - Quoted string
html_format($text)			// I - Original string
{
  $len    = strlen($text);
  $col    = 0;
  $list   = 0;
  $bold   = 0;
  $pre    = 0;
  $inlink = 0;
  $inpre  = 0;

  if (!strncasecmp($text, "<p>", 3))
    $ftext = "";
  else
    $ftext = "<p>";

  for ($i = 0; $i < $len; $i ++)
  {
    switch ($text[$i])
    {
      case '<' :
          $col ++;
	  if (strtolower(substr($text, $i, 2)) == "<a" ||
	      strtolower(substr($text, $i, 8)) == "<a name=" ||
	      strtolower(substr($text, $i, 4)) == "</a>" ||
	      strtolower(substr($text, $i, 3)) == "<b>" ||
	      strtolower(substr($text, $i, 4)) == "</b>" ||
	      strtolower(substr($text, $i, 12)) == "<blockquote>" ||
	      strtolower(substr($text, $i, 13)) == "</blockquote>" ||
	      strtolower(substr($text, $i, 4)) == "<br>" ||
	      strtolower(substr($text, $i, 5)) == "<br/>" ||
	      strtolower(substr($text, $i, 6)) == "<br />" ||
	      strtolower(substr($text, $i, 6)) == "<code>" ||
	      strtolower(substr($text, $i, 7)) == "</code>" ||
	      strtolower(substr($text, $i, 4)) == "<em>" ||
	      strtolower(substr($text, $i, 5)) == "</em>" ||
	      strtolower(substr($text, $i, 4)) == "<h1>" ||
	      strtolower(substr($text, $i, 5)) == "</h1>" ||
	      strtolower(substr($text, $i, 4)) == "<h2>" ||
	      strtolower(substr($text, $i, 5)) == "</h2>" ||
	      strtolower(substr($text, $i, 4)) == "<h3>" ||
	      strtolower(substr($text, $i, 5)) == "</h3>" ||
	      strtolower(substr($text, $i, 4)) == "<h4>" ||
	      strtolower(substr($text, $i, 5)) == "</h4>" ||
	      strtolower(substr($text, $i, 4)) == "<h5>" ||
	      strtolower(substr($text, $i, 5)) == "</h5>" ||
	      strtolower(substr($text, $i, 4)) == "<h6>" ||
	      strtolower(substr($text, $i, 5)) == "</h6>" ||
	      strtolower(substr($text, $i, 3)) == "<i>" ||
	      strtolower(substr($text, $i, 4)) == "</i>" ||
	      strtolower(substr($text, $i, 5)) == "<img " ||
	      strtolower(substr($text, $i, 4)) == "<li>" ||
	      strtolower(substr($text, $i, 5)) == "</li>" ||
	      strtolower(substr($text, $i, 4)) == "<ol>" ||
	      strtolower(substr($text, $i, 4)) == "<ol " ||
	      strtolower(substr($text, $i, 5)) == "</ol>" ||
	      strtolower(substr($text, $i, 3)) == "<p>" ||
	      strtolower(substr($text, $i, 4)) == "</p>" ||
	      strtolower(substr($text, $i, 4)) == "<pre" ||
	      strtolower(substr($text, $i, 6)) == "</pre>" ||
	      strtolower(substr($text, $i, 5)) == "<sub>" ||
	      strtolower(substr($text, $i, 6)) == "</sub>" ||
	      strtolower(substr($text, $i, 5)) == "<sup>" ||
	      strtolower(substr($text, $i, 6)) == "</sup>" ||
	      strtolower(substr($text, $i, 4)) == "<tt>" ||
	      strtolower(substr($text, $i, 5)) == "</tt>" ||
	      strtolower(substr($text, $i, 3)) == "<u>" ||
	      strtolower(substr($text, $i, 4)) == "</u>" ||
	      strtolower(substr($text, $i, 4)) == "<ul>" ||
	      strtolower(substr($text, $i, 5)) == "</ul>" ||
	      strtolower(substr($text, $i, 5)) == "<var>" ||
	      strtolower(substr($text, $i, 6)) == "</var>")
          {
            if (preg_match("/\\<a\\s+href=/i", substr($text, $i, 32)))
	    {
	      $inlink = 1;
	      $ftext .= "<a rel='nofollow'";
	      $i += 2;
	    }
	    else if (strtolower(substr($text, $i, 4)) == "</a>")
	      $inlink = 0;
	    else if (strtolower(substr($text, $i, 4)) == "<pre")
	      $inpre = 1;
	    else if (strtolower(substr($text, $i, 6)) == "</pre>")
	      $inpre = 0;

	    while ($i < $len && $text[$i] != '>')
	    {
	      $ftext .= $text[$i];
	      $i ++;
	    }

	    $ftext .= ">";
	  }
	  else
            $ftext .= "&lt;";
	  break;

      case '>' :
          $col ++;
          $ftext .= "&gt;";
	  break;

      case '&' :
          $col ++;
	  $temp = substr($text, $i);
	  if (preg_match("/^&([a-z]+|#[0-9]+|#x[0-9a-f]+);/i", $temp))
	    $ftext .= "&";
	  else
            $ftext .= "&amp;";
	  break;

      case "\n" :
          if ($inpre)
	  {
	    $ftext .= "\n";
	  }
	  else if (($i + 1) < $len &&
	           ($text[$i + 1] == "\n" || $text[$i + 1] == "\r"))
	  {
	    while (($i + 1) < $len &&
	           ($text[$i + 1] == "\n" || $text[$i + 1] == "\r"))
	      $i ++;

            if ($pre)
	    {
	      $ftext .= "</pre>";
	      $pre = 0;
	    }

            if (($i + 1) < $len && $text[$i + 1] != '-' && $list)
	    {
	      $ftext .= "\n</ul>\n<p>";
	      $list  = 0;
	    }
	    else
	      $ftext .= "\n<p>";
	  }
          else if (($i + 1) < $len &&
	           ($text[$i + 1] == " " || $text[$i + 1] == "\t"))
          {
            if ($pre)
	    {
	      $ftext .= "</pre>";
	      $pre = 0;
	    }
	    else if (!$inpre)
	      $ftext .= "<br />\n";
	  }
	  else
	    $ftext .= "\n";

          $col = 0;
	  break;

      case "\r" :
	  break;

      case "\t" :
          if ($col == 0)
	    $ftext .= "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	  else
            $ftext .= " ";
	  break;

      case " " :
          if ($col == 0 && !$pre && !$inpre)
	  {
	    for ($j = $i + 1; $j < $len; $j ++)
	      if ($text[$j] != " " && $text[$j] != "\t")
	        break;

            if ($j < $len && $text[$j] == '%' && !$inpre)
	    {
	      $ftext .= "\n<pre>";
	      $pre   = 1;
	    }

	    $ftext .= "&nbsp;";
	  }
	  else if (($i + 1) < $len && $text[$i + 1] == " ")
	    $ftext .= "&nbsp;";
	  else
            $ftext .= " ";

          if ($col > 0)
	    $col ++;
	  break;

      case '*' :
          if ($col == 0 && $text[$i + 1] == " ")
	  {
	    if (!$list)
	    {
	      $ftext .= "\n<ul>";
	      $list  = 1;
	    }

	    $ftext .= "\n<li>";
	    
	    while (($i + 1) < $len && $text[$i + 1] == " ")
	      $i ++;

	    break;
	  }
	  else if ($inpre)
	    $ftext .= "*";
	  else if ($bold)
	    $ftext .= "*</b>";
	  else
	    $ftext .= "<b>*";

	  $bold = 1 - $bold;
	  break;

      case '-' :
          // Possible list...
	  if ($col == 0 && $text[$i + 1] == " " && !$inpre)
	  {
	    if (!$list)
	    {
	      $ftext .= "\n<ul>";
	      $list  = 1;
	    }

	    $ftext .= "\n<li>";
	    
	    while (($i + 1) < $len && $text[$i + 1] == " ")
	      $i ++;
	    break;
	  }

          $col ++;
          $ftext .= $text[$i];
	  break;

      case 'f' :
      case 'h' :
          if (!$inlink &&
	      (substr($text, $i, 7) == "http://" ||
               substr($text, $i, 8) == "https://" ||
               substr($text, $i, 6) == "ftp://"))
	  {
	    // Extract the URL and make this a link...
	    for ($j = $i; $j < $len; $j ++)
	      if (!preg_match("/[-+~a-zA-Z0-9%_\\/:@.?#=&]/", $text[$j]))
	        break;

	    if ($text[$j - 1] == '.')
	      $j --;

            $count = $j - $i;
            $url   = substr($text, $i, $count);
	    $ftext .= "<a href='$url' rel='nofollow'>$url</a>";
	    $col   += $count;
	    $i     = $j - 1;
	    break;
	  }

      default :
          $col ++;
          $ftext .= $text[$i];
	  break;
    }

    if ($col >= 80 && $pre)
    {
      $ftext .= "\n";
      $col = 0;
    }
  }

  if ($bold)
    $ftext .= "</b>";

  if ($list)
    $ftext .= "</ul>";

  if ($pre)
    $ftext .= "</pre>";

  return ($ftext);
}


//
// End of "$Id: html.php,v 1.23 2005/06/01 18:38:54 mike Exp $".
//
?>
