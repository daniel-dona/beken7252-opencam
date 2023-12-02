FROM alpine:3.15

RUN set -eux; \
    apk add --no-cache bash make git gcc-arm-none-eabi newlib-arm-none-eabi scons; \
    rm -vrf /var/cache/apk/*; \
    rm -rf /tmp/*; \
    mkdir /project; \
    chmod a+w /project;

# rtt tools
RUN set -eux; \
    apk add --no-cache python3 py3-pip py3-psutil; \
    mkdir -p /root/.env/tools/scripts; \
    mkdir -p /root/.env/packages/packages; \
    git clone https://github.com/RT-Thread/env.git /root/.env/tools/scripts/; \
    git clone https://github.com/RT-Thread/packages.git /root/.env/packages/packages/; \
    pip install requests -qq; \
    rm -rf /tmp/*

# register rtt tools
ENV PATH="/root/.env/tools/scripts:$PATH"

# encrypt_crc tool
RUN set -eux; \
    apk add --no-cache build-base; \
    git clone https://github.com/Apache02/a9-hello-world.git /tmp/; \
    cd /tmp/tools; \
    make; \
    cp encrypt_crc/encrypt_crc /usr/bin; \
    apk del build-base; \
    rm -rf /tmp/*

WORKDIR /project

ENTRYPOINT ["/bin/bash", "-c"]
