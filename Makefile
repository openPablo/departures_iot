TARGET = departures_iot

$(TARGET): main.c
	gcc -Wall -O2 main.c -lcurl -o $(TARGET)

clean:
	rm -f $(TARGET)
