<?
//
// Sample "portal" script to convert the named HTML file to PDF on-the-fly.
//
// Usage: http://www.domain.com/path/topdf.php/path/filename.html
//

function topdf($filename, $options = "") {
    # Write the content type to the client...
    header("Content-Type: application/pdf");
    flush();

    # Run HTMLDOC to provide the PDF file to the user...
    passthru("htmldoc -t pdf --quiet --jpeg --webpage $options $filename");
}

global $SERVER_NAME;
global $SERVER_PORT;
global $PATH_INFO;

topdf("http://${SERVER_NAME}:${SERVER_PORT}$PATH_INFO");

?>
