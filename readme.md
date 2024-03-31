# gRPC Playground
Serves as a playground for exploring gRPC. It includes examples of four types of communication patterns:

## Simple server and client
includes four types of communication patterns:
1. **Unary RPCs:** The client sends a single request to the server and gets a single response back.
2. **Server streaming RPCs:** The client sends a request to the server and gets a stream of responses back.
3. **Client streaming RPCs:** The client sends a stream of requests to the server and gets a single response back.
4. **Bidirectional streaming RPCs:** Both client and server send a stream of messages to each other.

## Async Server and Client
The code for the asynchronous server and client is largely based on the [greeter_async_server](https://github.com/grpc/grpc/blob/v1.38.0/examples/cpp/helloworld/greeter_async_server.cc) and [greeter_async_client2](https://github.com/grpc/grpc/blob/v1.38.0/examples/cpp/helloworld/greeter_async_client2.cc) examples from the official gRPC repository.

Asynchronicity is decoupled from the client and server. This means that 
- A synchronous client can be used with an asynchronous server, and vice versa. 
- Of course, an asynchronous client can also be used with an asynchronous server. 

For more details, refer to the [`main` method in `AsyncGreeterClient.cpp`](./asyncclient/AsyncGreeterClient.cpp).

# Usage
## Prerequisite
These examples have only been tested on MAC OS Sonoma with Apple M1 Max chip.
1. install MAC OS command line tools
2. install build tools and grpc
```bash
brew install cmake
brew install autoconf automake libtool pkg-config
brew install grpc
```
## Build
```bash
mkdir build
cd build
# make inside build folder to make source folder clean
cmake ..
# will generate server and client folder
make -j 4
```
## Run
### Server
```bash
cd server
# will startup server, listen on port 9999
./trygrpc_server
```
### Client
```bash
# show usage
./trygrpc_client
# unary mode
./trygrpc_client 1
# server stream mode
./trygrpc_client 2
# client stream mode
./trygrpc_client 3
# bi-direction stream mode
./trygrpc_client 4
```
### Async Server
```bash
cd asyncserver
# will startup server, listen on port 50051
./trygrpc_asyncserver
```
### Async Client
```bash
cd asyncclient
./trygrpc_asyncclient
```

# Sample launch.json for vscode
```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "trygrpc_client",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/client/trygrpc_client",
            "args": ["1"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```