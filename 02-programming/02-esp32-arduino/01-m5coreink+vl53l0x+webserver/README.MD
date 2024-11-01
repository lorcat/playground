## Purpose
An example of M5CoreInk operation together with time of flight (TOF) VL53L0X sensor and HTTP webserver.
Code adjusted from [Arduino](https://docs.arduino.cc/) and M5Stack C libraries [1](https://github.com/m5stack/M5Core-Ink) and [2](https://github.com/m5stack/M5Unit-TOF). The corresponding licenses apply.

The combination can be used for collision detection, although VL53L0X operates within limit of +20-2000 mm. For collision detection it can be placed at a larger distance than 20 mm to handle larger values. The sensor is not that precise, but cheap. 

The setup is shown below.
![M5Stack CoreInk + TOF](./m5coreink.jpg | width=300)

## mDNS name (multicast DNS)
This value is set within the __local_wifi.h__

## WiFi SSID + password
These values are set within the __local_wifi.h__

## HTTP pages
The server publishes two pages. One of them is root - / (html, no-cache) and the other is /signal (json as plain text, no-cache).

## Notes
[M5CoreInk](https://docs.m5stack.com/en/core/coreink) is a nice device equipped with buttons, leds, battery (360 mA), magnet and convinient hardware headers.
It is a good small device for prototyping. CPU is 160 MHz (double core). [M5Stack TOF](https://docs.m5stack.com/en/unit/TOF) is cheaper than its Adafruit or SparkFun alternatives with pre-soldered headers.

Placing web-server processing or VL53L0X code detection into a separate thread using __xTaskCreatePinnedToCore()__ did not work for me.
