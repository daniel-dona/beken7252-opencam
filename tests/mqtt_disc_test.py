#!/usr/bin/env python3
"""
MQTT Discovery Test Script for Home Assistant
"""
import paho.mqtt.client as mqtt
import json
import time
from PIL import Image
import base64
import io

# MQTT Broker Configuration
MQTT_BROKER = "192.168.10.1"  # Change to your broker IP/hostname
MQTT_PORT = 1883
MQTT_USERNAME = None  # Set if authentication is required
MQTT_PASSWORD = None

# Device Information
DEVICE_ID = "ea334450945afc"
DEVICE_NAME = "OpenCam"

def on_connect(client, userdata, flags, rc):
    """Callback when connected to MQTT broker"""
    if rc == 0:
        print(f"✓ Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    else:
        print(f"✗ Connection failed with code {rc}")

def on_publish(client, userdata, mid):
    """Callback when message is published"""
    print(f"✓ Message published (mid: {mid})")

def publish_components_discovery(client):
    """Publish using the components format (recommended for multiple sensors)"""
    
    discovery_topic = f"homeassistant/device/{DEVICE_NAME.lower()}/config"
    
    payload = {
        "dev": {
            "ids": DEVICE_ID,
            "name": DEVICE_NAME,
            "mf": "OpenCam",
            "mdl": "BK7252",
            "sw": "1.0"
        },
        "o": {
            "name": "OpenCam HASS Integration",
            "sw": "1.0.0",
            "url": "https://github.com/daniel-dona/beken7252-opencam"  # Optional but recommended
        },
        "cmps": {
            "camera_feed": {
                "p": "camera",
                "device_class": "camera",
                "topic": "sensor/camera",
                "unique_id": "hum01ae_c",
                "image_encoding": "b64",
                "name": "Camera"
            }
        }
    }
    
    print(f"\n📤 Publishing components discovery to: {discovery_topic}")
    print(f"Payload:\n{json.dumps(payload, indent=2)}\n")
    
    client.publish(
        discovery_topic,
        json.dumps(payload),
        qos=2,
        retain=True
    )

def publish_test_data(client):
    """Publish test sensor data"""
    
    # Temperature data
    temp_topic = "sensor/temperature"
    temp_value = 23.5
    print(f"\n📊 Publishing temperature: {temp_value}°C to {temp_topic}")
    client.publish(temp_topic, str(temp_value), qos=0, retain=True)
    
    # Humidity data (for components example)
    hum_topic = "sensor/humidity"
    hum_value = 65
    print(f"📊 Publishing humidity: {hum_value}% to {hum_topic}")
    client.publish(hum_topic, str(hum_value), qos=0, retain=True)
    
    # Humidity data (for components example)
    camera_topic = "sensor/camera"
    camera_value = pil_jpeg_to_base64('/home/dani/Imágenes/test_pattern.jpg')
    
    print(f"📊 Publishing humidity: {hum_value}% to {hum_topic}")
    client.publish(camera_topic, str(camera_value), qos=0, retain=True)

def pil_jpeg_to_base64(file_path):
    """
    Open JPEG with PIL and encode to base64
    
    Args:
        file_path (str): Path to the JPEG file
    
    Returns:
        str: Base64 encoded string
    """
    # Open and verify the image
    with Image.open(file_path) as img:
        # Convert to RGB if necessary (for PNG with transparency)
        if img.mode in ('RGBA', 'LA', 'P'):
            img = img.convert('RGB')
        
        # Save to bytes buffer
        buffer = io.BytesIO()
        img.save(buffer, format='JPEG', quality=95)
        
        # Encode to base64
        encoded_string = base64.b64encode(buffer.getvalue()).decode('utf-8')
    
    return encoded_string

def main():
    """Main function"""
    
    # Create MQTT client
    client = mqtt.Client(client_id="hass_discovery_test")
    client.on_connect = on_connect
    client.on_publish = on_publish
    
    # Set credentials if needed
    if MQTT_USERNAME and MQTT_PASSWORD:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    
    # Connect to broker
    try:
        print(f"🔌 Connecting to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()
        time.sleep(1)  # Wait for connection
        

        publish_components_discovery(client)
        
        time.sleep(1)  # Wait for publish
        
        # Publish test data
        publish_test_data(client)
        
        time.sleep(2)  # Wait for all messages to be sent
        
        print("\n✅ Test complete!")
        print("Check Home Assistant -> Settings -> Devices & Services -> MQTT")
        
    except Exception as e:
        print(f"\n✗ Error: {e}")
    finally:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()