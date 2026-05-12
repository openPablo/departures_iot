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
make
```

## Running

```
export DELIJN_API_KEY="--------------"
./departures_iot
```


## re-generating proto files

gtfs_standard.proto is a standardized proto file, maintained by Google, which DeLijn uses.
```
protoc --c_out=. gtfs_standard.proto
```
