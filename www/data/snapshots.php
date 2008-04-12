#!/usr/bin/php -f
<?php

include_once "../phplib/globals.php";
include_once "../phplib/db.php";

include "revisions.php";

$repobase = "http://svn.easysw.com/public/htmldoc/";

$repos = array(
  "htmldoc-1.9.x" => "trunk"
);

$snapshots = fopen("snapshots.md5", "w");

reset($repos);
while (list($version, $path) = each($repos))
{
  $url      = "$repobase$path";
  $prevrev  = $revisions[$version];
  $nextrev  = $prevrev + 1;
  $revision = 0;
  $basever  = substr($version, 8);

  // Export this branch...
  print("Exporting $version...\n");
  system("/bin/rm -rf /tmp/$version-r$revision");
  system("/bin/rm -rf /tmp/htmldoc");
  print("svn export $url /tmp/htmldoc\n");

  $fp = popen("svn export $url /tmp/htmldoc", "r");

  while ($line = fgets($fp, 1024))
  {
    if (substr($line, 0, 8) == "Exported")
      $revision = (int)substr($line, 18);
  }

  pclose($fp);

  $log = "";

  if ($revision >= $nextrev)
  {
    // Get the log messages...
    print("svn log -r$revision:$nextrev $url\n");

    $fp  = popen("svn log -r$revision:$nextrev $url", "r");

    while ($line = fgets($fp, 1024))
      $log .= $line;

    pclose($fp);
  }

  rename("/tmp/htmldoc", "/tmp/$version-r$revision");

  if ($log != "" &&
      $log != "------------------------------------------------------------------------\n")
  {
    // Create the archives...
    system("cd /tmp/$version-r$revision; autoconf");
    system("cd /tmp/$version-r$revision; rm -rf autom4te*.cache");
    system("cd /tmp/$version-r$revision; rm -rf standards");
    system("cd /tmp; tar czf /home/ftp.easysw.com/pub/htmldoc/snapshots/$version-r$revision.tar.gz $version-r$revision");
    print("Created $version-r$revision.tar.gz...\n");
    system("cd /tmp; tar cjf /home/ftp.easysw.com/pub/htmldoc/snapshots/$version-r$revision.tar.bz2 $version-r$revision");
    print("Created $version-r$revision.tar.bz2...\n");
    system("cd /tmp; zip -qr9 /home/ftp.easysw.com/pub/htmldoc/snapshots/$version-r$revision.zip $version-r$revision");
    print("Created $version-r$revision.tar.zip...\n");

    // Add a news announcement...
    $date     = time();
    $abstract = "A new developer snapshot of HTMLDOC $basever "
               ."(r$revision) is now available";
    $body     = "$abstract on the download page:\n\n"
               ."    http://www.htmldoc.org/software.php\n\n"
	       ."<b>This snapshot contains pre-release software and should not "
	       ."be used on production systems.</b>\n\n"
	       ."Commit Log:\n\n<pre>" . db_escape(wordwrap($log)) . "</pre>\n";

    db_query("INSERT INTO article VALUES(NULL, 2, 1, "
            ."'HTMLDOC $basever Developer Snapshot, r$revision', '$abstract.', 'News', "
	    ."'$body', $date, 'mike', $date, 'mike')");

    $revisions[$version] = $revision;
  }
  else
    $revision = $prevrev;

  // Save MD5 sums of the snapshots...
  $fp = popen("cd /home/ftp.easysw.com/pub/htmldoc/snapshots; md5sum $version-r$revision.*", "r");
  while ($line = fgets($fp, 1024))
  {
    $data = explode("  ", trim($line));
    fwrite($snapshots, "$data[0] $basever-r$revision htmldoc/snapshots/$data[1]\n");
  }
  pclose($fp);

  system("/bin/rm -rf /tmp/$version-r$revision");
}

fclose($snapshots);

// Save the new revisions...
$fp = fopen("revisions.php", "w");
fwrite($fp, "<?php\n"
           ."\$revisions = array(\n"
	   ."  \"htmldoc-1.9.x\" => \"" . $revisions["htmldoc-1.9.x"] . "\",\n"
	   .");\n"
	   ."?>\n");
fclose($fp);

?>
