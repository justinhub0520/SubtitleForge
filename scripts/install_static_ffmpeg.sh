#!/bin/bash
set -e
cd /tmp
echo "Extracting..."
tar xf ffmpeg-release-amd64-static.tar.xz
FDIR=$(ls -d ffmpeg-*-amd64-static | head -1)
echo "Found: $FDIR"
echo "Installing..."
echo 123456 | sudo -S cp "$FDIR/ffmpeg" /usr/local/bin/
echo 123456 | sudo -S cp "$FDIR/ffprobe" /usr/local/bin/
sudo chmod +x /usr/local/bin/ffmpeg /usr/local/bin/ffprobe
echo "=== Version ==="
/usr/local/bin/ffmpeg -version 2>&1 | head -1
echo "=== Subtitles filter ==="
/usr/local/bin/ffmpeg -filters 2>&1 | grep subtitles || echo "subtitles filter NOT FOUND"
echo "DONE"