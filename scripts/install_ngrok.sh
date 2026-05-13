#!/bin/bash
if which ngrok > /dev/null 2>&1; then
  echo "ngrok already installed: $(which ngrok)"
  exit 0
fi
cd /tmp
wget -q https://bin.equinox.io/c/bNyj1mQVY4c/ngrok-v3-stable-linux-amd64.tgz
tar xzf ngrok-v3-stable-linux-amd64.tgz
sudo mv ngrok /usr/local/bin/
chmod +x /usr/local/bin/ngrok
echo "ngrok installed: $(which ngrok)"
ngrok version