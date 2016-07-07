#!/bin/bash

rmmod jprobe && make clean && make && insmod jprobe.ko && dmesg | tail
