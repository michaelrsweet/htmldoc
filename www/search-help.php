<?php
//
// "$Id: search-help.php 3 2007-04-11 15:10:22Z mike $"
//
// Search help page...
//

include_once "phplib/doc.php";

doc_header("Search Help");

?>

<p>When searching the article, bug, and links pages, you can create complex
search queries using a few simple logical operators.</p>

<h2>Searching for One or More Words</h2>

<p>The default logical operator is OR - list each word you want to find
separated by spaces. For example, to find all bug reports about HTTP or
networking, use:</p>

<pre class="command">
http network
</pre>

<p>You can also require both words using the AND keyword. For example,
to find all bug reports about HTTP against HTMLDOC 1.8.x, use:</p>

<pre class="command">
http and 1.8
</pre>

<h2>Searching for a Complete Phrase</h2>

<p>Enclose phrases containing spaces in quotes. For example, to find
all articles about weekly snapshots, use:</p>

<pre class="command">
"weekly snapshot"
</pre>

<h2>Excluding Words and Phrases</h2>

<p>The NOT operator can be used to exclude a word or phrase. For example,
to find all articles that are not about weekly snapshots, use:</p>

<pre class="command">
not "weekly snapshot"
</pre>

<p>Similarly, to find all 1.8 bug reports that are not feature
requests, use:</p>

<pre class="command">
1.8 and not feature
</pre>

<h2>Grouping Multiple Queries</h2>

<p>You can group multiple queries using parenthesis. For example, to
find all 1.8 bug reports concerning Solaris or HP-UX, use:</p>

<pre class="command">
1.8 and (Solaris HP-UX)
</pre>

<h2>Searching Specific Fields</h2>

<p>You can search specific fields by prefixing your search words with the
corresponding field name. For example, to find all bug reports submitted by
user "jane", use:</p>

<pre class="command">
creator:jane
</pre>

<p>The following field names are supported:</p>

<ul>

	<li><tt>creator</tt> - The creator of the article, bug
	report, feature request, or link</li>

	<li><tt>developer</tt> - The developer assigned to the bug
	report or feature request</li>

	<li><tt>fixversion</tt> - The fix version of the bug report
	or feature request</li>

	<li><tt>number</tt> - The article, bug report, feature
	request, or link number</li>

	<li><tt>subsystem</tt> - The subsystem of the bug report or
	feature request</li>

	<li><tt>title</tt> - The title of the article, bug report,
	feature request, or link</li>

	<li><tt>version</tt> - The version of bug report, feature
	request, or link</li>

</ul>

<?

doc_footer();

//
// End of "$Id: search-help.php 3 2007-04-11 15:10:22Z mike $".
//
?>
