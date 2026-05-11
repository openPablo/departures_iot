# departures_iot

## Building reqs
```
sudo dnf install protobuf-devel protoc libcurl-devel
git clone git@github.com:protobuf-c/protobuf-c.git
./autogen.sh && ./configure && make && make install
git clone git@github.com:protobuf-c/protobuf-c-rpc.git
./autogen.sh && ./configure && make && make install
```

## Compiling

```
gcc main.c gtfs-realtime.pb-c.c -lprotobuf-c -lcurl -o departures_iot
```

## re-generating proto files

gtfs_standard.proto is a standardized proto file, maintained by Google, which DeLijn uses.
```
gcc main.c gtfs_standard.pb-c.c -lprotobuf-c -lcurl -o departures_iot
```
