#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "DB_Handler.cpp"

using std::string;

const string ERROR_404_RESPONSE = 
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 42\r\n\r\n"
    "<html><body>404 Not Found</body></html>";

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
    UNKNOWN_REQUEST // Unable to establish type
};

enum class ENDPOINT_TYPE {
    FILE,
    DATA,
    UNKNOWN_ENDPOINT 
};

enum class FILE_TYPE {
    HTML,
    CSS,
    JS,
    JPG,
    PNG,
    ICO,
    UNKNOWN_FILE    // Unable to establish type
};

class File_Handler {
    using enum REQUEST_TYPE;
    using enum ENDPOINT_TYPE;
    using enum FILE_TYPE;

    private:
        DB_Handler db_handler;

        REQUEST_TYPE getRequestEnum(const string request_type_str) {
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
                default: return UNKNOWN_REQUEST;
            }
        };

        ENDPOINT_TYPE getEndpointEnum(const string endpoint_str) {
            if(endpoint_str.length() == 0)
                return UNKNOWN_ENDPOINT;

            if(endpoint_str.substr(0, 5) == "/data")
                return DATA;
            else
                return FILE;
        }

        FILE_TYPE getFileEnum(const string file_ext_str) {
            switch(file_ext_str[0]) {
                case 'c':           return CSS;
                case 'h':           return HTML;
                case 'i':           return ICO;
                case 'p':           return PNG;
                case 'j': {
                    switch(file_ext_str[1]) {
                        case 's':   return JS;
                        case 'p':   return JPG;
                        default: {}
                    }
                    break;
                }
                default: return UNKNOWN_FILE;
            }
        };

        string getFileHTTPType(FILE_TYPE file_type_enum) {
            switch(file_type_enum) {
                case HTML:          return "text/html";
                case CSS:           return "text/css";
                case JS:            return "application/javascript";
                case PNG:           return "image/png";
                case JPG:           return "image/jpeg";
                case ICO:           return "image/x-icon";
                case UNKNOWN_FILE:  return "application/octet-stream";
                default:            return "application/octet-stream";
            }
        };

        string readFile(const string filepath, bool binary_read) {
            std::ifstream file_reader;
            std::stringstream buffer;

            if(binary_read) 
                file_reader.open(filepath, std::ios::binary);
            else 
                file_reader.open(filepath);

            if (!file_reader.is_open())
                return "Error: file " + filepath + " was not found";
                
            buffer << file_reader.rdbuf();
            return buffer.str();
        };

        string getFileContent(string file) {
            string message, content, file_ext, file_directory = "../Source/Frontend";

            if(file == "/")
                file += ".html";

            size_t ext_index = file.find('.');
            file_ext = file.substr(ext_index + 1, file.length() - ext_index);

            FILE_TYPE type = getFileEnum(file_ext);
            
            switch(type) {
                case JPG:
                case PNG:
                case ICO: {
                    content = readFile(file_directory + file, true);
                    break;
                }
                case HTML:
                case CSS:
                case JS:
                case UNKNOWN_FILE:
                default: {
                    content = readFile(file_directory + file, false);
                }
            }

            message = "HTTP/1.1 200 OK\r\n";
            message += "Content-Type: " + getFileHTTPType(type) + "\r\n";
            message += "Content-Length: " + std::to_string(content.length()) + "\r\n";
            message += "Connection: close\r\n\r\n";
            message += content;

            return message;
        };

        string getDataContent(string data) {
            string message, content = db_handler.handleQuery("SELECT * FROM Users");

            message = "HTTP/1.1 200 OK\r\n";
            message += "Content-Type: application/json\r\n";
            message += "Content-Length: " + std::to_string(content.length()) + "\r\n";
            message += "Connection: close\r\n\r\n";
            message += content;

            return message;
        }

    public:
        File_Handler() {};
};