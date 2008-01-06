#!/bin/sh
#
# Make all class files...
#

#./makeclass.php article "Article" > ../phplib/db-article.php
./makeclass.php articlefile "Article File" > ../phplib/db-articlefile.php
./makeclass.php carboncopy "Carbon Copy" > ../phplib/db-carboncopy.php
./makeclass.php comment "Comment" > ../phplib/db-comment.php
./makeclass.php link "Link" > ../phplib/db-link.php
./makeclass.php poll "Poll" > ../phplib/db-poll.php
#./makeclass.php str "STR" > ../phplib/db-str.php
./makeclass.php strfile "STR File" > ../phplib/db-strfile.php
./makeclass.php strtext "STR Text" > ../phplib/db-strtext.php
./makeclass.php user "User" > ../phplib/db-user.php
