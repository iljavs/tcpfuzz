#!/usr/bin/bash

sudo brctl addbr br0

sudo ip tuntap add tap0 mode tap
sudo ip link set tap0 up
sudo brctl addif br0 tap0
sudo ip link set br0 up

# change tap0 and br0 IP if needed.
sudo ip addr 192.168.11.133/24 dev tap0
sudo ip addr 192.168.11.2/24 dev br0

sudo sysctl -w net.ipv4.ip_forward=1

# ens33 is the NIC I get in VMWare workstation. 
# Might need to change this.
sudo brctl addif br0 ens33

# NetworkManager keeps removing ens33 from the bridge.
# Tell NetworkManager to STFU.
sudo systemctl stop NetworkManager
