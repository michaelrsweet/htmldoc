<<
# Scandoc template file.
#
# This is an example set of templates that is designed to create several 
# different kinds of index files. It generates a "master index" which intended 
# for use with a frames browser; A "package index" which is the root page of 
# the index, and then "package files" containing documentation for all of the 
# classes within a single package.

######################################################################

## For quick and superficial customization, 
## simply change these variables

$project_name     = 'HTMLDOC';
$copyright        = '1997-2002 by Easy Software Products';

######################################################################

## Begin generating master index file for classes.

file "c-classes.html";
>>
<HTML>
<HEAD>
	<STYLE><!--
	H1, H2, H3, H4, H5, H6 { font-family: sans-serif; }
	P.indent { margin-left: 3em; }
	--></STYLE>
</HEAD>
<BODY>
<H1 ALIGN="RIGHT">C - HTMLDOC Class Reference</H1>
<P>This appendix provides a complete reference to the classes and structures
provided in the HTMLDOC library.
<H2>Class List</H2>
<UL>
<<

## For each package, generate an index entry.

foreach $p (packages()) {
  foreach $e ($p->classes()) {
        $_ = "#" . $e->anchor();
        s/\s/-/g;
>>
	<LI><A HREF="$_">$(e.fullname)</A></LI>
<<
  }
}

>>
</UL>
<<

######################################################################

my $p;
foreach $p (packages()) {

## Generate detailed class documentation
foreach $c ($p->classes()) {
>>
<!-- NEW PAGE -->
<H2><A NAME="$(c.anchor)">$(c.fullname)</A></H2>
<HR>
<H3>Description</H3>
<P>$(c.description)</P>
<<

  # Output "see also" information
  if ($c->seealso()) {
>>
<H3>See Also</H3>
<P>
<<
    my @r = ();
    foreach $a ($c->seealso()) {
      my $name = $a->name();
      if ($url = $a->anchor()) {
        $_ = "#" . $url;
        s/\s/-/g;
        push @r, "<A HREF=\"$_\">$name</A>";
      }
      else { push @r, $name; }
    }
    print join( ', ', @r );
  }

  # Output base class list
  if ($c->baseclasses()) {
>>
<H3>Base Classes</H3>
<P>
<<
    my @t = ();
    foreach $b ($c->baseclasses()) {
      my $name = $b->name();
      if ($url = $b->anchor()) {
        $_ = "#" . $url;
        s/\s/-/g;
        push @t, "<A HREF=\"$_\">$name</A>";
      }
      else { push @t, $name; }
    }
    print join( ', ', @t );
  }	

  # Output subclasses list
  if ($c->subclasses()) {
>>
<H3>Subclasses</H3>
<P>
<<
    my @t = ();
    foreach $s ($c->subclasses()) {
      my $name = $s->name();
      if ($url = $s->anchor()) {
        $_ = "#" . $url;
        s/\s/-/g;
        push @t, "<A HREF=\"$_\">$name</A>";
      }
      else { push @t, $name; }
    }
    print join( ', ', @t );
  }

  # Output class member variable documentation
>>
<H3>Members</H3>
<<
  if ($c->membervars()) {
>>
<P CLASS="indent"><TABLE BORDER="1">
<TR><TH>Member</TH><TH>Data Type</TH><TH>Description</TH></TR>
<<
    foreach $m ($c->membervars()) { &variable( $m ); }
>>
</TABLE>
<<
  } else {
>>
<P>This class does not provide any public members.
<<
  }

  # Output class member function documentation
>>
<H3>Methods</H3>
<<
  if ($c->memberfuncs()) {
    foreach $m ($c->memberfuncs()) { &function( $m ); }
  } else {
>>
<P>This class does not provide any public methods.
<<
  }
}

# Output global variables
if ($p->globalvars()) {
>>
<H3>Global Variables:</H3>
<P CLASS="indent"><TABLE BORDER="1">
<TR><TH>Variable</TH><TH>Data Type</TH><TH>Description</TH></TR>
<<
  foreach $m ($p->globalvars()) { &variable( $m ); }
}
>>
</TABLE>
<<

# Output global functions
if ($p->globalfuncs()) {
>>
<H3>Global Functions:</H3>
<<
  foreach $m ($p->globalfuncs()) { &function( $m ); }
}

>>
</BODY>
</HTML>
<<
} # end of foreach (packages) loop

######################################################################

## Subroutine to generate documentation for a member function or global function

sub function {
  local ($f) = @_;
  
>>
<H4><A NAME="$(f.name)">$(f.fullname)</A></H4>
<P>$(f.description)
<<

  if ($f->params()) {
>>
<P><B>Parameters</B>
<P CLASS="indent"><TABLE BORDER="1">
<TR><TH>Name</TH><TH>Data Type</TH><TH>Description</TH></TR>
<<
    foreach $a ($f->params()) {
>>
<TR><TD VALIGN="TOP"><TT>$(a.name)</TT></TD>
<TD VALIGN="TOP"><TT>$(a.type)</TT></TD>
<TD VALIGN="TOP">$(a.description)</TD></TR>
<<
    }
>>
</TABLE>
<<
  }
	
  if ($f->returnValue()) {
>>
<P><B>Returns</B>
<P>$(f.returnValue)
<<
  }
  
  if ($f->seealso()) {
>>
<P><B>See Also</B>
<P>
<<
    my @r = ();
    foreach $a ($f->seealso()) {
      my $name = $a->name();
      if ($url = $a->anchor()) {
        $_ = "#" . $url;
        s/\s/-/g;
        push @r, "<A HREF=\"$_\">$name</A>";
      }
      else { push @r, $name; }
    }
    print join( ', ', @r );
  }
}

######################################################################

## Subroutine to generate documentation for a member variable or global variable.

sub variable {
  local ($v) = @_;
  $_ = $v->fullname();
  s/\[.*\]$//;
  $t = substr($v->fullname(), 0, length($_) - length($v->name())) .
       substr($v->fullname(), length($_));
>>
<TR><TD VALIGN="TOP"><A NAME="$(v.name)"><TT>$(v.name)</TT></A></TD>
<TD VALIGN="TOP"><TT>$t</TT></TD>
<TD VALIGN="TOP">$(v.description)</TD></TR>
<<

  if ($v->seealso()) {
>>
<P><B>See Also</B>
<P>
<<
    my @r = ();
    foreach $a ($v->seealso()) {
      my $name = $a->name();
      if ($url = $a->anchor()) {
        $_ = "#" . $url;
        s/\s/-/g;
        push @r, "<A HREF=\"$_\">$name</A>";
      }
      else { push @r, $name; }
    }
    print join( ', ', @r );
  }
}
