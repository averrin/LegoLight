#pragma once
#include <Arduino.h>
#include <lvgl.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#define LOCATION_TZ 2
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 60 * 60 * LOCATION_TZ);

void updateTime(lv_timer_t * task) {
    timeClient.update();
        auto label1 = lv_obj_get_child(clock_widget, 0);
        lv_label_set_text_fmt(label1, timeClient.getFormattedTime().c_str());
}

void initClock() {
    timeClient.begin();
    timeClient.forceUpdate();

    lv_timer_create(updateTime, 500, nullptr);
}

lv_obj_t * clockWidget(lv_obj_t *parent) {
    auto w = textWidget(parent);
    lv_label_set_text_fmt(lv_obj_get_child(w, 0), "12:00:00");
    lv_obj_center(lv_obj_get_child(w, 0));
    return w;
}
