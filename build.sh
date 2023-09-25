#!/bin/bash


echo "Building BK7252 project..."

cd /build && /tools/scons/bin/./scons -c
cd /build && /tools/scons/bin/./scons 

#uhttpd -dir=/build -addr=0.0.0.0:8888
