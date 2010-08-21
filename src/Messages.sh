#! /bin/sh
$EXTRACTRC *.rc >> rc.cpp
$XGETTEXT interpreter/*.cpp *.cpp -o $podir/kturtle.pot
