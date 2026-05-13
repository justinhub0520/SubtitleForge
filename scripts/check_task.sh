#!/bin/bash
TASK_ID=$1
API_KEY=$(grep DASHSCOPE_API_KEY /home/project01/SubForge/.env | cut -d= -f2)

for i in $(seq 1 10); do
  sleep 5
  RESP=$(/usr/bin/curl -s \
    -H "Authorization: Bearer ${API_KEY}" \
    https://dashscope.aliyuncs.com/api/v1/tasks/${TASK_ID})
  echo "Attempt $i: $RESP"
  STATUS=$(echo "$RESP" | grep -o '"task_status":"[^"]*"' | cut -d'"' -f4)
  if [ "$STATUS" = "SUCCEEDED" ] || [ "$STATUS" = "FAILED" ]; then
    break
  fi
done