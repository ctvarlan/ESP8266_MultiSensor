// Use this file to store all of the private credentials
// and connection details

char ssid[] 	= "yourSSID";		//
char pass[] 	= "yourPASSWORD";	//

//------ThingSpeak settings-----------------------------------------------------
unsigned 	long 	ChannelNumber 	= XXXXXX;		//to read from and write to
const 		char* 	myWriteAPIKey 	= "xxxxxxxxxxxxxxxx"; 	//16 characters
const		char* 	myReadAPIKey	= "xxxxxxxxxxxxxxxx"; 	//16 characters

//------PushBullet settings-----------------------------------------------------
const 		char* 	host 				= "api.pushbullet.com";
const 		int 	httpsPort 	= 443; 			// the required port
const 		char* 	accessToken 		= "o.xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";// 34 characters starting with 'o.'
			//get it from the pushbullet account
const 		char* 	fingerprint 		= "XX XX XX ... XX";//
			//get it from the browser

// Hardware information.
#define SLEEP_TIME_SECONDS 	1800		// 
#define TWITTER_ACC		"your Twitter handler"  //@whatever
