## Purpose
An example of M5CoreInk operation together with time of flight (TOF) VL53L0X sensor and HTTP webserver.
Code adjusted from Arduino and M5Stack libraries.

The combination can be used for collision detection, although VL53L0X

## mDNS name (multicast DNS)
This value is set within the __local_wifi.h__

## WiFi SSID + password
These values are set within the __local_wifi.h__

## Notes
Placing web-server processing or VL53L0X code detection into a separate thread using __xTaskCreatePinnedToCore()__ did not work for me.