
#include <Arduino.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <AceButton.h>
#include <lvgl.h>
#include <misc/lv_timer.h>
#include <iomanip>
#include <sstream>

LilyGo_Class amoled;

#include "include/lv_helpers.hpp"
#include "include/shell.hpp"
Shell shell;

#define WIFI_MSG_ID 0x1001
#define I2C_ADDR_BASE 0x20
#define I2C_ADDR_COUNT 4

#include "include/config.hpp"
Config config;

#include "include/i2c_bus.hpp"
std::vector<Device> found_devices;

lv_obj_t *device_img;
lv_obj_t *button_back;
lv_obj_t *button_forward;
lv_obj_t *button_toggle;
lv_obj_t *device_widget;
lv_obj_t *device_name;
lv_obj_t *button_rgb;
lv_obj_t *table;

lv_style_t style_btn;
lv_style_t style_btn_checked;

lv_obj_t *wifi_widget;
#include "include/network.hpp"

lv_obj_t *clock_widget;
#include "include/clock.hpp"

lv_obj_t *sd_widget;
#include "include/sd.hpp"

int device_index = 0;
bool legolight_enabled = false;

using namespace ace_button;
uint8_t btnPin = 0;
AceButton button(btnPin);
bool sleep_enabled = false;

void toggleSleep()
{
    sleep_enabled = !sleep_enabled;
    auto b = 255;
    if (sleep_enabled)
        b = 0;
    amoled.setBrightness(b);
}

void handleEvent(AceButton * /* button */, uint8_t eventType,
                 uint8_t /* buttonState */)
{
    switch (eventType)
    {
    case AceButton::kEventPressed:
    {
        Serial.println("button");
        toggleSleep();
        break;
    }
    default:
        break;
    }
}

void topBar(lv_obj_t *parent)
{
    clock_widget = clockWidget(parent);
    lv_obj_set_grid_cell(clock_widget,
                         LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    wifi_widget = wifiWidget(parent);
    lv_obj_set_grid_cell(wifi_widget,
                         LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    sd_widget = sdWidget(parent);
    lv_obj_set_grid_cell(sd_widget,
                         LV_GRID_ALIGN_STRETCH, 2, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
}

void setDevice(int di)
{
    auto device = found_devices[di].meta;

    lv_label_set_text_fmt(lv_obj_get_child(device_widget, 0), device.name.c_str());
    lv_obj_center(lv_obj_get_child(device_widget, 0));
    lv_img_set_src(device_img, device.image.c_str());
    lv_obj_clear_flag(button_back, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(button_forward, LV_OBJ_FLAG_HIDDEN);

    if (di == 0)
    {
        lv_obj_add_flag(button_back, LV_OBJ_FLAG_HIDDEN);
    }
    if (di == found_devices.size() - 1)
    {
        lv_obj_add_flag(button_forward, LV_OBJ_FLAG_HIDDEN);
    }
}

static void screen_handler(lv_event_t *e)
{
    // lv_event_code_t code = lv_event_get_code(e);
    // if (code == LV_EVENT_CLICKED)
    // {
    if (sleep_enabled)
    {
        toggleSleep();
    }
    // }
}

static void button_forward_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        device_index++;
        device_index %= found_devices.size();
        setDevice(device_index);
    }
}

static void button_back_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        device_index--;
        // device_index %= config.devices.size();
        setDevice(device_index);
    }
}

static void button_toggle_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    auto bt = "OFF";
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        legolight_enabled = !legolight_enabled;
        if (legolight_enabled)
        {
            bt = "ON";
        }
        lv_label_set_text_fmt(lv_obj_get_child(button_toggle, 0), bt);
    }
}

void configDevice()
{
    auto device = found_devices[device_index];
    lv_label_set_text_fmt(lv_obj_get_child(device_name, 0), device.meta.name.c_str());

    lv_obj_clear_state(button_rgb, LV_STATE_CHECKED);
    if(device.rgb_enabled) {
        lv_obj_add_state(button_rgb, LV_STATE_CHECKED);
    }

    for (auto p = 0; p < 16; p++)
    {
        auto b = lv_obj_get_child(table, p);
        lv_obj_clear_state(b, LV_STATE_CHECKED);
        if(p < 8 && ((device.led_state0 >> 7-p) & 1)) {
            lv_obj_add_state(b, LV_STATE_CHECKED);
        }
        if(p >= 8 && ((device.led_state1 >> (15-p)) & 1)) {
            lv_obj_add_state(b, LV_STATE_CHECKED);
        }
    }
}

static void button_config_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        configDevice();
        shell.activate("device_config");
    }
}

void deviceSelector(lv_obj_t *parent)
{
    config = initConfig();
    found_devices = findDevices();

    for (auto &device : found_devices)
    {
        for (auto c : config.devices)
        {
            if (c.address == device.address)
            {
                device.meta = c;
                break;
            }
        }
    }

    lv_obj_t *led3 = lv_led_create(parent);
    lv_led_set_color(led3, lv_palette_main(LV_PALETTE_GREEN));
    lv_led_on(led3);
    lv_obj_set_grid_cell(led3,
                         LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    button_toggle = insertButton(parent, "OFF", 2, 1);
    lv_obj_add_flag(button_toggle, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_event_cb(button_toggle, button_toggle_handler, LV_EVENT_ALL, NULL);

    lv_obj_add_style(button_toggle, &style_btn, 0);
    lv_obj_add_style(button_toggle, &style_btn_checked, LV_STATE_CHECKED);

    device_widget = textWidget(parent);
    lv_obj_set_grid_cell(device_widget,
                         LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    device_img = lv_img_create(parent);
    button_back = insertButton(parent, LV_SYMBOL_LEFT, 0, 3);
    lv_obj_add_event_cb(button_back, button_back_handler, LV_EVENT_ALL, NULL);
    auto button_config = insertButton(parent, "Config", 1, 3);
    lv_obj_add_event_cb(button_config, button_config_handler, LV_EVENT_ALL, NULL);
    button_forward = insertButton(parent, LV_SYMBOL_RIGHT, 2, 3);
    lv_obj_add_event_cb(button_forward, button_forward_handler, LV_EVENT_ALL, NULL);

    lv_obj_set_grid_cell(device_img,
                         LV_GRID_ALIGN_CENTER, 0, 3,
                         LV_GRID_ALIGN_CENTER, 2, 1);

    setDevice(device_index);
}

static void button_home_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        shell.activate("home");
    }
}

lv_obj_t *homeScreen(lv_obj_t *parent)
{

    static lv_coord_t col_dsc[] = {130, 130, 130, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {54, 54, 380, 54, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *cont = lv_obj_create(parent);
    shell.addScreen("home", cont);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, amoled.width(), amoled.height());
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_set_style_pad_row(cont, 8, 0);
    lv_obj_set_style_pad_column(cont, 8, 0);

    topBar(cont);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    return cont;
}

lv_obj_t *deviceConfigScreen(lv_obj_t *parent)
{

    static lv_coord_t col_dsc[] = {130, 130, 130, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {54, 440, 54, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *cont = lv_obj_create(parent);
    shell.addScreen("device_config", cont);
    lv_obj_set_size(cont, amoled.width(), amoled.height());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(cont, 8, 0);
    lv_obj_set_style_pad_column(cont, 8, 0);

    device_name = textWidget(cont);

    lv_obj_set_grid_cell(device_name,
                         LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    button_rgb = insertButton(cont, "RGB", 2, 0);
    lv_obj_add_flag(button_rgb, LV_OBJ_FLAG_CHECKABLE);

    lv_obj_add_style(button_rgb, &style_btn, 0);
    lv_obj_add_style(button_rgb, &style_btn_checked, LV_STATE_CHECKED);

    auto button_back = insertButton(cont, LV_SYMBOL_LEFT, 0, 2);
    lv_obj_add_event_cb(button_back, button_home_handler, LV_EVENT_ALL, NULL);

    insertButton(cont, LV_SYMBOL_SHUFFLE, 1, 2);
    insertButton(cont, LV_SYMBOL_SAVE, 2, 2);

    lv_coord_t bs = 82;
    int pad = 12;

    static lv_coord_t dsc[] = {bs, bs, bs, bs, LV_GRID_TEMPLATE_LAST};

    table = lv_obj_create(cont);
    lv_obj_set_style_grid_column_dsc_array(table, dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(table, dsc, 0);
    lv_obj_set_layout(table, LV_LAYOUT_GRID);

    lv_obj_set_style_pad_row(table, pad, 0);
    lv_obj_set_style_pad_column(table, pad, 0);

    for (auto p = 0; p < 16; p++)
    {
        std::stringstream stream;
        stream << std::hex << p;
        std::string result(stream.str());
        auto b = insertButton(table, result, p % 4, p / 4);
        lv_obj_add_flag(b, LV_OBJ_FLAG_CHECKABLE);

        lv_obj_add_style(b, &style_btn, 0);
        lv_obj_add_style(b, &style_btn_checked, LV_STATE_CHECKED);

    }

    lv_obj_set_grid_cell(table,
                         LV_GRID_ALIGN_STRETCH, 0, 3,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    return cont;
}

void setup(void)
{
    Serial.begin(115200);

    bool rslt = false;

    rslt = amoled.begin();

    if (!rslt)
    {
        while (1)
        {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }
    Serial.println("LegoLight starting...");

    // Register lvgl helper
    beginLvglHelper(amoled);
    setRotation();

    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x555555));
    lv_style_init(&style_btn_checked);
    lv_style_set_bg_color(&style_btn_checked, lv_palette_main(LV_PALETTE_BLUE));

    lv_obj_add_event_cb(lv_scr_act(), screen_handler, LV_EVENT_GESTURE, NULL);

    auto home = homeScreen(lv_scr_act());
    auto deviceConfig = deviceConfigScreen(lv_scr_act());

    Serial.println("LegoLight services starting...");
    initWiFi("Averrin", "oWertryN8.");
    initClock();
    initSD();

    if (mountSD)
    {
        deviceSelector(home);
    }
    else
    {
        auto warn = textWidget(home);
        lv_label_set_text_fmt(lv_obj_get_child(warn, 0), "Please insert SD card with config");
        lv_obj_set_grid_cell(warn,
                             LV_GRID_ALIGN_CENTER, 0, 3,
                             LV_GRID_ALIGN_CENTER, 2, 1);
    }
    shell.activate("home");

    pinMode(btnPin, INPUT_PULLUP);
    button.setEventHandler(handleEvent);
    Serial.println("LegoLight started.");
}

void loop()
{
    lv_task_handler();
    button.check();
}
