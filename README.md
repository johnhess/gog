# `gog`, a Gateway for OHTTP by Guardian Project

This application provides an OHTTP gateway per [RFC 9458](https://www.rfc-editor.org/rfc/rfc9458.html).  This can stand alone or run on the same server as the OHTTP Target Resource.

`gog` is a [Crow](https://crowcpp.org/) app that uses []`ohttp-gp`](https://github.com/johnhess/ohttp-gp?tab=readme-ov-file) for heavy lifting.  It forwards requests to a pre-configured target resource and makes keys available to clients at `/ohttp-keys`. 

## Building

Be sure to have cmake and asio installed via either

```
brew install asio cmake
```

or 

```
sudo apt-get install cmake libasio-dev
```

Then build and run the server:

```
mkdir build
cd build
cmake .. && make && ./GogApp
```

This makes the server available on port 8081.  OHTTP keys will be automatically provisioned.

## Running this within in `nginx` as a gateway for your webserver

You can run this as `./build/GogApp` for testing, but you'll likely want to put this behind `nginx` or similar.  A config like this will do the trick, assuming you've got certificates in place via `certbot`.

```
server {
    listen 80;
    server_name ohttp-gateway.jthess.com;

    location / {
        return 301 https://$host$request_uri;
    }
}

server {
    listen 443 ssl;
    server_name ohttp-gateway.jthess.com;

    ssl_certificate /etc/letsencrypt/live/ohttp-gateway.jthess.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/ohttp-gateway.jthess.com/privkey.pem;
    include /etc/letsencrypt/options-ssl-nginx.conf;
    ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem;

    # gog, our own implementation.
    location /gog/ {
        proxy_pass http://localhost:8081;
        rewrite ^/gog/(.*)$ /$1 break;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```