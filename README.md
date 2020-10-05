# Building

The project requires [CMake] to be installed in order to build it.

To build the project and run the unit tests, follow these instructions:

```
$ mkdir build
$ cd build
$ make
$ make tests
```

# Streaming a file

To stream a file, first start the receiver server:

```
$ cd build
$ ./filereceiver 8080
```

Now, in another terminal window, use the client to upload a file:

```
cd build
./fileuploader --host 127.0.0.1 --port 8080 --limit-rate 1048576 testfile10Mb
```

Replace `testfile10Mb` with the file that you wish to upload. The file received
will have `.received` suffix appended to its file name. The `--limit-rate`
parameter is optional and restricts the uploading speed to given number of bytes
per second.

[CMake]: https://cmake.org/