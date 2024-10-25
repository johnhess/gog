#include "crow.h"
#include "ohttp.h"


int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "This is an OHTTP Gateway.  You can get ohttp-keys at /ohttp-keys; You can send requests to /ohttp-request";
    });

    app.port(8081).run();
}