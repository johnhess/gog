#include "crow.h"
#include "ohttp.h"

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return ohttp::GetFoo();
    });

    app.port(8080).run();
}