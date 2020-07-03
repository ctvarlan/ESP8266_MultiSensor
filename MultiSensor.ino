/* MultiSensor_2_5.ino

--------------------------------------------------------------------------------
*/

#include <ESP8266WiFi.h>
//#include <WiFiClientSecure.h>
#include <esp8266-google-home-notifier.h>		//ghn
#include "ThingSpeak.h"
#include "settings.h"

//#define MULT		1000000L
#define PIN_Vcc		4	//gpio4 or D2 on nodeMCU v3 to get Vcc
#define PIN_Mode    	5       //gpio5 or D1 on nodeMCU v3 to change the Mode

WiFiClient          	klient;		//ThingSpeak client
WiFiClient		glient;		//Google client
//WiFiClientSecure	client;		//PushBullet client
GoogleHomeNotifier 	ghn;


int		avgReads	= 5;		//@
String		myStatus	= "";
String		warningStatus 	= "";
String		msg		= "";
String		notif		= "";

String		notif1		= "Moisture level too low for ";
String		notif3		= " pots!";
String      	notif5      	= "Battery level bellow minimum";   //minimum = 2.9V [?]


//---------------------in settings.h-------------
String		google		= "www.google.com";	
const 		char displayName[] 	= "Hey Google";	        //The name of your Google Home (mini) as given in its settings
//-----------------------------------------------

int	count			=	0;
int	pw_sens[]		=	{14,12,13,15};	//pins to power the sensors
int	m_sens[]		=	{0, 0, 0, 0 };	//the measured values from sensors
int	m_per[]			=	{0, 0, 0, 0 };	//the percent values
bool	warn[]		=	{0, 0, 0, 0 };	//the fields with warning msg
int	active			=	1000;		//1sec power on to allow stabilization
int	idle			=	3000;		//3sec to allow discharge of output cap
int	H_max			=	582;            //get this values from calibration
int	H_min			=	339;            //get this values from calibration

int	hp_minW			=	40;		//min humidity Warning level 50%
int	second      	= 	0;
int	minute      	= 	0;
int	hour        	= 	0;

float   vcc     	=   0.0;
float   Vcc_min 	=   2.9;
float   V_corr  	=   1.5;    		//corection due to the measurement circuit
int 	d          	=   500;

String  getTime();
void    readMoist();

void setup()
{
	Serial.begin(115200);        	// Initialize serial
	Serial.println("\nFor calibration loop put a jumper from D1 (gpio5) to GND");

	pinMode(PIN_Mode,INPUT_PULLUP);
	while(digitalRead(PIN_Mode) == LOW)   //if PIN_Mode is LOW goes to Calibration
	{
		//calibration code -> see Calibration.ino
		//not yet now
		Serial.println("Calibration loop");
		delay(5000);
	}
	WiFi.disconnect();				// 
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass);
	delay(10);

	while(WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}//while
	
	Serial.print(" Connected to IP: ");		//@
	Serial.println(WiFi.localIP());			//@
	ThingSpeak.begin(klient);    			// Initialize ThingSpeak

	//--------------------ghn
	Serial.println("\nConnecting to Google Home Mini...");
	if (ghn.device(displayName, "en") != true) 
	{
		Serial.println(ghn.getLastError());
		return;
	}
	Serial.print("found Google Home(");		//@
	Serial.print(ghn.getIPAddress());		//@
	Serial.print(":");				//@
	Serial.print(ghn.getPort());			//@
	Serial.println(")");				//@
	//--------------------ghn
	Serial.println("\n\rMultiSensor_2_5.ino - Setup done.");

	//Vcc read:
	pinMode(PIN_Vcc, OUTPUT);
	digitalWrite(PIN_Vcc,HIGH); 			//enables the reading of Vcc
	delay(d);                   			//or less
	int Vcc = analogRead(A0);   			//Vcc to be sent to TS
	delay(d);
	digitalWrite(PIN_Vcc,LOW);  			//disables the reading of Vcc
	vcc = V_corr * Vcc / 100 ;             		//Vcc is integer, vcc is float
	Serial.print("Vcc = ");Serial.print(vcc);Serial.println(" V");
}

void loop()
{
//---ask Google for time--------------------------------------------------------
	String t = getTime();
	Serial.println(t);				//@
//TODO check for error messages!

//Read the four sensors
	Serial.println("readMoist()");			//@
	readMoist();        				//read the sensors and fill the values in m_per[]
    
//Write m_per[] to ThingSpeak
	Serial.println("TS.setFields -->");		//@
	ThingSpeak.setField(field1, m_per[0]);
	ThingSpeak.setField(field2, m_per[1]);
	ThingSpeak.setField(field3, m_per[2]);
	ThingSpeak.setField(field4, m_per[3]);
	ThingSpeak.setField(field5, vcc);

//check for warning level 
	int m = 0;
	int flag = 0;
	for (count = 0; count < 4; count++)
	{
		m = m_per[count];
		if (m <= hp_minW)
		{
			warn[count] = 1;//True
			flag++;
		}//if
	}//for
	
//Feed the ThingSpeak channel 
	msg = " -> ";
	for(int k = 0; k < 4; k++)
	{
		msg += "M" + String(k + 1) + "=" + String(m_per[k]) + ", ";
	}
	msg += "Vcc = " + String(vcc);  //Vcc is read from A0, vcc is calculated as float
	myStatus = String(t) + msg;
	// set the status
	Serial.println("TS.setStatus -->");//@
	ThingSpeak.setStatus(myStatus);

	int x = ThingSpeak.writeFields(ChannelNumber, myWriteAPIKey);
	if(x == 200)
	{
		Serial.println("Channel update OK.");
	}//if
	else
	{
		Serial.println("Problem updating channel. HTTP error code " + String(x));
	}//else
	
	if (flag > 0)
	{
		//message to Tweeter, from ThingSpeak
		int tt = ThingSpeak.setTwitterTweet(TWITTER_ACC, myStatus);
		
		if(tt == 200)
		{
			Serial.println("Tweet sent OK");
		}
		else
		{
			Serial.println("Twitter error code " + String (tt));
		}
		//-------------------------------------------------------Twitter
		
		//message to Google Home Notifier
		String notif2 = String(flag);   //how many pots are at low level
		notif = notif1 + notif2 + notif3;
		if ((hour >= 8) && (hour < 22))
		{
			if (ghn.notify(notif.c_str()) != true) 
			{
				Serial.println(ghn.getLastError());
				//return;
			}
		}
        
		//------------------------------------------Google Home Notifier
		flag = 0;
	}
    //message to Google Home Notifier if Vcc < Vcc_min
    if (vcc < Vcc_min)
    {
        notif = notif5;
        if ((hour >= 8) && (hour < 22))
		{
			if (ghn.notify(notif.c_str()) != true) 
			{
				Serial.println(ghn.getLastError());
				//return;
			}
		}
    }
    
    Serial.println("Hour: " + String(hour));
    WiFi.disconnect();
    Serial.println("WiFi disconnected.");
    long sleep = SLEEP_TIME_SECONDS * 1000000L - 8;		//some correction
    Serial.print("Go to sleep for ");
    Serial.print(SLEEP_TIME_SECONDS);
    Serial.println(" Seconds" );//
    ESP.deepSleep(sleep);

}

//----------Functions-----------------------------------------------------------
String getTime()
{
//in the main prog is defined the client to access google as glient
	Serial.println(F("connecting to google"));//@
	//Serial.println(google);			//@
	glient.setTimeout(5000);                  //at the beginning

	const int httpPort = 80;                  //at the beginning
	if (!glient.connect(google, httpPort)) 
	{
		Serial.println(F("connection failed"));
		return "google_fail";
	}

	// This will send the request to the server
	glient.println("HEAD / HTTP/1.1");
	glient.println("Host: www.google.com"); // "Host: www.google.com"
	glient.println("Accept: */*");
	glient.println("User-Agent: Mozilla/4.0 (compatible; esp8266 Arduino;)");
	glient.println("Connection: close");
	glient.println();
	delay(500);

	// Read all the characters of the reply from server and print them to Serial
	String reply = String("");
	while(glient.available())
	{
		char c = glient.read();
		reply = reply + String(c);
	}
	//Serial.print(reply);		//@

	//String d = reply.substring(reply.indexOf("Date: ")+11,reply.indexOf("Date: ")+23);	//string for date
	String t = reply.substring(reply.indexOf("Date: ")+23,reply.indexOf("Date: ")+35);		//string for time

	hour  = t.substring(0, 2).toInt();
	minute  = t.substring(3, 5).toInt();
	second  = t.substring(6, 8).toInt();

	hour = hour + 20;//for summer is 20, for winter is 19
	if (hour >= 24) 
	{
		hour = hour % 24;
	}

	if(!glient.connected())
	{
		//Serial.println("disconnecting");
		glient.stop();
	}
	Serial.println("connection closed");		//@
	return t;
}

void readMoist()    //new funtion
{
    int an = 0;
    for(count = 0; count < 4; count++)
    {//put all control pins at GND -> sensors are inactive
        pinMode(pw_sens[count],OUTPUT);
        digitalWrite(pw_sens[count], LOW);
    }//for

    for(count = 0; count < 4; count++)
    {
		int m = 0;
		digitalWrite(pw_sens[count], HIGH);	//the active state
		delay(active);
		//analog read for sensor 'count'
		an = analogRead(A0);
		m = an;

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
		//Serial.println("\t" + String(count) + "\t" + String(an) + "\t" + String(m));	//@
		m_sens[count] = m;
		//Serial.println(m);
		m_per[count] = map(m, H_min, H_max, 100, 0);
        Serial.println("\t" + String(count) + "\t" + String(an) + "\t" + String(m_per[count]));	//@
		digitalWrite(pw_sens[count], LOW);
		delay(idle);

    }//for
}//readMoist()
