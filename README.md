
# Pocketbase ESP32 

![![Logo](logo.svg)](logo.svg)


A fork of [pocketbase extended](https://github.com/PocketBaseExtended/PocketbaseExtended) to run on ESP32 devices.

It uses the platformIO framework to build the project. But it can be used with any other ESP32 project.


## Features

- A lot of fixes over the original Pocketbase Extended.
- Subscription support.
- Account management (with custom collections).
- Set/Update/Create records.
- Filters

# Example 

Example are sourced from a project made for our school. 


## Login 
`
```cpp
#include <PocketbaseESP32.h>

PocketbaseArduino pb("https://deathstar.cyp.sh/");
pb.login_passwd(USER_NAME, USER_PASSWORD, "launchers"); // using the collection "launchers" as the user collection
```

## Update 

```cpp
// update the connection record to put the device online
auto& rec = pb.getConnectionRecord(); 
rec["record"]["online"] = true;
String body = rec["record"].as<String>();
pb.update("launchers", rec["record"]["id"].as<String>(), body);
```

## Filtering 
```cpp
String request =  String("(should_load=true&&loaded_at=\"\")");
String result = pb.collection("launches").getList("1",            // page
                                                  "20",           // perPage
                                                  nullptr,        // sort
                                                  request.c_str() // filter
                                                    );
```

## Subscription
```cpp
pb.subscribe("launches", "*", [](SubscriptionEvent &ev, void *ctx){
    Serial.printf("Event: %s, Record: %s\n", ev.event.c_str(), ev.data.c_str());
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, ev.data);

    if (doc["record"]["launcher"] == "Ariane")
    {
        Serial.println("Ariane launch detected!");
    }
);
```

