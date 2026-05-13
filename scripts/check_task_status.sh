#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)
TASK_ID=$1
/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" https://dashscope.aliyuncs.com/api/v1/tasks/${TASK_ID}