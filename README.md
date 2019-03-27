# ESP8266_MultiSensor
Multi Analog Sensors using the only Analog Input on the ESP8266

The need of control over my indoor window garden pushed me to find this solution.
So, this measures the soil moisture for (by now) four pots but can be extended for how many GPIOs available are on the ESP8266. The next step is to use it for outdoor (garden or balcony) with the intention to power it up by solar panels with a minimum of energy used. So, for now, there is no automation that kicks is and start pumping water. The results are sent to ThingSpeak, which performs some data processing and graphical presentation, and - if conditions arise - send me a push notification by PushBullet.

Hardware Setup
Any ESP8266 based board can do the job with the minimal condition to have the Analog Input available (ESP-01 are not usable) and as many GPIOs avalable as the Moisture Sensors needed. This project handles four sensors.
The idea is simple: all four sensors are connected each GND to the general GND of the circuit, each Analog Output to the A0, the Analog Input of the ESP8266 and each other Vcc of the sensor to another GPIO of the ESP8266.
