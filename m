#!/bin/sh
# this script wrote by Eddy 
#
#		cdmount
#
# ������17-�� ���� 2003 ���� � 20:14
#
[ "$1" = "" ] && mount /dev/hdc
[ "$1" != "" ] && mount $1
#cd /mnt/cdrom
#konqueror "/mnt/cdrom"