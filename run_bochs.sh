#!/bin/bash
IMG=floppy.img
OPTIRUN=""
if [ -x "`which optirun`" ] ; then
    echo "FIXME: No Optirun"; #OPTIRUN=optirun
fi

sudo $OPTIRUN bochs -q "boot:floppy" "floppya: 1_44=$IMG, status=inserted"
