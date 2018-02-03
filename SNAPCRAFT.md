# Building HTMLDOC Snaps

Support for building HTMLDOC as a "snap" package can be found in the "snap"
directory.  In fact, every push to the Github repository triggers builds through
the Snapcraft buildbot, and the resulting snaps can be installed using the
"--edge" option, e.g.:

    sudo snap install --edge htmldoc
