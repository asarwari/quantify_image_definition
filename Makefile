CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c99
LDFLAGS = -lm

TARGET  = image_definition
SRCS    = image_definition.c main.c
OBJS    = $(SRCS:.c=.o)
STB     = stb_image.h

all: $(STB) $(TARGET)

$(STB):
	@echo "Fetching stb_image.h..."
	curl -sSfL \
	  https://raw.githubusercontent.com/nothings/stb/master/stb_image.h \
	  -o $(STB)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(STB) image_definition.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

distclean: clean
	rm -f $(STB)

.PHONY: all clean distclean
