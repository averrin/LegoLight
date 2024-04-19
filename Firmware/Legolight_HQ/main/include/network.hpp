#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>

void WiFiEvent(WiFiEvent_t event)
{
    // Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        lv_msg_send(WIFI_MSG_ID, NULL);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        lv_msg_send(WIFI_MSG_ID, NULL);
        auto label1 = lv_obj_get_child(wifi_widget, 0);
        lv_label_set_text_fmt(label1, "#222222 %s# %s", LV_SYMBOL_WIFI, WiFi.SSID());
        break;
}
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:{
        Serial.println("Lost IP address and IP address is reset to 0");
        lv_msg_send(WIFI_MSG_ID, NULL);
        auto label1 = lv_obj_get_child(wifi_widget, 0);
        lv_label_set_text_fmt(label1, "#ff0000 %s# Connection lost", LV_SYMBOL_WARNING);
        break;
}
    default: break;
    }
}


void initWiFi(const char* ssid, const char* password) {

    Serial.println("WiFi initiating...");

    WiFi.onEvent(WiFiEvent);
    WiFi.begin(ssid, password);
}

lv_obj_t * wifiWidget(lv_obj_t *parent) {
    auto w = textWidget(parent);
    lv_label_set_text_fmt(lv_obj_get_child(w, 0), "#ff0000 %s# Connecting...", LV_SYMBOL_WARNING);
    return w;
}