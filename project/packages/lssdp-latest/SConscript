from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

src += Glob('src/daemon.c')
src += Glob('src/lssdp.c')
src += Glob('src/lssdp_service.c')

if GetDepend('LSSDP_USING_SAMPLES'):
    src += Glob('samples/lssdp_sample.c')

path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('lssdp', src, depend = ['PKG_USING_LSSDP'], CPPPATH = path)

Return('group')
