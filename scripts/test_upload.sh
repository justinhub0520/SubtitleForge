#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)
echo "test audio data" > /tmp/test_audio.wav
/usr/bin/curl -s -w '\nHTTP_CODE:%{http_code}' \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "X-DashScope-OssResourceResolve: enable" \
  -F "file=@/tmp/test_audio.wav" \
  --connect-timeout 30 --max-time 60 \
  https://dashscope.aliyuncs.com/api/v1/files 2>&1