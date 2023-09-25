#include "player.h"
#include "audio_codec.h"

static int codec_dump(void)
{
    audio_codec_dump(); 
    return 0; 
}
MSH_CMD_EXPORT(codec_dump, dump system registered codecs.); 
