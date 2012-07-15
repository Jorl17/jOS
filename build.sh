#!/bin/bash
MOUNTPOINT=mountpint
KERNEL=kernel
IMG=floppy.img
make
if [ ! -e $MOUNTPOINT ] ; then
    mkdir $MOUNTPOINT
fi
sudo mount $IMG $MOUNTPOINT
sudo cp $KERNEL $MOUNTPOINT/$KERNEL
sudo umount $MOUNTPOINT
