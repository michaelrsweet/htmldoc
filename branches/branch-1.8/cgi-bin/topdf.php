<?
//
// Sample "portal" script to convert the named HTML file to PDF on-the-fly.
//
// Usage: http://www.domain.com/path/topdf.php/path/filename.html
//

//
// 'topdf()' - Convert the named file/URL to PDF.
//

function topdf($filename, $options = "") {
    # Write the content type to the client...
    header("Content-Type: application/pdf");
    flush();

    # Run HTMLDOC to provide the PDF file to the user...
    passthru("htmldoc -t pdf --quiet --jpeg --webpage $options \'$filename\'");
}


//
// 'bad_url()' - See if the URL contains bad characters...
//

function bad_url($url) {
    // See if the URL starts with http: or https:...
    if (strncmp($url, "http://", 7) != 0 &&
	strncmp($url, "https://", 8) != 0) {
        return 1;
    }

    // Check for bad characters in the URL...
    $len = strlen($url);
    for ($i = 0; $i < $len; $i ++) {
        if (!strchr("~_*()/:%?+-&@;=,$.", $url[$i]) &&
	    !ctype_alnum($url[$i])) {
	    return 1;
	}
    }

    return 0;
}

//
// MAIN ENTRY - Pass the trailing path info in to HTMLDOC...
//

global $SERVER_NAME;
global $SERVER_PORT;
global $PATH_INFO;
global $QUERY_STRING;

if ($QUERY_STRING != "") {
    $url = "http://${SERVER_NAME}:${SERVER_PORT}${PATH_INFO}?${QUERY_STRING}";
} else {
    $url = "http://${SERVER_NAME}:${SERVER_PORT}$PATH_INFO";
}

if (bad_url($url)) {
  print("<HTML><HEAD><TITLE>Bad URL</TITLE></HEAD>\n"
       ."<BODY><H1>Bad URL</H1>\n",
       ."<P>The URL <B><TT>$url</TT></B> is bad.</P>\n"
       ."</BODY></HTML>\n");
} else {
  topdf($url);
}
?>
