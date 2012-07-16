#!/bin/bash
IMG=floppy.img
OPTIRUN=""
if [ ! -z `which optirun` ] ; then
    OPTIRUN=optirun
fi

sudo $OPTIRUN bochs -q "boot:floppy" "floppya: 1_44=$IMG, status=inserted"
