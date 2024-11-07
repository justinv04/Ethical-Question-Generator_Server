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

#include "../Definitions/structs.h"

using std::string;
using std::vector;
using std::pair;
using std::cerr;

class JSON_Handler {
    public:
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

        static vector<User> parseUsers(const string& jsonString) {
            rapidjson::Document document;
            vector<User> result = {};

            if(document.Parse(jsonString.c_str()).HasParseError())
                return {};

            if(document.HasMember("Users") && document["Users"].IsArray()) {
                const rapidjson::Value& users = document["Users"];

                for(size_t i = 0; i < users.Size(); ++i) {
                    const rapidjson::Value& user = users[i];
                    User curr_user;

                    curr_user.id = std::atoi(user["User_ID"].GetString());
                    curr_user.name = user["User_Name"].GetString();
                    curr_user.email = user["User_Email"].GetString();
                    curr_user.passhash = user["User_Password_Hash"].GetString();
                    curr_user.date_joined = user["User_Date_Joined"].GetString();

                    result.push_back(curr_user);
                }
            }
            return result;
        }

        static vector<Report> parseReports(const string& jsonString) {
            rapidjson::Document document;
            vector<Report> result = {};

            if(document.Parse(jsonString.c_str()).HasParseError())
                return {};

            if(document.HasMember("Reports") && document["Reports"].IsArray()) {
                const rapidjson::Value& reports = document["Reports"];

                for(size_t i = 0; i < reports.Size(); ++i) {
                    const rapidjson::Value& report = reports[i];
                    Report curr_report;

                    curr_report.id = std::atoi(report["Report_ID"].GetString());
                    curr_report.user_id = std::atoi(report["User_ID"].GetString());
                    curr_report.text = report["Question_Text"].GetString();
                    curr_report.topic = report["Question_Topic"].GetString();
                    curr_report.date = report["Report_Date"].GetString();
                    curr_report.position = (Position)(std::atoi(report["User_Position"].GetString()));

                    result.push_back(curr_report);
                }
            }
            return result;
        }

        static vector<Question> parseQuestions(const string& jsonString) {
            rapidjson::Document document;
            vector<Question> result = {};

            if(document.Parse(jsonString.c_str()).HasParseError())
                return {};

            if(document.HasMember("Questions") && document["Questions"].IsArray()) {
                const rapidjson::Value& reports = document["Questions"];

                for(size_t i = 0; i < reports.Size(); ++i) {
                    const rapidjson::Value& report = reports[i];
                    Question curr_question;

                    curr_question.id = report["Question_ID"].GetInt();
                    curr_question.text = report["Question_Text"].GetString();
                    curr_question.topic = report["Question_Topic"].GetString();
                    
                    result.push_back(curr_question);
                }
            }
            return result;
        }

        template<typename object>
        static vector<pair<string, object>> jsonToPairs(const string& jsonString) {
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

        static string makeError(const string error_str) {return "{\"error\": \"" + error_str + "\"}";}
};

#endif