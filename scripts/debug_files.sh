#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

dd if=/dev/urandom of=/tmp/t.wav bs=1024 count=50 2>/dev/null

# Upload
RESP=$(/usr/bin/curl -s \
  -H "Authorization: Bearer ${API_KEY}" \
  -F "file=@/tmp/t.wav" \
  https://dashscope.aliyuncs.com/api/v1/files)
echo "UPLOAD: $RESP"
FILE_ID=$(echo "$RESP" | grep -o '"file_id":"[^"]*"' | head -1 | cut -d'"' -f4)

# GET file info
echo "=== GET /api/v1/files/$FILE_ID ==="
/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" "https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}"
echo ""

# Try download endpoints
echo "=== GET /api/v1/files/$FILE_ID/download ==="
/usr/bin/curl -s -w " HTTP:%{http_code}" -H "Authorization: Bearer ${API_KEY}" "https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/download"
echo ""

# Try content endpoint
echo "=== GET /api/v1/files/$FILE_ID/content ==="
/usr/bin/curl -s -w " HTTP:%{http_code}" -H "Authorization: Bearer ${API_KEY}" "https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/content"
echo ""

# OSS resolve with different header values
echo "=== Upload with X-DashScope-OssResourceResolve: true ==="
/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" -H "X-DashScope-OssResourceResolve: true" -F "file=@/tmp/t.wav" https://dashscope.aliyuncs.com/api/v1/files
echo ""