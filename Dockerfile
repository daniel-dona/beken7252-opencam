FROM ubuntu:22.04

RUN apt-get update;
RUN apt-get upgrade -y;

RUN apt-get install python2 libc6-dev-i386 micro-httpd -y;
RUN ln -s /usr/bin/python2 /usr/bin/python;


WORKDIR /

RUN mkdir /build
RUN mkdir /tools

COPY ./scons-3.1.2/ /tools/scons/

WORKDIR /tools/scons/

RUN python setup.py install --prefix=/tools/scons/

RUN ls /tools/scons/bin/

COPY ./gcc-arm-none-eabi-5_4-2016q3/ /tools/gcc/

RUN ls -l /tools/gcc/bin/arm-none-eabi-gcc; /tools/gcc/bin/./arm-none-eabi-gcc -v

#WORKDIR /build

#COPY ./project/ /build/

#RUN /tools/scons/bin/./scons

COPY ./build.sh /build.sh

CMD /bin/bash /build.sh
