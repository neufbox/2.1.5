#!/bin/sh +x

#****************************************************************************
#
#  Copyright c 2001, 2002  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16251 Laguna Canyon Road
#          Irvine, California 92618
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
#****************************************************************************
echo TARGET_DIR = [$1] 

chmod u+s $1/bin/busybox

# Create FIFO devices
mknod $1/dev/initctl p

#Create character devices
mkdir -p $1/dev/pts
mknod $1/dev/mem c 1 1
mknod $1/dev/kmem c 1 2
mknod -m 666 $1/dev/null c 1 3
mknod $1/dev/port c 1 4
mknod $1/dev/zero c 1 5
mknod $1/dev/ptyp0 c 2 0
mknod $1/dev/ptyp1 c 2 1
mknod $1/dev/ptyp2 c 2 2
mknod $1/dev/ptyp3 c 2 3
mknod $1/dev/ptyp4 c 2 4
mknod $1/dev/ptyp5 c 2 5
mknod $1/dev/ptyp6 c 2 6
mknod $1/dev/ptyp7 c 2 7
mknod $1/dev/ttyp0 c 3 0
mknod $1/dev/ttyp1 c 3 1
mknod $1/dev/ttyp2 c 3 2
mknod $1/dev/ttyp3 c 3 3
mknod $1/dev/ttyp4 c 3 4
mknod $1/dev/ttyp5 c 3 5
mknod $1/dev/ttyp6 c 3 6
mknod $1/dev/ttyp7 c 3 7
mknod $1/dev/tty0 c 4 0
mknod $1/dev/tty1 c 4 1
mknod $1/dev/tty2 c 4 2
mknod $1/dev/tty3 c 4 3
mknod $1/dev/tty4 c 4 4
mknod $1/dev/tty5 c 4 5
mknod $1/dev/tty6 c 4 6
mknod $1/dev/tty7 c 4 7
mknod $1/dev/ttyS0 c 4 64
mknod $1/dev/ttyS1 c 4 65
mknod $1/dev/tty c 5 0
mknod $1/dev/console c 5 1
mknod $1/dev/ptmx c 5 2
mknod $1/dev/ppp c 108 0
mknod $1/dev/urandom c 1 9
mknod $1/dev/tun c 10 200
mkdir $1/dev/shm
ln -s /dev/tun $1/dev/tun0
mkdir $1/dev/net/
ln -s /dev/tun $1/dev/net/tun
ln -s /dev/tun $1/dev/net/tun0
mknod $1/dev/tun1 c 10 201
mknod $1/dev/tun2 c 10 202
mknod $1/dev/fuse c 10 229

# Create Broadcom specific devices
mknod -m 666 $1/dev/bcmatm0 c 205 0
mknod -m 666 $1/dev/brcmboard c 206 0
chown 0.10 $1/dev/brcmboard
mknod -m 640 $1/dev/bcmvdsl0 c 207 0
mknod -m 666 $1/dev/bcmadsl0 c 208 0
mknod -m 640 $1/dev/bcmendpoint0 c 209 0
mknod -m 640 $1/dev/bcmaal20 c 210 0
mknod -m 640 $1/dev/bcmles0 c 211 0
mknod -m 640 $1/dev/bcm c 212 0
mknod -m 640 $1/dev/ac97 c 222 0
mknod -m 640 $1/dev/slac c 223 0
mknod -m 640 $1/dev/bcmprof c 224 0
mknod -m 640 $1/dev/si3215 c 225 0
mknod -m 666 $1/dev/bcmatmb0 c 226 0
mknod $1/dev/p8021ag0 c 227 0
mknod -m 666 $1/dev/bcmxtmcfg0 c 228 0
chmod +s $1/usr/sbin/wlctl


# Create Efixo devices
ln -s /var/log/log $1/dev/log
mknod $1/dev/ring0  c 229 0
mknod $1/dev/ring1  c 229 1
mknod $1/dev/ring2  c 229 2
mknod $1/dev/ring3  c 229 3
mknod $1/dev/ring4  c 229 4
mknod $1/dev/ring5  c 229 5
mknod $1/dev/ring6  c 229 6
mknod $1/dev/ring7  c 229 7
mknod $1/dev/ring8  c 229 8
mknod $1/dev/ring9  c 229 9
mknod $1/dev/ring10 c 229 10
mknod $1/dev/ring11 c 229 11

mknod $1/dev/gpio c 232 0

mknod $1/dev/voiceSubSys0 c 210 0
mknod $1/dev/events   c 10 222
mknod -m 666 $1/dev/leds    c 10 132
mknod $1/dev/watchdog    c 10 130

# Create block devices
mknod $1/dev/ram0 b 1 0
mknod $1/dev/ram1 b 1 1
mknod $1/dev/ram2 b 1 2
mknod $1/dev/ram3 b 1 3
ln -s ram1 $1/dev/ram


mknod $1/dev/mtdblock0 b 31 0
mknod $1/dev/mtdblock1 b 31 1
mknod $1/dev/mtdblock2 b 31 2
mknod $1/dev/mtdblock3 b 31 3
mknod $1/dev/mtdblock4 b 31 4
mknod $1/dev/mtdblock5 b 31 5
mknod $1/dev/mtdblock6 b 31 6
mknod $1/dev/mtdblock7 b 31 7


# Create mtd devices
mknod $1/dev/mtd0   c 90 0
mknod $1/dev/mtdro0 c 90 1
mknod $1/dev/mtd1   c 90 2
mknod $1/dev/mtdro1 c 90 3
mknod $1/dev/mtd2   c 90 4
mknod $1/dev/mtdro2 c 90 5
mknod $1/dev/mtd3   c 90 6
mknod $1/dev/mtdro3 c 90 7
mknod $1/dev/mtd4   c 90 8
mknod $1/dev/mtdro4 c 90 9
mknod $1/dev/mtd5   c 90 10
mknod $1/dev/mtdro5 c 90 11
mknod $1/dev/mtd6   c 90 12
mknod $1/dev/mtdro6 c 90 13
mknod $1/dev/mtd7   c 90 14
mknod $1/dev/mtdro7 c 90 15

# Symbolic links to avoid errors
ln -sf /dev/mtd0 $1/dev/mtd-bootloader
ln -sf /dev/mtdblock0 $1/dev/mtdblock-bootloader
ln -sf /dev/mtd1 $1/dev/mtd-main
ln -sf /dev/mtdblock1 $1/dev/mtdblock-main
ln -sf /dev/mtd2 $1/dev/mtd-config
ln -sf /dev/mtdblock2 $1/dev/mtdblock-config
ln -sf /dev/mtd3 $1/dev/mtd-rescue
ln -sf /dev/mtdblock3 $1/dev/mtdblock-rescue
ln -sf /dev/mtd4 $1/dev/mtd-dsldriver
ln -sf /dev/mtdblock4 $1/dev/mtdblock-adsl
ln -sf /dev/mtd5 $1/dev/mtd-bootcfg
ln -sf /dev/mtdblock5 $1/dev/mtdblock-bootcfg
ln -sf /dev/mtd7 $1/dev/mtd-flash
ln -sf /dev/mtdblock7 $1/dev/mtdblock-flash


# Scsi device for USB

mknod $1/dev/sda -m 644 b 8 0
mknod $1/dev/sda1 -m 644 b 8 1
mknod $1/dev/sda2 -m 644 b 8 2
mknod $1/dev/sda3 -m 644 b 8 3
mknod $1/dev/sda4 -m 644 b 8 4
mknod $1/dev/sda5 -m 644 b 8 5
mknod $1/dev/sda6 -m 644 b 8 6
mknod $1/dev/sda7 -m 644 b 8 7
mknod $1/dev/sda8 -m 644 b 8 8
mknod $1/dev/sdb -m 644 b 8 16
mknod $1/dev/sdb1 -m 644 b 8 17
mknod $1/dev/sdb2 -m 644 b 8 18
mknod $1/dev/sdb3 -m 644 b 8 19
mknod $1/dev/sdb4 -m 644 b 8 20
mknod $1/dev/sdb5 -m 644 b 8 21
mknod $1/dev/sdb6 -m 644 b 8 22
mknod $1/dev/sdb7 -m 644 b 8 23
mknod $1/dev/sdb8 -m 644 b 8 24

# HID 
mkdir -p $1/dev/usb
mknod $1/dev/usb/hiddev0 c 180 96
mknod $1/dev/usb/hiddev1 c 180 97
mknod $1/dev/usb/hiddev2 c 180 98
mknod $1/dev/usb/hiddev3 c 180 99
mknod $1/dev/usb/hiddev4 c 180 100
mknod $1/dev/usb/hiddev5 c 180 101
mknod $1/dev/usb/hiddev6 c 180 102

# SERIAL

mknod $1/dev/ttyUSB0 c 188 0
mknod $1/dev/ttyUSB1 c 188 1
mknod $1/dev/ttyUSB2 c 188 2
mknod $1/dev/ttyUSB3 c 188 3
mknod $1/dev/ttyUSB4 c 188 4
mknod $1/dev/ttyUSB5 c 188 5
mknod $1/dev/ttyUSB6 c 188 6
mknod $1/dev/ttyUSB7 c 188 7
mknod $1/dev/ttyUSB8 c 188 8
mknod $1/dev/ttyUSB9 c 188 9
mknod $1/dev/ttyUSB10 c 188 10
mknod $1/dev/ttyUSB11 c 188 11

# PRINTER
mknod $1/dev/printer0 c 180 0
mkdir -p $1/dev/usb/
mknod $1/dev/usb/lp0 c 180 0
mknod $1/dev/usb/lp1 c 180 1
ln -sf $1/dev/usb/lp0 $1/dev/lp0
ln -sf $1/dev/usb/lp1 $1/dev/lp1

# SOUND
#mkdir -p $1/dev/snd
#mknod $1/dev/snd/controlC0 c 116 0
#mknod $1/dev/snd/hwC0D0 c 116 4
#mknod $1/dev/snd/hwC0D1 c 116 5
#mknod $1/dev/snd/hwC0D2 c 116 6
#mknod $1/dev/snd/hwC0D3 c 116 7
#mknod $1/dev/snd/midiC0D0 c 116 8
#mknod $1/dev/snd/midiC0D1 c 116 9
#mknod $1/dev/snd/midiC0D2 c 116 10
#mknod $1/dev/snd/midiC0D3 c 116 11
#mknod $1/dev/snd/midiC0D4 c 116 12
#mknod $1/dev/snd/midiC0D5 c 116 13
#mknod $1/dev/snd/midiC0D6 c 116 14
#mknod $1/dev/snd/midiC0D7 c 116 15
#mknod $1/dev/snd/pcmC0D0c c 116 24
#mknod $1/dev/snd/pcmC0D0p c 116 16
#mknod $1/dev/snd/pcmC0D1c c 116 25
#mknod $1/dev/snd/pcmC0D1p c 116 17
#mknod $1/dev/snd/pcmC0D2c c 116 26
#mknod $1/dev/snd/pcmC0D2p c 116 18
#mknod $1/dev/snd/pcmC0D3p c 116 19
#mknod $1/dev/snd/pcmC0D3c c 116 27
#mknod $1/dev/snd/pcmC0D4c c 116 28
#mknod $1/dev/snd/pcmC0D4p c 116 20
#mknod $1/dev/snd/pcmC0D5c c 116 29
#mknod $1/dev/snd/pcmC0D5p c 116 21
#mknod $1/dev/snd/pcmC0D6c c 116 30
#mknod $1/dev/snd/pcmC0D6p c 116 22
#mknod $1/dev/snd/pcmC0D7c c 116 31
#mknod $1/dev/snd/pcmC0D7p c 116 23
#mknod $1/dev/snd/seq c 116 1
#mknod $1/dev/snd/timer c 116 33
#mknod $1/dev/admmidi0 c 14 14
#mknod $1/dev/admmidi1 c 14 30
#mknod $1/dev/admmidi2 c 14 46
#mknod $1/dev/admmidi3 c 14 62
#mknod $1/dev/dsp c 14 3
#mknod $1/dev/dsp0 c 14 16
#mknod $1/dev/dsp1 c 14 32
#mknod $1/dev/adsp0 c 14 12
#mknod $1/dev/adsp1 c 14 28
#mknod $1/dev/adsp2 c 14 44
#mknod $1/dev/adsp3 c 14 60
#mknod $1/dev/amidi0 c 14 13
#mknod $1/dev/amidi1 c 14 29
#mknod $1/dev/amidi2 c 14 45
#mknod $1/dev/amidi3 c 14 61
#mknod $1/dev/amixer0 c 14 11
#mknod $1/dev/amixer1 c 14 27
#mknod $1/dev/amixer2 c 14 43
#mknod $1/dev/amixer3 c 14 59
#mknod $1/dev/dmfm0 c 14 10
#mknod $1/dev/dmfm1 c 14 26
#mknod $1/dev/dmfm2 c 14 42
#mknod $1/dev/dmfm3 c 14 58
#mknod $1/dev/dmmidi0 c 14 9
#mknod $1/dev/dmmidi1 c 14 25
#mknod $1/dev/dmmidi2 c 14 41
#mknod $1/dev/dmmidi3 c 14 57
#mknod $1/dev/mixer0 c 14 0
#mknod $1/dev/mixer1 c 14 16
#mknod $1/dev/mixer2 c 14 32
#mknod $1/dev/mixer3 c 14 48
#mknod $1/dev/music c 14 8
#mknod $1/dev/sequencer c 14 1
#mknod $1/dev/sequencer2 c 14 8
#ln -s /dev/adsp0 $1/dev/adsp
#ln -s /dev/amidi0 $1/dev/amidi
#ln -s /dev/midi0 $1/dev/midi
#ln -s /dev/music $1/dev/sequencer1
