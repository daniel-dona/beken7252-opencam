#!/bin/bash

## /bdk_rtt is the SDK folder
## /project is the project code folder

## We merge both for compilation for now, probably would be good to make required changed on
## SCons to build both separatelly...

build_pre(){
  if [ ! -e /bdk_rtt/beken378/app ]; then
    ln -s /project/app /bdk_rtt/beken378/app
  fi
}

build_post(){
  if [ -L /bdk_rtt/beken378/app ]; then
    unlink /bdk_rtt/beken378/app
  fi
}



build(){

  build_pre
  #echo "Builing project libraries"
  #cd /bdk_rtt && /tools/scons/bin/./scons --cleanlib="beken_ble" –beken=bk7251 -j24
  #echo "Builing project libraries"
  #cd /bdk_rtt && /tools/scons/bin/./scons --buildlib="beken_ble" –beken=bk7251 -j24

	echo "Building BK7252 project..."
	cd /bdk_rtt && /tools/scons/bin/./scons --beken=bk7251 -j24

  build_post

}

clean(){
	cd /bdk_rtt && /tools/scons/bin/./scons -c
}

server(){
	uhttpd -dir=/bdk_rtt -addr=0.0.0.0:8888
}

## Using /dev/ttyBK0 to make it obvious is not the real device, adjust the device mounting when running the container!
flash(){
  /tools/flasher/uartprogram -d /dev/ttyBK0 -s 0x11000 -w /bdk_rtt/out/rtthread_uart_2M.1220.bin
}

monitor(){ 
  minicom -b 115200 -D /dev/ttyBK0
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