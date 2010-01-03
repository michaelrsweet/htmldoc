<?php

include_once "phplib/doc.php";
include_once "phplib/db-str.php";
include_once "phplib/db-article.php";

// Show the standard header...
html_header("Search");

if (array_key_exists("Q", $_GET))
  $search = $_GET["Q"];
else
  $search = "";

$html      = htmlspecialchars($search, ENT_QUOTES);
$urlsearch = urlencode($search);

print("<form method='GET' action='$html_path/search.php'><p align='center'>"
     ."<input type='search' name='Q' size='80' placeholder='Search Site' "
     ."autosave='org.htmldoc.search' results='20' value='$html'>"
     ."<input type='submit' value='Search Site'></p></form>\n"
     ."<hr noshade>\n");

$matches = array();
$urls    = array();

if ($search != "")
{
  // Search documentation...
  $list = doc_search($search);
  if (sizeof($list) > 0)
  {
    $matches["Documentation"] = $list;
    $urls["Documentation"]    = "documentation.php?SEARCH=$urlsearch";
  }

  // Search articles...
  $result = db_query("SELECT DISTINCT(type) AS type FROM article ORDER BY type");
  while ($row = db_next($result))
  {
    $type    = $row["type"];
    $article = new article();
    $temp    = $article->search($search, "-modify_date", $type, 1);
    $count   = sizeof($temp);

    if ($count > 0)
    {
      $list = array();
      for ($i = 0; $i < $count; $i ++)
      {
	$id = $temp[$i];
	$article->load($id);
	$list["articles.php?L$id"] = htmlspecialchars($article->title);
      }

      $matches[$type] = $list;
      $urls[$type]    = "articles.php?L+T" . urlencode($type) . "+Q$urlsearch";
    }
  }

  // Search bugs...
  $str   = new str();
  $temp  = $str->search($search, "-modify_date");
  $count = sizeof($temp);

  if ($count > 0)
  {
    $list = array();
    for ($i = 0; $i < $count; $i ++)
    {
      $id = $temp[$i];
      $str->load($id);
      $list["str.php?L$id"] = "STR #$id: " . htmlspecialchars($str->summary);
    }

    $matches["Bugs &amp; Features"] = $list;
    $urls["Bugs &amp; Features"]    = "str.php?L+Q$urlsearch";
  }
}

// Display results...
if (sizeof($matches) == 0)
  print("<p>No matches found.</p>\n");
else
{
  // Generate up to 2 columns of results...
  ksort($matches);

  $columns = array(array(), array());
  $lines   = array(0, 0);

  foreach ($matches as $section => $list)
  {
    if ($lines[0] <= $lines[1])
      $min_column = 0;
    else
      $min_column = 1;

    $columns[$min_column][$section] = $list;
    if (sizeof($list) > 7)
      $lines[$min_column] += 10;
    else
      $lines[$min_column] += sizeof($list) + 2;
  }

  print("<table width='100%' summary=''>\n"
       ."<tr>\n");
  for ($column = 0; $column < 2; $column ++)
  {
    if (sizeof($columns[$column]) == 0)
      break;

    if ($column > 0)
      $style = " style='padding-left: 20px;'";
    else
      $style = "";

    print("<td$style valign='top' width='50%'>");

    ksort($columns[$column]);
    foreach ($columns[$column] as $section => $list)
    {
      print("<h2 class='title'>$section</h2>\n");
      $line = 0;
      foreach ($list as $url => $text)
      {
	print("<p class='compact'><a href='$url'>$text</a></p>\n");
        $line ++;
	if ($line >= 7)
	  break;
      }

      if (($count = sizeof($list) - 7) > 0)
      {
        $url = $urls[$section];
        print("<p class='compact'><br>\n"
	     ."<em><a href='$url'>See $count more matches...</a></em></p>\n");
      }
    }

    print("</td>\n");
  }

  print("</tr>\n"
       ."</table>\n");
}

html_footer();

?>
