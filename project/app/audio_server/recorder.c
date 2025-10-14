#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dfs_posix.h>
#include "board.h"
#include "audio_device.h"
#include "vad.h"

#define READ_SIZE 2048
#define OUTPUT_FILE "/sd/record.wav"

// WAV file header structure
typedef struct {
    // RIFF Header
    char riff_header[4];      // "RIFF"
    uint32_t wav_size;        // File size - 8
    char wave_header[4];      // "WAVE"
    
    // Format chunk
    char fmt_header[4];       // "fmt "
    uint32_t fmt_chunk_size;  // Size of format chunk (16 for PCM)
    uint16_t audio_format;    // Audio format (1 for PCM)
    uint16_t num_channels;    // Number of channels
    uint32_t sample_rate;     // Sample rate
    uint32_t byte_rate;       // sample_rate * num_channels * bits_per_sample/8
    uint16_t block_align;     // num_channels * bits_per_sample/8
    uint16_t bits_per_sample; // Bits per sample
    
    // Data chunk
    char data_header[4];      // "data"
    uint32_t data_bytes;      // Size of data section
} wav_header_t;

static void write_wav_header(struct dfs_fd *fd, uint32_t sample_rate, uint16_t num_channels, uint16_t bits_per_sample, uint32_t data_size)
{
    wav_header_t header;
    
    // RIFF header
    memcpy(header.riff_header, "RIFF", 4);
    header.wav_size = data_size + sizeof(wav_header_t) - 8;
    memcpy(header.wave_header, "WAVE", 4);
    
    // Format chunk
    memcpy(header.fmt_header, "fmt ", 4);
    header.fmt_chunk_size = 16;
    header.audio_format = 1;  // PCM
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    header.block_align = num_channels * bits_per_sample / 8;
    header.bits_per_sample = bits_per_sample;
    
    // Data chunk
    memcpy(header.data_header, "data", 4);
    header.data_bytes = data_size;
    
    // Write header to file
    dfs_file_lseek(fd, 0);
    dfs_file_write(fd, &header, sizeof(wav_header_t));
}

/********************************************************
    argv[1]: vad_on       0: off, 1: on
    argv[2]: sample_rate  8000/16000
    argv[3]: duration     recording duration in seconds
    
    Command format: record [vad_on] [sample_rate] [duration]
    Example: record 1 16000 10
**********************************************************/

void record(int argc, char *argv[])
{
    struct dfs_fd fd;
    uint8_t *buffer = NULL;
    int actual_len;
    int total_bytes = 0;
    int sample_rate;
    int duration_sec;
    int vad_on;
    int max_bytes;
    
    if (argc < 4)
    {
        rt_kprintf("Usage: record [vad_on] [sample_rate] [duration]\n");
        rt_kprintf("Example: record 1 16000 10\n");
        return;
    }
    
    vad_on = atoi(argv[1]);
    sample_rate = atoi(argv[2]);
    duration_sec = atoi(argv[3]);
    
    // Allocate buffer for reading chunks
    buffer = (uint8_t *)malloc(READ_SIZE);
    if (buffer == NULL)
    {
        rt_kprintf("Not enough memory\n");
        return;
    }
    
    // Calculate maximum bytes to record (16-bit mono: sample_rate * 2 bytes/sample)
    max_bytes = sample_rate * 2 * duration_sec;
    
    // Initialize audio device
    audio_device_init();
    audio_device_mic_open();
    audio_device_mic_set_channel(1);  // Single microphone
    audio_device_mic_set_rate(sample_rate);
    
    // Open file for writing
    if (dfs_file_open(&fd, OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC) < 0)
    {
        rt_kprintf("Failed to open file: %s\n", OUTPUT_FILE);
        audio_device_mic_close();
        free(buffer);
        return;
    }
    
    // Write placeholder WAV header (will be updated later with actual size)
    write_wav_header(&fd, sample_rate, 1, 16, 0);
    
    // Initialize VAD if enabled
    if (vad_on)
    {
        rt_kprintf("VAD enabled\n");
        wb_vad_enter();
    }
    
    rt_kprintf("Recording started (sample_rate: %d Hz, duration: %d sec)\n", 
               sample_rate, duration_sec);
    
    // Recording loop
    while (total_bytes < max_bytes)
    {
        rt_thread_delay(1);
        
        actual_len = audio_device_mic_read(buffer, READ_SIZE);
        if (actual_len > 0)
        {
            // Write to file
            dfs_file_write(&fd, buffer, actual_len);
            total_bytes += actual_len;
            
            // VAD processing (if enabled)
            if (vad_on)
            {
                if (wb_vad_entry((char *)buffer, 320))
                {
                    rt_kprintf("VAD detected end of speech\n");
                    break;
                }
            }
        }
    }
    
    // Update WAV header with actual data size
    write_wav_header(&fd, sample_rate, 1, 16, total_bytes);
    
    rt_kprintf("Recording completed: %d bytes written to %s\n", 
               total_bytes, OUTPUT_FILE);
    
    // Cleanup
    dfs_file_close(&fd);
    audio_device_mic_close();
    
    if (vad_on)
    {
        wb_vad_deinit();
    }
    
    if (buffer)
    {
        free(buffer);
    }
}

MSH_CMD_EXPORT(record, record audio to SD card);