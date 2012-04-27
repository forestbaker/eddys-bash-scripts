#!/bin/sh
# this script has been written by Eddy 
#
#		l -- latex && xdvi
#
# Создан 27-го Сентябрь 2004 года в 22:22
#
KEY=""
RM=""
if [ "$1" = "-nd" ]; then
    shift
    KEY="n"
fi
if [ "$1" = "-nr" ]; then
    shift
    RM="n"
fi

name=$*

extn=`ext "$name"`

[ "$extn" = "tex" ] && name=`echo "$name" | sed "s/\.tex//"`

latex "$name"
latex "$name"
latex "$name" 
[ "$KEY" != "n" ] && xdvi "$name"
[ "$RM" != "n" ] && rm $name.aux $name.log $name.lof $name.lot $name.toc