#ifndef APP_H
#define APP_H

#include "crow.h"

ohttp::OHTTP_HPKE_KEY* getKeys();
crow::SimpleApp& initialize_app(crow::SimpleApp& app, ohttp::OHTTP_HPKE_KEY *keypair);

#endif // APP_H