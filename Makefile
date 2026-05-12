TARGET = departures_iot

$(TARGET): main.c gtfs_standard.pb-c.c
	gcc -Wall -O2 main.c gtfs_standard.pb-c.c -lprotobuf-c -lcurl -o $(TARGET)

clean:
	rm -f $(TARGET)
