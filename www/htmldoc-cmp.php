<?php

include_once "phplib/doc.php";

doc_header("Developer Guide");

?>

<p><i>Last Modified: December 24, 2011</i></p>

<p>The HTMLDOC developer guide defines how
releases are done, how bugs and feature requests are handled,
and how the HTMLDOC code is formatted and documented. This guide
is based upon the policies we have developed over the years for
our other software projects and has been tailored to
HTMLDOC.</p>

<p>This document is organized into the following sections:

<ul>

	<li><a href='#FILE_MANAGEMENT'>File Management</a></li>

	<li><a href='#BUG_FEATURE_REQUEST_PROCESSING'>Bug &amp;
	Feature Request Processing</a></li>

	<li><a href='#SOFTWARE_RELEASES'>Software
	Releases</a></li>

	<li><a href='#CODING_REQUIREMENTS'>Coding
	Requirements</a></li>

	<li><a href='#SECURE_PROGRAMMING'>Secure
	Programming</a></li>

</ul>


<h2><a name='FILE_MANAGEMENT'>File Management</a></h2>

<h3>Directory Structure</h3>

<p>Each source file shall be placed a sub-directory
corresponding to the software sub-system it belongs to ("doc",
"htmldoc", etc.) To remain compatible with older UNIX
filesystems, directory names shall not exceed 16 characters in
length.</p>

<h3>Source Files</h3>

<p>Source files shall be documented and formatted as described
in <a href='#CODING_REQUIREMENTS'>Coding Requirements</a>. To
remain compatible with older UNIX filesystems, source file names
shall not exceed 16 characters in length, including extension.</p>

<h3>Configuration Management</h3>

<p>Source files shall be placed under the control of the
Subversion ("SVN") software. Source files shall be "checked in"
with each change so that modifications can be tracked.</p>

<p>Documentation on the SVN software is available in the on-line
and hardcopy book, "<a
href='http://svnbook.red-bean.com/'>Version Control with
Subversion</a>".</p>

<p>The SVN repository URLs are:</p>

<pre>
    <a href='http://svn.easysw.com/public/htmldoc/'>http://svn.easysw.com/public/htmldoc/</a>

    <a href='https://svn.easysw.com/public/htmldoc/'>https://svn.easysw.com/public/htmldoc/</a>
</pre>

<p>HTMLDOC developers must use the <tt>https</tt> URL and
authenticate using their corresponding username and
password.</p>

<p>The SVN repository is organized as follows:</p>

<dl>

	<dt><var>/public/htmldoc/trunk</var></dt>

	<dd>The current version of HTMLDOC</dd>

	<dt><var>/public/htmldoc/branches/branch-<b>major.minor</b></var></dt>

	<dd>Branches of previous versions of HTMLDOC, used when
	generating patch releases</dd>

	<dt><var>/public/htmldoc/tags/release-<b>major.minor.patch</b></var></dt>

	<dd>Copies of each released version of HTMLDOC</dd>

</dl>


<h2><a name='BUG_FEATURE_REQUEST_PROCESSING'>Bug &amp; Feature
Request Processing</a></h2>

<p>A bug report shall be filed every time a user or vendor experiences a
problem with or wants a new feature in the HTMLDOC software. Trouble reports
are maintained in a database with one of the following states:</p>

<ol>

	<li>Bug is closed with complete resolution</li>

	<li>Bug is closed without resolution</li>

	<li>Bug is active</li>

	<li>Bug is pending</li>

	<li>Bug is new</li>
</ol>

<p>Trouble reports shall be processed using the following
steps:</p>

<ol>

	<li>Classification</li>

	<li>Identification</li>

	<li>Correction</li>

	<li>Notification</li>

</ol>

<h3>1. Classification</h3>

<p>When a trouble report is received it must be classified at
one of the following priority levels:</p>

<ol>

	<li>Request for enhancement, e.g. asking for a
	feature</li>

	<li>Low, e.g. a documentation error or undocumented
	side-effect</li>

	<li>Moderate, e.g. unable to convert a file or unable to
	compile the software</li>

	<li>High, e.g. key functionality not working</li>

	<li>Critical, e.g. software crashes with all inputs or
	contains a security vulnerability that leads to
	unauthorized remote access</li>

</ol>

<p>Level 4 and 5 trouble reports must be resolved in the next
software release. Level 1 to 3 trouble reports are scheduled for
resolution in a specific release at the discretion of the
release coordinator.</p>

<p>The scope of the problem must also be determined as:</p>

<ol>

	<li>Specific to a machine</li>

	<li>Specific to an operating system</li>

	<li>Applies to all machines and operating systems</li>

</ol>

<h3>2. Identification</h3>

<p>Once the level and scope of the trouble report is determined
the software sub-system(s) involved with the problem are
determined. This may involve additional communication with the
user or vendor to isolate the problem to a specific cause.</p>

<p>When the sub-system(s) involved have been identified, an
engineer will then determine the change(s) needed and estimate
the time required for the change(s).</p>

<h3>3. Correction</h3>

<p>Corrections are scheduled based upon the severity and
complexity of the problem. Once all changes have been made,
documented, and tested successfully a new software release
snapshot is generated. Additional tests are added as necessary
for proper testing of the changes.</p>

<h3>4. Notification</h3>

<p>The user or vendor is notified when the fix is available or
if the problem was caused by user error.</p>


<h2><a name='SOFTWARE_RELEASES'>Software Releases</a></h2>

<p>There are two types of software releases: feature releases
and patch releases. Feature releases implement new features as
defined in priority 1 (request for enhancement) bugs. Patch
releases implement bug fixes as defined in priority 2-5
bugs.</p>

<h3>Version Numbering</h3>

<p>HTMLDOC uses a three-part version number separated by periods
to represent the major, minor, and patch release numbers; patch
release number 0 denotes a feature release:</p>

<pre>
    MAJOR.MINOR.PATCH
    1.8.24 <var>(patch release)</var>
    1.9.0 <var>(minor feature release)</var>
    1.9.1 <var>(patch release)</var>
    1.10.0 <var>(minor feature release)</var>
    2.0.0 <var>(major feature release)</var>
</pre>

<p>Beta-test releases may be created before feature releases and
are indentified by appending the letter B followed by the build
number:</p>

<pre>
    MAJOR.MINOR.PATCHbBUILD
    1.9.0b1
    1.9.0b2
    1.10.0b1
</pre>

<p>Release candidates are created before feature releases and
are indentified by appending the letters RC followed by the
build number:</p>

<pre>
    MAJOR.MINOR.PATCHrcBUILD
    1.9.0rc1
    1.10.0rc1
    1.10.0rc2
</pre>

<p>Patch releases are only issued to correct priority 2-5 bugs.
Minor feature releases are created when a priority 1 bug
requires only minor changes to the software. Major feature
releases are created when a priority 1 bug requires a
redesign.</p>

<h3>Generation</h3>

<p>Software patch releases shall be generated for each
successfully completed priority 2-5 bug. All object and
executable files shall be deleted prior to performing a full
build to ensure that source files can be recompiled
successfully.</p>

<h3>Testing</h3>

<p>Software testing shall be conducted according to the HTMLDOC
Software Test Plan (TBD). Failed tests cause bugs to be
generated to correct the problems found.</p>

<h3>Releases</h3>

<p>When testing has been completed successfully a new
distribution image is created from the current SVN code
"snapshot". No release shall contain software that has not
passed the appropriate software tests. Patch releases are
distributed immediately upon successful completion of the
software tests.</p>

<p>Feature releases are distributed only after a period of
public testing. Public testing optionally begins with one or
more beta distributions followed by at least one release
candidate, distributed using the following basic schedule:</p>

<center><table border='1'>
<tr>
	<th>Week</th>
	<th>Version</th>
	<th>Description</th>
</tr>
<tr>
	<td>T-6 weeks</td>
	<td>1.9.0b1</td>
	<td>First beta distribution</td>
</tr>
<tr>
	<td>T-5 weeks</td>
	<td>1.9.0b2</td>
	<td>Second beta distribution</td>
</tr>
<tr>
	<td>T-4 weeks</td>
	<td>1.9.0b3</td>
	<td>Third beta distribution</td>
</tr>
<tr>
	<td>T-3 weeks</td>
	<td>1.9.0rc1</td>
	<td>First release candidate</td>
</tr>
<tr>
	<td>T-2 weeks</td>
	<td>1.9.0rc2</td>
	<td>Second release candidate</td>
</tr>
<tr>
	<td>T-0 weeks</td>
	<td>1.9.0</td>
	<td>Feature release</td>
</tr>
</table></center>

<p>A SVN copy to the <var>/tags</var> directory is generated for
every release and uses the version number, for example:</p>

<pre>
    <kbd>svn copy https://svn.easysw.com/public/htmldoc/branches/branch-1.8 \
        https://svn.easysw.com/public/htmldoc/tags/release-1.8.24 \
        -m "1.8.24 release by Developer Name" ENTER</kbd>

    <kbd>svn copy https://svn.easysw.com/public/htmldoc/trunk \
        https://svn.easysw.com/public/htmldoc/tags/release-1.9.0b1 \
        -m "1.9.0b1 release by Developer Name" ENTER</kbd>
</pre>

<h4>Beta Distributions</h4>

<p>Beta distributions are generated when substantial changes
have been made that may affect the reliability of the software.
Beta distributions may cause loss of data, functionality, or
services and are provided for testing by qualified
individuals.</p>

<p>Beta distributions are an OPTIONAL part of the release
process and are generated as deemed appropriate by the release
coordinator. Functional and design changes may be included in
subsequent beta releases until the first release candidate.</p>

<h4>Release Candidates</h4>

<p>Release candidates are generated at least two weeks prior to
a feature release. Release candidates are targeted for end-users
that wish to test new functionality or bug fixes prior to the
feature release. While release candidates are intended to be
substantially bug-free, they may still contain defects and/or
not compile on specific platforms. No functional or design
changes can be introduced in a release candidate.</p>

<p>At least one release candidate is REQUIRED prior to any
feature release. The distribution of a release candidate marks
the end of any functional improvements. Release candidates are
generated at weekly intervals until all level 4/5 trouble
reports are resolved. The last release candidate must be
available for at least two weeks before the production feature
release.</p>

<h4>Feature Releases</h4>

<p>Feature releases are generated after a successful release
candidate and represent a stable release of the software
suitable for all users.</p>

<h4>Patch Releases</h4>

<p>Patch releases are generated as needed to resolve bugs
against a feature release and represent the latest stable
release of the software suitable for all users. No functional or
design changes can be introduced in a patch release.</p>


<h2><a name='CODING_REQUIREMENTS'>Coding Requirements</a></h2>

<p>These coding requirements provide detailed information on
source file formatting and documentation content. These
guidelines shall be applied to all C and C++ source files
provided with HTMLDOC. Source code for other languages should
conform to these requirements as allowed by the language.</p>

<h3>Source Files</h3>

<h4>Naming</h4>

<p>All source files names shall be 16 characters or less in
length to ensure compatibility with older UNIX filesystems.
Source files containing functions shall have an extension of
".c" for ANSI C and ".cxx" for C++ source files. All other
"include" files shall have an extension of ".h".</p>

<h4>Documentation</h4>

<p>The top of each source file shall contain a header giving the
name of the file, the purpose or nature of the source file, the
copyright and licensing notice, and the functions contained in
the file.  The file name and revision information is provided by
the SVN "&#36;Id$" tag:</p>

<pre>
    /*
     * "&#36;Id$"
     *
     *   Description of file contents.
     *
     *   Copyright YYYY by First M Last.
     *
	 *   This program is free software.  Distribution and use rights are outlined in
	 *   the file "COPYING.txt".
     *
     * Contents:
     *
     *   function1() - Description 1.
     *   function2() - Description 2.
     *   function3() - Description 3.
     */
</pre>

<p>The bottom of each source file shall contain a trailer giving
the name of the file using the SVN "&#36;Id$" tag. The primary
purpose of this is to mark the end of a source file; if the
trailer is missing it is possible that code has been lost near
the end of the file:</p>

<pre>
    /*
     * End of "&#36;Id$".
     */
</pre>

<h3>Functions</h3>

<h4>Naming</h4>

<p>Functions with a global scope shall be capitalized ("DoThis",
"DoThat", "DoSomethingElse", etc.) The only exception to this
rule shall be the HTMLDOC interface library functions which may
begin with a prefix word in lowercase ("hdDoThis",
"hdDoThat", etc.)</p>

<p>Functions with a local scope shall be declared "static" and
be lowercase with underscores between words ("do_this",
"do_that", "do_something_else", etc.)</p>

<h4>Documentation</h4>

<p>Each function shall begin with a comment header describing
what the function does, the possible input limits (if any), and
the possible output values (if any), and any special information
needed:</p>

<pre>
    /*
     * 'do_this()' - Compute y = this(x).
     *
     * Notes: none.
     */

    static float     /* O - Inverse power value, 0.0 &lt;= y &lt;= 1.1 */
    do_this(float x) /* I - Power value (0.0 &lt;= x &lt;= 1.1) */
    {
      ...
      return (y);
    }
</pre>

<p>Return/output values are indicated using an "O" prefix, input
values are indicated using the "I" prefix, and values that are
both input and output use the "IO" prefix for the corresponding
in-line comment.</p>

<h3>Methods</h3>

<h4>Naming</h4>

<p>Methods shall be in lowercase with underscores between words
("do_this", "do_that", "do_something_else", etc.)</p>

<h4>Documentation</h4>

<p>Each method shall begin with a comment header describing what
the method does, the possible input limits (if any), and the
possible output values (if any), and any special information
needed:</p>

<pre>
    /*
     * 'class::do_this()' - Compute y = this(x).
     *
     * Notes: none.
     */

    float                   /* O - Inverse power value, 0.0 &lt;= y &lt;= 1.0 */
    class::do_this(float x) /* I - Power value (0.0 &lt;= x &lt;= 1.0) */
    {
      ...
      return (y);
    }
</pre>

<p>Return/output values are indicated using an "O" prefix, input
values are indicated using the "I" prefix, and values that are
both input and output use the "IO" prefix for the corresponding
in-line comment.</p>

<h3>Variables</h3>

<h4>Naming</h4>

<p>Variables with a global scope shall be capitalized, for
example "ThisVariable", "ThatVariable", "ThisStateVariable",
etc. The only exception to this rule shall be the HTMLDOC
interface library global variables which must begin with the
prefix "hd", for example "hdThisVariable", "hdThatVariable",
etc. Global variables shall be replaced by function arguments
whenever possible.</p>

<p>Variables with a local scope shall be lowercase with
underscores between words, for example "this_variable",
"that_variable", etc. Any local variables shared by functions
within a source file shall be declared "static".</p>

<h4>Documentation</h4>

<p>Each variable shall be declared on a separate line and shall
be immediately followed by a comment block describing the
variable:</p>

<pre>
    int this_variable;   /* The current state of this */
    int that_variable;   /* The current state of that */
</pre>

<h3>Types</h3>

<h4>Naming</h4>

<p>All type names shall be lowercase with underscores between
words, a prefix of "hd", and a suffix of "_t", for example
"hd_this_type_t", "hd_that_type_t", etc.</p>

<h4>Documentation</h4>

<p>Each type shall have a comment block immediately before the
typedef:</p>

<pre>
    /*
     * This type is for HTMLDOC foobar options.
     */

    typedef int hd_this_type_t;
</pre>

<h3>Structures</h3>

<h4>Naming</h4>

<p>All public structure names shall be capitalized with the
prefix "hd", for example "hdThisStruct", "hdThatStruct", etc.
All private structure names shall be lowercase with underscores
between words, a prefix of "hd", and a suffix of "_s", for
example "hd_this_struct_s", "hd_that_struct_s", etc.</p>

<h4>Documentation</h4>

<p>Each structure shall have a comment block immediately before
the struct and each member shall be documented in accordance
with the variable naming policy above:</p>

<pre>
    /*
     * This structure is for HTMLDOC foobar options.
     */

    struct hd_this_struct_s
    {
      int this_member;   /* Current state for this */
      int that_member;   /* Current state for that */
    };
</pre>

<h3>Classes</h3>

<h4>Naming</h4>

<p>All public class names shall be capitalized with the prefix
"hd", for example "hdThisClass", "hdThatClass", etc. All private
class names shall be lowercase with underscores between words, a
prefix of "hd", and a suffix of "_c", for example
"hd_this_struct_c", "hd_that_struct_c", etc.</p>

<h4>Documentation</h4>

<p>Each class shall have a comment block immediately before the
class and each member shall be documented in accordance with the
variable naming policy above:</p>

<pre>
    /*
     * This class is for HTMLDOC foobar options.
     */

    class hdThisClass
    {
      int this_member;   /* Current state for this */
      int that_member;   /* Current state for that */
    };
</pre>

<h3>Constants</h3>

<h4>Naming</h4>

<p>All constant names shall be uppercase with underscores
between words, for example "THIS_CONSTANT", "THAT_CONSTANT",
etc. Constants defined for the HTMLDOC interface library must
begin with an uppercase "HD" prefix, for example
"HD_THIS_CONSTANT", "HD_THAT_CONSTANT", etc.</p>

<p>Typed enumerations shall be used whenever possible to allow
for type checking by the compiler.</p>

<h4>Documentation</h4>

<p>Comment blocks shall immediately follow each constant:</p>

<pre>
    enum
    {
      HD_THIS_STYLE,   /* This style */
      HD_THAT_STYLE    /* That style */
    };
</pre>

<h3>Code</h3>

<h4>Documentation</h4>

<p>All source code shall utilize block comments within functions
to describe the operations being performed by a group of
statements:</p>

<pre>
    /*
     * Clear the state array before we begin...
     */

    for (i = (sizeof(array) / sizeof(sizeof(array[0])) - 1; i &gt;= 0; i --)
      array[i] = STATE_IDLE;

    /*
     * Wait for state changes...
     */

    do
    {
      for (i = (sizeof(array) / sizeof(sizeof(array[0])) - 1; i &gt;= 0; i --)
	if (array[i] != STATE_IDLE)
	  break;

      if (i &gt;= 0)
	sleep(1);
    } while (i &gt;= 0);
</pre>

<h4>Style</h4>

<h5>Indentation</h5>

<p>All code blocks enclosed by brackets shall begin with the
opening brace on a new line. The code then follows starting on a
new line after the brace and is indented two spaces. The closing
brace is then placed on a new line following the code at the
original indentation:</p>

<pre>
    {
      int i; /* Looping var */

     /*
      * Process foobar values from 0 to 999...
      */

      for (i = 0; i &lt; 1000; i ++)
      {
	do_this(i);
	do_that(i);
      }
    }
</pre>

<p>Single-line statements following "do", "else", "for", "if",
and "while" shall be indented two spaces as well. Blocks of code
in a "switch" block shall be indented four spaces after each
"case" and "default" case:

<pre>
    switch (array[i])
    {
      case STATE_IDLE :
	  do_this(i);
	  do_that(i);
	  break;
      default :
	  do_nothing(i);
	  break;
    }
</pre>

<h5>Spacing</h5>

<p>A space shall follow each reserved word ("if", "while", etc.)
Spaces shall not be inserted between a function name and the
arguments in parenthesis.</p>

<h5>Return Values</h5>

<p>Parenthesis shall surround values returned from a function
using "return":</p>

<pre>
    return (STATE_IDLE);
</pre>

<h5>Loops</h5>

<p>Whenever convenient loops should count downward to zero to
improve program performance:</p>

<pre>
    for (i = sizeof(array) / sizeof(array[0]) - 1; i >= 0; i --)
      array[i] = STATE_IDLE;
</pre>


<h2><a name='SECURE_PROGRAMMING'>Secure Programming</a></h2>

<p>Secure programming practices shall be used at all times. The
following basic guidelines shall be followed:</p>

<ol>

	<li><b>Write test code for all interfaces</b> so that
	you can automate testing of the software and find errors
	sooner.</li>

	<li><b>Use debugging tools</b> such as <a
	href='http://www.valgrind.org'
	target='_new'>Valgrind</a> to find problems in your
	code.</li>

	<li><b>Validate all input</b>, including function
	parameters, data from files and sockets, and global
	variables. Most security errors are caused by a lack of
	input validation.</li>

	<li><b>Test with invalid input</b> whenever possible to
	ensure that invalid input is handled properly and
	consistently.</li>

	<li><b>Define the behavior all interfaces</b> so there
	is no undefined behavior or output for arbitrary, even
	erroneous, input. Programs and interfaces which have
	undefined behavior are broken and must be corrected.</li>

	<li><b>Define interfaces</b> for common tasks to isolate
	complexity, reduce code size, and promote
	testability.</li>

	<li><b>Practice safe string manipulation</b> using
	length-limited functions such as <tt>strlcat</tt>,
	<tt>strlcpy</tt>, and <tt>snprintf</tt>. Avoid using
	<tt>strcat</tt>, <tt>strcpy</tt>, <tt>strncat</tt>, and
	<tt>strncpy</tt>, and specify maximum string sizes in
	<tt>scanf</tt> formats.</li>

	<li><b>Never use unsafe functions</b> such as
	<tt>gets()</tt>.</li>

	<li><b>Allocate variable-size buffers</b> instead of
	using fixed-size buffers whenever appropriate.
	Considerable memory savings and buffer overflow safety
	can be realized by allocating variable-length arrays and
	strings instead of storing them in large, fixed-size
	arrays.</li>

	<li><b>Prefer reentrant functions over non-reentrant
	functions</b> to ensure thread-safety and consistent
	behavior.</li>

	<li><b>Initialize freed pointers to <tt>NULL</tt></b> to
	avoid references to freed memory.</li>

	<li><b>Use <tt>memmove</tt> instead of <tt>memcpy</tt></b>
	when moving overlapping areas of memory.</li>

	<li><b>Never use <tt>strcpy</tt></b> to remove
	characters from a string, as overlapping copies have
	undefined behavior.</li>

</ol>

<p>The following secure programming sites offer valuable
information as well:</p>

<ul>

	<li><a href='http://www.dwheeler.com/secure-programs/'
	target='_new'>Secure Programming for Linux and Unix
	HOWTO</a>; an interesting and in-depth book (free)</li>

	<li><a href='http://www.secureprogramming.com/'
	target='_new'>SecureProgramming.com</a>; An on-line
	resource for books and articles from O'Reilly
	Publishing</li>

</ul>

<?php

doc_footer();

?>
