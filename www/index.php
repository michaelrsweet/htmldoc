<?php
//
// "$Id: index.php,v 1.18 2006/08/02 19:55:44 mike Exp $"
//
// HTMLDOC home page...
//

include_once "phplib/db-article.php";
include_once "phplib/common.php";
include_once "phplib/poll.php";

html_header();

print("<h1 align='center'>HTMLDOC Open Source Home Page</h1>");

print("<p><table width='100%' height='100%' border='0' cellpadding='0' "
     ."cellspacing='0'>\n"
     ."<tr><td valign='top' width='30%'>");

html_start_table(array("Quick Info"), "100%", "100%");
html_start_row();
print("<td>"
     ."<p align='center'>"
     ."Stable Release: <a href='software.php?1.8.27'>v1.8.27</a><br />\n"
     ."Test Releases: <a href='software.php'>Snapshots</a><br />\n"
     ."Developer Roadmap: <a href='roadmap.php'>View</a><br />\n"
     ."Binaries: <a href='http://www.easysw.com/htmldoc/'>easysw.com</a></p>\n"
     ."<small><p>HTMLDOC converts Hyper-Text Markup Language "
     ."(\"HTML\") input files into indexed HTML, Adobe<sup>&reg;</sup> "
     ."PostScript<sup>&reg;</sup>, or Adobe Portable Document Format (\"PDF\") "
     ."files.</p>\n"
     ."<p>HTMLDOC supports most HTML 3.2 elements, some HTML 4.0 "
     ."elements, and can generate title and table of contents pages. The "
     ."1.8.x releases do not support stylesheets.</p>\n"
     ."<p>HTMLDOC can be used as a standalone application, in a batch "
     ."document processing environment, or as a web-based report "
     ."generation application.</p>\n"
     ."<p>No restrictions are placed upon the output produced by "
     ."HTMLDOC.</p>\n"
     ."<p>HTMLDOC is available both as open source software under the "
     ."terms of the GNU General Public License and as commercial "
     ."software under the terms of a traditional commercial End-User "
     ."License Agreement.</p>\n"
     ."</small></td>");
html_end_row();
html_end_table();

$poll = get_recent_poll();
if ($poll > 0)
{
  html_start_table(array("Current Poll [&nbsp;<a href='poll.php'>"
                	."Show&nbsp;All</a>&nbsp;]"));
  html_start_row();
  print("<td>");
  show_poll($poll);
  print("</td>");
  html_end_row();
  html_end_table();
}

/*
html_start_table(array("Top 10 Links [ <a href='links.php'>Show All</a> ]"));

$result = db_query("SELECT * FROM link WHERE is_published > 0 "
                  ."AND is_category = 0 "
                  ."ORDER BY (modify_date + 864000 * rating_total / rating_count) "
		  ."DESC LIMIT 10");

while ($row = db_next($result))
{
  html_start_row();
  print("<td><small><a href='links.php?V$row[id]'>"
       ."$row[name] $row[version]</a></small></td>");
  html_end_row();
}

html_end_table();

db_free($result);
*/

print("</td><td>&nbsp;&nbsp;&nbsp;&nbsp;</td>"
     ."<td valign='top' width='70%'>"
     ."<h2><a href='http://www.easysw.com/htmldoc/'><img border='0' "
     ."src='images/htmldoc-cd.jpg' align='right' width='142' height='200' "
     ."alt='Buy HTMLDOC on CD-ROM!' title='Buy HTMLDOC on CD-ROM!' hspace='10'/></a>"
     ."Recent Articles [&nbsp;<a href='articles.php'>Show&nbsp;All</a>"
     ."&nbsp;]</h2>\n");

if ($poll > 0)
  $limit = 8;
else
  $limit = 4;

$article = new article();
$matches = $article->search("", "-modify_date", "!FAQ", 1);
$count   = sizeof($matches);
if ($count > $limit)
  $count = $limit;

if ($count == 0)
  print("<p>No articles found.</p>\n");
else
{
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

    print("<h3><a href='articles.php?L$id'>$title</a></h3>\n"
         ."<p><i>$date by $create_user, $ccount</i><br />$abstract [&nbsp;"
	 ."<a href='articles.php?L$id'>Read</a>&nbsp;]</p>\n");
  }
}

print("<p>[&nbsp;"
     ."<a type='application/rss+xml' href='index.rss''>RSS&nbsp;Feed</a>"
     ."&nbsp;]</p>");

print("</td></tr>\n"
     ."</table></p>\n");

html_footer();

//
// End of "$Id: index.php,v 1.18 2006/08/02 19:55:44 mike Exp $".
//
?>
