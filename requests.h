#ifndef REQUESTS_H
#define REQUESTS_H

#include <iostream>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

//#include <boost/property_tree/ini_parser.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>

#include <boost/exception/all.hpp>
#include <boost/stacktrace.hpp> //?

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <nlohmann/json.hpp>

struct ConfigError : public std::runtime_error
{
    explicit ConfigError(const std::string& msg) : std::runtime_error(msg) {}
};

class HTTP
{
private:
    int http_ver;
    std::string host;
    std::string port;
    std::string bucket_name;
    bool auth = false;
    std::string login_password_base64;

    void load_config(const std::string& path)
    {
        std::ifstream f(path);
        if (!f)
        {
            throw ConfigError("Cannot open config file: " + path);
        }

        nlohmann::json j;
        try
        {
            f >> j; // парсим json
        }
        catch (const nlohmann::json::parse_error& e)
        {
            // Ошибка синтаксиса JSON
            throw ConfigError(std::string("JSON parse error in '") + path + "': " + e.what());
        }

        // Проверяем наличие обязательных полей.
        if (!j.contains("http_ver"))
            throw ConfigError("Missing required field 'http_ver' in config file");
        if (!j.contains("host"))
            throw ConfigError("Missing required field 'host' in config file");
        if (!j.contains("port"))
            throw ConfigError("Missing required field 'port' in config file");
        if (!j.contains("bucket_name"))
            throw ConfigError("Missing required field 'bucket_name' in config file");
        if (!j.contains("auth"))
            throw ConfigError("Missing required field 'auth' in config file");

        //  Читаем значения
        try
        {
            this->http_ver = j.at("http_ver").get<int>();
            this->host = j.at("host").get<std::string>();
            this->port = j.at("port").get<std::string>();
            this->bucket_name = j.at("bucket_name").get<std::string>();
            this->auth = j.at("auth").get<bool>();
            if (this->auth)
            {
                if (!j.contains("authorization"))
                {
                    throw ConfigError("Missing required field 'authorization' in config file");
                }
                this->login_password_base64 = j.at("authorization").get<std::string>();
            }
        }
        catch (const nlohmann::json::type_error& e)
        {
            throw ConfigError(std::string("Type mismatch in config file: ") + e.what());
        }
    }

public:
    // Если путь не указан – берём файл "config.json" в текущей директории.
    explicit HTTP(const std::string& cfg_path = "config.json")
    {
        load_config(cfg_path);
    }
    //// основной запрос
    bool S3_PutJsonObject(std::string_view file_content);
};


#endif // REQUESTS_H
