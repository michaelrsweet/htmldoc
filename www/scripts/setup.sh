#!/bin/sh
#
# "$Id$"
#
# Setup the web directories and permissions for the local machine.
#
# Run this script from the document root, e.g.:
#
#     sudo scripts/setup.sh webuser webgroup
#

if test $# != 3 -o ! -d scripts; then
	echo Usage: scripts/setup.sh webuser webgroup
	exit 1
fi

chmod 755 strfiles articlefiles
chmod 644 data/authfile
chown $1:$2 strfiles articlefiles data/authfile

#
# End of "$Id$".
#
