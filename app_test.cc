#include <gtest/gtest.h>
#include <cpr/cpr.h>
#include "crow.h"

#include "ohttp.h"
#include "app.h"


class CrowAppEnvironment : public ::testing::Environment {
public:
    std::thread server_thread;

    // One-time setup
    void SetUp() override {
        server_thread = std::thread([]() {
            ohttp::OHTTP_HPKE_KEY* keypair = getKeys();
            crow::SimpleApp app;
            initialize_app(app, keypair);
            app.port(8081).run();
        });

        // Allow time for the server to start up
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // One-time teardown
    void TearDown() override {
        server_thread.detach();
    }
};

TEST(CrowAppTest, RootEndpoint_ReturnsNonEmptyResponseWith200Status) {
    cpr::Response response = cpr::Get(cpr::Url{"http://localhost:8081/"});
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.text.empty());
}

TEST(CrowAppTest, OhttpKeysEndpoint_ReturnsNonEmptyResponseWith200Status) {
    cpr::Response response = cpr::Get(cpr::Url{"http://localhost:8081/ohttp-keys"});
    
    EXPECT_EQ(response.status_code, 200);
    EXPECT_FALSE(response.text.empty());
    std::vector<uint8_t> pk = ohttp::get_public_key(std::vector<uint8_t>(response.text.begin(), response.text.end()));
    EXPECT_EQ(pk.size(), 32);
}

TEST(CrowAppTest, OhttpGatewayEndpoint_ReturnsNonEmptyResponseWith200Status) {
    cpr::Response config_resp = cpr::Get(cpr::Url{"http://localhost:8081/ohttp-keys"});
    std::vector<uint8_t> pk = ohttp::get_public_key(std::vector<uint8_t>(config_resp.text.begin(), config_resp.text.end()));

    // Encapsulate that request
    ohttp::OHTTP_HPKE_CTX* sender_context = ohttp::createHpkeContext();
    uint8_t client_enc[ohttp::OHTTP_HPKE_MAX_ENC_LENGTH];
    size_t client_enc_len;
    std::vector<uint8_t> erequest = ohttp::get_encapsulated_request(
        sender_context,
        // Use information endpoint for our test to keep it self contained.
        "GET", "https", "httpbin.org", "/status/200", "",
        client_enc, &client_enc_len,
        pk.data(), pk.size()
    );
    std::string body = std::string(erequest.begin(), erequest.end());
    // Send request to local gateway
    cpr::Response response = cpr::Post(
        cpr::Url{"http://localhost:8081/gateway"},
        cpr::Body{body}
    );
    EXPECT_EQ(response.status_code, 200); // GATEWAY's response, not the target's
    EXPECT_FALSE(response.text.empty());
    EXPECT_EQ(response.header["content-type"], "message/ohttp-res");

    // Decapsulate the response
    std::vector<uint8_t> eresponse = std::vector<uint8_t>(response.text.begin(), response.text.end());
    std::vector<uint8_t> dresponse(eresponse.size());
    size_t dresponse_len;
    int response_code;
    ohttp::DecapsulationErrorCode rv = ohttp::decapsulate_response(
        sender_context,
        client_enc,
        client_enc_len,
        eresponse,
        dresponse.data(),
        &dresponse_len,
        eresponse.size()
    );
    EXPECT_EQ(rv, ohttp::DecapsulationErrorCode::SUCCESS);
    EXPECT_GT(dresponse_len, 0);
    EXPECT_EQ(dresponse[0], 1); // Framing indicator
    // BHTTP expects QUIC multi-byte integers, not a single uint8_t.
    EXPECT_EQ(dresponse[1], 200); // Will fail when properly implemented
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Register the one-time setup and teardown environment
    ::testing::AddGlobalTestEnvironment(new CrowAppEnvironment);

    return RUN_ALL_TESTS();
}