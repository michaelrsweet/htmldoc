<?
//
// "$Id: common.php,v 1.20 2005/08/10 19:45:29 mike Exp $"
//
// Common utility functions for PHP pages...
//
// This file should be included using "include_once"...
//
// Contents:
//
//   abbreviate()          - Abbreviate long strings...
//   count_comments()      - Count visible comments for the given path...
//   quote_text()          - Quote a string...
//   sanitize_email()      - Convert an email address to something a SPAMbot
//                           can't read...
//   sanitize_text()       - Sanitize text.
//   show_comments()       - Show comments for the given path...
//   validate_email()      - Validate an email address...
//


//
// 'abbreviate()' - Abbreviate long strings...
//

function				// O - Abbreviated string
abbreviate($text,			// I - String
           $maxlen = 32)		// I - Maximum length of string
{
  $newtext   = "";
  $textlen   = strlen($text);
  $inelement = 0;

  for ($i = 0, $len = 0; $i < $textlen && $len < $maxlen; $i ++)
    switch ($text[$i])
    {
      case '<' :
          $inelement = 1;
	  break;

      case '>' :
          if ($inelement)
	    $inelement = 0;
	  else
	  {
	    $newtext .= "&gt;";
	    $len     ++;
	  }
	  break;

      case '&' :
          $len ++;

	  if (substr($text, $i, 5) == "&reg;")
	  {
	    $newtext .= "(r)";
	    $i += 4;
	  }
	  else
	  {
	    for ($j = $i; $j < $textlen; $j ++)
	      if ($text[$j] == ';' || $text[$j] == ' ')
	        break;

            if ($j < $textlen && $text[$j] == ';')
	    {
	      while ($i < $textlen)
	      {
		$newtext .= $text[$i];

		if ($text[$i] == ';')
		  break;

		$i ++;
	      }
	    }
	    else
	      $newtext .= "&amp;";
	  }
	  break;

      default :
          if (!$inelement)
	  {
	    $newtext .= $text[$i];
	    $len ++;
	  }
	  break;
    }
	    
  if ($i < $textlen)
    return ($newtext . "...");
  else
    return ($newtext);
}


//
// 'count_comments()' - Count visible comments for the given path...
//

function				// O - Number of comments
count_comments($url,			// I - URL for comment
               $parent_id = 0)		// I - Parent comment
{
  $result = db_query("SELECT * FROM comment WHERE "
                    ."url = '" . db_escape($url) ."' "
                    ."AND parent_id = $parent_id "
		    ."ORDER BY id");

  $num_comments = 0;

  while ($row = db_next($result))
  {
    if ($row["status"] > 0)
      $num_comments ++;

    $num_comments += count_comments($url, $row['id']);
  }

  db_free($result);

  return ($num_comments);
}


//
// 'quote_text()' - Quote a string...
//

function				// O - Quoted string
quote_text($text,			// I - Original string
           $quote = 0)			// I - Add ">" to front of message
{
  $len   = strlen($text);
  $col   = 0;
  $word  = 0;

  if ($quote)
    $qtext = "&gt; ";
  else
    $qtext = "";

  for ($i = 0; $i < $len; $i ++)
  {
    switch ($text[$i])
    {
      case '<' :
          $col ++;
	  $word ++;
          $qtext .= "&lt;";
	  break;

      case '>' :
          $col ++;
	  $word ++;
          $qtext .= "&gt;";
	  break;

      case '&' :
          $col ++;
	  $word ++;
          $qtext .= "&amp;";
	  break;

      case "\n" :
          if ($quote)
            $qtext .= "\n&gt; ";
	  else
            $qtext .= "<br />";

          $col = 0;
	  $word = 0;
	  break;

      case "\r" :
	  break;

      case "\t" :
          if ($col == 0)
	    $qtext .= "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	  else
            $qtext .= " ";

	  $word = 0;
	  break;

      case " " :
	  $word = 0;
          if ($col == 0 || (($i + 1) < $len && $text[$i + 1] == " "))
	    $qtext .= "&nbsp;";
	  else if ($col > 65 && $quote)
	  {
	    $qtext .= "\n&gt; ";
	    $col    = 0;
	  }
	  else
            $qtext .= " ";

          if ($col > 0)
	    $col ++;
	  break;

      case 'f' :
      case 'h' :
          if (substr($text, $i, 7) == "http://" ||
              substr($text, $i, 8) == "https://" ||
              substr($text, $i, 6) == "ftp://")
	  {
	    // Extract the URL and make this a link...
	    for ($j = $i; $j < $len; $j ++)
	      if (!preg_match("/[-+~a-zA-Z0-9%_\\/:@.?#=&]/", $text[$j]))
	        break;

	    if ($text[$j - 1] == '.')
	      $j --;

            $count = $j - $i;
            $url   = substr($text, $i, $count);
	    $qtext .= "<a href='$url'>$url</a>";
	    $col   += $count;
	    $word  += $count;
	    $i     = $j - 1;
	    break;
	  }

      default :
          $col ++;
	  $word ++;
          $qtext .= $text[$i];
	  if ($word >= 80)
	  {
            if ($quote)
              $qtext .= "\n&gt; ";
	    else
              $qtext .= "<br />";

            $col  = 0;
	    $word = 0;
	  }
	  break;
    }
  }

  return ($qtext);
}


//
// 'sanitize_email()' - Convert an email address to something a SPAMbot
//                      can't read...
//

function				// O - Sanitized email
sanitize_email($email,			// I - Email address
               $html = 1)		// I - HTML format?
{
  $nemail = "";
  $len    = strlen($email);

  for ($i = 0; $i < $len; $i ++)
  {
    switch ($email[$i])
    {
      case '@' :
          if ($i > 0)
	    $i = $len;
          else if ($html)
            $nemail .= " <I>at</I> ";
	  else
            $nemail .= " at ";
	  break;

      case '<' :
          if ($i > 0)
	    $i = $len;
          break;

      case '>' :
          break;

      case '&' ;
          $nemail .= "&amp;";
	  break;

      default :
          $nemail .= $email[$i];
	  break;
    }
  }

  return (trim($nemail));
}


//
// 'sanitize_text()' - Sanitize text.
//

function				// O - Sanitized text
sanitize_text($text)			// I - Original text
{
  $len   = strlen($text);
  $word  = "";
  $qtext = "";

  for ($i = 0; $i < $len; $i ++)
  {
    switch ($text[$i])
    {
      case "\n" :
          if (!strncmp($word, "http://", 7) ||
	      !strncmp($word, "https://", 8) ||
	      !strncmp($word, "ftp://", 6))
            $qtext .= "<a href='$word'>$word</a>";
          else if (strchr($word, '@'))
            $qtext .= sanitize_email($word);
	  else
            $qtext .= quote_text($word);

          $qtext .= "<br />";
	  $word  = "";
	  break;

      case "\r" :
	  break;

      case "\t" :
      case " " :
          if (!strncmp($word, "http://", 7) ||
	      !strncmp($word, "https://", 8) ||
	      !strncmp($word, "ftp://", 6))
            $qtext .= "<a href='$word'>$word</a>";
          else if (strchr($word, '@'))
            $qtext .= sanitize_email($word);
	  else
            $qtext .= quote_text($word);

          if ($word)
            $qtext .= " ";
	  else
            $qtext .= "&nbsp;";

	  $word  = "";
	  break;

      default :
          $word .= $text[$i];
	  break;
    }
  }

  if (!strncmp($word, "http://", 7) ||
      !strncmp($word, "https://", 8) ||
      !strncmp($word, "ftp://", 6))
    $qtext .= "<a href='$word'>$word</a>";
  else if (strchr($word, '@'))
    $qtext .= sanitize_email($word);
  else
    $qtext .= quote_text($word);

  return ($qtext);
}


//
// 'show_comments()' - Show comments for the given path...
//

function				// O - Number of comments
show_comments($url,			// I - URL for comment
              $path = "",		// I - Path component
              $parent_id = 0,		// I - Parent comment
	      $heading = 3)		// I - Heading level
{
  global $_COOKIE, $LOGIN_LEVEL, $LOGIN_USER, $PROJECT_MODULE;


  $result = db_query("SELECT * FROM comment WHERE "
                    ."url = '" . db_escape($url) ."' "
                    ."AND parent_id = $parent_id "
		    ."ORDER BY status DESC, create_date DESC");

  if ($LOGIN_USER == "")
    $modpoints = 0;
  else if (array_key_exists("${PROJECT_MODULE}MODPOINTS", $_COOKIE))
    $modpoints = $_COOKIE["${PROJECT_MODULE}MODPOINTS"];
  else
    $modpoints = 5;

  if ($parent_id == 0 && $modpoints > 0)
    print("<P>You have $modpoints moderation points available.</P>\n");
  
  if ($heading > 6)
    $heading = 6;

  $safeurl      = urlencode($url);
  $num_comments = 0;
  $div          = 0;

  while ($row = db_next($result))
  {
    if ($row["status"] > 0)
    {
      if ($heading > 3 && !$div)
      {
	print("<div style='margin-left: 3em;'>\n");
	$div = 1;
      }

      $num_comments ++;

      $create_date  = date("H:i M d, Y", $row['create_date']);
      $contents     = html_format($row['contents']);
      $display_name = htmlspecialchars($row['display_name']);

      print("<h$heading><a name='_USER_COMMENT_$row[id]'>From</a> "
           ."$display_name, $create_date (score=$row[status])</h$heading>\n"
	   ."$contents\n");

      html_start_links();

      if ($LOGIN_LEVEL >= AUTH_DEVEL || $row['create_user'] == $LOGIN_USER)
      {
        html_link("Edit", "${path}comment.php?U$row[id]+P$safeurl");
        html_link("Delete", "${path}comment.php?D$row[id]+P$safeurl");
      }

      html_link("Reply", "${path}comment.php?U+R$row[id]+P$safeurl");

      if ($modpoints > 0)
      {
	if ($row['status'] > 0)
          html_link("Moderate Down", "${path}comment.php?m$row[id]+P$safeurl");

	if ($row['status'] < 5)
          html_link("Moderate Up", "${path}comment.php?M$row[id]+P$safeurl");
      }

      html_end_links();
    }

    $num_comments += show_comments($url, $path, $row['id'], $heading + 1);
  }

  db_free($result);

  if ($div)
    print("</div>\n");

  return ($num_comments);
}


//
// 'validate_email()' - Validate an email address...
//

function				// O - TRUE if OK, FALSE otherwise
validate_email($email)			// I - Email address
{
  // Check for both "name@domain.com" and "Full Name <name@domain.com>"
  return (preg_match("/^[a-zA-Z0-9_\.+-]+@[a-zA-Z0-9\.-]+\.[a-zA-Z]{2,4}$/i",
                     $email) ||
          preg_match("/^[^<]*<[a-zA-Z0-9_\.+-]+@[a-zA-Z0-9\.-]+\."
	            ."[a-zA-Z]{2,4}>$/i", $email));
}


//
// End of "$Id: common.php,v 1.20 2005/08/10 19:45:29 mike Exp $".
//
?>
