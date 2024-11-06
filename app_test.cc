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
            EVP_HPKE_KEY* keypair = getKeys();
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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Register the one-time setup and teardown environment
    ::testing::AddGlobalTestEnvironment(new CrowAppEnvironment);

    return RUN_ALL_TESTS();
}