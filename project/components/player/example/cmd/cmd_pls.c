#include "player.h"
#include "pls_work.h"

static int pls_dump(void)
{
    pls_dump_music(); 
    return 0; 
}
MSH_CMD_EXPORT(pls_dump, dump player pls playing status.); 
