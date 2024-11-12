
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <map>
#include <vector>

#include "Database_Handler.cpp"
#include "JSON_Handler.cpp"
#include "Auth_Handler.cpp"
#include "CGPT_Handler.cpp"

using std::string;
using std::vector;
using std::pair;
using std::map;

static const string SERVER_URL = "http://127.0.0.1:8080",
                    CLIENT_URL = "http://127.0.0.1:8081";

static const string SERVER_OPTIONS_APPENDIX =
    "Access-Control-Allow-Origin: " + CLIENT_URL + "\r\n"
    "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
    "Access-Control-Max-Age: 3600\r\n";

static const string RESPONSE_404 =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n\r\n"
    "404 Not Found";

static const string RESPONSE_401 =
    "HTTP/1.1 401 Unauthorized\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 22\r\n\r\n"
    "User Not Unauthorized";

enum class REQUEST_TYPE {
    CON,            // CONNECT - Establish a network connection to a resource
    DEL,            // DELETE - Delete a resource
    GET,            // GET - Request a resource
    HED,            // HEAD - Request headers for a resource (no body)
    POS,            // POST - Submit data to be processed
    PUT,            // PUT - Replace a resource or create it if it doesn't exist
    PAT,            // PATCH - Apply partial modifications to a resource
    OPT,            // OPTION - Request communication options available for a resource
    TRC,            // TRACE - Echo back the received request (used for debugging)
    UNK             // Unable to establish type
};

static map<int, string> STATUS_CODE_MAP = {
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {204, "No Content"},

    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"}
};

class HTTP_Handler {
    using enum REQUEST_TYPE;
    
    private:

        REQUEST_TYPE getRequestType(const string request_type_str) {
            switch(request_type_str[0]) {
                case 'C':           return CON;
                case 'D':           return DEL;
                case 'G':           return GET;
                case 'H':           return HED;
                case 'O':           return OPT;
                case 'T':           return TRC;
                case 'P': {
                    switch(request_type_str[1]) {
                        case 'A':   return PAT;
                        case 'O':   return POS;
                        case 'U':   return PUT;
                        default: {}
                }}
                default: return UNK;
            }
        };

        string handleGetRequest(const string endpoint, const string data_str) {
            int status;
            string content_type, content;
            vector<pair<string, string>> data;

            status = 200, content_type = "application/json";
            return makeResponse(status, content_type, content);
        };

        string handlePostRequest(const string endpoint, const string data_str) {
            int status;
            string content_type, content;
            vector<pair<string, string>> data = JSON_Handler::jsonToPairs<string>(data_str);

            if(endpoint == "login") {
                content = Database_Handler::loginUser(data.at(0).second /* name */, data.at(1).second /* passhash */);
            }
            else if(endpoint == "create_user") {
                content = Database_Handler::createUser(data.at(0).second /* name */, data.at(1).second /* email */, data.at(2).second /* passhash */);
            }
            else if(endpoint == "get_question") {
                content = Database_Handler::getQuestion(data.at(0).second /* topic */);
            }
            else if(endpoint == "generate_question") {
                content = CGPT_Handler::getQuestion();
            }
            else if(endpoint == "get_reports") {
                content = Database_Handler::getUserReports(data.at(0).second /* user_id */, data.at(1).second /* topic */);
            }
            else if(endpoint == "generate_report") {
                content = Database_Handler::generateReport(data.at(0).second /* user_id */, data.at(1).second /* question_id */, data.at(2).second /* position */, data.at(3).second /* date */);
            }

            status = 200;
            content_type = "application/json";
            return makeResponse(status, content_type, content);
        };

        static void parseRequestElements(const string &request, string &type, string &endpoint, string &auth_token, string &data) {
            size_t index = 0;

            while(request[index] != ' ') 
                type += request[index++];

            index += 2;

            while(request[index] != ' ')
                endpoint += request[index++];

            index = request.find("Authorization:") + 22;

            while(request[index] != '\r')
                auth_token += request[index++];

            index = request.find("\r\n\r\n") + 4;
            data = request.substr(index, request.length() - index);
        }

    public: 

        string handleRequest(const string request) {
            string type_str, endpoint_str, auth_str, data_str;
            parseRequestElements(request, type_str, endpoint_str, auth_str, data_str);

            REQUEST_TYPE request_type = getRequestType(type_str);
            if(request_type != OPT && endpoint_str != "login") {
                if(!Auth_Handler::validateJWT(auth_str))
                    return RESPONSE_401;
            }

            string content;
            switch(request_type) {
                case GET: {
                    content = handleGetRequest(endpoint_str, data_str);
                    break;
                }
                case POS: {
                    content = handlePostRequest(endpoint_str, data_str);
                    break;
                }
                case OPT: {
                    content = makeResponse(204);
                    break;
                }
                default: content = RESPONSE_404;
            }
            return content;
        };

        static string makeResponse(const int status, const string content_type = "", const string content = "") {
            string response = "HTTP/1.1 " + std::to_string(status) + " " + STATUS_CODE_MAP[status] + "\r\n";
                
            if(status != 204)
                response += ("Content-Type: " + content_type + "\r\n");
            
            response += SERVER_OPTIONS_APPENDIX;
            response += ("Content-Length: " + std::to_string(content.length()) + "\r\nConnection: keep-alive\r\n\r\n" + content);

            return response;
        };
};