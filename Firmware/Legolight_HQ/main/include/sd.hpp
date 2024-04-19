#pragma once
#include <Arduino.h>
#include <lvgl.h>

static bool mountSD = false;

void updateSD(lv_timer_t * task) {
    // auto label1 = lv_obj_get_child(sd_widget, 0);
}

void initSD() {
    int retry = 3;
    while (retry--) {
        mountSD =  amoled.installSD();
        if (mountSD) {
            break;
        }
    }

    auto label1 = lv_obj_get_child(sd_widget, 0);
    if (mountSD) {
        Serial.println("SD card installed successfully");
        lv_label_set_text_fmt(label1, "#222222 %s# %s", LV_SYMBOL_SD_CARD, "Mounted");
    } else {
        Serial.println("SD card installed failed");
        lv_label_set_text_fmt(label1, "#aa0000 %s# %s", LV_SYMBOL_SD_CARD, "Not mounted");
    }

    // lv_timer_create(updateSD, 500, nullptr);
}


lv_obj_t * sdWidget(lv_obj_t *parent) {
    auto w = textWidget(parent);
    lv_label_set_text_fmt(lv_obj_get_child(w, 0), "#222222 %s# Connecting...", LV_SYMBOL_SD_CARD);
    return w;
}