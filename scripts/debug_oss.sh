#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

dd if=/dev/urandom of=/tmp/audio_test.wav bs=1024 count=50 2>/dev/null

echo "=== UPLOAD with OSS resolve ==="
RESP=$(/usr/bin/curl -s \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "X-DashScope-OssResourceResolve: enable" \
  -F "file=@/tmp/audio_test.wav;type=audio/wav" \
  https://dashscope.aliyuncs.com/api/v1/files)
echo "Response: $RESP" | python3 -m json.tool 2>/dev/null || echo "$RESP"

FILE_ID=$(echo "$RESP" | grep -o '"file_id":"[^"]*"' | head -1 | cut -d'"' -f4)
echo "FILE_ID=$FILE_ID"

if [ -n "$FILE_ID" ]; then
  echo "=== GET file info ==="
  /usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" "https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}" | python3 -m json.tool 2>/dev/null
fi