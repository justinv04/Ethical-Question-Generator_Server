#ifndef CGPT_HANDLER_CPP
#define CGPT_HANDLER_CPP

#include <string>
#include <iostream>

#include <curl/curl.h>

using std::string;

static const string OPEN_AI_KEY = "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";

class CGPT_Handler {
    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* data) {
            ((string*)data)->append((char*)contents, size * nmemb);
            return (size * nmemb);
        }

    public:
        static string getQuestion() {
            CURL* curl;
            CURLcode result;
            string response;

            curl_global_init(CURL_GLOBAL_DEFAULT);
            curl = curl_easy_init();


            if(curl) {
                string payload = R"({"model": "gpt-3.5-turbo", "messages": [{"role": "system", "content": "Generate a random ethical question for discussion."}]})";

                curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // THIS IS MUY MUY MALO BUT OTHERWISE WE DO MORE WORK SO LEAVE IT

                struct curl_slist *headers = NULL;
                headers = curl_slist_append(headers, ("Authorization: Bearer " + OPEN_AI_KEY).c_str());
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

                result = curl_easy_perform(curl);

                long http_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
                std::cout << "HTTP Response Code: " << http_code << "\n\n\n";
                
                if(result != CURLE_OK)
                    return "Failed to Connect to Open AI";

                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
            }

            
            curl_global_cleanup();

            string question;
            size_t contentPos = response.find("\"content\":\"");

            if (contentPos != string::npos) {
                contentPos += 11; 
                size_t endPos = response.find("\"", contentPos);

                if (endPos != string::npos)
                    question = response.substr(contentPos, endPos - contentPos);
            }

            return question;
        }
};

#endif