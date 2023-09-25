# beken7252-opencam
RT-Thread alternative project for A9 cameras


# build docker env

From the root folder of this repository

`docker build -t bk7252-builder . `


# build project

`docker run -v$(pwd)/project:/build bk7252`

#TODO

- Document variants for PCB and sensors
- Document pinouts for camera variants
- Document flashing procedure
- Cleanup Beken SDK
- Migrate SDK code to more modern GCC and Scons
- Other (?)
