<?php
//
// "$Id: db-poll.php,v 1.5 2006/07/11 13:55:14 mike Exp $"
//
// Class for the poll table.
//
// Contents:
//
//   poll::poll()	- Create an Poll object.
//   poll::clear()	- Initialize a new an Poll object.
//   poll::delete()	- Delete an Poll object.
//   poll::edit()	- Display a form for an Poll object.
//   poll::load()	- Load an Poll object.
//   poll::loadform()	- Load an Poll object from form data.
//   poll::save()	- Save an Poll object.
//   poll::search()	- Get a list of Poll IDs.
//   poll::validate()	- Validate the current Poll object values.
//   poll::view()	- View the current Poll object.
//

include_once "html.php";


class poll
{
  //
  // Instance variables...
  //

  var $id;
  var $is_published;
  var $poll_type, $poll_type_valid;
  var $question, $question_valid;
  var $answer0, $answer0_valid;
  var $count0, $count0_valid;
  var $answer1, $answer1_valid;
  var $count1, $count1_valid;
  var $answer2, $answer2_valid;
  var $count2, $count2_valid;
  var $answer3, $answer3_valid;
  var $count3, $count3_valid;
  var $answer4, $answer4_valid;
  var $count4, $count4_valid;
  var $answer5, $answer5_valid;
  var $count5, $count5_valid;
  var $answer6, $answer6_valid;
  var $count6, $count6_valid;
  var $answer7, $answer7_valid;
  var $count7, $count7_valid;
  var $answer8, $answer8_valid;
  var $count8, $count8_valid;
  var $answer9, $answer9_valid;
  var $count9, $count9_valid;
  var $votes, $votes_valid;
  var $create_date;
  var $create_user;
  var $modify_date;
  var $modify_user;


  //
  // 'poll::poll()' - Create an Poll object.
  //

  function				// O - New Poll object
  poll($id = 0)			// I - ID, if any
  {
    if ($id > 0)
      $this->load($id);
    else
      $this->clear();
  }


  //
  // 'poll::clear()' - Initialize a new an Poll object.
  //

  function
  clear()
  {
    $this->id = 0;
    $this->is_published = 0;
    $this->poll_type = 0;
    $this->question = "";
    $this->answer0 = "";
    $this->count0 = 0;
    $this->answer1 = "";
    $this->count1 = 0;
    $this->answer2 = "";
    $this->count2 = 0;
    $this->answer3 = "";
    $this->count3 = 0;
    $this->answer4 = "";
    $this->count4 = 0;
    $this->answer5 = "";
    $this->count5 = 0;
    $this->answer6 = "";
    $this->count6 = 0;
    $this->answer7 = "";
    $this->count7 = 0;
    $this->answer8 = "";
    $this->count8 = 0;
    $this->answer9 = "";
    $this->count9 = 0;
    $this->votes = 0;
    $this->create_date = 0;
    $this->create_user = "";
    $this->modify_date = 0;
    $this->modify_user = "";
  }


  //
  // 'poll::delete()' - Delete an Poll object.
  //

  function
  delete()
  {
    db_query("DELETE FROM poll WHERE id=$this->id");
    $this->clear();
  }


  //
  // 'poll::edit()' - Display a form for an Poll object.
  //

  function
  edit()
  {
    global $LOGIN_USER, $LOGIN_LEVEL, $PHP_SELF;

    if ($this->id <= 0)
      $action = "Submit Poll";
    else
      $action = "Modify Poll #$this->id";

    print("<h1>$action</h1>\n"
         ."<form action='$PHP_SELF?U$this->id' method='POST'>\n"
         ."<p><table class='edit'>\n");

    // is_published
    $html = htmlspecialchars($this->is_published, ENT_QUOTES);
    print("<tr><th class='valid' align='right' valign='top' nowrap>Published:</th><td>");
    html_select_is_published($this->is_published);
    print("</td></tr>\n");

    // poll_type
    $html = htmlspecialchars($this->poll_type, ENT_QUOTES);
    if ($this->poll_type_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Poll Type:</th><td>");
    print("<input type='text' name='poll_type' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // question
    $html = htmlspecialchars($this->question, ENT_QUOTES);
    if ($this->question_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Question:</th><td>");
    print("<input type='text' name='question' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer0
    $html = htmlspecialchars($this->answer0, ENT_QUOTES);
    if ($this->answer0_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer0:</th><td>");
    print("<input type='text' name='answer0' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count0
    $html = htmlspecialchars($this->count0, ENT_QUOTES);
    if ($this->count0_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count0:</th><td>");
    print("<input type='text' name='count0' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer1
    $html = htmlspecialchars($this->answer1, ENT_QUOTES);
    if ($this->answer1_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer1:</th><td>");
    print("<input type='text' name='answer1' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count1
    $html = htmlspecialchars($this->count1, ENT_QUOTES);
    if ($this->count1_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count1:</th><td>");
    print("<input type='text' name='count1' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer2
    $html = htmlspecialchars($this->answer2, ENT_QUOTES);
    if ($this->answer2_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer2:</th><td>");
    print("<input type='text' name='answer2' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count2
    $html = htmlspecialchars($this->count2, ENT_QUOTES);
    if ($this->count2_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count2:</th><td>");
    print("<input type='text' name='count2' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer3
    $html = htmlspecialchars($this->answer3, ENT_QUOTES);
    if ($this->answer3_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer3:</th><td>");
    print("<input type='text' name='answer3' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count3
    $html = htmlspecialchars($this->count3, ENT_QUOTES);
    if ($this->count3_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count3:</th><td>");
    print("<input type='text' name='count3' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer4
    $html = htmlspecialchars($this->answer4, ENT_QUOTES);
    if ($this->answer4_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer4:</th><td>");
    print("<input type='text' name='answer4' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count4
    $html = htmlspecialchars($this->count4, ENT_QUOTES);
    if ($this->count4_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count4:</th><td>");
    print("<input type='text' name='count4' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer5
    $html = htmlspecialchars($this->answer5, ENT_QUOTES);
    if ($this->answer5_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer5:</th><td>");
    print("<input type='text' name='answer5' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count5
    $html = htmlspecialchars($this->count5, ENT_QUOTES);
    if ($this->count5_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count5:</th><td>");
    print("<input type='text' name='count5' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer6
    $html = htmlspecialchars($this->answer6, ENT_QUOTES);
    if ($this->answer6_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer6:</th><td>");
    print("<input type='text' name='answer6' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count6
    $html = htmlspecialchars($this->count6, ENT_QUOTES);
    if ($this->count6_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count6:</th><td>");
    print("<input type='text' name='count6' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer7
    $html = htmlspecialchars($this->answer7, ENT_QUOTES);
    if ($this->answer7_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer7:</th><td>");
    print("<input type='text' name='answer7' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count7
    $html = htmlspecialchars($this->count7, ENT_QUOTES);
    if ($this->count7_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count7:</th><td>");
    print("<input type='text' name='count7' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer8
    $html = htmlspecialchars($this->answer8, ENT_QUOTES);
    if ($this->answer8_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer8:</th><td>");
    print("<input type='text' name='answer8' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count8
    $html = htmlspecialchars($this->count8, ENT_QUOTES);
    if ($this->count8_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count8:</th><td>");
    print("<input type='text' name='count8' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // answer9
    $html = htmlspecialchars($this->answer9, ENT_QUOTES);
    if ($this->answer9_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Answer9:</th><td>");
    print("<input type='text' name='answer9' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // count9
    $html = htmlspecialchars($this->count9, ENT_QUOTES);
    if ($this->count9_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Count9:</th><td>");
    print("<input type='text' name='count9' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // votes
    $html = htmlspecialchars($this->votes, ENT_QUOTES);
    if ($this->votes_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Votes:</th><td>");
    print("<input type='text' name='votes' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // create_date
    $html = htmlspecialchars($this->create_date, ENT_QUOTES);
    if ($this->create_date_valid)
      $hclass = "valid";
    else
      $hclass = "invalid";
    print("<tr><th class='$hclass' align='right' valign='top' nowrap>Create Date:</th><td>");
    print("<input type='text' name='create_date' "
         ."value='$html' size='72'/>");
    print("</td></tr>\n");

    // Submit
    print("<tr><td></td><td>"
         ."<input type='submit' value='$action'/>"
         ."</td></tr>\n"
         ."</table></p>\n"
         ."</form>\n");
  }


  //
  // 'poll::load()' - Load an Poll object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  load($id)				// I - Object ID
  {
    $this->clear();

    $result = db_query("SELECT * FROM poll WHERE id = $id");
    if (db_count($result) != 1)
      return (FALSE);

    $row = db_next($result);
    $this->id = $row["id"];
    $this->is_published = $row["is_published"];
    $this->poll_type = $row["poll_type"];
    $this->question = $row["question"];
    $this->answer0 = $row["answer0"];
    $this->count0 = $row["count0"];
    $this->answer1 = $row["answer1"];
    $this->count1 = $row["count1"];
    $this->answer2 = $row["answer2"];
    $this->count2 = $row["count2"];
    $this->answer3 = $row["answer3"];
    $this->count3 = $row["count3"];
    $this->answer4 = $row["answer4"];
    $this->count4 = $row["count4"];
    $this->answer5 = $row["answer5"];
    $this->count5 = $row["count5"];
    $this->answer6 = $row["answer6"];
    $this->count6 = $row["count6"];
    $this->answer7 = $row["answer7"];
    $this->count7 = $row["count7"];
    $this->answer8 = $row["answer8"];
    $this->count8 = $row["count8"];
    $this->answer9 = $row["answer9"];
    $this->count9 = $row["count9"];
    $this->votes = $row["votes"];
    $this->create_date = $row["create_date"];
    $this->create_user = $row["create_user"];
    $this->modify_date = $row["modify_date"];
    $this->modify_user = $row["modify_user"];

    db_free($result);

    return ($this->validate());
  }


  //
  // 'poll::loadform()' - Load an Poll object from form data.
  //

  function				// O - TRUE if OK, FALSE otherwise
  loadform()
  {
    global $_GET, $_POST, $LOGIN_LEVEL;


    if ($LOGIN_LEVEL < AUTH_DEVEL)
      $this->is_published = 0;
    else if (array_key_exists("is_published", $_GET))
      $this->is_published = $_GET["is_published"];
    else if (array_key_exists("is_published", $_POST))
      $this->is_published = $_POST["is_published"];

    if (array_key_exists("poll_type", $_GET))
      $this->poll_type = $_GET["poll_type"];
    else if (array_key_exists("poll_type", $_POST))
      $this->poll_type = $_POST["poll_type"];

    if (array_key_exists("question", $_GET))
      $this->question = $_GET["question"];
    else if (array_key_exists("question", $_POST))
      $this->question = $_POST["question"];

    if (array_key_exists("answer0", $_GET))
      $this->answer0 = $_GET["answer0"];
    else if (array_key_exists("answer0", $_POST))
      $this->answer0 = $_POST["answer0"];

    if (array_key_exists("count0", $_GET))
      $this->count0 = $_GET["count0"];
    else if (array_key_exists("count0", $_POST))
      $this->count0 = $_POST["count0"];

    if (array_key_exists("answer1", $_GET))
      $this->answer1 = $_GET["answer1"];
    else if (array_key_exists("answer1", $_POST))
      $this->answer1 = $_POST["answer1"];

    if (array_key_exists("count1", $_GET))
      $this->count1 = $_GET["count1"];
    else if (array_key_exists("count1", $_POST))
      $this->count1 = $_POST["count1"];

    if (array_key_exists("answer2", $_GET))
      $this->answer2 = $_GET["answer2"];
    else if (array_key_exists("answer2", $_POST))
      $this->answer2 = $_POST["answer2"];

    if (array_key_exists("count2", $_GET))
      $this->count2 = $_GET["count2"];
    else if (array_key_exists("count2", $_POST))
      $this->count2 = $_POST["count2"];

    if (array_key_exists("answer3", $_GET))
      $this->answer3 = $_GET["answer3"];
    else if (array_key_exists("answer3", $_POST))
      $this->answer3 = $_POST["answer3"];

    if (array_key_exists("count3", $_GET))
      $this->count3 = $_GET["count3"];
    else if (array_key_exists("count3", $_POST))
      $this->count3 = $_POST["count3"];

    if (array_key_exists("answer4", $_GET))
      $this->answer4 = $_GET["answer4"];
    else if (array_key_exists("answer4", $_POST))
      $this->answer4 = $_POST["answer4"];

    if (array_key_exists("count4", $_GET))
      $this->count4 = $_GET["count4"];
    else if (array_key_exists("count4", $_POST))
      $this->count4 = $_POST["count4"];

    if (array_key_exists("answer5", $_GET))
      $this->answer5 = $_GET["answer5"];
    else if (array_key_exists("answer5", $_POST))
      $this->answer5 = $_POST["answer5"];

    if (array_key_exists("count5", $_GET))
      $this->count5 = $_GET["count5"];
    else if (array_key_exists("count5", $_POST))
      $this->count5 = $_POST["count5"];

    if (array_key_exists("answer6", $_GET))
      $this->answer6 = $_GET["answer6"];
    else if (array_key_exists("answer6", $_POST))
      $this->answer6 = $_POST["answer6"];

    if (array_key_exists("count6", $_GET))
      $this->count6 = $_GET["count6"];
    else if (array_key_exists("count6", $_POST))
      $this->count6 = $_POST["count6"];

    if (array_key_exists("answer7", $_GET))
      $this->answer7 = $_GET["answer7"];
    else if (array_key_exists("answer7", $_POST))
      $this->answer7 = $_POST["answer7"];

    if (array_key_exists("count7", $_GET))
      $this->count7 = $_GET["count7"];
    else if (array_key_exists("count7", $_POST))
      $this->count7 = $_POST["count7"];

    if (array_key_exists("answer8", $_GET))
      $this->answer8 = $_GET["answer8"];
    else if (array_key_exists("answer8", $_POST))
      $this->answer8 = $_POST["answer8"];

    if (array_key_exists("count8", $_GET))
      $this->count8 = $_GET["count8"];
    else if (array_key_exists("count8", $_POST))
      $this->count8 = $_POST["count8"];

    if (array_key_exists("answer9", $_GET))
      $this->answer9 = $_GET["answer9"];
    else if (array_key_exists("answer9", $_POST))
      $this->answer9 = $_POST["answer9"];

    if (array_key_exists("count9", $_GET))
      $this->count9 = $_GET["count9"];
    else if (array_key_exists("count9", $_POST))
      $this->count9 = $_POST["count9"];

    if (array_key_exists("votes", $_GET))
      $this->votes = $_GET["votes"];
    else if (array_key_exists("votes", $_POST))
      $this->votes = $_POST["votes"];

    return ($this->validate());
  }


  //
  // 'poll::save()' - Save an Poll object.
  //

  function				// O - TRUE if OK, FALSE otherwise
  save()
  {
    global $LOGIN_USER, $PHP_SELF;


    $this->modify_date = time();
    $this->modify_user = $LOGIN_USER;

    if ($this->id > 0)
    {
      return (db_query("UPDATE poll "
                      ." SET is_published = $this->is_published"
                      .", poll_type = $this->poll_type"
                      .", question = '" . db_escape($this->question) . "'"
                      .", answer0 = '" . db_escape($this->answer0) . "'"
                      .", count0 = $this->count0"
                      .", answer1 = '" . db_escape($this->answer1) . "'"
                      .", count1 = $this->count1"
                      .", answer2 = '" . db_escape($this->answer2) . "'"
                      .", count2 = $this->count2"
                      .", answer3 = '" . db_escape($this->answer3) . "'"
                      .", count3 = $this->count3"
                      .", answer4 = '" . db_escape($this->answer4) . "'"
                      .", count4 = $this->count4"
                      .", answer5 = '" . db_escape($this->answer5) . "'"
                      .", count5 = $this->count5"
                      .", answer6 = '" . db_escape($this->answer6) . "'"
                      .", count6 = $this->count6"
                      .", answer7 = '" . db_escape($this->answer7) . "'"
                      .", count7 = $this->count7"
                      .", answer8 = '" . db_escape($this->answer8) . "'"
                      .", count8 = $this->count8"
                      .", answer9 = '" . db_escape($this->answer9) . "'"
                      .", count9 = $this->count9"
                      .", votes = $this->votes"
                      .", modify_date = $this->modify_date"
                      .", modify_user = '" . db_escape($this->modify_user) . "'"
                      ." WHERE id = $this->id") !== FALSE);
    }
    else
    {
      $this->create_date = time();
      $this->create_user = $LOGIN_USER;

      if (db_query("INSERT INTO poll VALUES"
                  ."(NULL"
                  .", $this->is_published"
                  .", $this->poll_type"
                  .", '" . db_escape($this->question) . "'"
                  .", '" . db_escape($this->answer0) . "'"
                  .", $this->count0"
                  .", '" . db_escape($this->answer1) . "'"
                  .", $this->count1"
                  .", '" . db_escape($this->answer2) . "'"
                  .", $this->count2"
                  .", '" . db_escape($this->answer3) . "'"
                  .", $this->count3"
                  .", '" . db_escape($this->answer4) . "'"
                  .", $this->count4"
                  .", '" . db_escape($this->answer5) . "'"
                  .", $this->count5"
                  .", '" . db_escape($this->answer6) . "'"
                  .", $this->count6"
                  .", '" . db_escape($this->answer7) . "'"
                  .", $this->count7"
                  .", '" . db_escape($this->answer8) . "'"
                  .", $this->count8"
                  .", '" . db_escape($this->answer9) . "'"
                  .", $this->count9"
                  .", $this->votes"
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
  // 'poll::search()' - Get a list of Poll objects.
  //

  function				// O - Array of Poll objects
  search($search = "",			// I - Search string
         $order = "")			// I - Order fields
  {
    if ($search != "")
    {
      // Convert the search string to an array of words...
      $words = html_search_words($search);

      // Loop through the array of words, adding them to the query...
      $query  = " WHERE (";
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
        else
        {
          $query .= "$prefix$logic (";
          $subpre = "";

          if (ereg("^[0-9]+\$", $word))
          {
            $query .= "${subpre}id = $word";
            $subpre = " OR ";
          }

          $query .= "${subpre}question LIKE \"%$word%\"";
          $subpre = " OR ";
          $query .= "${subpre}answer0 LIKE \"%$word%\"";
          $query .= "${subpre}answer1 LIKE \"%$word%\"";
          $query .= "${subpre}answer2 LIKE \"%$word%\"";
          $query .= "${subpre}answer3 LIKE \"%$word%\"";
          $query .= "${subpre}answer4 LIKE \"%$word%\"";
          $query .= "${subpre}answer5 LIKE \"%$word%\"";
          $query .= "${subpre}answer6 LIKE \"%$word%\"";
          $query .= "${subpre}answer7 LIKE \"%$word%\"";
          $query .= "${subpre}answer8 LIKE \"%$word%\"";
          $query .= "${subpre}answer9 LIKE \"%$word%\"";

          $query .= ")";
          $prefix = $next;
          $logic  = '';
        }
      }

      $query .= ")";
    }
    else
      $query = "";

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

    // Do the query and convert the result to an array of objects...
    $result  = db_query("SELECT id FROM poll$query");
    $matches = array();

    while ($row = db_next($result))
      $matches[sizeof($matches)] = $row["id"];

    // Free the query result and return the array...
    db_free($result);

    return ($matches);
  }


  //
  // 'poll::validate()' - Validate the current Poll object values.
  //

  function				// O - TRUE if OK, FALSE otherwise
  validate()
  {
    $valid = TRUE;

    if ($this->question == "")
    {
      $this->question_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->question_valid = TRUE;

    if ($this->answer0 == "")
    {
      $this->answer0_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer0_valid = TRUE;

    if ($this->answer1 == "")
    {
      $this->answer1_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer1_valid = TRUE;

    if ($this->answer2 == "")
    {
      $this->answer2_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer2_valid = TRUE;

    if ($this->answer3 == "")
    {
      $this->answer3_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer3_valid = TRUE;

    if ($this->answer4 == "")
    {
      $this->answer4_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer4_valid = TRUE;

    if ($this->answer5 == "")
    {
      $this->answer5_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer5_valid = TRUE;

    if ($this->answer6 == "")
    {
      $this->answer6_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer6_valid = TRUE;

    if ($this->answer7 == "")
    {
      $this->answer7_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer7_valid = TRUE;

    if ($this->answer8 == "")
    {
      $this->answer8_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer8_valid = TRUE;

    if ($this->answer9 == "")
    {
      $this->answer9_valid = FALSE;
      $valid = FALSE;
    }
    else
      $this->answer9_valid = TRUE;

    return ($valid);
  }


  //
  // 'poll::view()' - View the current Poll object.
  //

  function
  view()
  {
    print("<p><table class='view'>\n");

    // is_published
    print("<tr><th align='right' valign='top' nowrap>Published:</th><td>");
    if ($this->is_published)
      print("Yes");
    else
      print("No");
    print("</td></tr>\n");

    // poll_type
    $html = htmlspecialchars($this->poll_type, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Poll Type:</th><td>");
    print($html);
    print("</td></tr>\n");

    // question
    $html = htmlspecialchars($this->question, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Question:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer0
    $html = htmlspecialchars($this->answer0, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer0:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count0
    $html = htmlspecialchars($this->count0, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count0:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer1
    $html = htmlspecialchars($this->answer1, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer1:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count1
    $html = htmlspecialchars($this->count1, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count1:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer2
    $html = htmlspecialchars($this->answer2, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer2:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count2
    $html = htmlspecialchars($this->count2, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count2:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer3
    $html = htmlspecialchars($this->answer3, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer3:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count3
    $html = htmlspecialchars($this->count3, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count3:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer4
    $html = htmlspecialchars($this->answer4, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer4:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count4
    $html = htmlspecialchars($this->count4, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count4:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer5
    $html = htmlspecialchars($this->answer5, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer5:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count5
    $html = htmlspecialchars($this->count5, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count5:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer6
    $html = htmlspecialchars($this->answer6, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer6:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count6
    $html = htmlspecialchars($this->count6, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count6:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer7
    $html = htmlspecialchars($this->answer7, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer7:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count7
    $html = htmlspecialchars($this->count7, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count7:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer8
    $html = htmlspecialchars($this->answer8, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer8:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count8
    $html = htmlspecialchars($this->count8, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count8:</th><td>");
    print($html);
    print("</td></tr>\n");

    // answer9
    $html = htmlspecialchars($this->answer9, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Answer9:</th><td>");
    print($html);
    print("</td></tr>\n");

    // count9
    $html = htmlspecialchars($this->count9, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Count9:</th><td>");
    print($html);
    print("</td></tr>\n");

    // votes
    $html = htmlspecialchars($this->votes, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Votes:</th><td>");
    print($html);
    print("</td></tr>\n");

    // create_date
    $html = date("H:i M d, Y", $this->create_date);
    print("<tr><th align='right' valign='top' nowrap>Create Date:</th><td>");
    print($html);
    print("</td></tr>\n");

    // create_user
    $html = htmlspecialchars($this->create_user, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Create User:</th><td>");
    print($html);
    print("</td></tr>\n");

    // modify_date
    $html = date("H:i M d, Y", $this->modify_date);
    print("<tr><th align='right' valign='top' nowrap>Modify Date:</th><td>");
    print($html);
    print("</td></tr>\n");

    // modify_user
    $html = htmlspecialchars($this->modify_user, ENT_QUOTES);
    print("<tr><th align='right' valign='top' nowrap>Modify User:</th><td>");
    print($html);
    print("</td></tr>\n");
    print("</table></p>\n");
  }
}


//
// End of "$Id: db-poll.php,v 1.5 2006/07/11 13:55:14 mike Exp $".
//
?>
