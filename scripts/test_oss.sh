#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)
echo test > /tmp/t.wav
echo "=== WITH OSS RESOLVE ==="
/usr/bin/curl -s \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "X-DashScope-OssResourceResolve: enable" \
  -F "file=@/tmp/t.wav" \
  https://dashscope.aliyuncs.com/api/v1/files
echo ""
echo "=== WITHOUT OSS RESOLVE ==="
/usr/bin/curl -s \
  -H "Authorization: Bearer ${API_KEY}" \
  -F "file=@/tmp/t.wav" \
  https://dashscope.aliyuncs.com/api/v1/files