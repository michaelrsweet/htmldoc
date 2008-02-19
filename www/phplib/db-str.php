<?php
//
// "$Id: db-str.php,v 1.17 2006/08/02 19:55:45 mike Exp $"
//
// Class for the str table.
//
// Contents:
//
//   str::str()			- Create a STR object.
//   str::clear()		- Initialize a new a STR object.
//   str::delete()		- Delete a STR object.
//   str::edit()		- Display a form for a STR object.
//   str::load()		- Load a STR object.
//   str::loadform()		- Load a STR object from form data.
//   str::save()		- Save a STR object.
//   str::search()		- Get a list of STR IDs.
//   str::validate()		- Validate the current STR object values.
//   str::view()		- View the current STR object.
//   str::select_subsystem()	- Select a subsystem...
//   str::select_version()	- Select a version number...
//

include_once "db-strfile.php";
include_once "db-strtext.php";


// Standard messages...
$STR_MESSAGES = array(
  "Fixed in SVN" =>
      "Fixed in Subversion repository.",
  "No Support" =>
      "General support is not available via the STR form. Please post to the "
     ."$PROJECT_NAME forums and/or mailing lists for general support.",
  "Old STR" =>
      "This STR has not been updated by the submitter for two or more weeks "
     ."and has been closed as required by the $PROJECT_NAME Configuration Management "
     ."Plan. If the issue still requires resolution, please re-submit a new "
     ."STR.",
  "Unresolvable" =>
      "We are unable to resolve this problem with the information provided. "
     ."If you discover new information, please file a new STR referencing "
     ."this one."
);
    

// Subsystems...
$STR_SUBSYSTEMS = array(
  "Book",
  "Build Files",
  "CGI",
  "Command-Line",
  "Config Files",
  "Documentation",
  "File",
  "GUI",
  "Image",
  "Installer",
  "Multiple",
  "None",
  "Render",
  "Style",
  "Tree",
  "Web Site"
);


// Version numbers...
$STR_VERSIONS = array(
  "2.0-feature",
  "+2.0-current",
  "1.10-feature",
  "+1.10-current",
  "1.9-feature",
  "1.9-current",
  "+1.9b1",
  "1.8-current",
  "1.8.27",
  "1.8.26",
  "1.8.25",
  "1.8.24",
  "1.8.24rc4",
  "1.8.24rc3",
  "1.8.24rc2",
  "1.8.24rc1",
  "1.8.23",
  "1.8.22",
  "1.8.21",
  "1.8.20 or older",
  "Web Site",
  "+None",
  "+Will Not Fix"
);


//
// STR constants...
//

define("STR_STATUS_RESOLVED", 1);
define("STR_STATUS_UNRESOLVED", 2);
define("STR_STATUS_ACTIVE", 3);
define("STR_STATUS_PENDING", 4);
define("STR_STATUS_NEW", 5);

define("STR_PRIORITY_ANY", 0);
define("STR_PRIORITY_RFE", 1);
define("STR_PRIORITY_LOW", 2);
define("STR_PRIORITY_MODERATE", 3);
define("STR_PRIORITY_HIGH", 4);
define("STR_PRIORITY_CRITICAL", 5);

define("STR_SCOPE_ANY", 0);
define("STR_SCOPE_UNIT", 1);
define("STR_SCOPE_FUNCTION", 2);
define("STR_SCOPE_SOFTWARE", 3);

//
// String definitions for STR constants...
//

$STR_STATUS_SHORT = array(
  STR_STATUS_RESOLVED => "Resolved",
  STR_STATUS_UNRESOLVED => "Unresolved",
  STR_STATUS_ACTIVE => "Active",
  STR_STATUS_PENDING => "Pending",
  STR_STATUS_NEW => "New"
);

$STR_STATUS_LONG = array(
  STR_STATUS_RESOLVED => "1 - Closed w/Resolution",
  STR_STATUS_UNRESOLVED => "2 - Closed w/o Resolution",
  STR_STATUS_ACTIVE => "3 - Active",
  STR_STATUS_PENDING => "4 - Pending",
  STR_STATUS_NEW => "5 - New"
);

$STR_PRIORITY_SHORT = array(
  STR_PRIORITY_RFE => "RFE",
  STR_PRIORITY_LOW => "LOW",
  STR_PRIORITY_MODERATE => "MOD",
  STR_PRIORITY_HIGH => "HIGH",
  STR_PRIORITY_CRITICAL => "CRIT"
);

$STR_PRIORITY_LONG = array(
  STR_PRIORITY_RFE => "1 - Request for Enhancement, e.g. asking for a feature",
  STR_PRIORITY_LOW => "2 - Low, e.g. a documentation error or undocumented side-effect",
  STR_PRIORITY_MODERATE => "3 - Moderate, e.g. unable to compile the software",
  STR_PRIORITY_HIGH => "4 - High, e.g. key functionality not working",
  STR_PRIORITY_CRITICAL => "5 - Critical, e.g. nothing working at all"
);

$STR_SCOPE_SHORT = array(
  STR_SCOPE_UNIT => "MACH",
  STR_SCOPE_FUNCTION => "OS",
  STR_SCOPE_SOFTWARE => "ALL"
);

$STR_SCOPE_LONG = array(
  STR_SCOPE_UNIT => "1 - Specific to a machine",
  STR_SCOPE_FUNCTION => "2 - Specific to an operating system",
  STR_SCOPE_SOFTWARE => "3 - Applies to all machines and operating systems"
);


//
// Get the list of valid developers from the users table...
//

$STR_MANAGERS = array();
$result = db_query("SELECT * FROM user WHERE is_published = 1 AND "
                  ."level >= " . AUTH_DEVEL);
while ($row = db_next($result))
  $STR_MANAGERS[$row["name"]] = $row["email"];
db_free($result);


class str
{
  //
  // Instance variables...
  //

  var $id;
  var $master_id, $master_id_valid;
  var $is_published;
  var $status, $status_valid;
  var $priority, $priority_valid;
  var $scope, $scope_valid;
  var $summary, $summary_valid;
  var $subsystem, $subsystem_valid;
  var $str_version, $str_version_valid;
  var $fix_version, $fix_version_valid;
  var $fix_revision, $fix_revision_valid;
  var $manager_user, $manager_user_valid;
  var $create_date;
  var $create_user;
  var $modify_date;
  var $modify_user;


  //
  // 'str::str()' - Create a STR object.
  //

  function				// O - New STR object
  str($id = 0)				// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'str::clear()' - Initialize a new a STR object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->master_id = 0;
    $this->is_published = 1;
    $this->status = STR_STATUS_NEW;
    $this->priority = 0;
    $this->scope = 0;
    $this->summary = "";
    $this->subsystem = "";
    $this->str_version = "";
    $this->fix_version = "";
    $this->fix_revision = 0;
    $this->manager_user = "";
    $this->create_date = 0;
    $this->create_user = "";
    $this->modify_date = 0;
    $this->modify_user = "";
  }


  //
  // 'str::delete()' - Delete a STR object.
  //

  function
  delete()
  {
    db_query("DELETE FROM str WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'str::edit()' - Display a form for a STR object.
  //

  function
  edit($action,				// I - Action text
       $options = "")			// I - URL options
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF, $REQUEST_METHOD;
    global $STR_MANAGERS, $STR_SCOPE_LONG, $STR_STATUS_LONG;
    global $STR_PRIORITY_LONG;
    global $STR_MESSAGES;
    global $_GET, $_POST, $_FILES;


    print("<form action='$PHP_SELF?U$this->id$options' method='POST' "
         ."enctype='multipart/form-data'>\n"
         ."<input type='hidden' name='MAX_FILE_SIZE' value='10485760'>"
	 ."<p align='center'><table class='edit'>\n");

    // master_id
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      $html = htmlspecialchars($this->master_id, ENT_QUOTES);
      if ($this->master_id_valid)
	$hclass = "valid";
      else
	$hclass = "invalid";
      print("<tr><th class='$hclass' align='right' valign='top' nowrap>Duplicate Of:</th><td>");
      print("<input type='text' name='master_id' "
           ."value='$html' size='4'>");
      print("</td></tr>\n");
    }

    // is_published
    print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
    html_select_is_published($this->is_published);
    print(" (choose \"no\" for security advisories)</td></tr>\n");

    // status
    if ($this->status_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Status:</th><td>");
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      print("<select name='status'>");
      for ($i = 1; $i <= 5; $i ++)
        if ($i == $this->status)
	  print("<option value='$i' selected>$STR_STATUS_LONG[$i]</option>");
	else
	  print("<option value='$i'>$STR_STATUS_LONG[$i]</option>");
      print("</select></td></tr>\n");
    }
    else
      print("<input type='hidden' name='status' value='$this->status'>"
          . $STR_STATUS_LONG[$this->status] ."</td></tr>\n");

    // priority
    if ($this->priority_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Priority:</th><td>");
    print("<select name ='priority'>");
    if ($this->id <= 0)
      print("<option value=''>Choose Priority</option>");
    for ($i = 1; $i <= 5; $i ++)
      if ($i == $this->priority)
	print("<option value='$i' selected>$STR_PRIORITY_LONG[$i]</option>");
      else
	print("<option value='$i'>$STR_PRIORITY_LONG[$i]</option>");
    print("</select></td></tr>\n");

    // scope
    if ($this->scope_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Scope:</th><td>");
    print("<select name ='scope'>");
    if ($this->id <= 0)
      print("<option value=''>Choose Scope</option>");
    for ($i = 1; $i <= 3; $i ++)
      if ($i == $this->scope)
	print("<option value='$i' selected>$STR_SCOPE_LONG[$i]</option>");
      else
	print("<option value='$i'>$STR_SCOPE_LONG[$i]</option>");
    print("</select></td></tr>\n");

    // subsystem
    if ($this->subsystem_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Subsystem:</th><td>");
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
      $this->select_subsystem("subsystem", $this->subsystem);
    else
      print("<input type='hidden' name='subsystem' value=''><i>Unassigned</i>");
    print("</td></tr>\n");

    // summary
    $html = htmlspecialchars($this->summary, ENT_QUOTES);
    if ($this->summary_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Summary:</th><td>");
    print("<input type='text' name='summary' "
         ."value='$html' size='72'>");
    print("</td></tr>\n");

    // str_version
    if ($this->str_version_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Software Version:</th><td>");
    $this->select_version("str_version", $this->str_version);
    print("</td></tr>\n");

    // create_user
    $html = htmlspecialchars($this->create_user);
    print("<tr><th class='valid' align='right' valign='top' nowrap>Created By:</th><td>");
    print("$html</td></tr>\n");

    // manager_user
    if ($this->manager_user_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Assigned To:</th><td>");
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      print("<select name='manager_user'><option value=''>Unassigned</option>");
      reset($STR_MANAGERS);
      while (list($user, $manager) = each($STR_MANAGERS))
      {
	$name = sanitize_email($manager);

	if ($manager == $this->manager_user ||
	    $name == $this->manager_user ||
	    $user == $this->manager_user)
          print("<option value='$user' selected>$user ($name)</option>");
	else
          print("<option value='$user'>$user ($name)</option>");
      }
      print("</select>");
    }
    else
      print("<input type='hidden' name='manager_user' value=''><i>Unassigned</i>");
    print("</td></tr>\n");

    // fix_version
    if ($this->fix_version_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Fix Version:</th><td>");
    if ($LOGIN_LEVEL >= AUTH_DEVEL)
    {
      $this->select_version("fix_version", $this->fix_version);

      if ($this->fix_revision_valid)
	$hclass = "valid";
      else
	$hclass = "invalid";

      print(" <b class='$hclass'>SVN: r</b><input type='text' size='6' "
           ."name='fix_revision' value='$this->fix_revision'>");
    }
    else
    {
      print("<input type='hidden' name='fix_version' value=''><i>Unassigned</i>");
      print("<input type='hidden' name='fix_revision' value=''>");
    }
    print("</td></tr>\n");

    // Text...
    if (array_key_exists("contents", $_GET))
      $contents = $_GET["contents"];
    else if (array_key_exists("contents", $_POST))
      $contents = $_POST["contents"];
    else
      $contents = "";

    if ($contents == "" && $this->id == 0 && $REQUEST_METHOD == "POST")
      $hclass = "invalid";
    else
      $hclass = "valid";

    print("<tr><th class='$hclass' align='right' valign='top'>");
    if ($this->id <= 0)
      print("Detailed Description of Problem:");
    else
      print("Additional Text:");

    print("</th><td>");

    if ($LOGIN_LEVEL >= AUTH_DEVEL && $this->id > 0)
    {
      if (array_key_exists("message", $_GET))
	$message = $_GET["message"];
      else if (array_key_exists("message", $_POST))
	$message = $_POST["message"];
      else
	$message = "";

      print("<select name='message'>"
	   ."<option value=''>--- Pick a Standard Message ---</option>");

      reset($STR_MESSAGES);
      while (list($key, $val) = each($STR_MESSAGES))
      {
	$temp = abbreviate($val, 72);
	if ($key == $message)
	  print("<option value='$key' selected>$temp</option>");
	else
	  print("<option value='$key'>$temp</option>");
      }
      print("</select><br>\n");
    }

    $html = htmlspecialchars($contents);
    print("<textarea name='contents' cols='72' rows='12' wrap='virtual'>"
         ."$html</textarea></td></tr>\n");

    // File...
    print("<tr><th align='right' valign='top'>Attach File:</th><td>"
         ."<input name='file' type='file'></td></tr>\n");

    // Submit
    print("<tr><td></td><td>"
         ."<input type='submit' value='$action'>"
         ."</td></tr>\n"
         ."</table></p>\n"
         ."</form>\n");
  }


  //
  // 'str::load()' - Load a STR object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM str WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->master_id = $row["master_id"];
    $this->is_published = $row["is_published"];
    $this->status = $row["status"];
    $this->priority = $row["priority"];
    $this->scope = $row["scope"];
    $this->summary = $row["summary"];
    $this->subsystem = $row["subsystem"];
    $this->str_version = $row["str_version"];
    $this->fix_version = $row["fix_version"];
    $this->fix_revision = $row["fix_revision"];
    $this->manager_user = $row["manager_user"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];
    $this->modify_date = $row["modify_date"];
    $this->modify_user = $row["modify_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'str::loadform()' - Load a STR object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if (array_key_exists("master_id", $_GET))
      $this->master_id = $_GET["master_id"];
    else if (array_key_exists("master_id", $_POST))
      $this->master_id = $_POST["master_id"];

    if (array_key_exists("is_published", $_GET))
      $this->is_published = $_GET["is_published"];
    else if (array_key_exists("is_published", $_POST))
      $this->is_published = $_POST["is_published"];

    if (array_key_exists("status", $_GET))
      $this->status = $_GET["status"];
    else if (array_key_exists("status", $_POST))
      $this->status = $_POST["status"];

    if (array_key_exists("priority", $_GET))
      $this->priority = $_GET["priority"];
    else if (array_key_exists("priority", $_POST))
      $this->priority = $_POST["priority"];

    if (array_key_exists("scope", $_GET))
      $this->scope = $_GET["scope"];
    else if (array_key_exists("scope", $_POST))
      $this->scope = $_POST["scope"];

    if (array_key_exists("summary", $_GET))
      $this->summary = $_GET["summary"];
    else if (array_key_exists("summary", $_POST))
      $this->summary = $_POST["summary"];

    if (array_key_exists("subsystem", $_GET))
      $this->subsystem = $_GET["subsystem"];
    else if (array_key_exists("subsystem", $_POST))
      $this->subsystem = $_POST["subsystem"];

    if (array_key_exists("str_version", $_GET))
      $this->str_version = $_GET["str_version"];
    else if (array_key_exists("str_version", $_POST))
      $this->str_version = $_POST["str_version"];

    if (array_key_exists("fix_version", $_GET))
      $this->fix_version = $_GET["fix_version"];
    else if (array_key_exists("fix_version", $_POST))
      $this->fix_version = $_POST["fix_version"];

    if (array_key_exists("fix_revision", $_GET))
      $this->fix_revision = (int)$_GET["fix_revision"];
    else if (array_key_exists("fix_revision", $_POST))
      $this->fix_revision = (int)$_POST["fix_revision"];

    if (array_key_exists("manager_user", $_GET))
      $this->manager_user = $_GET["manager_user"];
    else if (array_key_exists("manager_user", $_POST))
      $this->manager_user = $_POST["manager_user"];

    if (array_key_exists("create_user", $_GET))
      $this->create_user = $_GET["create_user"];
    else if (array_key_exists("create_user", $_POST))
      $this->create_user = $_POST["create_user"];

    return ($this->validate());
  }


  //
  // 'str::save()' - Save a STR object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    $this->modify_date = time();
    $this->modify_user = $LOGIN_USER;

    if ($this->id > 0)
    {
      return (db_query("UPDATE str "
                      ." SET master_id = $this->master_id"
                      .", is_published = $this->is_published"
                      .", status = $this->status"
                      .", priority = $this->priority"
                      .", scope = $this->scope"
                      .", summary = '" . db_escape($this->summary) . "'"
                      .", subsystem = '" . db_escape($this->subsystem) . "'"
                      .", str_version = '" . db_escape($this->str_version) . "'"
                      .", fix_version = '" . db_escape($this->fix_version) . "'"
                      .", fix_revision = $this->fix_revision"
                      .", manager_user = '" . db_escape($this->manager_user) . "'"
                      .", modify_date = $this->modify_date"
                      .", modify_user = '" . db_escape($this->modify_user) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO str VALUES"
                  ."(NULL"
                  .", $this->master_id"
                  .", $this->is_published"
                  .", $this->status"
                  .", $this->priority"
                  .", $this->scope"
                  .", '" . db_escape($this->summary) . "'"
                  .", '" . db_escape($this->subsystem) . "'"
                  .", '" . db_escape($this->str_version) . "'"
                  .", '" . db_escape($this->fix_version) . "'"
		  .", $this->fix_revision"
                  .", '" . db_escape($this->manager_user) . "'"
                  .", $this->create_date"
                  .", '" . db_escape($this->create_user) . "'"
                  .", $this->modify_date"
                  .", '" . db_escape($this->modify_user) . "'"
                  .")") === FALSE)
        return (FALSE);

      $this->id = db_insert_id();
    }

    return (TRUE);
  }


  //
  // 'str::search()' - Get a list of STR objects.
  //

  function				// O - Array of STR objects
  search($search = "",			// I - Search string
         $order = "",			// I - Order fields
	 $priority = 0,			// I - Priority
	 $status = 0,			// I - Status
	 $scope = 0,			// I - Scope
	 $whose = 0)			// I - Whose 
  {
    global $LOGIN_EMAIL, $LOGIN_LEVEL, $LOGIN_USER;


    $query  = "";
    $prefix = " WHERE ";

    if ($priority > 0)
    {
      $query .= "${prefix}priority = $priority";
      $prefix = " AND ";
    }

    if ($status > 0)
    {
      $query .= "${prefix}status = $status";
      $prefix = " AND ";
    }
    else if ($status == -1) // Show closed
    {
      $query .= "${prefix}status <= " . STR_STATUS_UNRESOLVED;
      $prefix = " AND ";
    }
    else if ($status == -2) // Show open
    {
      $query .= "${prefix}status >= " . STR_STATUS_ACTIVE;
      $prefix = " AND ";
    }

    if ($scope > 0)
    {
      $query .= "${prefix}scope = $scope";
      $prefix = " AND ";
    }

    if ($whose)
    {
      if ($LOGIN_LEVEL >= AUTH_DEVEL)
      {
	$query .= "${prefix}(manager_user = '' OR manager_user = '$LOGIN_USER')";
	$prefix = " AND ";
      }
      else if ($LOGIN_USER != "")
      {
	$query .= "${prefix}create_user = '$LOGIN_USER'";
	$prefix = " AND ";
      }
    }
    else if ($LOGIN_LEVEL < AUTH_DEVEL)
    {
      $query .= "${prefix}(is_published = 1 OR create_user = '$LOGIN_USER')";
      $prefix = " AND ";
    }

    if ($search != "")
    {
      // Convert the search string to an array of words...
      $words = html_search_words($search);

      // Loop through the array of words, adding them to the query...
      $query  .= "${prefix}(";
      $prefix = "";
      $next   = " OR";
      $logic  = "";

      reset($words);
      foreach ($words as $word)
      {
        if ($word == "or")
        {
          $next = ' OR';
          if ($prefix != '')
            $prefix = ' OR';
        }
        else if ($word == "and")
        {
          $next = ' AND';
          if ($prefix != '')
            $prefix = ' AND';
        }
        else if ($word == "not")
          $logic = ' NOT';
        else if (substr($word, 0, 8) == "creator:")
	{
	  $word   = substr($word, 8);
          $query .= "$prefix$logic create_user LIKE \"$word\"";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 10) == "developer:")
	{
	  $word   = substr($word, 10);
          $query .= "$prefix$logic manager_user LIKE \"$word\"";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 11) == "fixversion:")
	{
	  $word   = substr($word, 11);
          $query  .= "$prefix$logic fix_version LIKE \"$word%\"";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 7) == "number:")
	{
	  $number = (int)substr($word, 7);
          $query  .= "$prefix$logic id = $number";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 10) == "subsystem:")
	{
	  $word   = substr($word, 10);
          $query  .= "$prefix$logic subsystem LIKE \"$word\"";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 6) == "title:")
	{
	  $word   = substr($word, 6);
          $query  .= "$prefix$logic summary LIKE \"%$word%\"";
          $prefix = $next;
          $logic  = '';
	}
        else if (substr($word, 0, 8) == "version:")
	{
	  $word   = substr($word, 8);
          $query  .= "$prefix$logic str_version LIKE \"$word%\"";
          $prefix = $next;
          $logic  = '';
	}
        else
        {
          $query .= "$prefix$logic (";
          $subpre = "";

          if (ereg("^[0-9]+\$", $word))
          {
            $query .= "${subpre}id = $word";
            $subpre = " OR ";
          }

          $query .= "${subpre}summary LIKE \"%$word%\"";
          $subpre = " OR ";
          $query .= "${subpre}subsystem LIKE \"$word\"";
          $query .= "${subpre}str_version LIKE \"$word%\"";
          $query .= "${subpre}fix_version LIKE \"$word%\"";
          $query .= "${subpre}create_user LIKE \"$word\"";
          $query .= "${subpre}manager_user LIKE \"$word\"";

          $query .= ")";
          $prefix = $next;
          $logic  = '';
        }
      }

      $query .= ")";
    }

    if ($order != "")
    {
      // Separate order into array...
      $fields = explode(" ", $order);
      $prefix = " ORDER BY ";

      // Add ORDER BY stuff...
      foreach ($fields as $field)
      {
        if ($field[0] == '+')
          $query .= "${prefix}" . substr($field, 1);
        else if ($field[0] == '-')
          $query .= "${prefix}" . substr($field, 1) . " DESC";
        else
          $query .= "${prefix}$field";

        $prefix = ", ";
      }
    }

//    print("<p>SELECT id FROM str$query</p>\n");

    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM str$query");
    $matches = array();

//    print("<p>mysql_error is \"" . mysql_error() . "\"...</p>\n");

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'str::select_subsystem()' - Select a subsystem...
  //

  function
  select_subsystem($name,		// I - Form name
		   $value)		// I - Current value
  {
    global $STR_SUBSYSTEMS;

    print("<select name='$name'><option value=''>Choose Subsystem</option>");

    reset($STR_SUBSYSTEMS);
    foreach ($STR_SUBSYSTEMS as $subsystem)
    {
      if ($subsystem == $value)
        print("<option value='$subsystem' selected>$subsystem</option>");
      else
        print("<option value='$subsystem'>$subsystem</option>");
    }

    print("</select><!-- $value -->");
  }


  //
  // 'str::select_version()' - Select a version number...
  //

  function
  select_version($name,			// I - Form name
		 $value)		// I - Current value
  {
    global $LOGIN_LEVEL, $STR_VERSIONS;


    print("<select name='$name'><option value=''>Choose Version</option>");

    reset($STR_VERSIONS);
    foreach ($STR_VERSIONS as $version)
    {
      if ($version[0] == '+' && $LOGIN_LEVEL < AUTH_DEVEL)
        continue;

      if ($name != "fix_version" && $this->priority == STR_PRIORITY_RFE &&
          !strpos($version, "-feature"))
        continue;

      if ($version[0] == '+')
        $version = substr($version, 1);

      if ($version == $value)
        print("<option value='$version' selected>$version</option>");
      else
        print("<option value='$version'>$version</option>");
    }

    print("</select>");
  }


  //
  // 'str::validate()' - Validate the current STR object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->master_id > 0)
    {
      $temp = new str($this->master_id);

      if ($temp->id != $this->master_id)
      {
	$this->master_id_valid = FALSE;
	$valid = FALSE;
      }
    }
    else if ($this->master_id < 0)
    {
      $this->master_id_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->master_id_valid = TRUE;

    if ($this->status < STR_STATUS_RESOLVED ||
        $this->status > STR_STATUS_NEW)
    {
      $this->status_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->status_valid = TRUE;

    if ($this->priority < STR_PRIORITY_RFE ||
        $this->priority > STR_PRIORITY_CRITICAL)
    {
      $this->priority_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->priority_valid = TRUE;

    if ($this->scope < STR_SCOPE_UNIT ||
        $this->scope > STR_SCOPE_SOFTWARE)
    {
      $this->scope_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->scope_valid = TRUE;

    if ($this->summary == "")
    {
      $this->summary_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->summary_valid = TRUE;

    if ($this->subsystem == "" && $this->status != STR_STATUS_NEW)
    {
      $this->subsystem_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->subsystem_valid = TRUE;

    if ($this->str_version == "" ||
        ($this->priority == STR_PRIORITY_RFE &&
	 !strpos($this->str_version, "-feature")))
    {
      $this->str_version_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->str_version_valid = TRUE;

    if ($this->fix_version == "" && $this->status == STR_STATUS_RESOLVED)
    {
      $this->fix_version_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->fix_version_valid = TRUE;

    if ($this->fix_revision < 0)
    {
      $this->fix_revision_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->fix_revision_valid = TRUE;

    if ($this->manager_user == "" && $this->status != STR_STATUS_NEW)
    {
      $this->manager_user_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->manager_user_valid = TRUE;

    return ($valid);
  }


  //
  // 'str::view()' - View the current STR object.
  //

  function
  view($options = "")
  {
    global $LOGIN_EMAIL, $STR_PRIORITY_LONG, $STR_SCOPE_LONG, $STR_STATUS_LONG,
           $PHP_SELF;


    $create_user  = sanitize_email($this->create_user);
    $manager_user = sanitize_email($this->manager_user);
    $subsystem    = $this->subsystem;
    $summary      = htmlspecialchars($this->summary, ENT_QUOTES);
    $prtext       = $STR_PRIORITY_LONG[$this->priority];
    $sttext       = $STR_STATUS_LONG[$this->status];
    $sctext       = $STR_SCOPE_LONG[$this->scope];
    $str_version  = $this->str_version;
    $fix_version  = $this->fix_version;

    if ($manager_user == "")
      $manager_user = "<i>Unassigned</i>";

    if ($subsystem == "")
      $subsystem = "<i>Unassigned</i>";

    if ($fix_version == "")
      $fix_version = "<i>Unassigned</i>";

    if ($this->fix_revision > 0)
      $fix_revision = " (SVN: r$this->fix_revision)";
    else
      $fix_revision = "";

    print("<p align='center'><table class='view'>\n");

    // master_id
    if ($this->master_id > 0)
      print("<tr><th align='right'>Duplicate Of:</th>"
	   ."<td><a href='$PHP_SELF?L$this->master_id$options'>STR "
	   ."#$this->master_id</a></td></tr>\n");

    // is_published
    if (!$this->is_published)
      print("<tr><th align='center' colspan='2'>This STR is "
	   ."currently hidden from public view.</td></tr>\n");

    // status
    print("<tr><th align='right' valign='top' nowrap>Status:</th><td>");
    print($sttext);
    print("</td></tr>\n");

    // priority
    print("<tr><th align='right' valign='top' nowrap>Priority:</th><td>");
    print($prtext);
    print("</td></tr>\n");

    // scope
    print("<tr><th align='right' valign='top' nowrap>Scope:</th><td>");
    print($sctext);
    print("</td></tr>\n");

    // subsystem
    print("<tr><th align='right' valign='top' nowrap>Subsystem:</th><td>");
    print($subsystem);
    print("</td></tr>\n");

    // summary
    print("<tr><th align='right' valign='top' nowrap>Summary:</th><td>");
    print($summary);
    print("</td></tr>\n");

    // str_version
    print("<tr><th align='right' valign='top' nowrap>Version:</th><td>");
    print($str_version);
    print("</td></tr>\n");

    // create_user
    print("<tr><th align='right' valign='top' nowrap>Created By:</th><td>");
    print($create_user);
    print("</td></tr>\n");

    // manager_user
    print("<tr><th align='right' valign='top' nowrap>Assigned To:</th><td>");
    print($manager_user);
    print("</td></tr>\n");

    // fix_version
    print("<tr><th align='right' valign='top' nowrap>Fix Version:</th><td>");
    print("$fix_version$fix_revision");
    print("</td></tr>\n");

    $email = htmlspecialchars($LOGIN_EMAIL, ENT_QUOTES);

    print("<tr><th align='right' valign='top'>Update Notification:</th><td>"
	 ."<form method='POST' action='$PHP_SELF?N$this->id$options'>"
	 ."<input type='text' size='40' maxsize='128' name='EMAIL' value='$email'>"
	 ."<input type='submit' value='Change Notification Status'>"
	 ."<br><input type='radio' name='NOTIFICATION' checked value='ON'>Receive EMails "
	 ."<input type='radio' name='NOTIFICATION' value='OFF'>Don't Receive EMails"
	 ."</form>"
	 ."</td></tr>\n");
    print("</table></p>\n");
  }
}


//
// End of "$Id: db-str.php,v 1.17 2006/08/02 19:55:45 mike Exp $".
//
?>
