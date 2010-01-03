#!/usr/bin/php -f
<?php

include_once "../phplib/globals.php";
include_once "../phplib/db.php";

$oldtime = time() - 30 * 86400;
$result  = db_query("SELECT id FROM user WHERE is_published = 0 AND "
                   ."modify_date < $oldtime");
$count   = db_count($result);
if ($count > 0)
{
  db_query("DELETE FROM user WHERE is_published=0 AND modify_date < $oldtime");
  print("Cleaned out $count disabled htmldoc.org user accounts.\n");
}

?>
