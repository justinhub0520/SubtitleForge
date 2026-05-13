#!/bin/bash
set -e
echo "=== Downloading static FFmpeg with libass ==="
cd /tmp
wget -q https://johnvansickle.com/ffmpeg/releases/ffmpeg-release-amd64-static.tar.xz
tar xf ffmpeg-release-amd64-static.tar.xz
FFDIR=$(ls -d ffmpeg-*-amd64-static | head -1)
echo "Found: $FFDIR"

echo "=== Installing to /usr/local/bin ==="
sudo cp $FFDIR/ffmpeg $FFDIR/ffprobe /usr/local/bin/
sudo chmod +x /usr/local/bin/ffmpeg /usr/local/bin/ffprobe

echo "=== Verify ==="
/usr/local/bin/ffmpeg -version 2>&1 | head -2
/usr/local/bin/ffmpeg -filters 2>&1 | grep subtitles

echo "=== Test subtitles filter ==="
/usr/local/bin/ffprobe -version 2>&1 | head -1
echo "DONE"