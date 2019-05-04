//
// Created by yihan on 2/15/19.
//

#ifndef SPACEHUB_CONFIG_READER_H
#define SPACEHUB_CONFIG_READER_H

#include <fstream>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <variant>

namespace SpaceH {

    enum class ConfigDtype{
        Integer,Float, String, Empty
    };

    auto classify_string(const std::string& s)
    {
        bool with_point{false};
        auto it = s.begin();
        while (it != s.end() && std::isdigit(*it)) {
            if(*it == '.')
                with_point = true;
            ++it;
        }

        if(s.empty()){
            return ConfigDtype::Empty;
        } else {
            if(it != s.end()){
                return ConfigDtype::String;
            } else {
                if(with_point){
                    return ConfigDtype::Float;
                } else {
                    return ConfigDtype::Integer;
                }
            }
        }
    }

    class ConfigReader {
    public:
        explicit ConfigReader(std::string const &file_name, char divider = '=', char commenter = '#') {
            std::fstream file(file_name);
            if (file.is_open()) {
                std::string line;
                while (std::getline(file, line)) {
                    line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

                    if (line[0] == commenter || line.empty())
                        continue;

                    auto split = line.find(divider);
                    auto key = line.substr(0, split);
                    auto val = line.substr(split + 1);
                    map_[key] = val;
                }
            } else {
                std::cout << "Cannot open the configure file: " << file_name << "\r\n";
                exit(0);
            }
            file.close();
        }

        explicit ConfigReader(char const *file_name, char divider = '=', char commenter = '#')
                : ConfigReader(std::string(file_name), divider, commenter) {};

        template<typename T>
        T get(std::string const &key) {
            auto iskey = (map_.end() != map_.find(key));
            if (iskey) {
                std::stringstream ss(map_[key]);
                T value;
                ss >> value;
                return value;
            } else {
                std::cout << "Invalid key for configure file!\n";
                exit(0);
            }
        }

        std::variant<double,int,std::string> auto_get(std::string const &key) {
            auto iskey = (map_.end() != map_.find(key));
            if (iskey) {
                auto type = classify_string(key);
                if(type == ConfigDtype::Integer){
                    return std::stoi(map_[key]);
                } else if(type == ConfigDtype::Float){
                    return std::stod(map_[key]);
                } else {
                    return map_[key];
                }
            } else {
                std::cout << "Invalid key for configure file!\n";
                exit(0);
            }
        }
    private:
        std::unordered_map<std::string, std::string> map_;
    };

    template <typename ...Args>
    auto config_map(ConfigReader & config, Args const & ...args) {
        std::make_tuple(config.auto_get(args)...);
    }
    
#define CONFIG_MAPPING(FILE_NAME,...) SpaceH::ConfigReader config(FILE_NAME);                                                      \
auto[__VA_ARGS__] = SpaceH::config_map(config, #__VA_ARGS__);

    template<typename ...Args>
    void read_command_line(int argc, char **argv, Args &&...args) {
        constexpr size_t n = sizeof...(Args);

        if (argc != n + 1) {
            std::cout << "Wrong args number!\n";
            exit(0);
        } else {
            std::stringstream ss;
            for (int i = 1; i < argc; ++i) {
                ss << argv[i] << ' ';
            }
            ((ss >> std::forward<Args>(args)),...);
        }
    }
}


#endif //SPACEHUB_CONFIG_READER_H
