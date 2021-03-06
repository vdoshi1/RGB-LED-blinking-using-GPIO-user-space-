TOOLDIR = /opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux

	CC=$(TOOLDIR)/i586-poky-linux-gcc

LIBS = -pthread
CFLAGS = -Wall
DEPS = led.h
TARGET = RGBLed


all: $(TARGET)

$(TARGET): $(TARGET).c $(DEPS)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LIBS)

clean:
	$(RM) $(TARGET)

