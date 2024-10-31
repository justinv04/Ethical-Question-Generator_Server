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

        static string getQuery(const char* query) {
            MYSQL* connection;
            string result_data, error_message;

            if(!openConnection(&connection))
                return JSON_Handler::make_Error("Query Failed: Unable to connect to server");

            if(mysql_query(connection, query)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::make_Error("Query Failed: " + error_message);
            }

            MYSQL_RES* result = mysql_store_result(connection);
            if(result == nullptr) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::make_Error("Query Failed: " + error_message);
            }

            result_data = JSON_Handler::get_MYSQL_JSON(result);

            mysql_free_result(result);
            mysql_close(connection);

            return result_data;
        };

        static string getQueryAfterAction(const char* query, const char* action) {
            MYSQL* connection;
            string result_data, error_message;

            if(!openConnection(&connection))
                return JSON_Handler::make_Error("Query Failed: Unable to connect to server");

            if(mysql_query(connection, action)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::make_Error("Query Failed: " + error_message);
            }

            if(mysql_query(connection, query)) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::make_Error("Query Failed: " + error_message);
            }

            MYSQL_RES* result = mysql_store_result(connection);
            if(result == nullptr) {
                error_message = mysql_error(connection);
                mysql_close(connection);
                return JSON_Handler::make_Error("Query Failed: " + error_message);
            }

            result_data = JSON_Handler::get_MYSQL_JSON(result);

            mysql_free_result(result);
            mysql_close(connection);

            return result_data;
        }

        public:
            static string authenticateUser(const string username, const string password) {
                string query_str = "SELECT * FROM Users WHERE User_Name = '" + username + "' AND User_Password_Hash = '" + password + "'";

                vector<vector<string>> data_vector = JSON_Handler::parseUsers( getQuery(query_str.c_str()) );

                if(data_vector[0][1] == username && data_vector[0][3] == password) // username and passwords match
                    return Auth_Handler::createJWT(data_vector[0][0], data_vector[0][1]);
                
                return "{\"Auth_Token\":\"Failed_Login\"}";
            };

            static string createUser(const string username, const string email, const string password_hash, const string date_joined) {
                string query = "SELECT * FROM Users WHERE User_Name = '" + username + "';";
                string action = "INSERT INTO Users (User_Name, User_Email, User_Password_Hash, User_Date_Joined) VALUES ('" + username + "', '" + email + "', '" + password_hash + "', '" + date_joined + "');";

                string query_data = getQueryAfterAction(query.c_str(), action.c_str());

                vector<vector<string>> data_vector = JSON_Handler::parseUsers(query_data);

                if(data_vector[0][1] == username && data_vector[0][3] == password_hash)
                    return Auth_Handler::createJWT(data_vector[0][0], data_vector[0][1]);
                
                return "{\"Auth_Token\":\"Failed_Login\"}";
            };
};

#endif

