#!/bin/bash
MOUNTPOINT=mountpoint
KERNEL=kernel
IMG=floppy.img

if [ "`uname`" == "Darwin" ] ; then
	expect -f copy_to_vm.expect >/dev/null
	expect -f make_in_vm.expect
	expect -f copy_from_vm.expect >/dev/null
else
	make -f makefile.real 

    if [ ! -e $MOUNTPOINT ] ; then
        mkdir $MOUNTPOINT
    fi
    sudo mount -o loop -t auto $IMG $MOUNTPOINT
    sudo cp $KERNEL $MOUNTPOINT/$KERNEL
    sleep 2s
    sudo umount $MOUNTPOINT
fi;
