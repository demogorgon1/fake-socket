# fake-socket
[![CMake](https://github.com/demogorgon1/fake-socket/actions/workflows/cmake-single-platform.yml/badge.svg?cache-control=no-cache)](https://github.com/demogorgon1/fake-socket/actions/workflows/cmake-single-platform.yml)

_fake-socket_ is a simple C library that allows you to make "fake sockets" that function like normal Unix-style sockets, except that they only work within the same process. This library was created because I wanted to add an offline mode to a game with a server-client architecture. The game already existed and relied heavily on various microservices (and the client) communicating with each other through sockets. I found that the least intrusive way to implement this feature was to make a "drop in" socket replacement with all microservices being launched inside the client process. Obviously, I could just have continued to use normal sockets, but I'd rather avoid the risk of firewalls potentially blocking anything. The library might also be useful for testing.

## Usage
First of all you'll need git and cmake to acquire and build the project. Then you can run:

```
git clone https://github.com/demogorgon1/fake-socket.git
cd fake-socket
mkdir build
cd build
cmake ..
cmake --build .
```

If succesfull, this will build a static library and tests. Run ```ctest``` immediately to verify that everything works as intended.

Optionally you can include _fake-socket_ directly in your cmake build system using FetchContent:

```
FetchContent_Declare(fake-socket
  GIT_REPOSITORY https://github.com/demogorgon1/fake-socket.git
)
FetchContent_MakeAvailable(fake-socket)
```

Use ```fake-socket::fake-socket``` to link your target with _fake-socket_.

## API
As mentioned it's largely a drop-in replacement of normal Unix-style socket programming.

* ```fs_socket()``` replaces ```socket()```. 
* ```fs_close()``` replaces ```close()```.
* ```fs_send()``` replaces ```send()```.
* ```fs_recv()``` replaces ```recv()```.
* ```fs_listen()``` replaces ```listen()```.
* ```fs_accept()``` replaces ```accept()```.
* ```fs_bind()``` replaces ```bind()```.
* ```fs_connect()``` replaces ```connect()```.
* ```fs_shutdown()``` replaces ```shutdown()```.

In addition to the above Unix-style socket functions, _fake-socket_ also offers a few others:
* ```fs_init()``` and ```fs_uninit()``` to initialize and uninitialize the library respectively.
* ```fs_is_valid_socket()``` checks if the specified socket is valid.
* ```fs_is_closed_socket()``` checks if the specified socket has been closed. 
* ```fs_is_connected_socket()``` checks if the specified socket is connected. 

Things to note:
* Only AF_INET and SOCK_STREAM sockets are currently supported.
* All sockets are non-blocking.
* ```fs_connect()``` and ```fs_bind()``` ```sockaddr``` arguments are ignored. Loopback interface is assumed everywhere.
* ```fs_accept()``` will always return a remote address of ```127.0.0.1```.
* Since there is no replacement for ```select()```, you'll need to use ```fs_is_closed_socket()``` and ```fs_is_connected_socket()``` to poll whether a socket is closed or connected.
* Errors are set to ```err```. ```EAGAIN``` is reported by ```fs_recv()``` and ```fs_accept()``` if a call would have blocked.

