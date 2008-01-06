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
//   html_start_box()           - Start a rounded, shaded box.
//   html_end_box()             - End a rounded, shaded box.
//   html_start_table()         - Start a rounded, shaded table.
//   html_end_table()           - End a rounded, shaded table.
//   html_start_row()           - Start a table row.
//   html_end_row()             - End a table row.
//   html_search_words()        - Generate an array of search words.
//   html_select_is_published() - Do a <select> for the "is published" field...
//


//
// Include necessary headers...
//

include_once "globals.php";
include_once "common.php";
include_once "auth.php";


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
  "macos x",
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


// Image/link path...
$html_path = "";

// Show all content...
$html_show_all = FALSE;


//
// 'html_header()' - Show the standard page header and navbar...
//

function				// O - User information
html_header($title = "",		// I - Additional document title
            $path = "",			// I - Relative path to root
	    $refresh = "",		// I - Refresh URL
	    $links = "")		// I - Array of links
{
  global $html_keywords, $html_path, $argc, $argv, $PHP_SELF, $LOGIN_USER,
	 $_SERVER, $html_show_all;


  // Save the path and see if Wget is the client...
  $html_path = $path;

  if (array_key_exists("HTTP_USER_AGENT", $_SERVER))
    $html_show_all = !eregi("Wget.*", $_SERVER["HTTP_USER_AGENT"]);
  else
    $html_show_all = TRUE;

  // Check for a logout on the command-line...
  if ($argc == 1 && $argv[0] == "logout")
  {
    auth_logout();
    $argc = 0;
  }

  // Common stuff...
  header("Cache-Control: no-cache");

  print("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" "
       ."\"http://www.w3.org/TR/REC-html40/loose.dtd\">\n");
  print("<html>\n");
  print("<head>\n");

  // Title...
  if ($title != "")
    $html_title = htmlspecialchars("$title -");
  else
    $html_title = "";

  print("  <title>$html_title HTMLDOC</title>\n"
       ."  <meta http-equiv='Pragma' content='no-cache'/>\n"
       ."  <meta http-equiv='Content-Type' content='text/html; "
       ."charset=utf-8'/>\n"
       ."  <link rel='stylesheet' type='text/css' href='${path}style.css'/>\n"
       ."  <link rel='alternate' title='HTMLDOC RSS' "
       ."type='application/rss+xml' href='${path}index.rss'/>\n"
       ."  <link rel='shortcut icon' href='${path}favicon.ico' "
       ."type='image/x-icon'/>\n");

  // If refresh URL is specified, add the META tag...
  if ($refresh != "")
    print("  <meta http-equiv='refresh' content='3; $refresh'/>\n");

  // Search engine keywords...
  reset($html_keywords);

  list($key, $val) = each($html_keywords);
  print("  <meta name='keywords' content='$val");

  while (list($key, $val) = each($html_keywords))
    print(",$val");

  print("'/>\n");

  print("</head>\n"
       ."<body>\n");

  // Standard navigation stuff...
  if ($html_show_all)
  {
    print("<p><table width='100%' border='0' cellspacing='0' "
	 ."cellpadding='0'>\n"
	 ."<tr class='header'>"
	 ."<td valign='top' rowspan='2'><img src='${path}images/top-left.gif' width='15' "
	 ."height='15' alt=''/></td>"
	 ."<td colspan='2' nowrap><h2 class='title'>");

    if ($title != "")
      print(htmlspecialchars($title));
    else
      print("HTMLDOC");

    print("</h2></td>"
	 ."<td align='right' valign='top' width='33' height='48' rowspan='2'>"
	 ."<a href='http://www.easysw.com/htmldoc/'>"
	 ."<img src='${path}images/logo.gif' width='33' height='48' "
	 ."alt='Buy HTMLDOC on CD-ROM!' title='Buy HTMLDOC on CD-ROM!' "
	 ."border='0' align='middle'/></a></td>"
	 ."<td align='right' valign='top' width='15' height='48' rowspan='2'>"
	 ."<a href='http://www.easysw.com/htmldoc/'>"
	 ."<img src='${path}images/logo2.gif' width='15' height='48' "
	 ."alt='Buy HTMLDOC on CD-ROM!' title='Buy HTMLDOC on CD-ROM!' "
	 ."border='0' align='middle'/></a></td></tr>\n");
    print("<tr class='header'>"
         ."<td width='100%' nowrap>[&nbsp;<a href='${path}index.php'>Home</a> | "
	 ."<a href='${path}articles.php'>Articles &amp; FAQs</a> | "
	 ."<a href='${path}str.php'>Bugs &amp; Features</a> | "
	 ."<a href='${path}documentation.php'>Documentation</a> | "
	 ."<a href='${path}software.php'>Download</a> | "
	 ."<a href='${path}newsgroups.php'>Forums</a> | "
	 ."<a href='${path}links.php'>Links</a>&nbsp;]</td>"
	 ."<td align='right'>[&nbsp;");

    if ($LOGIN_USER)
      print("<a href='${path}account.php'>$LOGIN_USER</a>");
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

      print("<a href='${path}login.php?PAGE=$url'>Login</a>");
    }

    print("&nbsp;]&nbsp;&nbsp;&nbsp;</td>"
	 ."</tr>\n");

    print("<tr class='page'><td></td>"
	 ."<td colspan='3' width='100%' valign='top'>"
	 ."<table width='100%' height='100%' border='0' cellpadding='5' "
	 ."cellspacing='0'><tr><td valign='top'>");
  }
  else
  {
    // Just show hidden links for Wget to follow...
    print("<a href='${path}index.php'></a>"
	 ."<a href='${path}articles.php'></a>"
	 ."<a href='${path}str.php'></a>"
	 ."<a href='${path}documentation.php'></a>"
	 ."<a href='${path}software.php'></a>"
	 ."<a href='${path}newsgroups.php'></a>"
	 ."<a href='${path}links.php'></a>");
  }

  if ($links != "")
    html_links($links);
}


//
// 'html_footer()' - Show the standard footer for a page.
//

function
html_footer()
{
  global $html_path, $html_show_all;


  if ($html_show_all)
  {
    print("</td></tr></table></td><td></td></tr>\n");
    print("<tr class='page'><td colspan='5'>&nbsp;</td></tr>\n");
    print("<tr class='header'>"
	 ."<td valign='bottom'><img src='${html_path}images/bottom-left.gif' "
	 ."width='15' height='15' alt=''/></td>"
	 ."<td colspan='3'><small> <br />"
	 ."Copyright 1997-2005 by Easy Software Products. HTMLDOC and "
	 ."&lt;HTML&gt;DOC are the trademark property of Easy Software Products. "
	 ."HTMLDOC is free software; you can redistribute it and/or modify it "
	 ."under the terms of the GNU General Public License as published by the "
	 ."Free Software Foundation.<br />&nbsp;</small></td>"
	 ."<td align='right' valign='bottom' width='15'><img src='${html_path}images/bottom-right.gif' "
	 ."width='15' height='15' alt=''/></td>"
	 ."</tr>\n");
    print("</table></p>\n");
  }

  print("</body>\n"
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
    print("<p class='center' align='center'>[&nbsp;");
  else
    print("<p>[&nbsp;");
}


//
// 'html_end_links()' - End of series of hyperlinks.
//

function
html_end_links()
{
  print("&nbsp;]</p>\n");
}


//
// 'html_link()' - Show a single hyperlink.
//

function
html_link($text,			// I - Text for hyperlink
          $link)			// I - URL for hyperlink
{
  global $html_firstlink;

  if ($html_firstlink)
    $html_firstlink = 0;
  else
    print(" | ");

  $safetext = str_replace(" ", "&nbsp;", htmlspecialchars($text));

  print("<a href='$link'>$safetext</a>");
}


//
// 'html_links()' - Show an array of links.
//

function
html_links($links)			// I - Associated array of hyperlinks
{
  html_start_links(1);
  reset($links);
  while (list($key, $val) = each($links))
    html_link($key, $val);
  html_end_links();
}


//
// 'html_start_table()' - Start a rounded, shaded table.
//

function
html_start_table($headings)		// I - Array of heading strings
{
  global $html_row, $html_cols;


  print("<p><table border='0' cellpadding='0' cellspacing='0' width='100%'>"
       ."<tr class='header'><th align='left' valign='top'>"
       ."<img src='images/hdr-top-left.gif' width='16' height='16' "
       ."alt=''/></th>");

  $html_row  = 0;
  $html_cols = count($headings);

  for ($i = 0; $i < count($headings); $i ++)
  {
    $header = $headings[$i];

    if (strlen($header))
      print("<th>$header</th>");
    else
      print("<th>&nbsp;</th>");
  }

  print("<th align='right' valign='top'>"
       ."<img src='images/hdr-top-right.gif' "
       ."width='16' height='16' alt=''/></th></tr>\n");
}


//
// 'html_end_table()' - End a rounded, shaded table.
//

function
html_end_table()
{
  global $html_cols;

  print("<tr class='header'><th align='left' valign='bottom'>"
       ."<img src='images/hdr-bottom-left.gif' width='16' height='16' "
       ."alt=''/></th>"
       ."<th colspan='$html_cols'>&nbsp;</th>"
       ."<th align='right' valign='bottom'><img src='images/hdr-bottom-right.gif' "
       ."width='16' height='16' alt=''/></th></tr>\n"
       ."</table></p>\n");
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

  print("<tr class='$classname'><td>&nbsp;</td>");
}


//
// 'html_end_row()' - End a table row.
//

function
html_end_row()
{
  global $html_row;

  $html_row = 1 - $html_row;

  print("</td><td>&nbsp;</td></tr>\n");
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
	    $words[sizeof($words)] = db_escape(strtolower($temp));
	    $temp = "";
	  }

	  $i ++;

	  while ($i < $len && $search[$i] != "\"")
	  {
	    $temp .= $search[$i];
	    $i ++;
	  }

	  $words[sizeof($words)] = db_escape(strtolower($temp));
	  $temp = "";
          break;

      case " " :
      case "\t" :
      case "\n" :
          if ($temp != "")
	  {
	    $words[sizeof($words)] = db_escape(strtolower($temp));
	    $temp = "";
	  }
	  break;

      default :
          $temp .= $search[$i];
	  break;
    }
  }

  if ($temp != "")
    $words[sizeof($words)] = db_escape(strtolower($temp));

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
// End of "$Id: html.php,v 1.23 2005/06/01 18:38:54 mike Exp $".
//
?>
