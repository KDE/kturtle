#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp || exit 11
$XGETTEXT interpreter/*.cpp *.cpp -o $podir/kturtle.pot
