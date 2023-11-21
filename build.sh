#!/bin/bash





build(){
	echo "Building BK7252 project..."
	cd /build && /tools/scons/bin/./scons
	exit 0
}

clean(){
	cd /build && /tools/scons/bin/./scons -c
	exit 0
}

server(){
	uhttpd -dir=/build -addr=0.0.0.0:8888
	exit 0
}


case "$1" in
  build)
    build
    ;;
  clean)
    clean
    ;;
  server)
    server
    ;;
  *)
    echo "Usage: $0 {build|clean|server}" >&2
    exit 1
    ;;
esac
