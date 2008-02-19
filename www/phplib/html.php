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
	    $links = "",		// I - Array of links
	    $javascript = "")		// I - Javascript, if any
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

  print("  <title>$html_title &lt;HTML&gt;DOC</title>\n"
       ."  <meta http-equiv='Content-Type' content='text/html; "
       ."charset=utf-8'>\n"
       ."  <link rel='stylesheet' type='text/css' href='${path}style.css'>\n"
       ."  <link rel='alternate' title='HTMLDOC RSS' "
       ."type='application/rss+xml' href='${path}index.rss'>\n"
       ."  <link rel='shortcut icon' href='${path}images/htmldoc.gif' "
       ."type='image/gif'>\n");

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
  if ($html_show_all)
  {
    print("<table class='page' summary=''>\n"
	 ."<tr>"
	 ."<td class='pagelogo' rowspan='2'>"
	 ."<a href='http://www.easysw.com/htmldoc/'>"
	 ."<img src='${path}images/htmldoc.gif' width='64' height='64' "
	 ."border='0' alt='&lt;HTML&gt;DOC'></a></td>"
	 ."<td class='pagetitle' colspan='2'>");

    if ($title != "")
      print(htmlspecialchars($title));
    else
      print("&lt;HTML&gt;DOC");

    print("</td></tr>\n"
         ."<tr>"
         ."<td class='pagelinks'>"
	 ."<a href='${path}index.php'>Home</a>"
	 ." &middot; "
	 ."<a href='${path}articles.php'>Articles &amp; FAQs</a>"
	 ." &middot; "
	 ."<a href='${path}str.php'>Bugs &amp; Features</a>"
	 ." &middot; "
	 ."<a href='${path}documentation.php'>Documentation</a>"
	 ." &middot; "
	 ."<a href='${path}software.php'>Download</a>"
	 ." &middot; "
	 ."<a href='${path}newsgroups.php'>Forums</a>"
	 ."</td>"
	 ."<td align='right' class='pagelinks'>");

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

    print("</td>"
	 ."</tr>\n");

    print("<tr>"
	 ."<td class='page' colspan='3'>");
  }
  else
  {
    // Just show hidden links for Wget to follow...
    print("<a href='${path}index.php'></a>"
	 ."<a href='${path}articles.php'></a>"
	 ."<a href='${path}str.php'></a>"
	 ."<a href='${path}documentation.php'></a>"
	 ."<a href='${path}software.php'></a>"
	 ."<a href='${path}newsgroups.php'></a>");
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
  global $html_show_all;


  if ($html_show_all)
  {
    print("</td></tr>\n"
         ."<tr><td class='pagefooter' colspan='3'>"
	 ."Copyright 1997-2008 by Easy Software Products. HTMLDOC and "
	 ."&lt;HTML&gt;DOC are the trademark property of Easy Software Products. "
	 ."HTMLDOC is free software; you can redistribute it and/or modify it "
	 ."under the terms of the GNU General Public License as published by the "
	 ."Free Software Foundation.</td></tr>\n"
         ."</table>\n");
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
    print("<p class='center' align='center'>");
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
  html_start_links(1);
  reset($links);
  while (list($key, $val) = each($links))
  {
    $data = explode(" ", $val);

    if (sizeof($data) == 2)
      html_link($key, $data[0], $data[1]);
    else
      html_link($key, $val);
  }
  html_end_links();
}


//
// 'html_start_table()' - Start a rounded, shaded table.
//

function
html_start_table($headings)		// I - Array of heading strings
{
  global $html_row, $html_cols;


  print("<table class='standard' summary=''>"
       ."<tr class='header'>");

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
