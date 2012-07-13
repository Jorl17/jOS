#!/bin/bash
make clean && make
mkdir test
sudo mount floppy.img test
sudo cp kernel test/kernel
sudo umount test
