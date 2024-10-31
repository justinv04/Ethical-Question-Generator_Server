#ifndef JSON_HANDLER_CPP
#define JSON_HANDLER_CPP

#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>
#include <chrono>

#include <mysql.h>
#include <mysqld_error.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

using std::string;
using std::vector;
using std::pair;
using std::cerr;

class JSON_Handler {
    public:

    JSON_Handler() {}

    static string get_MYSQL_JSON(MYSQL_RES* result) {
        MYSQL_ROW row;
        MYSQL_FIELD* fields = mysql_fetch_fields(result);
        size_t num_fields = mysql_num_fields(result);

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Value tableArray(rapidjson::kArrayType);
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        rapidjson::Value tableName(fields[0].table, allocator);

        while((row = mysql_fetch_row(result)) != nullptr) {
            rapidjson::Value rowObject(rapidjson::kObjectType);

            for(size_t i = 0; i < num_fields; ++i) {
                rapidjson::Value key(fields[i].name, allocator);
                rapidjson::Value value((row[i] ? row[i] : "N/A"), allocator);
                rowObject.AddMember(key, value, allocator);
            }

            tableArray.PushBack(rowObject, allocator);
        }

        document.AddMember(tableName, tableArray, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        return string(buffer.GetString());
    }

    static vector<vector<string>> parseUsers(const string& jsonString) {
        rapidjson::Document document;
        vector<vector<string>> result = {};

        if(document.Parse(jsonString.c_str()).HasParseError())
            return {};

        if(document.HasMember("Users") && document["Users"].IsArray()) {
            const rapidjson::Value& users = document["Users"];

            for(size_t i = 0; i < users.Size(); ++i) {
                const rapidjson::Value& user = users[i];
                result.push_back({});

                result.at(i).push_back(user["User_ID"].GetString());
                result.at(i).push_back(user["User_Name"].GetString());
                result.at(i).push_back(user["User_Email"].GetString());
                result.at(i).push_back(user["User_Password_Hash"].GetString());
                result.at(i).push_back(user["User_Date_Joined"].GetString());
            }
        }
        return result;
    }

    template<typename object>
    static vector<pair<string, object>> parseToPairs(const string& jsonString) {
        vector<pair<string, object>> pairs;
        rapidjson::Document document;

        if (document.Parse(jsonString.c_str()).HasParseError()) {
            cerr << "Error parsing JSON: " << rapidjson::GetParseErrorFunc(document.GetParseError()) << " (at " << document.GetErrorOffset() << ")\n";
            return {};
        }

        if (!document.IsObject()) {
            cerr << "JSON is not an object\n";
            return {};
        }

        for(const auto& member: document.GetObject())
            pairs.emplace_back(member.name.GetString(), static_cast<object>(member.value.GetString()));

        return pairs;
    }

    template<typename object>
    static string pairsToJSON(const vector<pair<string, object>>& pairs) {
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        for (const auto& pair : pairs) {
            rapidjson::Value key(pair.first.c_str(), allocator);
            rapidjson::Value value;

            value.SetString(pair.second.c_str(), allocator);
            document.AddMember(key, value, allocator);
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        return buffer.GetString();
    }

    static string make_Error(const string error_str) {return "{\"error\": \"" + error_str + "\"}";}

    static string make_Response(const int status, const string status_code, const string content_type, const string content) {
        string response = "HTTP/1.1 " + std::to_string(status) + " " + status_code + "\r\n";
            
        if(status != 204) 
            response += ("Content-Type: " + content_type + "\r\n");
        
        response += "Access-Control-Allow-Origin: http://127.0.0.1:8081\r\n" // Global Defintion
                    "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";

        response += ("Content-Length: " + std::to_string(content.length()) + "\r\nConnection: keep-alive\r\n\r\n" + content);

        return response;
    }
};

#endif