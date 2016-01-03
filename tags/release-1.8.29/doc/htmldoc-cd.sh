#!/bin/sh
#
# Script to make a PDF file for printing the HTMLDOC CD booklet.
#
# Requires our internal "makebook" program which adds things like crop
# marks, and Ghostscript which converts the whole shebang to PDF...
#

../htmldoc/htmldoc --datadir .. --verbose --batch htmldoc-cd.book -f htmldoc-cd.ps

makebook -c -i 4.75x7in -o letter htmldoc-cd.ps htmldoc-cd2.ps
ps2pdf13 htmldoc-cd2.ps htmldoc-cd.pdf
