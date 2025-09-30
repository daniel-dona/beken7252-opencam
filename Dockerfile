FROM ubuntu:24.04

# Install system updates and essential packages
RUN apt-get update && apt-get upgrade -y

# Install ARM GCC toolchain from APT and other build tools
RUN apt-get install -y \
    python3 \
    python3-pip \
    build-essential \
    minicom \
    libc6-dev-i386 \
    micro-httpd \
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    && rm -rf /var/lib/apt/lists/*

# Create python symlink for SCons compatibility
RUN ln -s /usr/bin/python3 /usr/bin/python

# Create necessary directories
RUN mkdir -p /build /tools

# Install SCons from PIP (latest version)
RUN python3 -m pip install scons --break-system-packages

# Copy flasher tools and install requirements
COPY ./hid_download_py/ /tools/flasher/
WORKDIR /tools/flasher/
RUN python3 -m pip install -r requirements.txt --break-system-packages

# Create symlink for GCC toolchain to match expected path
RUN ln -s /usr /tools/gcc

# Copy build script
COPY ./build.sh /build.sh

# Make build.sh executable
RUN chmod +x /build.sh


# Use ENTRYPOINT to match original Dockerfile behavior
ENTRYPOINT ["/bin/bash", "/build.sh"]