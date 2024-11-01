## Purpose
An example of M5CoreInk operation together with time of flight (TOF) VL53L0X sensor and HTTP webserver.
Code adjusted from Arduino and M5Stack C libraries.

The combination can be used for collision detection, although VL53L0X operates within limit of +20-2000 mm. For collision detection it can be placed at a larger distance than 20 mm to handle larger values. The sensor is not that too, but cheap. 

## mDNS name (multicast DNS)
This value is set within the __local_wifi.h__

## WiFi SSID + password
These values are set within the __local_wifi.h__

## HTTP pages
The server publishes two pages. One of them is root - / (html, no-cache) and the other is /signal (json as plain text, no-cache).

## Notes
Placing web-server processing or VL53L0X code detection into a separate thread using __xTaskCreatePinnedToCore()__ did not work for me.