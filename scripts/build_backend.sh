#!/bin/bash
set -e

echo "Building SubForge backend..."

cd "$(dirname "$0")/.."

mkdir -p build/backend
cd build/backend

cmake ../.. -DBUILD_BACKEND=ON -DBUILD_FRONTEND=OFF
make -j$(nproc)

echo "Backend built successfully."
echo "Run with: ./subforge_backend"