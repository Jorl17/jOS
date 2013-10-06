#!/bin/bash
IMG=floppy.img
OPTIRUN=""
QEMU="qemu"
if [ ! -x "`which $QEMU`" ] ; then
        QEMU="qemu-system-x86_64"
fi        
if [ -x "`which optirun`" ] ; then
    echo "FIXME: No Optirun"; #OPTIRUN=optirun
fi
sudo /sbin/losetup /dev/loop0 $IMG
sudo $OPTIRUN $QEMU -fda /dev/loop0 -boot a
sudo /sbin/losetup -d /dev/loop0 
