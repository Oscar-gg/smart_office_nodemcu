#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <functional>

using namespace websockets;

typedef std::function<void(WebsocketsClient &, WebsocketsMessage)> MessageCallback;

class Websockets
{
private:
    WebsocketsClient client;
    String connectionString;
    void setClientType(String type, String name);
    void sendResponse(DynamicJsonDocument doc, String dataType, String id);

public:
    Websockets(String connectionString);
    void initialize(String type, String name);
    void ping();
    void poll();

    void sendResponse(String data, String dataType, String id = "");
    void sendResponse(float data, String dataType, String id = "");

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
