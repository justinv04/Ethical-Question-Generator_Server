#ifndef AUTH_HANDLER_CPP
#define AUTH_HANDLER_CPP

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <jwt-cpp/jwt.h>

#include "JSON_Handler.cpp"

using std::string;

static const string AUTH_ISSUER = "Ethical Question Generator";
static const string AUTH_SECRET_KEY = "4f27e852b53d811f5a5064e5d490ca470417a3a79fae200a87e5ffeae5bb9eb3";

class Auth_Handler {
    public:
        static string createJWT(const string user_ID, const string user_Name) {

            string token = jwt::create()
                            .set_issuer(AUTH_ISSUER)
                            .set_subject(user_Name)         
                            .set_id(user_ID)                    
                            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(1))
                            .sign(jwt::algorithm::hs256{AUTH_SECRET_KEY}); 

            return JSON_Handler::pairsToJSON<string>({{"Auth_Token", token}});
        }

        static bool validateJWT(const string token) {
            try {
                jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded_token = jwt::decode(token);

                jwt::verifier<jwt::default_clock, jwt::traits::kazuho_picojson> verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256{AUTH_SECRET_KEY})
                    .with_issuer(AUTH_ISSUER);

                verifier.verify(decoded_token);

                return true;
            } 
            catch (const std::exception& error) {
                std::cerr << "An error occurred: " << error.what() << std::endl;
                return false;
            }
        }
};

#endif