#!/bin/sh
#
# Sample CGI shell script to convert the named HTML file to PDF on-the-fly.
#
# Usage: http://www.domain.com/path/topdf.cgi?/path/filename.html
#
# The filename on the command-line is relative to the document root of the
# server.  This doesn't prevent malicious users from accessing files they
# might not otherwise be able to access!
#

docroot=/insert/directory/here
cd $docroot

echo "Content-Type: application/pdf"
echo ""

htmldoc -t pdf --webpage $1
