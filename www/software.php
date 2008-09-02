<?php
//
// "$Id: software.php,v 1.17 2006/10/03 15:35:07 mike Exp $"
//
// Software download page.
//

//
// Include necessary headers...
//

include_once "phplib/html.php";
include_once "phplib/mirrors.php";



// Get the list of software files...
$files = array();

$fp = fopen("data/snapshots.md5", "r");

while ($line = fgets($fp, 255))
  $files[sizeof($files)] = trim($line);

fclose($fp);

$fp = fopen("data/software.md5", "r");

while ($line = fgets($fp, 255))
  $files[sizeof($files)] = trim($line);

fclose($fp);

// Get form data, if any...
if (array_key_exists("FILE", $_GET))
  $file = $_GET["FILE"];
else
  $file = "";

if (array_key_exists("SITE", $_GET))
{
  $site = $_GET["SITE"];
  setcookie("SITE", $site, time() + 90 * 86400, "/");
}
else if (array_key_exists("SITE", $_COOKIE) &&
         array_key_exists($_COOKIE["SITE"], $sitelist))
  $site = $_COOKIE["SITE"];
else
  $site = mirror_closest();

if (array_key_exists("VERSION", $_GET))
  $version = $_GET["VERSION"];
else
{
  $data    = explode(" ", $files[0]);
  $version = $data[1];
}

// Build a list of links to show at the top...
$links = array();

$curversion = "";
for ($i = 0; $i < sizeof($files); $i ++)
{
  // Grab the data for the current file...
  $data     = explode(" ", $files[$i]);
  $fversion = $data[1];

  if ($fversion != $curversion)
  {
    $curversion = $fversion;
    $links["v$fversion"] = "$PHP_SELF?VERSION=$fversion";
  }
}

$links["Binaries"]   = "$PHP_SELF#BINARIES";
$links["Subversion"] = "$PHP_SELF#SVN";

// Show the standard header...
if ($site != "" && $file != "")
  html_header("Download", "", "$site/$file", $links);
else
  html_header("Download", "", "", $links);

// Show files or sites...
if ($file != "")
{
  print("<p>Your download should begin shortly. If not, please "
       ."<a href='$site/$file'>click here</a> to download the file "
       ."from the current mirror.</p>\n"
       ."<h2>Change Mirror Site:</h2>\n"
       ."<form action='$PHP_SELF' method='GET' name='download'>\n"
       ."<input type='hidden' name='FILE' value='"
       . htmlspecialchars($file, ENT_QUOTES) . "'>\n"
       ."<input type='hidden' name='VERSION' value='"
       . htmlspecialchars($version, ENT_QUOTES) . "'>\n");

  if ($site == "")
    print("<input type='radio' name='SITE' value='' checked>None<br>\n");

  reset($MIRRORS);
  while (list($key, $val) = each($MIRRORS))
  {
    print("<input type='radio' name='SITE' value='$key' "
         ."onClick='document.download.submit();'");
    if ($site == $key)
      print("  checked");
    print(">$val[0]<br>\n");
  }

  print("<input type='submit' value='Change Mirror Site'>\n"
       ."</form>\n");
}
else
{
  // Show files...
  print("<h2>Source Code</h2>\n");

  html_start_table(array("Version", "Filename", "Size", "MD5 Sum"));

  $curversion = "";

  for ($i = 0; $i < sizeof($files); $i ++)
  {
    // Grab the data for the current file...
    $data     = explode(" ", $files[$i]);
    $md5      = $data[0];
    $fversion = $data[1];
    $filename = $data[2];
    $basename = basename($filename);

    html_start_row();

    if ($fversion == $version)
    {
      $cs = "<th>";
      $ce = "</th>";
    }
    else
    {
      $cs = "<td align='center'>";
      $ce = "</td>";
    }

    if ($fversion != $curversion)
    {
      if ($curversion != "")
      {
	print("<td colspan='4'></td>");
	html_end_row();
	html_start_row();
      }

      $curversion = $fversion;
      print("$cs<a name='$fversion'>$fversion</a>$ce");
    }
    else
      print("$cs$ce");

    if (file_exists("/home/ftp.easysw.com/pub/$filename"))
      $kbytes = (int)((filesize("/home/ftp.easysw.com/pub/$filename") + 1023) / 1024);
    else
      $kbytes = "???";

    print("$cs<a href='$PHP_SELF?VERSION=$version&amp;FILE=$filename'>"
         ."<tt>$basename</tt></a>$ce"
	 ."$cs${kbytes}k$ce"
	 ."$cs<tt>$md5</tt>$ce");

    html_end_row();
  }

  html_end_table();

  print("<h2><a name='BINARIES'>Binaries</a></h2>\n"
       ."<p>Binaries for $PROJECT_NAME are available on the "
       ."<a href='http://www.easysw.com/htmldoc/software.php'>Easy "
       ."Software Products download page</a> to users that "
       ."have <a href='http://www.easysw.com/htmldoc/'>purchased a "
       ."commercial license</a> for the software. Free binaries may be available "
       ."elsewhere - search <a href='http://www.google.com/search?"
       ."q=htmldoc+binary+package&btnG=Search'>Google</a> to find them.</p>\n"
       ."<h2><a name='SVN'>Subversion Access</a></h2>\n"
       ."<p>The $PROJECT_NAME software is available via Subversion "
       ."using the following URL:</p>\n"
       ."<pre>\n"
       ."    <a href='http://svn.easysw.com/public/$PROJECT_MODULE/'>"
       ."http://svn.easysw.com/public/$PROJECT_MODULE/</a>\n"
       ."</pre>\n"
       ."<p>The following command can be used to checkout the current "
       ."$PROJECT_NAME source from Subversion:</p>\n"
       ."<pre>\n"
       ."    <kbd>svn co http://svn.easysw.com/public/$PROJECT_MODULE/trunk/ $PROJECT_MODULE</kbd>\n"
       ."</pre>\n");
}

// Show the standard footer...
html_footer();

//
// End of "$Id: software.php,v 1.17 2006/10/03 15:35:07 mike Exp $".
//
?>
