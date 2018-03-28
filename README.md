# Hum-Temp-data-monitor-with-OLED-display-

This repo contains the necessary Arduino code to run a humidity and temperature monitoring project using Adafruit hardware and IoT interface.

# Hardware

This project uses hardware used mostly from Adafruit.com.
- Adafruit Feather HUZZAH with ESP8266 [https://www.adafruit.com/product/2821]
- Adafruit Sensiron SHT31-D Temperature & Humidity Sensor Breakout [https://www.adafruit.com/product/2857] 
- 128x64 I2C OLED display (optional) [https://www.ebay.com/sch/i.html?_odkw=oled+display+i2c&_osacat=0&_from=R40&_trksid=p2045573.m570.l1311.R6.TR7.TRC1.A0.H0.X128x64+.TRS0&_nkw=128x64+oled+i2c&_sacat=0]



<img src="https://github.com/jsafavi/Hum-Temp-data-monitor-with-OLED-display-/blob/readme-ed/hardware_setup.jpg" width="700">


# Setup 

As it can be seen in the image above, the hum/temp sensor and OLED display are connected to the feather board using I2C with white and blue wires (default pins 4 and 5). Power for both of these is provided from the board 3.3v and GND outputs.


