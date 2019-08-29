#!/bin/sh
#
# Script to benchmark the run time of HTMLDOC with various test files...
#
# Usage:
#
#   ./benchmark.sh [path-to-htmldoc]
#

if test $# -gt 0; then
	htmldoc="$1"
else
	htmldoc="../htmldoc/htmldoc"
fi

HTMLDOC_DATA=".."; export HTMLDOC_DATA

for file in *.html; do
	echo "$file,`/usr/bin/time -p $htmldoc --quiet --webpage -f t.pdf $file 2>&1 | grep real | awk '{print $2}'`"
done

for file in *.md; do
	echo "$file,`/usr/bin/time -p $htmldoc --quiet --charset utf-8 --webpage -f t.pdf $file 2>&1 | grep real | awk '{print $2}'`"
done

echo "htmldoc.book,`/usr/bin/time -p $htmldoc --quiet --batch ../doc/htmldoc.book -f t.pdf 2>&1 | grep real | awk '{print $2}'`"
