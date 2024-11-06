# include <cassert>

#include "cpr/cpr.h"
#include "crow.h"
#include "ohttp.h"
#include "app.h"

// TODO: Deal with persisted keys via config.
EVP_HPKE_KEY* getKeys() {
  EVP_HPKE_KEY *keypair = EVP_HPKE_KEY_new();
  const EVP_HPKE_KEM *kem = EVP_hpke_x25519_hkdf_sha256();
  int rv = EVP_HPKE_KEY_generate(keypair, kem);
  assert(rv == 1);
  return keypair;
}

cpr::Response do_binary_request(std::vector<uint8_t> binary_request) {
    std::string method = ohttp::get_method_from_binary_request(binary_request);
    std::string url = ohttp::get_url_from_binary_request(binary_request);
    std::string body = ohttp::get_body_from_binary_request(binary_request);
    if (method == "POST") {
        return cpr::Post(cpr::Url{url}, cpr::Body{body});
    } if (method == "GET") {
        return cpr::Get(cpr::Url{url}, cpr::Body{body});
    } else {
        // TODO: Handle uknonwn method, or require a known method.
        return cpr::Response();
    }
}

crow::SimpleApp& initialize_app(crow::SimpleApp& app, EVP_HPKE_KEY *keypair) {
    // Generate config once, since it will be requested many times.
    std::vector<uint8_t> config = ohttp::generate_key_config(keypair);
    std::string config_str = std::string(config.begin(), config.end());
    std::vector<uint8_t> pubk = ohttp::get_public_key(config);

    std::cout << "Public Key: " << std::endl;
    std::cout << "{";
    for (size_t i = 0; i < pubk.size(); i++) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)pubk[i] << ", ";
    }
    std::cout << "}";
    std::cout << std::endl;

    CROW_ROUTE(app, "/")([](){
        return "This is an OHTTP Gateway.  You can get ohttp-keys at /ohttp-keys; You can send requests to /ohttp-request";
    });

    CROW_ROUTE(app, "/ohttp-keys")([config_str](){
        return crow::response{config_str};
    });

    // POSTs to the gateway
    CROW_ROUTE(app, "/gateway").methods(crow::HTTPMethod::POST)([keypair](const crow::request& req){
        EVP_HPKE_CTX receiver_context;
        std::string body = req.body;
        std::vector<uint8_t> body_as_vec = std::vector<uint8_t>(body.begin(), body.end());
        uint8_t decapsulated_request[body.size()];
        size_t out_len;
        size_t enc_len = 32;
        uint8_t enc[enc_len];
        ohttp::DecapsulationErrorCode result = ohttp::decapsulate_request(
            /* receiver_context */ &receiver_context,
            /* erequest */ body_as_vec,
            /* drequest */ decapsulated_request,
            /* drequest_len */ &out_len,
            /* enc */ enc,
            /* enc_len */ enc_len,
            /* max_drequest_len */ body.size(),
            /* recipient_keypair */ *keypair
        );
        if (result != ohttp::DecapsulationErrorCode::SUCCESS) {
            return crow::response{500, DecapsulationErrorCodeToString(result)};
        }
        cpr::Response response = do_binary_request(std::vector<uint8_t>(decapsulated_request, decapsulated_request + out_len));

        // Encapsulate response
        std::vector<uint8_t> encapsulated_response = ohttp::encapsulate_response(
            /* receiver_context */ &receiver_context,
            /* enc */ enc,
            /* enc_len */ enc_len,
            /* response_code */ (int)response.status_code,
            /* response_body */ response.text
        );

        crow::response resp = crow::response{
            std::string(encapsulated_response.begin(),
            encapsulated_response.end())
        };
        resp.set_header("content-type", "application/ohttp-res");
        return resp;
    });

    return app;
}

int main() {
    EVP_HPKE_KEY* keypair = getKeys();
    crow::SimpleApp app;
    initialize_app(app, keypair);
    app.port(8081).run();
}