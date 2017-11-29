#!/bin/sh
#
# Make a ZIP file for distribution / publishing.

#VERSION=$(git describe --abbrev=0 --tags 2> /dev/null)
VERSION=$(git describe --tags 2> /dev/null)
[ -z "${VERSION}" ] && { echo "ERROR: Cannot determine version. Did you do a git-tag?"; exit 1; }

FILES=$(git ls-files|grep -v .gitignore)
[ -z "${FILES}" ] && { echo "ERROR: No files to put in ZIP."; exit 1; }
if [ -x ./make-version.sh ]
then
    ./make-version.sh
    [ ! -f version.h ] && { echo "ERROR: Missing version.h after running make-version.sh"; exit 1; }
    FILES="$FILES version.h"
fi

DIRNAME="$(basename $(pwd))"
ZIPNAME="${DIRNAME}-${VERSION}"

# The temp directory to prepare the ZIP contents in.
TDIR=/tmp/$DIRNAME
[ -d "${TDIR}" ] && { echo "ERROR: Directory '${TDIR}' exists. Please remove."; exit 1; }

# Copy all the files into the temp directory
mkdir -p ${TDIR}
for f in $FILES
do
    echo $f
done |
cpio -pLmud ${TDIR}/

# Copy the binary files for convenience
if [ -d Release ]
then
    cp -f Release/*.hex ${TDIR}/ 2> /dev/null
    cp -f Release/*.bin ${TDIR}/ 2> /dev/null
    cp -f Release/*.elf ${TDIR}/ 2> /dev/null
    cp -f Release/*.map ${TDIR}/ 2> /dev/null
fi

echo "This ZIP file was created from this git commit: $VERSION" > ${TDIR}/README-ZIP

(cd /tmp && zip -r $ZIPNAME.zip $DIRNAME)
mv /tmp/$ZIPNAME.zip .
ln -sf -T $ZIPNAME.zip ${DIRNAME}.zip
rm -fr ${TDIR}
