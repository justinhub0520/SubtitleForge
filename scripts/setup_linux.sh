#!/bin/bash
set -e

echo "Installing SubForge dependencies..."

sudo apt update

sudo apt install -y ffmpeg
sudo apt install -y libssl-dev
sudo apt install -y zlib1g-dev
sudo apt install -y cmake
sudo apt install -y build-essential

sudo mkdir -p /var/lib/subforge/uploads
sudo mkdir -p /var/log/subforge
sudo mkdir -p /tmp/subforge

sudo chmod 777 /var/lib/subforge/uploads
sudo chmod 777 /var/log/subforge
sudo chmod 777 /tmp/subforge

echo "Dependencies installed successfully."
echo "Copy .env.example to .env and set your WHISPER_API_KEY"