#include "pocketbase.hpp"

#include <json_parser.h>

#include <ArduinoJson.h>

#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

int post_func(HTTPClient &client, const String &payload)
{
    // client.addHeader("Content-Type", "application/json");
    int httpCode = client.POST(payload);
    if (httpCode > 0)
    {
        Serial.println("response code: " + String(httpCode));
        Serial.println("Response from server:");

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED)
        {
            return httpCode; // Success
        }
        else if (httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String newLocation = client.header("Location");
            Serial.printf("Redirected to: %s\n", newLocation.c_str());
            // Optionally, you can follow the redirect
            client.begin(newLocation);
            httpCode = client.GET();
            if (httpCode > 0)
            {
                return httpCode; // Assuming the redirect is successful
            }
            else
            {
                Serial.printf("Redirect failed: %s\n", client.errorToString(httpCode).c_str());
            }
        }
        else
        {
            Serial.printf("HTTP error: %d\n", httpCode);
        }
    }
    else
    {
        Serial.printf("HTTP request failed: %s\n", client.errorToString(httpCode).c_str());
    }

    return httpCode;
}

int get_func(HTTPClient &client)
{

    // print raw header

    int httpCode = client.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            return httpCode; // Success
        }
        else
        {
            Serial.printf("HTTP error: %d\n", httpCode);
        }
    }
    else
    {
        Serial.printf("HTTP request failed: %s\n", client.errorToString(httpCode).c_str());
    }

    return httpCode;
}

DynamicJsonDocument PocketbaseConnection::login_passwd(const char *username, const char *password, const char *collection)
{

    String endpoint = base_url + "collections/" + String(collection) + "/auth-with-password";
    // std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
    // client->setInsecure();

    HTTPClient http;

    http.begin(*client, endpoint);

    http.addHeader("Content-Type", "application/json");

    String payload = "{\"identity\":\"" + String(username) + "\",\"password\":\"" + String(password) + "\"}";
    http.setTimeout(10000); // Set timeout to 10 seconds
    int httpCode = post_func(http, payload);

    if (httpCode > 0)
    {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println("Endpoint: " + endpoint);
        Serial.println(response);
        Serial.println("HTTP Code: " + String(httpCode));
        if (httpCode == HTTP_CODE_OK)
        {
            // Parse the response to extract auth_user and auth_token
            // Assuming the response is in JSON format

            Serial.println("Login successful, parsing response...");
            DynamicJsonDocument doc(1024);
            Serial.println("Parsing JSON response...");
            DeserializationError error = deserializeJson(doc, response);
            Serial.println("Deserialization complete.");
            if (!error)
            {

                auth_token = doc["token"].as<String>();
                Serial.printf("Login successful. User: %s, Token: %s\n", username, auth_token.c_str());
                http.end();
                return doc;
            }
            else
            {
                Serial.printf("JSON parse error: %s\n", error.c_str());
            }
        }

        else
        {
            Serial.printf("HTTP error: %d\n", httpCode);
        }
        http.end();
    }
    else
    {
        Serial.printf("HTTP request failed: %s\n", http.errorToString(httpCode).c_str());
    }
    return DynamicJsonDocument(0); // Return false if login fails
}

String PocketbaseConnection::performGETRequest(const char *endpoint)
{
#if defined(ESP32)
    if (strncmp(endpoint, "https", 5) == 0)
    {
        //   std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
        //    client->setInsecure();
        HTTPClient https;
        https.setTimeout(10000); // Set timeout to 10 seconds
      //  Serial.println("token: " + auth_token);

        Serial.print("[HTTPS] Full URL: ");
        Serial.println(endpoint);
       
        if (https.begin(*client, endpoint))
        {
            https.addHeader("Authorization", auth_token.c_str());

            Serial.print("[HTTPS] GET...\n");
            int httpCode = get_func(https);
            if (httpCode > 0)
            {
                Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                String payload = https.getString();
                // print request contents (must be removed)
                //  Serial.println(payload);
                https.end();
                return payload;
            }
            else
            {
                Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
        // TODO: improve return value in case failure happens
        return ""; // Return an empty string on failure
    }
    else
    {
        Serial.print("[HTTP] Full URL: ");
        Serial.println(endpoint);

        HTTPClient http;
        Serial.print("[HTTP] begin...\n");

        if (http.begin(endpoint))
        {
            http.addHeader("Authorization", auth_token.c_str());
            //  http.setAuthorization(auth_token.c_str());

            Serial.print("[HTTP] GET...\n");
            int httpCode = http.GET();
            if (httpCode > 0)
            {
                Serial.printf("[HTTP] GET... code: %d\n", httpCode);
                // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                if (httpCode)
                {
                    String payload = http.getString();
                    Serial.println(payload);
                    http.end();
                    return payload;
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        else
        {
            Serial.printf("[HTTP] Unable to connect\n");
        }
        // TODO: improve return value in case failure happens
        return ""; // Return an empty string on failure
    }
#endif
}

String PocketbaseConnection::performDELETERequest(const char *endpoint)
{
#if defined(ESP32)

    if (strncmp(endpoint, "https", 5) == 0)
    {
        //  std::unique_ptr<WiFiClientSecure> client(new WiFiClientSecure);
        //  client->setInsecure();
        HTTPClient https;

        Serial.print("[HTTPS] Full URL: ");
        Serial.println(endpoint);

        if (https.begin(*client, endpoint))
        {
            Serial.print("[HTTPS] DELETE...\n");
            int httpCode = https.sendRequest("DELETE");
            if (httpCode > 0)
            {
                Serial.printf("[HTTPS] DELETE... code: %d\n", httpCode);
                // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                if (httpCode)
                {
                    String payload = https.getString();
                    // print request contents (must be removed)
                    Serial.println(payload);
                    https.end();
                    return payload;
                }
            }
            else
            {
                Serial.printf("[HTTPS] DELETE... failed, error: %s\n", https.errorToString(httpCode).c_str());
            }
            https.end();
        }
        else
        {
            Serial.printf("[HTTPS] Unable to connect\n");
        }
        // TODO: improve return value in case failure happens
        return ""; // Return an empty string on failure
    }
    else
    {
        Serial.print("[HTTP] Full URL: ");
        Serial.println(endpoint);

        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        if (http.begin(endpoint))
        {
            Serial.print("[HTTP] DELETE...\n");
            int httpCode = http.sendRequest("DELETE");
            if (httpCode > 0)
            {
                Serial.printf("[HTTP] DELETE... code: %d\n", httpCode);
                // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                if (httpCode)
                {
                    String payload = http.getString();
                    Serial.println(payload);
                    http.end();
                    return payload;
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        }
        else
        {
            Serial.printf("[HTTP] Unable to connect\n");
        }
        // TODO: improve return value in case failure happens
        return ""; // Return an empty string on failure
    }
#endif
}

String PocketbaseConnection::performPOSTRequest(const char *endpoint, const String &requestBody)
{
#if defined(ESP32)
    HTTPClient http;

    Serial.print("[HTTPS] Full URL: ");
    Serial.println(endpoint);

    if (http.begin(*client, endpoint))
    {
        Serial.print("[HTTPS] POST...\n");

        int httpCode = post_func(http, requestBody);
        if (httpCode > 0)
        {
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
            if (httpCode)
            {
                String payload = http.getString();
                Serial.println(payload);
                http.end();
                return payload;
            }
        }
        else
        {
            Serial.printf("[HTTPS] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    else
    {
        Serial.printf("[HTTPS] Unable to connect\n");
    }

    // TODO: improve return value in case of failure
    return ""; // Return an empty string on failure
#endif
}
