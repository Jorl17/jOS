#!/bin/bash
IMG=floppy.img
OPTIRUN=""
if [ ! -z `which optirun` ] ; then
    OPTIRUN=optirun
fi
sudo /sbin/losetup /dev/loop0 $IMG
sudo $OPTIRUN qemu -fda /dev/loop0 -boot a
sudo /sbin/losetup -d /dev/loop0 
