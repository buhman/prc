#!/bin/sh -xue

git submodule init
git submodule update
git submodule foreach git checkout master

if type -p colorgcc > /dev/null ; then
   export CC=colorgcc
fi

autoreconf --force --install --verbose

./configure $@
