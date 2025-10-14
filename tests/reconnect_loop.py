import requests
import time
import random

def read_mjpeg_stream(url, duration=30):
    """
    Connect to a MJPEG stream (multipart/x-mixed-replace) for a specified duration
    and report the number of JPEG frames received.
    """
    print(f"Connecting to {url}...")
    
    frame_count = 0
    start_time = time.time()
    last_report_time = start_time
    
    try:
        response = requests.get(url, stream=True, timeout=35)
        response.raise_for_status()
        
        print(f"Connected! Status: {response.status_code}")
        print(f"Reading MJPEG stream for {duration} seconds...")
        print("-" * 50)
        
        buffer = b""
        
        for chunk in response.iter_content(chunk_size=8192):
            current_time = time.time()
            elapsed = current_time - start_time
            
            # Check if we've exceeded the time limit
            if elapsed > duration:
                print(f"\n{duration} seconds elapsed. Disconnecting...")
                break
                
            if chunk:
                buffer += chunk
                
                # Look for JPEG frame markers
                jpeg_start = b'\xff\xd8'  # JPEG start marker
                jpeg_end = b'\xff\xd9'   # JPEG end marker
                
                # Count complete JPEG frames in the buffer
                start_pos = 0
                while True:
                    frame_start = buffer.find(jpeg_start, start_pos)
                    if frame_start == -1:
                        break
                        
                    frame_end = buffer.find(jpeg_end, frame_start)
                    if frame_end == -1:
                        break
                        
                    # We found a complete JPEG frame
                    frame_count += 1
                    start_pos = frame_end + 2  # Move past the end marker
                    
                    # Report progress every second
                    if current_time - last_report_time >= 1.0:
                        fps = frame_count / elapsed if elapsed > 0 else 0
                        print(f"Frames: {frame_count} | Rate: {fps:.1f} fps | Elapsed: {elapsed:.1f}s")
                        last_report_time = current_time
                
                # Keep only the last incomplete part of the buffer
                if start_pos > 0:
                    buffer = buffer[start_pos:]
                    
    except requests.exceptions.RequestException as e:
        print(f"Error connecting to the stream: {e}")
    except KeyboardInterrupt:
        print("\nInterrupted by user. Disconnecting...")
        raise KeyboardInterrupt
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        total_elapsed = time.time() - start_time
        print("-" * 50)
        print("STREAM SUMMARY:")
        print(f"Total frames received: {frame_count}")
        print(f"Total duration: {total_elapsed:.2f} seconds")
        if total_elapsed > 0:
            print(f"Average frame rate: {frame_count/total_elapsed:.2f} fps")
        print("Connection closed.")

if __name__ == "__main__":
    
    urls = [
        #"http://192.168.30.220/",
        "http://192.168.30.221/",
        "http://192.168.30.222/",
        "http://192.168.30.223/",
        "http://192.168.30.224/",
    ]
    
    runtime = 7*24*60
    
    for i in range(int(runtime)):
        try:
            read_mjpeg_stream(random.choice(urls), duration=random.randint(0,60))
        except Exception as e:
            print(e)
            quit()