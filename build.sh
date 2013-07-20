#!/bin/bash
MOUNTPOINT=mountpoint
KERNEL=kernel
IMG=floppy.img
make
if [ ! -e $MOUNTPOINT ] ; then
    mkdir $MOUNTPOINT
fi
sudo mount -o loop $IMG $MOUNTPOINT
sudo cp $KERNEL $MOUNTPOINT/$KERNEL
sleep 2s
sudo umount $MOUNTPOINT
