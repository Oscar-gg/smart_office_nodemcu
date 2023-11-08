#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include "config.h"
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <functional>

using namespace websockets;

typedef std::function<void(WebsocketsClient &, WebsocketsMessage)> MessageCallback;

class Websockets
{
private:
    WebsocketsClient client;
    void setClientType(String type, String name);

public:
    Websockets();
    void initialize(String type, String name);
    void ping();
    void poll();
    void sendStringResponse(String data, String dataType);
    void sendIntResponse(int data, String dataType);
    void sendFloatResponse(float data, String dataType);
    void onMessage(const MessageCallback callback);
    void onMessage(const PartialMessageCallback callback);
};

static void onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Connnection Opened");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
    }
}

#endif
