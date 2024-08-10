#!/bin/bash

MACHINE_TYPE="virt"
ACCELERATOR="hvf"
CPU_TYPE="host"
NUM_CPUS=4
MEMORY_SIZE=3000
BIOS_PATH="vm/QEMU_EFI.fd"
DRIVE_FILE="vm/ubuntu-arm64.raw"
DRIVE_FORMAT="raw"
DRIVE_INTERFACE="virtio"
DRIVE_CACHE="writethrough"
NET_ID="net0"
HOST_FWD_PORT="2222"
GUEST_PORT="22"
CDROM_FLAG=false
CDROM_PATH="vm/ubuntu-24.04-live-server-arm64.iso"

qemu-system-aarch64 \
    -M $MACHINE_TYPE \
    -accel $ACCELERATOR \
    -cpu $CPU_TYPE \
    -smp $NUM_CPUS \
    -m $MEMORY_SIZE \
    -bios $BIOS_PATH \
    -nographic \
    -drive file=$DRIVE_FILE,format=$DRIVE_FORMAT,if=$DRIVE_INTERFACE,cache=$DRIVE_CACHE \
    -netdev user,id=$NET_ID,hostfwd=tcp::$HOST_FWD_PORT-:$GUEST_PORT \
    -device virtio-net-pci,netdev=$NET_ID \
    $(if $CDROM_FLAG; then echo "-cdrom $CDROM_PATH"; fi)
