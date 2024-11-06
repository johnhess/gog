#ifndef APP_H
#define APP_H

#include "crow.h"

EVP_HPKE_KEY* getKeys();
crow::SimpleApp& initialize_app(crow::SimpleApp& app, EVP_HPKE_KEY *keypair);

#endif // APP_H