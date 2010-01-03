<?php
//
// "$Id: documentation.php,v 1.7 2005/11/18 22:01:43 mike Exp $"
//
// Documentation page...
//

//
// Include necessary headers...
//

include_once "phplib/doc.php";

if (array_key_exists("SEARCH", $_GET))
  $search = $_GET["SEARCH"];
else
  $search = "";

if (array_key_exists("PATH_INFO", $_SERVER))
{
  $filename = $_SERVER["PATH_INFO"];
  if (strpos($filename, "..") !== FALSE ||
      !preg_match("/^(\\/docfiles\\/|\\/).*\\..*/", $filename))
    $filename = "";
}
else
  $filename = "";

function
show_search()
{
  global $search;

  $html = htmlspecialchars($search, ENT_QUOTES);

  print("<form action='documentation.php' method='GET'>\n"
       ."<p align='center'><input type='search' name='SEARCH' value='$html' "
       ."size='80' placeholder='Search Documentation' "
       ."autosave='org.htmldoc.search' results='20'>"
       ."<input type='submit' value='Search'></p>\n"
       ."</form>\n"
       ."<hr>\n");
}


if ($filename != "")
{
  if (strpos($filename, "/", 1) === FALSE)
    $filename = "docfiles$filename";
  else
    $filename = substr($filename, 1);

  if (!file_exists($filename))
  {
    header("Status: 404");
    exit();
  }
  else if (preg_match("/\\.html\$/", $filename))
  {
    // HTML files get the standard header/footer treatment...
    $fp     = fopen($filename, "r");
    $title  = "";
    $inbody = FALSE;

    while ($line = fgets($fp, 1024))
    {
      if (preg_match("/<title>(.*)<\\/title>/i", $line, $matches))
      {
        if ($title == "")
	{
	  $title = $matches[1];
	  doc_header($title);
	  show_search();
        }
      }
      else if (preg_match("/<body/i", $line))
      {
	$inbody = TRUE;

	if ($title == "")
	{
	  $title = htmlspecialchars(basename($filename));
	  doc_header($title);
	  show_search();
	}
      }
      else if (preg_match("/<\\/body>/i", $line))
        break;
      else if ($inbody)
        print($line);
    }

    fclose($fp);

    doc_footer();
  }
  else if (preg_match("/\\.txt\$/", $filename))
  {
    // Text files get the standard header/footer treatment...
    $title = htmlspecialchars(basename($filename));
    doc_header($title);
    show_search();

    print("<pre>");
    print(htmlspecialchars(file_get_contents($filename)));
    print("</pre>");

    doc_footer();
  }
  else
  {
    // Other files get tagged appropriately...
    if (preg_match("/\\.gif\$/", $filename))
      header("Content-Type: image/gif");
    else if (preg_match("/\\.jpg\$/", $filename))
      header("Content-Type: image/jpeg");
    else if (preg_match("/\\.pdf\$/", $filename))
      header("Content-Type: application/pdf");
    else if (preg_match("/\\.png\$/", $filename))
      header("Content-Type: image/png");
    else
      header("Content-Type: application/octet-stream");

    readfile($filename);
  }

  exit();
}

doc_header();
show_search();

if ($search == "")
{
?>

<p>You can view the HTMLDOC Software Users Manual in a number of
formats on-line:</p>

<ul>

	<li><a href='documentation.php/toc.html'>HTML in
	separate files with Comments</a>

	<ul>

		<li><a
		href='documentation.php/Introduction.html'>Introduction</a></li>

		<li><a
		href='documentation.php/Chapter1InstallingHTMLDOC.html'>Chapter
		1 - Installing HTMLDOC</a></li>

		<li><a
		href='documentation.php/Chapter2GettingStarted.html'>Chapter
		2 - Getting Started</a></li>

		<li><a
		href='documentation.php/Chapter3GeneratingBooks.html'>Chapter
		3 - Generating Books</a></li>

		<li><a
		href='documentation.php/Chapter4HTMLDOCfromtheCommandLine.html'>Chapter
		4 - HTMLDOC from the Command-Line</a></li>

		<li><a
		href='documentation.php/Chapter5UsingHTMLDOConaWebServer.html'>Chapter
		5 - Using HTMLDOC on a Web Server</a></li>

		<li><a
		href='documentation.php/Chapter6HTMLReference.html'>Chapter
		6 - HTML Reference</a></li>

		<li><a
		href='documentation.php/Chapter7GUIReference.html'>Chapter
		7 - GUI Reference</a></li>

		<li><a
		href='documentation.php/Chapter8CommandLineReference.html'>Chapter
		8 - Command Line Reference</a></li>

		<li><a
		href='documentation.php/AppendixALicenseAgreement.html'>Appendix
		A - License Agreement</a></li>

		<li><a
		href='documentation.php/AppendixBBookFileFormat.html'>Appendix
		B - Book File Format</a></li>

		<li><a
		href='documentation.php/AppendixCReleaseNotes.html'>Appendix
		C - Release Notes</a></li>

		<li><a
		href='documentation.php/AppendixDCompilingHTMLDOCfromSource.html'>Appendix
		D - Compiling HTMLDOC from Source</a></li>

	</ul></li>

	<li><a href='htmldoc.html'>HTML in one file (217k)</a></li>

	<li><a href='htmldoc.pdf'>PDF (122 pages, 1166k)</a></li>

</ul>

<p>The following developer documentation is also available:</p>

<ul>

	<li><a href='htmldoc-cmp.php'>Developer Guide</a></li>

<!--
	<li><a href='htmldoc-sdd.php'>Software Design Description</a></li>

	<li><a href='htmldoc-stp.php'>Software Test Plan</a></li>
-->

</ul>

<?php
}
else
{
  $list  = doc_search($search);
  $count = sizeof($list);
  if ($count > 0)
  {
    if ($count == 1)
      $count = "1 match";
    else
      $count = "$count matches";

    print("<p>Found $count:</p>\n"
         ."<ul class='compact'>\n");
    reset($list);
    foreach ($list as $url => $text)
      print("<li><a href='documentation.php/$url'>$text</a></li>\n");
    print("</ul>\n");
  }
  else
    print("<p>No matches found.</p>\n");
}

doc_footer();

//
// End of "$Id: documentation.php,v 1.7 2005/11/18 22:01:43 mike Exp $".
//
?>
