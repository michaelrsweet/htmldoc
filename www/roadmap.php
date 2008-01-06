<?php

include_once "phplib/db-str.php";

// List of versions that are planned...
$versions = array(
  "1.9"  => "The 1.9 series will focus on supporting UTF-8, basic CSS1 "
           ."functionality, <tt>INPUT</tt> and <tt>TEXTAREA</tt> form "
	   ."fields, background images in tables, and custom Type 1 fonts.",
  "1.10" => "The 1.10 series will focus on more advanced font support, "
           ."including TrueType font support, character pair kerning, "
	   ."and Hebrew/Arabic/Asian text formatting.",
  "2.0"  => "The 2.0 series will focus on converting the HTMLDOC core "
           ."into a C++ class library so that applications can link to "
	   ."HTMLDOC rather than calling the command-line program."
);

$links = array();

reset($versions);
while (list($version, $description) = each($versions))
  $links["HTMLDOC $version"] = "#$version";

html_header("Development Roadmap", "", "", $links);

print("<p>This page provides a dynamic look at all Requests For Enhancement "
     ."(\"RFEs\") that have been filed and accepted through the <a href='str.php'>"
     ."Bugs &amp; Features</a> page. If you would like to contribute code to "
     ."implement any of the RFEs below, please consult the <a "
     ."href='htmldoc-cmp.php'>Configuration Management Plan</a> for the "
     ."coding standards we follow and then post your changes to the "
     ."corresponding RFE.</p>\n");

$completed = 0;
$count     = 0;

reset($versions);
while (list($version, $description) = each($versions))
{
  $result = db_query("SELECT id, status FROM str WHERE "
                    ."str_version LIKE '${version}%'"
		    ." AND status != " . STR_STATUS_UNRESOLVED
		    ." ORDER BY status DESC,priority DESC,id");

  if (db_count($result) > 0)
  {
    $completed = 0;
    while ($row = db_next($result))
      if ($row["status"] == STR_STATUS_RESOLVED)
	$completed ++;

    $count   = db_count($result);
    $percent = (int)(100 * $completed / $count);

    print("<h2><a name='$version'>HTMLDOC $version ($completed of $count "
         ."completed, $percent%)</a></h2>\n"
	 ."<p>$description</p>\n");

    html_start_table(array("STR #", "Summary", "Priority", "Status"));

    db_seek($result, 0);
    while ($row = db_next($result))
    {
      if ($row["status"] == STR_STATUS_RESOLVED)
        continue;

      $id        = $row["id"];
      $str       = new str($id);
      $summary   = htmlspecialchars($str->summary);
      $priority  = $STR_PRIORITY_SHORT[$str->priority];
      $status    = $STR_STATUS_SHORT[$str->status];

      html_start_row();
      print("<td align='center'><a href='str.php?L$id'>$id</a></td>"
           ."<td><a href='str.php?L$id'>$summary</a></td>"
           ."<td align='center'><a href='str.php?L$id'>$priority</a></td>"
           ."<td align='center'><a href='str.php?L$id'>$status</a></td>");
      html_end_row();
    }

    html_end_table();

    db_free($result);
  }
  else
    print("<h2><a name='$version'>HTMLDOC $version (planned)</a></h2>\n"
	 ."<p>$description</p>\n");
}

html_footer();

?>
