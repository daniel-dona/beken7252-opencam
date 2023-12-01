FROM alpine:3.15

RUN set -eux; \
    apk add --no-cache bash make git gcc-arm-none-eabi newlib-arm-none-eabi scons; \
    rm -vrf /var/cache/apk/*; \
    rm -rf /tmp/*; \
    mkdir /project; \
    chmod a+w /project;

# tools
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

WORKDIR /project

ENTRYPOINT ["/bin/bash", "-c"]
