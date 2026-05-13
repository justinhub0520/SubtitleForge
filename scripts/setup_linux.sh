#!/bin/bash
set -e

echo "Installing SubForge dependencies on Ubuntu 20.04..."

sudo apt update

sudo apt install -y build-essential cmake
sudo apt install -y ffmpeg
sudo apt install -y libssl-dev zlib1g-dev

sudo mkdir -p /var/lib/subforge/uploads
sudo mkdir -p /var/log/subforge
sudo mkdir -p /tmp/subforge

sudo chmod 777 /var/lib/subforge/uploads
sudo chmod 777 /var/log/subforge
sudo chmod 777 /tmp/subforge

echo "Dependencies installed successfully."
echo "VMware Ubuntu 20.04 IP: 192.168.199.132"
echo "Copy .env.example to .env and set your WHISPER_API_KEY"
echo "Then run: ./scripts/build_backend.sh"