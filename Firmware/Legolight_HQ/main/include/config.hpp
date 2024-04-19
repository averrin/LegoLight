#pragma once
#include <vector>
#include <string>
#include <SD.h>
#include <ArduinoJson.h>

const char* config_path = "/Legolight/config.json";

struct DeviceConfig {
    uint8_t address;
    std::string name;
    std::string image;
    // int image_height;
};

struct Config {
    std::vector<DeviceConfig> devices;
};

Config initConfig() {
    Config config;
    File file = SD.open(config_path);
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error)
        Serial.println(F("Failed to read file, using default configuration"));

    JsonArray devices = doc["devices"].as<JsonArray>();
    for (auto dc : devices) {
        DeviceConfig device;
        device.address = dc["address"].as<uint8_t>();
        // device.image_height = dc["image_height"].as<int>();
        device.name = dc["name"].as<std::string>();
        device.image = "A:" + dc["image"].as<std::string>();
        config.devices.push_back(device);
    }
    return config;
}