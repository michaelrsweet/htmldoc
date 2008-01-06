#!/usr/bin/php -q
<?php
//
// "$Id: makeclass.php,v 1.4 2004/10/14 05:47:41 mike Exp $"
//
// Make a class template for a database table.
//

include_once "../phplib/db.php";


// Validate args...
global $_SERVER;

$argc = $_SERVER["argc"];
$argv = $_SERVER["argv"];

if ($argc != 3)
{
  $fp = fopen("php://stderr", "w");
  fwrite($fp, "Usage: ./makeclass.php tablename \"Table Name\" >../phplib/db-tablename.php\n");
  return (1);
}

$table = $argv[1];
$tname = $argv[2];

if (eregi("[aeiou].*", $tname))
  $atname = "an $tname";
else
  $atname = "a $tname";

// Get the table definition...
$result = db_query("describe $table");
if (!$result)
{
  $fp = fopen("php://stderr", "w");
  fwrite($fp, "ERROR: Unable to get description of table $table:\n");
  fwrite($fp, db_error() . "\n");
  return (1);
}

// See if this table has a modify_date field...
$has_modify = 0;
db_seek($result, 0);
while ($row = db_next($result))
  if ($row["Field"] == "modify_date")
  {
    $has_modify = 1;
    break;
  }

// Write the standard header...
print("<?php\n"
     ."//\n"
     ."// \"\$Id\$\"\n"
     ."//\n"
     ."// Class for the $table table.\n"
     ."//\n"
     ."// Contents:\n"
     ."//\n"
     ."//   $table::$table()	- Create $atname object.\n"
     ."//   $table::clear()	- Initialize a new $atname object.\n"
     ."//   $table::delete()	- Delete $atname object.\n"
     ."//   $table::edit()	- Display a form for $atname object.\n"
     ."//   $table::load()	- Load $atname object.\n"
     ."//   $table::loadform()	- Load $atname object from form data.\n"
     ."//   $table::save()	- Save $atname object.\n"
     ."//   $table::search()	- Get a list of $tname IDs.\n"
     ."//   $table::validate()	- Validate the current $tname object values.\n"
     ."//   $table::view()	- View the current $tname object.\n"
     ."//\n"
     ."\n"
     ."include_once \"html.php\";\n"
     ."\n"
     ."\n"
     ."class $table\n"
     ."{\n"
     ."  //\n"
     ."  // Instance variables...\n"
     ."  //\n"
     ."\n");

db_seek($result, 0);
while ($row = db_next($result))
  switch ($row["Field"])
  {
    case "id" :
    case "is_published" :
    case "create_date" :
    case "create_user" :
    case "modify_date" :
    case "modify_user" :
	print("  var \$$row[Field];\n");
        break;
    default :
	print("  var \$$row[Field], \$$row[Field]_valid;\n");
	break;
  }

// Constructor...
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::$table()' - Create $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - New $tname object\n"
     ."  $table(\$id = 0)			// I - ID, if any\n"
     ."  {\n"
     ."    if (\$id > 0)\n"
     ."      \$this->load(\$id);\n"
     ."    else\n"
     ."      \$this->clear();\n"
     ."  }\n");

// clear()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::clear()' - Initialize a new $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function\n"
     ."  clear()\n"
     ."  {\n");

db_seek($result, 0);
while ($row = db_next($result))
  if (strncasecmp($row["Type"], "int", 3))
    print("    \$this->$row[Field] = \"\";\n");
  else
    print("    \$this->$row[Field] = 0;\n");

print("  }\n");

// delete()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::delete()' - Delete $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function\n"
     ."  delete()\n"
     ."  {\n"
     ."    db_query(\"DELETE FROM $table WHERE id=\$this->id\");\n"
     ."    \$this->clear();\n"
     ."  }\n");

// edit()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::edit()' - Display a form for $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function\n"
     ."  edit()\n"
     ."  {\n"
     ."    global \$LOGIN_USER, \$LOGIN_LEVEL, \$PHP_SELF;\n"
     ."\n"
     ."    if (\$this->id <= 0)\n"
     ."      \$action = \"Submit $tname\";\n"
     ."    else\n"
     ."      \$action = \"Modify $tname #\$this->id\";\n"
     ."\n"
     ."    print(\"<h1>\$action</h1>\\n\"\n"
     ."         .\"<form action='\$PHP_SELF?U\$this->id' method='POST'>\\n\"\n"
     ."         .\"<p><table class='edit'>\\n\");\n");

db_seek($result, 0);
while ($row = db_next($result))
{
  if (ereg("(id|create_data|create_user|modify_date|modify_user)",
           $row["Field"]))
    continue;

  $rname = ucwords(str_replace("_", " ", $row["Field"]));

  print("\n"
       ."    // $row[Field]\n"
       ."    \$html = htmlspecialchars(\$this->$row[Field], ENT_QUOTES);\n");

  if ($row["Field"] == "is_published")
    print("    print(\"<tr><th class='valid' align='right' valign='top' "
	 ."nowrap>Published:</th><td>\");\n");
  else
    print("    if (\$this->$row[Field]_valid)\n"
	 ."      \$hclass = \"valid\";\n"
	 ."    else\n"
	 ."      \$hclass = \"invalid\";\n"
	 ."    print(\"<tr><th class='\$hclass' align='right' valign='top' "
	 ."nowrap>$rname:</th><td>\");\n");

  switch ($row["Field"])
  {
    case "is_published" :
        print("    html_select_is_published(\$this->is_published);\n");
        break;

    case "contents" :
    case "description" :
    case "abstract" :
        if ($row["Field"] == "abstract")
	  $rows = "2";
	else
	  $rows = "10";

        print("    print(\"<textarea name='$row[Field]' cols='72' "
	     ."rows='$rows' wrap='virtual'>\$html</textarea>\\n\"\n"
	     ."         .\"<p>May contain the following HTML elements: \"\n"
	     ."         .\"<tt>A</tt>, <tt>B</tt>, <tt>BLOCKQUOTE</tt>, \"\n"
	     ."         .\"<tt>CODE</tt>, <tt>EM</tt>, <tt>H1</tt>, <tt>H2</tt>, \"\n"
	     ."         .\"<tt>H3</tt>, <tt>H4</tt>, <tt>H5</tt>, <tt>H6</tt>, <tt>I</tt>, \"\n"
	     ."         .\"<tt>IMG</tt>, <tt>LI</tt>, <tt>OL</tt>, <tt>P</tt>, <tt>PRE</tt>, \"\n"
	     ."         .\"<tt>SUP</tt>, <tt>TT</tt>, <tt>U</tt>, <tt>UL</tt></p>\\n\");\n");
	break;

    default :
        print("    print(\"<input type='text' name='$row[Field]' \"\n"
	     ."         .\"value='\$html' size='72'/>\");\n");
        break;
  }

  print("    print(\"</td></tr>\\n\");\n");
}

print("\n"
     ."    // Submit\n"
     ."    print(\"<tr><td></td><td>\"\n"
     ."         .\"<input type='submit' value='\$action'/>\"\n"
     ."         .\"</td></tr>\\n\"\n"
     ."         .\"</table></p>\\n\"\n"
     ."         .\"</form>\\n\");\n"
     ."  }\n");

// load()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::load()' - Load $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - TRUE if OK, FALSE otherwise\n"
     ."  load(\$id)				// I - Object ID\n"
     ."  {\n"
     ."    \$this->clear();\n"
     ."\n"
     ."    \$result = db_query(\"SELECT * FROM $table WHERE id = \$id\");\n"
     ."    if (db_count(\$result) != 1)\n"
     ."      return (FALSE);\n"
     ."\n"
     ."    \$row = db_next(\$result);\n");

db_seek($result, 0);
while ($row = db_next($result))
  print("    \$this->$row[Field] = \$row[\"$row[Field]\"];\n");

print("\n"
     ."    db_free(\$result);\n"
     ."\n"
     ."    return (\$this->validate());\n"
     ."  }\n");

// loadform()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::loadform()' - Load $atname object from form data.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - TRUE if OK, FALSE otherwise\n"
     ."  loadform()\n"
     ."  {\n"
     ."    global \$_GET, \$_POST, \$LOGIN_LEVEL;\n"
     ."\n");

db_seek($result, 0);
while ($row = db_next($result))
  switch ($row["Field"])
  {
    case "id" :
    case "create_date" :
    case "create_user" :
    case "modify_date" :
    case "modify_user" :
        break;

    case "is_published" :
	print("\n"
	     ."    if (\$LOGIN_LEVEL < AUTH_DEVEL)\n"
	     ."      \$this->is_published = 0;\n"
	     ."    else if (array_key_exists(\"$row[Field]\", \$_GET))\n"
	     ."      \$this->$row[Field] = \$_GET[\"$row[Field]\"];\n"
	     ."    else if (array_key_exists(\"$row[Field]\", \$_POST))\n"
	     ."      \$this->$row[Field] = \$_POST[\"$row[Field]\"];\n");
        break;

    default :
	print("\n"
	     ."    if (array_key_exists(\"$row[Field]\", \$_GET))\n"
	     ."      \$this->$row[Field] = \$_GET[\"$row[Field]\"];\n"
	     ."    else if (array_key_exists(\"$row[Field]\", \$_POST))\n"
	     ."      \$this->$row[Field] = \$_POST[\"$row[Field]\"];\n");
        break;
  }

print("\n"
     ."    return (\$this->validate());\n"
     ."  }\n");

// save()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::save()' - Save $atname object.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - TRUE if OK, FALSE otherwise\n"
     ."  save()\n"
     ."  {\n"
     ."    global \$LOGIN_USER, \$PHP_SELF;\n"
     ."\n"
     ."\n");

if ($has_modify)
  print("    \$this->modify_date = time();\n"
       ."    \$this->modify_user = \$LOGIN_USER;\n"
       ."\n");

print("    if (\$this->id > 0)\n"
     ."    {\n"
     ."      return (db_query(\"UPDATE $table \"\n");

$prefix = " SET ";

db_seek($result, 0);
while ($row = db_next($result))
  switch ($row["Field"])
  {
    case "id" :
    case "create_date" :
    case "create_user" :
        break;

    default :
        print("                      .\"${prefix}$row[Field] = ");
	if (!strncasecmp($row["Type"], "int", 3))
	  print("\$this->$row[Field]\"\n");
	else
	  print("'\" . db_escape(\$this->$row[Field]) . \"'\"\n");

        $prefix = ", ";
	break;
  }

print("                      .\" WHERE id = \$this->id\") !== FALSE);\n"
     ."    }\n"
     ."    else\n"
     ."    {\n"
     ."      \$this->create_date = time();\n"
     ."      \$this->create_user = \$LOGIN_USER;\n"
     ."\n"
     ."      if (db_query(\"INSERT INTO $table VALUES\"\n");

$prefix = "(";

db_seek($result, 0);
while ($row = db_next($result))
{
  switch ($row["Field"])
  {
    case "id" :
        print("                  .\"${prefix}NULL\"\n");
        break;

    default :
        print("                  .\"${prefix}");
	if (!strncasecmp($row["Type"], "int", 3))
	  print("\$this->$row[Field]\"\n");
	else
	  print("'\" . db_escape(\$this->$row[Field]) . \"'\"\n");
	break;
  }

  $prefix = ", ";
}

print("                  .\")\") === FALSE)\n"
     ."        return (FALSE);\n"
     ."\n"
     ."      \$this->id = db_insert_id();\n"
     ."    }\n"
     ."\n"
     ."    return (TRUE);\n"
     ."  }\n");

// search()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::search()' - Get a list of $tname objects.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - Array of $tname objects\n"
     ."  search(\$search = \"\",			// I - Search string\n"
     ."         \$order = \"\")			// I - Order fields\n"
     ."  {\n"
     ."    if (\$search != \"\")\n"
     ."    {\n"
     ."      // Convert the search string to an array of words...\n"
     ."      \$words = html_search_words(\$search);\n"
     ."\n"
     ."      // Loop through the array of words, adding them to the query...\n"
     ."      \$query  = \" WHERE (\";\n"
     ."      \$prefix = \"\";\n"
     ."      \$next   = \" OR\";\n"
     ."      \$logic  = \"\";\n"
     ."\n"
     ."      reset(\$words);\n"
     ."      foreach (\$words as \$word)\n"
     ."      {\n"
     ."        if (\$word == \"or\")\n"
     ."        {\n"
     ."          \$next = ' OR';\n"
     ."          if (\$prefix != '')\n"
     ."            \$prefix = ' OR';\n"
     ."        }\n"
     ."        else if (\$word == \"and\")\n"
     ."        {\n"
     ."          \$next = ' AND';\n"
     ."          if (\$prefix != '')\n"
     ."            \$prefix = ' AND';\n"
     ."        }\n"
     ."        else if (\$word == \"not\")\n"
     ."          \$logic = ' NOT';\n"
     ."        else\n"
     ."        {\n"
     ."          \$query .= \"\$prefix\$logic (\";\n"
     ."          \$subpre = \"\";\n"
     ."\n"
     ."          if (ereg(\"[0-9]+\", \$word))\n"
     ."          {\n"
     ."            \$query .= \"\${subpre}id = \$word\";\n"
     ."            \$subpre = \" OR \";\n"
     ."          }\n"
     ."\n");

$first = 1;
db_seek($result, 0);
while ($row = db_next($result))
  switch ($row["Field"])
  {
    default :
        if (strncasecmp($row["Type"], "int", 3) &&
	    $row["Field"] != "create_user" &&
	    $row["Field"] != "modify_user")
	{
          print("          \$query .= \"\${subpre}$row[Field] LIKE "
	       ."\\\"%\$word%\\\"\";\n");
          if ($first)
	  {
	    print("          \$subpre = \" OR \";\n");
	    $first = 0;
	  }
	}
	break;
  }

print("\n"
     ."          \$query .= \")\";\n"
     ."          \$prefix = \$next;\n"
     ."          \$logic  = '';\n"
     ."        }\n"
     ."      }\n"
     ."\n"
     ."      \$query .= \")\";\n"
     ."    }\n"
     ."    else\n"
     ."      \$query = \"\";\n"
     ."\n"
     ."    if (\$order != \"\")\n"
     ."    {\n"
     ."      // Separate order into array...\n"
     ."      \$fields = explode(\" \", \$order);\n"
     ."      \$prefix = \" ORDER BY \";\n"
     ."\n"
     ."      // Add ORDER BY stuff...\n"
     ."      foreach (\$fields as \$field)\n"
     ."      {\n"
     ."        if (\$field[0] == '+')\n"
     ."          \$query .= \"\${prefix}\" . substr(\$field, 1);\n"
     ."        else if (\$field[0] == '-')\n"
     ."          \$query .= \"\${prefix}\" . substr(\$field, 1) . \" DESC\";\n"
     ."        else\n"
     ."          \$query .= \"\${prefix}\$field\";\n"
     ."\n"
     ."        \$prefix = \", \";\n"
     ."      }\n"
     ."    }\n"
     ."\n"
     ."    // Do the query and convert the result to an array of objects...\n"
     ."    \$result  = db_query(\"SELECT id FROM $table\$query\");\n"
     ."    \$matches = array();\n"
     ."\n"
     ."    while (\$row = db_next(\$result))\n"
     ."      \$matches[sizeof(\$matches)] = \$row[\"id\"];\n"
     ."\n"
     ."    // Free the query result and return the array...\n"
     ."    db_free(\$result);\n"
     ."\n"
     ."    return (\$matches);\n"
     ."  }\n");

// validate()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::validate()' - Validate the current $tname object values.\n"
     ."  //\n"
     ."\n"
     ."  function				// O - TRUE if OK, FALSE otherwise\n"
     ."  validate()\n"
     ."  {\n"
     ."    \$valid = TRUE;\n");

db_seek($result, 0);
while ($row = db_next($result))
  switch ($row["Field"])
  {
    case "create_user" :
    case "modify_user" :
        break;

    default :
        if (strncasecmp($row["Type"], "int", 3))
	{
	  print("\n"
	       ."    if (\$this->$row[Field] == \"\")\n"
	       ."    {\n"
	       ."      \$this->$row[Field]_valid = FALSE;\n"
	       ."      \$valid = FALSE;\n"
	       ."    }\n"
	       ."    else\n"
	       ."      \$this->$row[Field]_valid = TRUE;\n");
	}
	break;
  }

print("\n"
     ."    return (\$valid);\n"
     ."  }\n");

// view()
print("\n"
     ."\n"
     ."  //\n"
     ."  // '$table::view()' - View the current $tname object.\n"
     ."  //\n"
     ."\n"
     ."  function\n"
     ."  view()\n"
     ."  {\n"
     ."    print(\"<p><table class='view'>\\n\");\n");

db_seek($result, 0);
while ($row = db_next($result))
{
  if ($row["Field"] == "id")
    continue;

  if ($row["Field"] == "is_published")
    $rname = "Published";
  else
    $rname = ucwords(str_replace("_", " ", $row["Field"]));

  print("\n"
       ."    // $row[Field]\n");
  if (ereg(".*_date", $row["Field"]))
    print("    \$html = date(\"H:i M d, Y\", \$this->$row[Field]);\n");
  else if (ereg("(contents|description)", $row["Field"]))
    print("    \$html = html_format(\$this->$row[Field]);\n");
  else if ($row["Field"] != "is_published")
    print("    \$html = htmlspecialchars(\$this->$row[Field], ENT_QUOTES);\n");

  print("    print(\"<tr><th align='right' valign='top' nowrap>$rname:"
       ."</th><td>\");\n");

  switch ($row["Field"])
  {
    case "is_published" :
        print("    if (\$this->is_published)\n"
	     ."      print(\"Yes\");\n"
	     ."    else\n"
	     ."      print(\"No\");\n");
        break;

    default :
        print("    print(\$html);\n");
        break;
  }

  print("    print(\"</td></tr>\\n\");\n");
}

print("    print(\"</table></p>\\n\");\n"
     ."  }\n");

// End of class...
print("}\n"
     ."\n"
     ."\n"
     ."//\n"
     ."// End of \"\$Id\$\".\n"
     ."//\n"
     ."?>\n");


//
// End of "$Id: makeclass.php,v 1.4 2004/10/14 05:47:41 mike Exp $".
//
?>
