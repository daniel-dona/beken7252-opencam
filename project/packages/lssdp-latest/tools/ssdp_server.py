#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket

SSDP_ADDR    = '239.255.255.250'
SSDP_PORT    =  1900
SERVICE_NAME = "urn:rt-thread:service:ssdp"

class Connection():
    def __init__(self, s, data, addr):
        self.__s = s
        self.__data = data
        self.__addr = addr
        self.is_find_service = False

    def handle_request(self):
        if self.__data.startswith('NOTIFY * HTTP/1.1\r\n'):
            self.__handle_notify()

    def __handle_notify(self):
        props = self.__parse_props(['HOST'])

        if isinstance(props, dict):
            if props.has_key('NT') and props['NT'] == SERVICE_NAME:
                print("NT        : %s"%props['NT'])
                print("SM_ID     : %s"%props['SM_ID'])
                print("DEV_TYPE  : %s"%props['DEV_TYPE'])
                print("USN       : %s"%props['USN'])
                print("HOST      : %s"%props['HOST'])
                print("LOCATION  : %s\r\n"%props['LOCATION'])

    def __parse_props(self, target_keys):
        lines = self.__data.split('\r\n')

        props = {}
        for i in range(1, len(lines)):
            if not lines[i]:
                continue

            index = lines[i].find(':')
            if index == -1:
                return None

            props[lines[i][:index]] = lines[i][index + 1:].strip()

        if not set(target_keys).issubset(set(props.keys())):
            return None

        return props

class SSDPServer():
    def __init__(self):
        self.__s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.__s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        def get_address():
            try:
                address = socket.gethostbyname(socket.gethostname())
            except:
                address = ''
            if not address or address.startswith('127.'):
                # ...the hard way.
                s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                s.connect(('1.1.1.1', 0))
                address = s.getsockname()[0]
            return address

        local_ip = get_address()
        any_ip = '0.0.0.0'

        self.__s.bind((any_ip, SSDP_PORT))

        # self.__s.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_TTL, 20)
        # self.__s.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_LOOP, 1)
        # self.__s.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF,
        # socket.inet_aton(intf) + socket.inet_aton('0.0.0.0'))

        self.__s.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP,
                            socket.inet_aton(SSDP_ADDR) + socket.inet_aton(local_ip))
        self.local_ip = local_ip

    def start(self):

        print("The SSDP service has been started.\n")

        while True:
            data, addr = self.__s.recvfrom(2048)
            conn = Connection(self.__s, data, addr)
            conn.handle_request()
        self.__s.setsockopt(socket.SOL_IP, socket.IP_DROP_MEMBERSHIP,
                            socket.inet_aton(SSDP_ADDR) + socket.inet_aton(self.local_ip))
        self.__s.close()

if __name__ == '__main__':
    port = SSDPServer()
    port.start()
