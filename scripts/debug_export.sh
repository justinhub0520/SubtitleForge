#!/bin/bash
cd /home/project01/SubForge
UPLOAD_DIR=/var/lib/subforge/uploads
TEMP_DIR=/tmp/subforge

echo "=== Uploaded videos ==="
ls -la $UPLOAD_DIR/

VIDEO_ID=$(ls $UPLOAD_DIR/ | head -1 | sed 's/\.mp4//')
echo "VIDEO_ID=$VIDEO_ID"

echo "=== Temp dir ==="
ls -la $TEMP_DIR/$VIDEO_ID/ 2>&1

echo "=== SRT file ==="
cat $TEMP_DIR/$VIDEO_ID/subtitles.srt 2>&1 | head -5

echo "=== FFmpeg subtitles check ==="
/usr/local/bin/ffmpeg -filters 2>&1 | grep subtitles

echo "=== Try export ==="
/usr/local/bin/ffmpeg -y -i "$UPLOAD_DIR/$VIDEO_ID.mp4" -vf "subtitles=$TEMP_DIR/$VIDEO_ID/subtitles.srt" -c:a copy "$TEMP_DIR/$VIDEO_ID/output_test.mp4" 2>&1