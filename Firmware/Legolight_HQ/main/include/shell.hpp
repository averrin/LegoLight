#pragma once
#include <lvgl.h>
#include <map>
#include <string>

class Shell {
    std::map<std::string,lv_obj_t*> screens;
public:
    std::string currentScreen;
    void addScreen(std::string key, lv_obj_t *screen) {
        screens[key] = screen;
    }
    void activate(std::string key) {

        for(auto [k, screen] : screens) {
            lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_clear_flag(screens[key], LV_OBJ_FLAG_HIDDEN);
        currentScreen = key;
    }
};