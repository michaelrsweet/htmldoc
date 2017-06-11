# Building HTMLDOC Snaps

Experimental support for building HTMLDOC as a "snap" package can be found in
the "snap" directory.  In fact, every push to the Github repository triggers
builds through the Snapcraft buildbot, and the resulting snaps can be installed
using the "--devmode" option, e.g.:

    sudo snap install --devmode --edge htmldoc19

That said, there is a known bug affecting non-GTK+/Qt applications that prevents
the HTMLDOC GUI from working when installed as a snap.  I'm not sure when this
issue will be resolved, so if you want to use the HTMLDOC GUI, install from
source or a traditional distro package.

Also, you'll notice that the snap name is "htmldoc19" - apparently Canonical has
reserved the name "htmldoc" (requests to free the name have gone unanswered) so
to run the installed snap you'll need to use "htmldoc19.htmldoc".

Because of these two issues, support for HTMLDOC as a "snap" package is firmly
experimental...
