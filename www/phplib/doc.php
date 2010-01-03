<?php
//
// "$Id$"
//
// Common documentation functions.
//
// Contents:
//
//   doc_header() - Start the page.
//   doc_footer() - End the page.
//   doc_search() - Search the documentation and return an array of matches.
//

//
// Include necessary headers...
//

include_once "html.php";


//
// 'doc_header()' - Start the page.
//

function
doc_header($title = "")			// I - Title string
{
  global $html_path, $PHP_SELF;


  $base = basename($PHP_SELF);
  if ($base == "htmldoc-cmp.php")
    $links = array("Documentation" => "${html_path}/documentation.php",
		   "Articles" => "${html_path}/articles.php?L",
		   "Search Help" => "${html_path}/search-help.php");
  else if ($base == "search-help.php")
    $links = array("Documentation" => "${html_path}/documentation.php",
		   "Articles" => "${html_path}/articles.php?L",
		   "Developer Guide" => "${html_path}/htmldoc-cmp.php");
  else
    $links = array("Articles" => "${html_path}/articles.php?L",
		   "Developer Guide" => "${html_path}/htmldoc-cmp.php",
		   "Search Help" => "${html_path}/search-help.php");

  if ($title != "")
    html_header($title, "", $links);
  else
    html_header("Documentation", "", $links);
}


//
// 'doc_footer()' - End the page.
//

function
doc_footer()
{
  html_footer();
}


//
// 'doc_search()' - Search the documentation and return an array of matches.
//

function				// O - Array of matches
doc_search($search,			// I - Search string
           $dirname = "docfiles")	// I - What documentation to search
{
  // Run the CUPS websearch utility to search the documentation...
  $shsearch  = escapeshellarg($search);
  $shdirname = escapeshellarg($dirname);

  $fp      = popen("/usr/local/bin/websearch $shdirname $shsearch", "r");
  $matches = array();

  // Skip the match count...
  fgets($fp);

  // Read the results: score|link|text
  while ($line = fgets($fp, 2048))
  {
    if (preg_match("/^[0-9]+\\|([^|]+)\\|([^|]+)\\|(.*\$)/", trim($line), $data))
    {
      $link  = trim($data[1]);
      $text  = trim($data[2]);
      $title = trim($data[3]);

      if (strpos($link, "#") !== FALSE)
	$matches["documentation.php/$dirname/$link"] = "$text";
    }
  }

  pclose($fp);

  return ($matches);
}


//
// End of "$Id$".
//
?>
