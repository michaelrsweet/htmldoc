<?php
//
// "$Id: documentation.php,v 1.7 2005/11/18 22:01:43 mike Exp $"
//
// Documentation page...
//

//
// Include necessary headers...
//

include_once "phplib/html.php";


//
// Get the web server path information and serve the named file as needed...
//

if (array_key_exists("PATH_INFO", $_SERVER))
  $PATH_INFO = $_SERVER["PATH_INFO"];
else
  $PATH_INFO = "";

if ($PATH_INFO != "/" && $PATH_INFO != "")
{
  $path = substr($PATH_INFO, 1);

  if (strstr($path, ".gif"))
    $type = "gif";
  else if (strstr($path, ".jpg"))
    $type = "jpeg";
  else if (strstr($path, ".png"))
    $type = "png";
  else
    $type = "html";

  if (strstr($path, ".."))
  {
    if ($type == "html")
    {
      html_header("Documentation Error", "../");

      print("<p>The path '$path' is bad.</p>\n");

      html_footer();
    }
  }
  else
  {
    $fp = fopen("docfiles/$path", "rb");
    if (!$fp)
    {
      if ($type == "html")
      {
	html_header("Documentation Error", "../");

	print("<p>Unable to open path '$path'.</p>\n");

	html_footer();
      }
    }
    else if ($type == "html")
    {
      $saw_body = 0;
      $last_nav = 0;

      while ($line = fgets($fp, 1024))
      {
        if (strstr($line, "<TITLE>"))
	{
	  $start = strpos($line, "<TITLE>") + 7;
	  $end   = strpos($line, "</TITLE>");

          html_header(substr($line, $start, $end - $start), "../");
        }
        else if (strstr($line, "<BODY"))
	{
	  $saw_body = 1;
	}
	else if (strstr($line, "</BODY>"))
	{
	  break;
	}
	else if ($saw_body)
	{
	  if (strstr($line, "Contents</A") ||
	      strstr($line, "Previous</A>") ||
	      strstr($line, "Next</A>"))
	  {
	    if ($last_nav)
	      print("|\n");
	    else
	      print("<p align='center'>[ <a href='#_USER_COMMENTS'>Comments</a> |\n");

            $last_nav = 1;
	  }
	  else if (strstr($line, "<HR NOSHADE>"))
	  {
	    if ($last_nav)
	      print("]</p>\n");

	    $last_nav = 0;
	  }

	  print($line);
	}
      }

      fclose($fp);

      if ($last_nav)
        print("]\n");

      print("<hr noshade/>\n"
           ."<h2><a name='_USER_COMMENTS'>User Comments</a> [&nbsp;"
	   ."<a href='../../comment.php?U+Pdocumentation.php$path'>Add&nbsp;Comment</a>"
	   ."&nbsp;]</h2>\n");

      $num_comments = show_comments("documentation.php$path");

      if ($num_comments == 0)
        print("<p>No comments for this page.</p>\n");

      html_footer();
    }
    else
    {
      header("Content-Type: image/$type");
      
      print(fread($fp, filesize("docfiles/$path")));

      fclose($fp);
    }
  }
}
else
{
  html_header("Documentation");

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

	<li><a href='htmldoc-cmp.php'>Configuration Management Plan</a></li>

	<li><a href='roadmap.php'>Development Roadmap</a></li>


<!--
	<li><a href='htmldoc-sdd.php'>Software Design Description</a></li>

	<li><a href='htmldoc-stp.php'>Software Test Plan</a></li>
-->

</ul>

<?php

  html_footer();
}

//
// End of "$Id: documentation.php,v 1.7 2005/11/18 22:01:43 mike Exp $".
//
?>
