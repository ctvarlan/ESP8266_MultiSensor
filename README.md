# ESP8266_MultiSensor
Multi Analog Sensors using the only Analog Input on the ESP8266

The need to have control over my indoor window garden pushed me to find this solution.
So, this measures the soil moisture for (by now) four pots but can be extended for how many GPIOs are available on the ESP8266 (nodeMCU v1.0). The next step was to use it for outdoor (garden or balcony) with the intention to power it up by solar panels with a minimum of energy used. The solar charger is present and do the job but everything is still for indoor application. For now, there is no automation that kicks in and start pumping water. The results are sent to ThingSpeak, which performs some data processing and graphical presentation and - if conditions arise - send me a notification using the IFTTT platform and also to Google Home Mini to warn me that either the moisture is low or the battery is low.



Hardware Setup

At the beginning is checked the status of PIN_Mode: if it is LOW then start a calibration loop, if it is HIGH continues with the main program. This is not implemented yet. Be sure to have PIN_Mode not connected to GND!
Do not forget to setup the Twitter settings in Thingspeak.

Any ESP8266 based board can do the job with the minimal condition to have the Analog Input available (ESP-01 are not usable) and as many GPIOs avalable as the Moisture Sensors needed. This project handles four sensors.
The idea is simple: all four sensors have connected each each one the GND to the general GND of the circuit, all four Analog Outputs to the A0, the Analog Input of the ESP8266 and each other Vcc of the sensor to another GPIO of the ESP8266.

The data is visible here https://thingspeak.com/channels/15699
