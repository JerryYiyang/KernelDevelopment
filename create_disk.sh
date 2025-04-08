#!/bin/bash

IMAGE_NAME="kernel_disk.img"
MOUNT_POINT="/mnt/osfiles"

dd if=/dev/zero of=$IMAGE_NAME bs=512 count=32768
parted $IMAGE_NAME mklabel msdos
parted $IMAGE_NAME mkpart primary ext2 2048s 30720s
parted $IMAGE_NAME set 1 boot on
LOOP_DEV=$(sudo losetup -f)
LOOP_PART=$(sudo losetup -f)
sudo losetup $LOOP_DEV $IMAGE_NAME
sudo losetup $LOOP_PART $IMAGE_NAME -o 1048576
sudo mkfs.ext2 $LOOP_PART
if [ ! -d "$MOUNT_POINT" ]; then
    echo "Creating mount point..."
    sudo mkdir -p $MOUNT_POINT
fi
sudo mount $LOOP_PART $MOUNT_POINT
sudo grub-install --root-directory=$MOUNT_POINT --no-floppy --modules="normal part_msdos ext2 multiboot" $LOOP_DEV
sudo cp -r build/kernel-x86_64.bin $MOUNT_POINT/boot/kernel.bin
sudo mkdir -p $MOUNT_POINT/boot/grub
sudo cp isofiles/boot/grub/grub.cfg $MOUNT_POINT/boot/grub/
sudo umount $MOUNT_POINT
sudo losetup -d $LOOP_DEV
sudo losetup -d $LOOP_PART