
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "Database_Handler.cpp"
#include "JSON_Handler.cpp"
#include "Auth_Handler.cpp"

using std::string;
using std::vector;
using std::pair;

static const string SERVER_URL = "http://127.0.0.1:8080",
                    CLIENT_URL = "http://127.0.0.1:8081";

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

        string handleGetRequest(const string endpoint) {
            string content;

            if(endpoint == "ping")
                content = "hi there ;)";
            
            return JSON_Handler::make_Response(200, "OK", "application/json", content);
        };

        string handlePostRequest(const string endpoint, const string data_str) {
            string content;
            vector<pair<string, string>> data;

            if(endpoint == "login") {
                data = JSON_Handler::parseToPairs<string>(data_str);
                content = Database_Handler::authenticateUser(data.at(0).second, data.at(1).second);
            }
            else if(endpoint == "create_user") {
                data = JSON_Handler::parseToPairs<string>(data_str);
                content = Database_Handler::createUser(data.at(0).second, data.at(1).second, data.at(2).second, data.at(3).second);
            }

            return JSON_Handler::make_Response(200, "OK", "application/json", content);
        };

    public: 

        const string SERVER_OPTIONS_RESPONSE =
            "HTTP/1.1 204 No Content\r\n"
            "Access-Control-Allow-Origin: " + CLIENT_URL + "\r\n"
            "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Authorization, Content-Type\r\n"
            "Access-Control-Max-Age: 3600\r\n"
            "Connection: keep-alive\r\n\r\n";

        const string RESPONSE_404 =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 42\r\n\r\n"
            "<html><body>404 Not Found</body></html>";

        const string RESPONSE_401 =
            "HTTP/1.1 401 Unauthorized\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 22\r\n\r\n"
            "User Not Unauthorized";

        string handleRequest(const string request) {
            string type_str, endpoint_str, auth_str, data_str;
            size_t index = 0;

            while(request[index] != ' ') 
                type_str += request[index++];

            index += 2;

            while(request[index] != ' ')
                endpoint_str += request[index++];

            index = request.find("Authorization:") + 22;

            while(request[index] != '\r')
                auth_str += request[index++];

            index = request.find("\r\n\r\n") + 4;
            data_str = request.substr(index, request.length() - index);

            string content;
            REQUEST_TYPE request_type = getRequestType(type_str);

            if(request_type != OPT && endpoint_str != "login") {
                if(!Auth_Handler::validateJWT(auth_str))
                    return RESPONSE_401;
            }

            switch(request_type) {
                case GET: {
                    content = handleGetRequest(endpoint_str);
                    break;
                }
                case POS: {
                    content = handlePostRequest(endpoint_str, data_str);
                    break;
                }
                case OPT: {
                    content = SERVER_OPTIONS_RESPONSE;
                    break;
                }
                default: content = RESPONSE_404;
            }
            return content;
        };
};