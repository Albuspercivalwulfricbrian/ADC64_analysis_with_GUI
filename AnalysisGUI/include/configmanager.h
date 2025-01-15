#ifndef CONFIGMANAGER
#define CONFIGMANAGER

#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

class ConfigManager {
public:
    ConfigManager(int id, const std::string& name, double leftBoundary, double rightBoundary, bool UseSpline, bool UseSmartScope)
        : id(id), name(name), leftBoundary(leftBoundary), rightBoundary(rightBoundary), UseSpline(UseSpline), UseSmartScope(UseSmartScope) {}

    // Метод для сохранения данных в JSON файл
    static void saveToJson(const std::string& filename, const std::unordered_map<int, ConfigManager*>& channels) {
        nlohmann::json j;
        
        for (const auto& channel : channels) {
            j["channels"][std::to_string(channel.first)] = {
                {"name", channel.second->name},
                {"left_boundary", channel.second->leftBoundary},
                {"right_boundary", channel.second->rightBoundary},
                {"Use_Spline", channel.second->UseSpline},
                {"Use_Smart_Area", channel.second->UseSmartScope}
            };
        }

        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4); // Сохранение с отступами
            file.close();
        } else {
            std::cerr << "Не удалось открыть файл для записи: " << filename << std::endl;
        }
    }

    // Метод для загрузки данных из JSON файла
    static std::map<int, ConfigManager*> loadFromJson(const std::string& filename) 
    {
        nlohmann::json j;
        std::ifstream file(filename);
        if (file.is_open()) {
            file >> j;
            file.close();

            std::map<int, ConfigManager*> channels = {};
            for (const auto& item : j["channels"].items()) 
            {
                int id = std::stoi(item.key());
                const auto& channelInfo = item.value();
                channels[id] = new ConfigManager(id, channelInfo["name"].get<std::string>(), 
                channelInfo["left_boundary"].get<double>(), channelInfo["right_boundary"].get<double>(),
                channelInfo["Use_Spline"].get<bool>(),channelInfo["Use_Smart_Area"].get<bool>());
            }
            return channels;
        } else {
            std::cerr << "Не удалось открыть файл для чтения: " << filename << std::endl;
            throw std::runtime_error("Ошибка при загрузке данных");
        }
    }

public:
    int id; // Идентификатор канала
    std::string name;
    double leftBoundary;
    double rightBoundary;
    bool UseSpline;
    bool UseSmartScope;
};

#endif CONFIGMANAGER