Import('RTT_ROOT')
from building import *

# get current directory
cwd = GetCurrentDir()

# The set of source files associated with this SConscript file.
src = Glob('MQTTPacket/src/*.c')

if GetDepend('RT_USING_POSIX') and GetDepend('RT_USING_DFS_NET'):
    src += ['MQTTClient-RT/paho_mqtt_pipe.c']
else:
    src += ['MQTTClient-RT/paho_mqtt_udp.c']

if GetDepend(['PKG_USING_PAHOMQTT_EXAMPLE']):
    src += Glob('samples/*.c')

if GetDepend(['PKG_USING_PAHOMQTT_TEST']):
    src += Glob('tests/*.c')

path = [cwd + '/MQTTPacket/src']
path += [cwd + '/MQTTClient-RT']

group = DefineGroup('paho-mqtt', src, depend = ['PKG_USING_PAHOMQTT'], CPPPATH = path)

Return('group')
