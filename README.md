# Gateway for OHTTP by Guardian Project

Crow app that runs an OHTTP gateway.  Uses `ohttp-gp` for heavy lifting.

## Installing

Be sure to have cmake and asio installed via either

```
brew install asio cmake
```

or 

```
sudo apt-get install cmake libasio-dev
```

```
brew install cmake asio
mkdir build
cd build
cmake ..
make
```

## Running this within in `nginx` as a gateway for your webserver

