#!/bin/bash

## Creates a distribution tar

VER=r6438

DIST=/tmp/ldd-$VER
rm -rf $DIST
mkdir $DIST

svn export cudd-2.4.2 $DIST/cudd-2.4.2
svn export src $DIST/src
svn export doc $DIST/doc
svn export LICENSE $DIST/LICENSE
svn export Makefile $DIST/Makefile
svn export README $DIST/README
svn export configure $DIST/configure
svn export autoconf $DIST/autoconf
(cd /tmp ; tar cvzf ldd-$VER.tar.gz ldd-$VER)
rm -rf $DIST
echo "Created distribution: ldd-$VER.tar.gz"


