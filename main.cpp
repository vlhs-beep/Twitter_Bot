#include <cstdio>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <time.h>
#include <Windows.h> // Windows Specific
#include <iostream>
#include <fstream>
#include <CkGlobalW.h>
#include <CkHttpW.h>
#include <CkHttpRequestW.h>
#include <CkHttpResponseW.h>
#include <CkXmlW.h>
#include <CkRestW.h>
#include <CkJsonObjectW.h>
#include <CkOAuth1W.h>
#include <CkStringBuilderW.h>
#include <CkFileAccessW.h>
#include <CkByteData.h>

using namespace std;

// Functions declarations
void global_Unlock();
void downloadWeather();
void twitterPost();
char * getTime();
int triggerPost();
const string currentDateTime();
void twitterPostWoWeather();
void checkFile();
wchar_t *mediaUpload();
void downloadSatellite();

/*
**************************************
main
**************************************
*/
void main(void)
{
	global_Unlock();
	triggerPost();
}

/* global unlock */
void global_Unlock(void){
	CkGlobalW glob;
	bool success = glob.UnlockBundle(L"Anything for 30-day trial");
	if (success != true) {
		wprintf(L"%s\n",glob.lastErrorText());
		return;
	}
}

/*
**************************************
downloadWeather
**************************************
*/
void downloadWeather(void)
{
	CkHttpW http;
	bool geturl;
	const wchar_t *url = (L"http://YOUR_OPENWEATHER.ORG_URL");
	const wchar_t *localFilePath = L"weather.xml";
	geturl = http.Download(url, localFilePath);
	if (geturl != true)	{
		wprintf(L"Error, Can't connect to open weather.\r\n");
		twitterPostWoWeather();
	}
	wprintf(L"Downloading the weather data.\n");
	wprintf(L"Success: weather.xml\r\n\n");
	checkFile();
}

/*
**************************************
checkFile
**************************************
*/
void checkFile(void)
{
	int length;
	ifstream filestr;

	filestr.open("weather.xml", ios::binary); // open your file
	filestr.seekg(0, ios::end); // put the "cursor" at the end of the file
	length = filestr.tellg(); // find the position of the cursor
	filestr.close(); // close your file

	if ( length == 0 ){
		wprintf(L"weather.xml is corrupt.\n");
		twitterPostWoWeather();

	}
	else {
		wprintf(L"weather.xml is OK!\n");
        	downloadSatellite();
		
    }
}

/*
**************************************
downloadSatellite
**************************************
*/
 
void downloadSatellite(void)
{
    CkHttpW http;
    bool success;
    const wchar_t *localFilePath = 0;
    localFilePath = L"qa_data/jpg/hisasat.gif";
    success = http.Download(L"http://images.intellicast.com/WxImages/Satellite/hisasat.gif",localFilePath);
    if (success != true) {
        wprintf(L"Error, Can't download the satellite image.\r\n");
        twitterPostWoWeather();
        //return;
    }
    wprintf(L"Successfully downloaded the satellite image\n");
    twitterPost();
}

/*
**************************************
mediaUpload
**************************************
*/
 
wchar_t *mediaUpload()
{
    //  It requires the Chilkat API to have been previously unlocked.
    //  See Global Unlock Sample for sample code.
 
    //  Assume we've previously obtained an access token and saved it to a JSON file..
    CkJsonObjectW jsonToken;
    bool success = jsonToken.LoadFile(L"qa_data/tokens/twitter.json");
 
    CkHttpW http;
 
    //  Provide the OAuth 1.0a credentials:
    http.put_OAuth1(true);
    http.put_OAuthConsumerKey(L"YOUR_CONSUMERKEY");
    http.put_OAuthConsumerSecret(L"YOUR_CONSUMERSECRET");
    http.put_OAuthToken(jsonToken.stringOf(L"oauth_token"));
    http.put_OAuthTokenSecret(jsonToken.stringOf(L"oauth_token_secret"));
 
    CkHttpRequestW req;
    req.put_HttpVerb(L"POST");
    req.put_ContentType(L"multipart/form-data");
    req.put_Path(L"/1.1/media/upload.json");
 
    req.AddHeader(L"Expect", L"100-continue");
 
    //  Add a JPEG image file to the upload.
    CkFileAccessW fac;
    CkByteData jpgBytes;
    success = fac.ReadEntireFile(L"qa_data/jpg/hisasat.gif", jpgBytes);
    req.AddBytesForUpload(L"media", L"hisasat.gif", jpgBytes);
 
    CkHttpResponseW *response = http.SynchronousRequest(L"upload.twitter.com", 443, true, req);
    if (http.get_LastMethodSuccess() != true) {
        wprintf(L"%s\n", http.lastErrorText());
        return 0;
    }
 
    CkJsonObjectW jsonResponse;
    jsonResponse.put_EmitCompact(false);
    jsonResponse.Load(response->bodyStr());
 
    if (response->get_StatusCode() != 200) {
        wprintf(L"%s\n", jsonResponse.emit());
        return 0;
    }
 
    delete response;
 
    //  Show the successful response:
    wprintf(L"%s\n", jsonResponse.emit());
    wprintf(L"Success.\n");
 
    //  Get the information from the JSON:
    wprintf(L"media_id_string = %s\n", jsonResponse.stringOf(L"media_id_string"));
    wprintf(L"size = %d\n", jsonResponse.IntOf(L"size"));
    wprintf(L"image_type = %s\n", jsonResponse.stringOf(L"image.image_type"));
    wprintf(L"height/width = %d,%d\n", jsonResponse.IntOf(L"image.w"), jsonResponse.IntOf(L"image.h"));
 
    // We need to pass the value of media_id_string to another function
    const wchar_t * global_media_id = jsonResponse.stringOf(L"media_id_string");
    wchar_t* t = new wchar_t[50];
    wcscpy(t, global_media_id);
    return t;
 
}

/*
**************************************
twitterPost
**************************************
*/
void twitterPost(void)
{
	//  Assume we've previously obtained an access token and saved it to a JSON file..
	CkJsonObjectW json;
	bool success = json.LoadFile(L"qa_data/tokens/twitter.json");
	CkRestW rest;
	CkOAuth1W oauth1;
	oauth1.put_ConsumerKey(L"YOUR_CONSUMERKEY");
	oauth1.put_ConsumerSecret(L"YOUR_CONSUMERSECRET");
	oauth1.put_Token(json.stringOf(L"YOUR_OAUTH_TOKEN"));
	oauth1.put_TokenSecret(json.stringOf(L"YOUR_OAUTH_TOKEN_SECRET"));
	oauth1.put_SignatureMethod(L"HMAC-SHA1");
	oauth1.GenNonce(16);
	rest.SetAuthOAuth1(oauth1, false);
	bool bTls = true;
	int port = 443;
	bool bAutoReconnect = true;
	success = rest.Connect(L"api.twitter.com",port,bTls,bAutoReconnect);
	wprintf(L"Checking the connection with twitter...\n\n");
	if (success != true) {
		wprintf(L"Error, Can't connect to twitter.");
		triggerPost();
}
	CkXmlW xml;
	CkXmlW *cityNode = 0;
	CkXmlW *tempNode = 0;
	CkXmlW *cloudsNode = 0;
	CkXmlW *humidityNode = 0;
	//	Load the xml file where we have the weather information.
	xml.LoadXmlFile(L"weather.xml");
	tempNode = xml.GetChild(1);
	const wchar_t *tempwchar(tempNode->getAttrValue(L"value"));
	cityNode = xml.FirstChild();
	const wchar_t *citywchar(cityNode->getAttrValue(L"name"));
	cloudsNode = xml.GetChild(5);
	const wchar_t *cloudswchar(cloudsNode->getAttrValue(L"name"));
	humidityNode = xml.GetChild(2);
	const wchar_t *humiditywchar(humidityNode->getAttrValue(L"value"));
	/* Get current time/date */
	// Convert from const string to const wchar
	const string tempdate = currentDateTime();
	wstring ws;
	//	Assigns new character values to the contents of a string.
	ws.assign(tempdate.begin(), tempdate.end());
	//	Get temporary LPCWSTR
	LPCWSTR pcwstr = ws.c_str();
	//
	CkStringBuilderW sbText;
	sbText.LoadFile(L"qa_data/txt/ghost_emoji.txt", L"utf-8");
	sbText.Prepend(L"#HoraEnChile #ChileCurrentTime\n");
	sbText.Prepend(L" \n");
	sbText.Prepend(pcwstr);
	sbText.Prepend(L" \n");
	sbText.Prepend(humiditywchar);
	sbText.Prepend(L"Humidity: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(cloudswchar);
	sbText.Prepend(L"Clouds: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(tempwchar);
	sbText.Prepend(L"Current temp: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(citywchar);
	sbText.Prepend(L"City: ");
	sbText.Prepend(L" \n");
	wprintf(L"%s\n",sbText.getAsString());
	//  Send a tweet...
	rest.ClearAllQueryParams();
	rest.AddQueryParam(L"status", sbText.getAsString());
	// We need to upload the latest satellite image
	// Add list of media_ids to associate with the Tweet, get the media_ids from mediaUpload function.
	rest.AddQueryParam(L"media_ids", mediaUpload());
	const wchar_t *response = rest.fullRequestFormUrlEncoded(L"POST", L"/1.1/statuses/update.json");
	if (rest.get_LastMethodSuccess() != true) {
	wprintf(L"Error\n");
	return;
	}
	CkJsonObjectW jsonResponse;
	jsonResponse.put_EmitCompact(false);
	jsonResponse.Load(response);
	if (rest.get_ResponseStatusCode() != 200) {
	wprintf(L"%s\n", jsonResponse.emit());
	return;
	}
	//  Show the successful response:
	wprintf(L"%s\n", jsonResponse.emit());
	wprintf(L"Success.\n");
	//	Delete the temperature node, tempNode, etc...
	delete tempNode;
	delete cityNode;
	delete cloudsNode;
	delete humidityNode;
	return;
}

/*
**************************************
twitterPostWoWeather
**************************************
*/
void twitterPostWoWeather(void)
{
	//  Assume we've previously obtained an access token and saved it to a JSON file..
	CkJsonObjectW json;
	bool success = json.LoadFile(L"qa_data/tokens/twitter.json");
	CkRestW rest;
	CkOAuth1W oauth1;
	oauth1.put_ConsumerKey(L"YOUR_CONSUMERKEY");
	oauth1.put_ConsumerSecret(L"YOUR_CONSUMERSECRET");
    	oauth1.put_Token(json.stringOf(L"oauth_token"));
    	oauth1.put_TokenSecret(json.stringOf(L"oauth_token_secret"));
	oauth1.put_SignatureMethod(L"HMAC-SHA1");
	oauth1.GenNonce(16);
	rest.SetAuthOAuth1(oauth1, false);
	bool bTls = true;
	int port = 443;
	bool bAutoReconnect = true;
	success = rest.Connect(L"api.twitter.com",port,bTls,bAutoReconnect);
	wprintf(L"Checking the connection with twitter...\n\n");
	if (success != true) {
		wprintf(L"Error, Can't connect to twitter.");
		triggerPost();
	}
	/* Get current time/date */
	// Convert from const string to const wchar
	const string tempdate1 = currentDateTime();
	wstring ws;
	//	Assigns new character values to the contents of a string.
	ws.assign(tempdate1.begin(), tempdate1.end());
	//	Get temporary LPCWSTR
	LPCWSTR pcwstr = ws.c_str();
	//
	CkStringBuilderW sbText;
	sbText.LoadFile(L"qa_data/txt/ghost_emoji.txt", L"utf-8");
	sbText.Prepend(L"#HoraEnChile #ChileCurrentTime\n");
	sbText.Prepend(L" \n");
	sbText.Prepend(pcwstr);
	sbText.Prepend(L" \n");
	sbText.Prepend(L"NO DATA");
	sbText.Prepend(L"Humidity: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(L"NO DATA");
	sbText.Prepend(L"Clouds: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(L"NO DATA");
	sbText.Prepend(L"Current temp: ");
	sbText.Prepend(L" \n");
	sbText.Prepend(L"NO DATA");
	sbText.Prepend(L"City: ");
	sbText.Prepend(L" \n");
	wprintf(L"%s\n",sbText.getAsString());
	//  Send a tweet...
	rest.ClearAllQueryParams();
	rest.AddQueryParam(L"status", sbText.getAsString());
	const wchar_t *response = rest.fullRequestFormUrlEncoded(L"POST", L"/1.1/statuses/update.json");
	if (rest.get_LastMethodSuccess() != true) {
		wprintf(L"ERROR\n");
		return;
	}
	CkJsonObjectW jsonResponse;
	jsonResponse.put_EmitCompact(false);
	jsonResponse.Load(response);
	if (rest.get_ResponseStatusCode() != 200) {
		wprintf(L"%s\n", jsonResponse.emit());
		return;
	}
	//  Show the successful response:
	wprintf(L"%s\n", jsonResponse.emit());
	wprintf(L"Success.\n");
	return;
}

/*
**************************************
currentDateTime
**************************************
*/
// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
	return buf;
}

/*
**************************************
getTime
**************************************
*/
char * getTime()
{
	char buffer[20];
	char *times();
	time_t now = time(0);
	// tm structure to store and retrieve time information.
	struct tm *RightNow;
	RightNow = localtime((const time_t*)&now);
	// Format a time string.
	strftime(buffer, 20, "%H:%M:%S", RightNow);
	//	Return buffer data to char *times()
	return buffer;
}

/*
**************************************
triggerPost
**************************************
*/
int triggerPost()
{
	wprintf(L"Checking local machine time... \n\nA Twitter status post will be made at specified hours.\n");
	while (1)
	{
		// Call the check_connection function at the specified hour.
		if (strcmp(getTime(), "00:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "00:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "00:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "00:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "01:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "01:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "01:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "01:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "02:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "02:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "02:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "02:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "03:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "03:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "03:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "03:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "04:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "04:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "04:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "04:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "05:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "05:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "05:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "05:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "06:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "06:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "06:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "06:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "07:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "07:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "07:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "07:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "08:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "08:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "08:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "08:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "09:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "09:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "09:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "09:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "10:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "10:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "10:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "10:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "11:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "11:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "11:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "11:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "12:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "12:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "12:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "12:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "13:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "13:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "13:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "13:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "14:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "14:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "14:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "14:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "15:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "15:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "15:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "15:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "16:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "16:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "16:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "16:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "17:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "17:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "17:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "17:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "17:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "18:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "18:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "18:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "18:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "19:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "19:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "19:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "19:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "20:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "20:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "20:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "20:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "21:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "21:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "21:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "21:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "22:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "22:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "22:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "22:45:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "23:00:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "23:15:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "23:30:00") == 0) {
			downloadWeather();
		}
		if (strcmp(getTime(), "23:45:00") == 0) {
			downloadWeather();
		}
		//	Sleep 1 second to avoid the CPU to consume 100%
		Sleep(1);
	}
}
