<?php
//
// "$Id: rdf.php,v 1.10 2005/02/05 05:51:24 mike Exp $"
//
// This file exports the 10 most recent articles to an RDF file
// that sites and news ticker apps can grab to show the latest news.
//
// Contents:
//
//   make_rdf_file() - Create an RDF file from the current articles table.
//

include_once "db-article.php";


//
// 'make_rdf_file()' - Create an RDF file from the current articles table.
//

function
make_rdf_file($file,			// I - File to create
              $baseurl,			// I - Base URL
	      $title,			// I - Title of site
	      $description)		// I - Description of site
{
  // Create the RDF file...
  $fp = fopen($file, "w");
  if (!$fp) return;

  // Get a list of articles that are not FAQ's...
  $article = new article();
  $matches = $article->search("", "-modify_date", "!FAQ", 1);
  $count   = sizeof($matches);
  if ($count > 20)
    $count = 20;

  // XML header...
  fwrite($fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             ."<rdf:RDF\n"
	     ." xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
	     ." xmlns=\"http://purl.org/rss/1.0/\"\n"
	     ." xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
	     ." xmlns:syn=\"http://purl.org/rss/1.0/modules/syndication/\"\n"
	     .">\n");

  // Description of channel...
  $secs = time();
  $date = gmdate("Y-m-d", $secs);
  $time = gmdate("H:i:s", $secs);
  fwrite($fp, "<channel rdf:about=\"${baseurl}index.rss\">\n"
	     ."  <title>$title</title>\n"
	     ."  <link>$baseurl</link>\n"
	     ."  <description>$description</description>\n"
	     ."  <dc:language>en-us</dc:language>\n"
	     ."  <dc:date>${date}T$time+00:00</dc:date>\n"
	     ."  <dc:publisher>Easy Software Products</dc:publisher>\n"
	     ."  <dc:creator>webmaster@easysw.com</dc:creator>\n"
	     ."  <dc:subject>Technology</dc:subject>\n"
	     ."  <syn:updatePeriod>hourly</syn:updatePeriod>\n"
	     ."  <syn:updateFrequency>1</syn:updateFrequency>\n"
	     ."  <syn:updateBase>1970-01-01T00:00+00:00</syn:updateBase>\n"
	     ."  <items>\n"
	     ."    <rdf:Seq>\n");

  // Item index...
  for ($i = 0; $i < $count; $i ++)
    fwrite($fp, "      <rdf:li rdf:resource=\"${baseurl}articles.php?L$matches[$i]\" />\n");

  fwrite($fp, "    </rdf:Seq>\n"
             ."  </items>\n"
	     ."  <image rdf:resource=\"${baseurl}images/htmldoc-128.png\" />\n"
	     ."  <textinput rdf:resource=\"${baseurl}search.php\" />\n"
	     ."</channel>\n"
	     ."<image rdf:about=\"${baseurl}images/htmldoc-128.png\">\n"
	     ."  <title>$title</title>\n"
	     ."  <url>${baseurl}images/htmldoc-128.png</url>\n"
	     ."  <link>$baseurl</link>\n"
	     ."</image>\n");

  // Now the news items...
  for ($i = 0; $i < $count; $i ++)
  {
    $article->load($matches[$i]);

    $headline    = htmlspecialchars($article->title, ENT_QUOTES, "UTF-8");
    $description = htmlspecialchars($article->abstract, ENT_QUOTES, "UTF-8");
    $date        = gmdate("Y-m-d", $article->modify_date);
    $time        = gmdate("H:i:s", $article->modify_date);

    fwrite($fp, "<item rdf:about=\"${baseurl}articles.php?L$article->id\">\n"
	       ."  <title>$headline</title>\n"
	       ."  <link>${baseurl}articles.php?L$article->id</link>\n"
	       ."  <description>$description</description>\n"
	       ."  <dc:creator>$article->modify_user</dc:creator>\n"
	       ."  <dc:subject>$article->type</dc:subject>\n"
	       ."  <dc:date>${date}T$time+00:00</dc:date>\n"
	       ."</item>\n");
  }

  // Finally a search link and close the file...
  fwrite($fp, "<textinput rdf:about=\"${baseurl}search.php\">\n"
	     ."<title>Search</title>\n"
	     ."<description>Search Site</description>\n"
	     ."<name>Q</name>\n"
	     ."<link>${baseurl}search.php</link>\n"
	     ."</textinput>\n");

  fwrite($fp, "</rdf:RDF>\n");
  fclose($fp);
}

?>
