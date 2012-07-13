#!/bin/bash
sudo /sbin/losetup /dev/loop0 floppy.img
sudo qemu -fda /dev/loop0 -boot a
sudo /sbin/losetup -d /dev/loop0 
