#!/bin/sh
# this script wrote by Eddy 
#
#		cdmount
#
# Создан17-го Июля 2003 года в 20:14
#
[ "$1" = "" ] && mount /dev/hdc
[ "$1" != "" ] && mount $1
#cd /mnt/cdrom
#konqueror "/mnt/cdrom"