# try gRPC
playground for gRPC, includes four types of communication patterns:
1. Unary RPCs: The client sends a single request to the server and gets a single response back.
2. Server streaming RPCs: The client sends a request to the server and gets a stream of responses back.
3. Client streaming RPCs: The client sends a stream of requests to the server and gets a single response back.
4. Bidirectional streaming RPCs: Both client and server send a stream of messages to each other.
# how to use
## pre-requisite
Only tried in MAC OS Sonoma Apple M1 Max.
1. install MAC OS command line tools
2. install build tools and grpc
```bash
brew install cmake
brew install autoconf automake libtool pkg-config
brew install grpc
```
## build
```bash
mkdir build
cd build
# make inside build folder to make source folder clean
cmake ..
# will generate server and client folder
make -j 4
```
## run
### server
```bash
cd server
# will startup server, listen on port 9999
./trygrpc_server
```
### client
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