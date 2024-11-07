#ifndef DATABASE_HANDLER_CPP
#define DATABASE_HANDLER_CPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include <utility>

#include <mysql.h>
#include <mysqld_error.h>

#include "JSON_Handler.cpp"
#include "Auth_Handler.cpp"

using std::string;
using std::vector;
using std::pair;

static const int    PORT = 3306;
static const char   *HOST = "localhost",
                    *USER = "root",
                    *PASSWORD = "mysqlcF7MRBtb8244!165",
                    *DATA = "EQG_Database";



class Database_Handler {
    private:
        static bool openConnection(MYSQL** connection) {
            if(!(*connection = mysql_init(0))) {
                mysql_close(*connection);
                return false;
            }
            if(!mysql_real_connect(*connection, HOST, USER, PASSWORD, DATA, PORT, NULL, 0)) {
                mysql_close(*connection);
                return false;
            }
            return true;
        }

        static string getQueryJSON(const char* query) {
            MYSQL* connection;
            string result_data, error_message;

            if(!openConnection(&connection))
                return JSON_Handler::makeError("Query Failed: Unable to connect to server");

            if(mysql_query(connection, query)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::makeError("Query Failed: " + error_message);
            }

            MYSQL_RES* result = mysql_store_result(connection);
            if(result == nullptr) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::makeError("Query Failed: " + error_message);
            }

            result_data = JSON_Handler::get_MYSQL_JSON(result);

            mysql_free_result(result);
            mysql_close(connection);

            return result_data;
        };

        static string getQueryJSONAfterAction(const char* query, const char* action) {
            MYSQL* connection;
            string result_data, error_message;

            if(!openConnection(&connection))
                return JSON_Handler::makeError("Query Failed: Unable to connect to server");

            if(mysql_query(connection, action)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::makeError("Query Failed: " + error_message);
            }

            if(mysql_query(connection, query)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::makeError("Query Failed: " + error_message);
            }

            MYSQL_RES* result = mysql_store_result(connection);
            if(result == nullptr) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::makeError("Query Failed: " + error_message);
            }

            result_data = JSON_Handler::get_MYSQL_JSON(result);

            mysql_free_result(result);
            mysql_close(connection);

            return result_data;
        }

    public:
        static string loginUser(const string username, const string passhash) {
            string query =  "SELECT * "
                            "FROM Users "
                            "WHERE User_Name = '" + username + "' AND User_Password_Hash = '" + passhash + "'";

            vector<User> data = JSON_Handler::parseUsers(getQueryJSON(query.c_str()));

            if(data.size() == 1 && data[0].name == username && data[0].passhash == passhash)
                return Auth_Handler::createJWT(data[0].id, data[0].name);
            
            return "{\"Auth_Token\":\"Failed_Login\"}";
        };

        static string createUser(const string name, const string email, const string passhash) {
            std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm localTime;

            localtime_s(&localTime, &currentTime);
            std::ostringstream date_stream;

            date_stream << std::put_time(&localTime, "%Y/%m/%d");

            string date_joined = date_stream.str();

            string query =  "SELECT * "
                            "FROM Users "
                            "WHERE User_Name = '" + name + "';";

            string action = "INSERT INTO Users "
                            "(User_Name, User_Email, User_Password_Hash, User_Date_Joined) "
                            "VALUES ('"
                            + name + "', '"
                            + email + "', '" 
                            + passhash + "', '" 
                            + date_joined + 
                            "');";

            string query_json = getQueryJSONAfterAction(query.c_str(), action.c_str());

            vector<User> data = JSON_Handler::parseUsers(query_json);

            if(data.size() == 1)
                return Auth_Handler::createJWT(data[0].id, name);
            
            return "{\"Auth_Token\":\"Failed_Creation\"}";
        };

        static string getAllUserReports(const size_t user_id) {
            string query =  "SELECT Reports.Report_ID, Reports.User_ID, Questions.Question_Text, Questions.Questions_Topics "
                            "FROM Reports "
                            "INNER JOIN Questions ON Questions.Question_ID = Reports.Question_ID "
                            "WHERE User_ID = " + std::to_string(user_id) + " "
                            "ORDER BY Report_Date DESC;";

            string query_json = getQueryJSON(query.c_str());
            if(query_json.substr(0, 13) != "Query Failed:") {
                return query_json;
            }
        }
};

#endif

