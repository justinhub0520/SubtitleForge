#!/bin/bash
set -e
cd /home/project01/SubForge
killall -9 subforge_backend 2>/dev/null || true
sleep 1
rm -f build/backend/CMakeFiles/subforge_backend.dir/src/export_handler.cpp.o
rm -f build/backend/tests/CMakeFiles/test_server.dir/__/src/export_handler.cpp.o
cd build
make -j4
echo "BINARY_SIZE=$(stat -c%s backend/subforge_backend)"
cd /home/project01/SubForge
nohup ./build/backend/subforge_backend > backend.log 2>&1 &
sleep 2
cat backend.log
wget -q -O- http://localhost:8080/api/health
echo ""
echo "DONE"