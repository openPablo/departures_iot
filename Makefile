TARGET = departures_iot

$(TARGET): main.c
	gcc -g -Wall -O2 main.c -lcurl -o $(TARGET)

clean:
	rm -f $(TARGET)
