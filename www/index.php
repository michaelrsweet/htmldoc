<?php
//
// "$Id: index.php,v 1.18 2006/08/02 19:55:44 mike Exp $"
//
// HTMLDOC home page...
//

include_once "phplib/db-article.php";
include_once "phplib/mirrors.php";

html_header();

$fp   = fopen("data/software.md5", "r");
while ($line = fgets($fp, 1024))
{
  $data = explode(" ", trim($line));
  if (preg_match("/^[1-9]\\.[0-9]+\\.[0-9+]\$/", $data[1]) &&
      preg_match("/\\.tar\\.bz2\$/", $data[2]))
    break;
}
fclose($fp);

$mirror  = mirror_closest();
$url     = "$mirror/$data[2]";
$version = $data[1];
$file    = "/home/ftp.easysw.com/pub/$data[2]";
if (file_exists($file))
  $size = sprintf("%.1fM", filesize($file) / 1024 / 1024);
else
  $size = "?.?M";

?>

<h1><img src='images/htmldoc.jpg' width='128' height='128'
align='right'>HTMLDOC Home Page</h1>

<p>HTMLDOC converts Hyper-Text Markup Language ("HTML") files and web pages into
indexed HTML, Adobe<sup>&reg;</sup> PostScript<sup>&reg;</sup>, or Adobe
Portable Document Format ("PDF") files. HTMLDOC is provided under the terms of <a href="COPYING.html">version 2 of the GNU General Public License</a> with an exception to allow building against OpenSSL.</p>

<table border='0' cellspacing='0' cellpadding='0' summary='Download' background='images/download.jpg'>
<tr><td class='download'><?
print("<a href='$url'>Download v$version ($size)</a>");
?></td></tr>
</table>

<p>&nbsp;</p>

<table width='100%' border='0' cellpadding='0' cellspacing='0' summary=''>
<tr><td valign='top' width='50%'>

<h2>Documentation</h2>

<p><a href='documentation.php/Introduction.html'>Introduction</a></p>

<p><a href='documentation.php/Chapter4HTMLDOCfromtheCommandLine.html'>HTMLDOC
from the Command-Line</a></p>

<p><a href='documentation.php/Chapter5UsingHTMLDOConaWebServer.html'>Using
HTMLDOC on a Web Server</a></p>

<p><a href='documentation.php/AppendixALicenseAgreement.html'>License Agreement</a></p>

<p><a href='documentation.php/AppendixCReleaseNotes.html'>Release Notes</a></p>

</td>
<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>
<td valign='top' width='50%'>

<h2>Recent Articles</h2>

<?

$article = new article();
$matches = $article->search("", "-modify_date", "!FAQ", 1);
$count   = sizeof($matches);
if ($count > 3)
  $count = 3;

for ($i = 0; $i < $count; $i ++)
{
  $id = $matches[$i];

  $article->load($id);

  $title       = htmlspecialchars($article->title);
  $abstract    = html_format($article->abstract);
  $create_user = sanitize_email($article->create_user);
  $date        = date("H:i M d, Y", $article->modify_date);
  $ccount      = count_comments("articles.php_L$id");

  if ($ccount == 1)
    $ccount .= " comment";
  else
    $ccount .= " comments";

  print("<p><a href='articles.php?L$id'>$title</a><br>\n"
       ."<i>$date by $create_user, $ccount</i></p>\n");
}

?>

<p><a href='index.rss'>RSS Feed</a></p>

</td></tr>
</table>

<? html_footer(); ?>
