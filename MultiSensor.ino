/* MultiSensor_2_3.ino

ESP8266 uses 4 of its GPIOs and the only AnalogIn to drive 4 sensors.

What is needed:
[_]	WiFi access:
	[_] ssid		The name of your Access Point (a.k.a. the router)
	[_] pass		The password to access the router

[_]	ThingSpeak account [free version]
	[_]	the Channel Number
	[_]	the WriteAPIKey
[_]	Tweeter account [free]
	[_]	the handler (@...)
	[_]	the APIKey 
	[_]	the settings done within ThingSpeak
[_]	PushBullet account [free version]
	[_]	the Access Tokens from your account
	[_] the fingerprint from the browser

All these should be handled through a separate file, here is named "settings.h"

*/

#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "settings.h"

WiFiClient          klient;

int			avgReads	= 5;
String		myStatus	= "";
String		warningStatus = "";

String		note1		= "{\"type\": \"note\", \"title\": \"Moisture warning\", \"body\": \"";
String		note2		= "\"}\r\n";

int	statusCode;

int	count		=	0;
int	pw_sens[]	=	{14,12,13,15};	//pins to power the sensors
int	m_sens[]	=	{0,0,0,0};		//the measured values from sensors
int	m_per[]		=	{0,0,0,0};		//the percent values
int	active		=	1000;			//1sec power on to allow stabilization
int	idle		=	3000;			//3sec to allow discharge of output cap
int	H_max		=	770;
int	H_min		=	390;


//---Functions------------------------------------------------------------------
void readMoist()    //new funtion
{
	int an = 0;
	int m = 0;
	for(count = 0; count < 4; count++)
	{//put all control pins at GND -> sensors are inactive
		pinMode(pw_sens[count],OUTPUT);
		digitalWrite(pw_sens[count], LOW);
		//Serial.print(pw_sens[count]);//@
	}//for

    for(count = 0; count < 4; count++)
    {
		int m = 0;
		digitalWrite(pw_sens[count], HIGH);	//the active state
		delay(active);
		//analog read for sensor 'count'
		an = analogRead(A0);
		m = an;
//		digitalWrite(pw_sens[count],HIGH);
//		Serial.println(pw_sens[count]);
//		Serial.println("\t"+String(count)+"\t"+String(m));//@ to remove
		//to prevent outliers above 100% and below 0%
		if (an < H_min)
		{
			m = H_min;
		}
		if (an > H_max)
		{
			m = H_max;
		}
		//put the result in m_sens[count]
		Serial.println("\t" + String(count) + "\t" + String(an) + "\t" + String(m));	//@
		m_sens[count] = m;
		//Serial.println(m);
		m_per[count] = map(m, H_min, H_max, 100, 0);
		digitalWrite(pw_sens[count], LOW);
		delay(idle);
		//Serial.print(String(m_per[count]));
		//Serial.println("\t" + String(count) + "\t" + String(an) + "\t" + String(m));	//@

    }//for
}//readMoist() new function

String setStatus()
{
    String status = "";
	for(count = 0; count < 4; count++)
	{
		status += "\t m[" + String(count) +"] = " + String(m_per[count]);
	}
    return status;
}

//---End-Functions--------------------------------------------------------------


void setup()
{
    Serial.begin(115200);        // Initialize serial
    ThingSpeak.begin(klient);    // Initialize ThingSpeak
    //WiFi.disconnect();            // Just to be sure
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    delay(10);

    //WiFiClientSecure    client;

    Serial.println("\n\rSetup done.");
}

void loop()
{
//Connection to WiFi
    while(WiFi.status() != WL_CONNECTED)
        {
          delay(500);
          Serial.print(".");
        }//while
    Serial.print(" WiFi connected to ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
//....
//Connection to PushBullet
//Use WiFiClientSecure class to create TLS connection

	WiFiClientSecure  client;
    Serial.print("Connecting to: ");    //@
    Serial.println(host);

    //Serial.printf("Using fingerprint '%s'\n", fingerprint);    //@
    client.setFingerprint(fingerprint);

    if (!client.connect(host, httpsPort))
    {
        Serial.println("----- Connection failed");
        return;
    }

    if (client.verify(fingerprint, host))
    {
        Serial.println("   Certificate matches");
    }
    else
    {
        Serial.println("   Certificate doesn't match");
    }

//Read the four sensors
	Serial.println("readMoist()");//@
    readMoist();        //read the sensors and fill the values in m_per[]

//Write m_per[] to ThingSpeak
	Serial.println("setFields");//@
    ThingSpeak.setField(5, m_per[0]);
    ThingSpeak.setField(6, m_per[1]);
    ThingSpeak.setField(7, m_per[2]);
    ThingSpeak.setField(8, m_per[3]);

//check for warning level (m <= 50 AND m > 40)
    int m = 0;
    for (count = 0; count < 4; count++)
    {
        m = m_per[count];
        if ((m <= 50) && (m > 40))
        {// for m between 40 and 50 the Tweet warning goes from here
         // for m less than 40 then ThingSpeak takes care for Tweeting it.
		 //TODO: to send a push notification by PushBullet using ThingHTTP
            warningStatus = "\n\rField" + String(count + 5) + ": warning level " + String(m);
            int tt = ThingSpeak.setTwitterTweet(TWITTER_ACC, warningStatus);
			Serial.println(warningStatus);	//@
            if(tt == 200)
            {
                Serial.println("Tweet sent OK");
            }
            else
            {
                Serial.println("Twitter error code " + String (tt));
            }
		//note1 = "{\"type\": \"note\", \"title\": \"Moisture warning\", \"body\": \"";
		//note2 = "\"}\r\n"		;
			String msg = "";
			for(count = 0; count < 4; count++)
			{
				msg += "\tM" +String(count + 1) + " = " + String(m_per[count]);
			}
			String note = note1 + String(msg) + note2;
			Serial.println(note);        //@

			//This sends a note to PushBullet.
			//TODO: to look after the reply from PushBullet and check the error code.
			String url = "/v2/pushes";
			client.print(String("POST ") + url + " HTTP/1.1\r\n" +
					"Host: " + host + "\r\n" +
					"User-Agent: ESP8266\r\n" +
					"Access-Token: " + accessToken + "\r\n" +
					"Content-length: " + String(note.length()) + "\r\n"
					"Content-Type: application/json\r\n" +
					"Connection: close\r\n\r\n");
			client.print(note);
			Serial.println(note);//@
			//here comes the code for treating the reply from PushBullet

			while (client.connected())
			{
				String line = client.readStringUntil('\n');
				if (line == "\r")
				{
					Serial.println("headers received");
					break;
				}
			}//while
			String line = client.readStringUntil('\n');
			//	String line = client.readString(); // this line is good for debugging error messages more then a single line
			if (line.startsWith("{\"active\":true"))
			{
				Serial.println("ESP8266_Solar_Moisture push successfully !");
			}
			else
			{
				Serial.println("ESP8266_Solar_Moisture push has failed");
			}
			Serial.println("reply was:");
			Serial.println(line);
			Serial.println("closing connection");
		}//if
	}//for

    myStatus = setStatus() + warningStatus;
    // set the status
    ThingSpeak.setStatus(myStatus);
	Serial.println(myStatus);//@
	int x;
    x = ThingSpeak.writeFields(ChannelNumber, myWriteAPIKey);
    if(x == 200)
    {
        Serial.println("Channel update OK.");
    }//if
    else
    {
        Serial.println("Problem updating channel. HTTP error code " + String(x));
    }//else

    WiFi.disconnect();

    long sleep = SLEEP_TIME_SECONDS * 1000000;
	//long sleep = 10*1000000;
    Serial.println("Go to sleep for "+ String(sleep) + " microSeconds" );//@ to remove
    ESP.deepSleep(sleep);
	//delay(sleep/1000);//@ to remove when use the deep sleep
}
