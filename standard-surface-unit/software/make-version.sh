#!/bin/sh
#
# Create a header file with a version number to be displayed
# at startup.

VERSION=$(git describe --tags 2> /dev/null)
[ -z "${VERSION}" ] && { echo "ERROR: Cannot determine version. Did you do a git-tag?"; exit 1; }

> version.h.tmp
echo "#define VERSION \"$VERSION\"" >> version.h.tmp

# Add a define with the program name. Use the directory name
# as the program name.
echo "#define PROGRAM_NAME \"$(basename "$PWD")\"" >> version.h.tmp

if [ ! -s version.h ]
then
    mv version.h.tmp version.h
else
    if cmp -s version.h.tmp version.h
    then
        # No changes
        rm -f version.h.tmp
    else
        # Use the new file
        mv version.h.tmp version.h
    fi
fi

if [ -f Release/makefile ]
then
    make -C Release all
fi

exit 0
