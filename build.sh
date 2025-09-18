#!/bin/bash





build(){
	echo "Building BK7252 project..."
	cd /build && /tools/scons/bin/./scons –beken=bk7251 -j12
}

clean(){
	cd /build && /tools/scons/bin/./scons -c
}

server(){
	uhttpd -dir=/build -addr=0.0.0.0:8888
}

flash(){
  /tools/flasher/uartprogram -d /dev/ttyUSB1 -s 0x11000 -w /build/out/rtthread_uart_2M.1220.bin
}

monitor(){ 
  minicom -b 115200 -D /dev/ttyUSB1
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
  flash)
    flash
    ;;
  build-flash-monitor)
    build && flash && monitor
    ;;
  *)
    echo "Usage: $0 {build|clean|server|flash|build-flash-monitor}" >&2
    exit 1
    ;;
esac

exit 0