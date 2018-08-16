#include <cstdio>
#include <iostream>
#include <ctime>
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

int main() {
    
CkGlobalW glob;
CkHttpW http;
CkJsonObjectW json;
CkStringBuilderW sbText;
CkStringBuilderW sb2Text;
CkRestW rest;
CkOAuth1W oauth1;
CkHttpRequestW req;
CkFileAccessW fac;
CkByteData jpgBytes;

    // Get current date/time, format is YYYY-MM-DD.HH:mm:ss
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
    // Convert from const string to const wchar
    const std::string tempHora = buf;
    std::wstring ws;
    // Assigns new character values to the contents of a string.
    ws.assign(tempHora.begin(), tempHora.end());
    
    wchar_t const * horaActual = ws.c_str();

    //////////

    bool successUnlock = glob.UnlockBundle(L"Anything for 30-day trial");
    if (successUnlock != true) {
        wprintf(L"%s\n", glob.lastErrorText());
        return 0;
    }

    //////////

    bool getUrlWeather;
    const wchar_t *urlWeather =
            (L"OPENWEATHER URL");
    const wchar_t *localFilePathWeather = L"qa_data/json/weather.json";
    getUrlWeather = http.Download(urlWeather, localFilePathWeather);
    if (getUrlWeather != true) {
        wprintf(L"Error, Can't connect to open weather.\r\n");
        return 0;
    }

    //////////

    bool getUrlSatellite;
    const wchar_t *urlSatellite =
            (L"IMAGE URL");
    const wchar_t *localFilePathSatellite = L"qa_data/image/snapshot.jpg";
    getUrlSatellite = http.Download(urlSatellite, localFilePathSatellite);
    if (getUrlSatellite != true) {
        wprintf(L"Error, Can't connect to open weather.\r\n");
        return 0;
    }

    //////////

    rest.SetAuthOAuth1(oauth1, false);
    bool bAutoReconnect = true;
    bool successC = json.LoadFile(L"qa_data/tokens/twitter.json");
    oauth1.put_ConsumerKey(L"CONSUMER KEY");
    oauth1.put_ConsumerSecret(L"CONSUMER SECRET");
    oauth1.put_Token(json.stringOf(L"oauth_token"));
    oauth1.put_TokenSecret(json.stringOf(L"oauth_token_secret"));
    oauth1.put_SignatureMethod(L"HMAC-SHA1");
    oauth1.GenNonce(16);

    successC = rest.Connect(L"api.twitter.com", 443, true, bAutoReconnect);
    if (successC != true) {
        wprintf(L"%ls\n", rest.lastErrorText());
        return 0;
    }

    //////////

    // Assume we've previously obtained an access token and saved it to a JSON file..
    CkJsonObjectW jsonToken;
    bool success = jsonToken.LoadFile(L"qa_data/tokens/twitter.json");

    // Provide the OAuth 1.0a credentials:
    http.put_OAuth1(true);
    http.put_OAuthConsumerKey(L"CONSUMER KEY");
    http.put_OAuthConsumerSecret(L"CONSUMER SECRET");
    http.put_OAuthToken(jsonToken.stringOf(L"oauth_token"));
    http.put_OAuthTokenSecret(jsonToken.stringOf(L"oauth_token_secret"));

    req.put_HttpVerb(L"POST");
    req.put_ContentType(L"multipart/form-data");
    req.put_Path(L"/1.1/media/upload.json");

    req.AddHeader(L"Expect", L"100-continue");

    // Add a JPEG image file to the upload.
    success = fac.ReadEntireFile(L"qa_data/image/snapshot.jpg", jpgBytes);
    req.AddBytesForUpload(L"media", L"snapshot.jpg", jpgBytes);

    CkHttpResponseW *response = http.SynchronousRequest(L"upload.twitter.com",
                                                        443, true, req);
    if (http.get_LastMethodSuccess() != true) {
        wprintf(L"%ls\n", http.lastErrorText());
        return 0;
    }

    CkJsonObjectW jsonResponse;
    jsonResponse.put_EmitCompact(false);
    jsonResponse.Load(response->bodyStr());

    if (response->get_StatusCode() != 200) {
        wprintf(L"%ls\n", jsonResponse.emit());
        return 0;
    }

    delete response;

    // Show the successful response:
    wprintf(L"%ls\n", jsonResponse.emit());
    wprintf(L"Success.\n");

    // Get the information from the JSON:
    wprintf(L"media_id_string = %ls\n", jsonResponse.stringOf(L"media_id_string"));
    wprintf(L"size = %d\n", jsonResponse.IntOf(L"size"));
    wprintf(L"image_type = %ls\n", jsonResponse.stringOf(L"image.image_type"));
    wprintf(L"height/width = %d,%d\n", jsonResponse.IntOf(L"image.w"), jsonResponse.IntOf(L"image.h"));

    //////////

    bool successA = json.LoadFile(L"qa_data/json/weather.json");
    if (successA != true) {
        std::cout << "Can't load file" << std::endl;
        return 0;
    }

    wchar_t const *broken_clouds = L"broken clouds";
    wchar_t const *clear_sky = L"clear sky";
    wchar_t const *few_clouds = L"few clouds";
    wchar_t const *mist = L"mist";
    wchar_t const *rain = L"rain";
    wchar_t const *scattered_clouds = L"scattered clouds";
    wchar_t const *shower_rain = L"shower rain";
    wchar_t const *snow = L"snow";
    wchar_t const *thunderstorm = L"thunderstorm";

    {
        bool successB = json.LoadFile(L"qa_data/json/weather.json");
        if (successB != true) {
            std::cout << "Can't load file" << std::endl;
            return 0;
        }
        const wchar_t *cloudswchar(json.stringOf(L"weather[0].description"));
        wchar_t *t = new wchar_t[50];
        wcscpy(t, cloudswchar);

        int result1 = wcscmp(broken_clouds, t);
        if (result1 == 0) {
            sbText.LoadFile(L"qa_data/txt/broken_clouds", L"utf-8");
            std::cout << "Broken Clouds" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result2 = wcscmp(clear_sky, t);
        if (result2 == 0) {
            sb2Text.LoadFile(L"qa_data/txt/clear_sky.txt", L"utf-8");
            std::cout << "Clear Sky" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result3 = wcscmp(few_clouds, t);
        if (result3 == 0) {
            sbText.LoadFile(L"qa_data/txt/few_clouds.txt", L"utf-8");
            std::cout << "Few Clouds" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result4 = wcscmp(mist, t);
        if (result4 == 0) {
            sbText.LoadFile(L"qa_data/txt/mist.txt", L"utf-8");
            std::cout << "Mist" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result5 = wcscmp(rain, t);
        if (result5 == 0) {
            sbText.LoadFile(L"qa_data/txt/rain.txt", L"utf-8");
            std::cout << "Rain" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result6 = wcscmp(scattered_clouds, t);
        if (result6 == 0) {
            sbText.Prepend(L" \n");
            sbText.LoadFile(L"qa_data/txt/scattered_clouds.txt", L"utf-8");
            std::cout << "Scattered Clouds" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result7 = wcscmp(shower_rain, t);
        if (result7 == 0) {
            sbText.LoadFile(L"qa_data/txt/shower_rain.txt", L"utf-8");
            std::cout << "Shower Rain" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result8 = wcscmp(snow, t);
        if (result8 == 0) {
            sbText.LoadFile(L"qa_data/txt/snow.txt", L"utf-8");
            std::cout << "Snow" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        int result9 = wcscmp(thunderstorm, t);
        if (result9 == 0) {
            sbText.LoadFile(L"qa_data/txt/thunderstorm.txt", L"utf-8");
            std::cout << "Thunderstorm" << std::endl;
            sbText.Prepend(L" \n");
            const wchar_t *emoji = sb2Text.getAsString();
            sbText.Prepend(L"#HoraEnChile #ChileCurrentTime");
            sbText.Prepend(L" \n");
            sbText.Prepend(horaActual);
            sbText.Prepend(L" \n");
            sbText.Prepend(emoji);
            sbText.Prepend(json.stringOf(L"weather[0].description"));
            sbText.Prepend(L"Cielo: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.humidity"));
            sbText.Prepend(L"Humedad: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.pressure"));
            sbText.Prepend(L"Presión: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"main.temp"));
            sbText.Prepend(L"Temperatura: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"sys.country"));
            sbText.Prepend(L"Pais: ");
            sbText.Prepend(L" \n");
            sbText.Prepend(json.stringOf(L"name"));
            sbText.Prepend(L"Ciudad: ");
            sbText.Prepend(L" \n");
        }
        else
            std::cout << "NO" << std::endl;

        //////////

        // Send a tweet...
        rest.ClearAllQueryParams();
        rest.AddQueryParam(L"status", sbText.getAsString());
        rest.AddQueryParam(L"media_ids", jsonResponse.stringOf(L"media_id_string"));
        const wchar_t *response = rest.fullRequestFormUrlEncoded(L"POST", L"/1.1/statuses/update.json");
        if (rest.get_LastMethodSuccess() != true) {
            wprintf(L"%ls\n", rest.lastErrorText());
            return 0;
        }

        CkJsonObjectW jsonResponse;
        jsonResponse.put_EmitCompact(false);
        jsonResponse.Load(response);

        if (rest.get_ResponseStatusCode() != 200) {
            wprintf(L"%ls\n", jsonResponse.emit());
            return 0;
        }

        // Show the successful response:
        wprintf(L"%ls\n", jsonResponse.emit());
        wprintf(L"Success.\n");
    }

    //////////

    return 0;

}
