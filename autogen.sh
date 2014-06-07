#!/bin/sh -xue

git submodule init
git submodule update
git submodule foreach git checkout master
git submodule foreach git pull

if type -p colorgcc > /dev/null ; then
   export CC=colorgcc
fi

autoreconf --force --install --verbose

./configure -C $@
