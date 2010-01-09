<?php

include_once "phplib/db-str.php";

// List of versions that are planned...
$versions = array(
  "1.8"  => "HTMLDOC 1.8.x is the current stable branch. Bug fix (patch) "
           ."releases will come out sporadically as needed.",
  "1.9"  => "HTMLDOC 1.9.x is the current development branch and is focused on "
	   ."supporting UTF-8, basic CSS1 functionality, <tt>INPUT</tt> and "
	   ."<tt>TEXTAREA</tt> form fields, background images in tables, and "
	   ."custom Type 1 fonts.",
  "1.10" => "HTMLDOC 1.10.x is a future development branch and will focus on "
           ."more advanced font support including subsetting, "
	   ."TrueType/OpenType font support, character pair kerning, and "
	   ."Hebrew/Arabic/Asian text formatting."
);

$links = array(
  "List" => "str.php",
  "New Bug Report" => "str.php?U"
);

foreach ($versions as $version => $description)
  $links["HTMLDOC $version"] = "#$version";

// Batch update bugs...
if ($LOGIN_LEVEL >= AUTH_DEVEL && $REQUEST_METHOD == "POST")
{
  if (array_key_exists("status", $_POST) &&
      ($_POST["status"] != "" ||
       $_POST["str_version"] != "" ||
       $_POST["fix_version"] != "" ||
       $_POST["priority"] != "" ||
       $_POST["manager_user"] != "" ||
       $_POST["message"] != ""))
  {
    foreach ($_POST as $key => $val)
    {
      if (ereg("ID_[0-9]+", $key))
      {
	$id  = (int)substr($key, 3);
	$str = new str($id);

	if ($str->id != $id)
	  continue;

	if ($_POST["status"] != "")
	  $str->status = (int)$_POST["status"];
	if ($_POST["str_version"] != "")
	  $str->str_version = $_POST["str_version"];
	if ($_POST["fix_version"] != "")
	  $str->fix_version = $_POST["fix_version"];
	if ($_POST["priority"] != "")
	  $str->priority = (int)$_POST["priority"];
	if ($_POST["manager_user"] != "")
	  $str->manager_user = $_POST["manager_user"];

	if ($str->validate())
	{
	  $str->save();

	  if ($_POST["message"] != "")
	    $contents = add_text($str);
	  else
	    $contents = "";

	  if ($contents !== FALSE)
	    notify_users($str, $contents);
	}
      }
    }
  }

  header("Location: $PHP_SELF");
  exit(0);
}

html_header("Development Roadmap", "", $links);

print("<p>This page provides a dynamic look at all bug reports that have been "
     ."filed and accepted through the <a href='str.php'>Bugs &amp; "
     ."Features</a> page. If you would like to contribute code to implement "
     ."any of the bugs below, please consult the <a href='htmldoc-cmp.php'>"
     ."Developer Guide</a> for the coding standards we follow and then post "
     ."your changes to the corresponding bug.</p>\n");

reset($versions);
while (list($version, $description) = each($versions))
{
  if ($LOGIN_LEVEL >= AUTH_DEVEL)
    $is_published = "";
  else
    $is_published = " AND is_published = 1";

  $result = db_query("SELECT id, priority, status FROM str WHERE "
                    ."str_version LIKE '${version}%'"
		    ." AND status > " . STR_STATUS_UNRESOLVED . $is_published
		    ." ORDER BY is_published,status DESC,priority DESC,id");

  if (db_count($result) > 0)
  {
    $rfecount = 0;
    $bugcount = 0;

    $firstid = 0;
    while ($row = db_next($result))
    {
      if ($firstid == 0)
	$firstid = $row["id"];

      if ($row["priority"] == STR_PRIORITY_RFE)
	$rfecount ++;
      else
	$bugcount ++;
    }

    $prefix = ", ";

    if ($rfecount == 1)
      $html = "1 Feature";
    else if ($rfecount > 0)
      $html = "$rfecount Features";
    else
    {
      $html   = "";
      $prefix = "";
    }

    if ($bugcount == 1)
      $html .= "${prefix}1 Bug";
    else if ($bugcount > 0)
      $html .= "$prefix$bugcount Bugs";

    if ($html != "")
      $html = "($html)";

    if ($LOGIN_LEVEL >= AUTH_DEVEL)
      $linkop = "U";
    else
      $linkop = "L";

    print("<h2><a name='$version'>HTMLDOC $version</a> "
	 ."<a href='str.php?$linkop$firstid+Qversion:$version'>$html</a></h2>\n"
	 ."<p>$description</p>\n");

    if ($LOGIN_LEVEL >= AUTH_DEVEL)
      print("<form method='POST' action='$PHP_SELF'>\n");

    html_start_table(array("STR #", "Summary", "Status", "Priority"));

    $curtime = time();

    db_seek($result, 0);
    while ($row = db_next($result))
    {
      $id       = $row["id"];
      $str      = new str($id);
      $summary  = htmlspecialchars($str->summary);
      $priority = $STR_PRIORITY_SHORT[$str->priority];
      $status   = $STR_STATUS_SHORT[$str->status];

      if ($LOGIN_LEVEL >= AUTH_DEVEL)
      {
	$days = (int)(($curtime - $str->modify_date) / 86400);

	if ($days == 0)
	  $age = "";
	else if ($days < 30)
	  $age = sprintf("&nbsp;%.0fD", $days);
	else if ($days < 365)
	  $age = sprintf("&nbsp;%.0fM", $days / 30);
	else
	  $age = sprintf("&nbsp;%.1fY", $days / 365);

	$box = "<input type='checkbox' name='ID_$id'>";
      }
      else
      {
	$age = "";
	$box = "";
      }

      if ($str->is_published == 0)
	$summary .= " <img src='images/private.gif' align='absmiddle' "
		   ."width='16' height='16' alt='Private'>";

      html_start_row();
      print("<td align='center' nowrap>$box<a href='str.php?$linkop$id+Qversion:$version'>$id</a></td>"
	   ."<td><a href='str.php?$linkop$id+Qversion:$version'>$summary</a></td>"
	   ."<td align='center'><a href='str.php?$linkop$id+Qversion:$version'>$status$age</a></td>"
	   ."<td align='center'><a href='str.php?$linkop$id+Qversion:$version'>$priority</a></td>");
      html_end_row();
    }

    html_end_table();

    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      print("<p align='center'>Status:&nbsp;<select name='status'>"
	   ."<option value=''>No Change</option>");
      for ($i = 1; $i <= 5; $i ++)
	print("<option value='$i'>$STR_STATUS_SHORT[$i]</option>");
      print("</select>\n");

      print("Priority:&nbsp;<select name='priority'>"
	   ."<option value=''>No Change</option>");
      for ($i = 1; $i <= 5; $i ++)
	print("<option value='$i'>$STR_PRIORITY_SHORT[$i]</option>");
      print("</select>\n");

      print("Version:&nbsp;<select name='str_version'>"
	   ."<option value=''>No Change</option>"
	   ."<option>Not Applicable</option>");

      foreach ($STR_VERSIONS as $val)
      {
        if ($val[0] == '+')
	  $val = substr($val, 1);

	print("<option value='$val'>$val</option>");
      }
      print("</select>\n");

      print("Fix Version:&nbsp;<select name='fix_version'>"
	   ."<option value=''>No Change</option>"
	   ."<option>Not Applicable</option>");

      foreach ($STR_VERSIONS as $val)
      {
        if (strpos($val, "-feature") !== FALSE)
	  continue;

        if ($val[0] == '+')
	  $val = substr($val, 1);

	print("<option value='$val'>$val</option>");
      }
      print("</select>\n");

      print("<br>\n"
           ."Assigned To:&nbsp;<select name='manager_user'>"
	   ."<option value=''>No Change</option>");
      foreach($STR_MANAGERS as $key => $val)
      {
	$temp = sanitize_email($val);
	print("<option value='$key'>$temp</option>");
      }
      print("</select>\n");

      print("Text:&nbsp;<select name='message'>"
	   ."<option value=''>No Message</option>");
      foreach($STR_MESSAGES as $key => $val)
      {
	$temp = abbreviate($val);
	print("<option value='$key'>$temp</option>");
      }
      print("</select>\n");

      print("<input type='submit' value='Modify Selected Bugs'>"
	   ."</p></form>\n");
    }

    db_free($result);
  }
  else
    print("<h2><a name='$version'>HTMLDOC $version (No Features or Bugs)</a></h2>\n"
	 ."<p>$description</p>\n");
}

html_footer();

?>
