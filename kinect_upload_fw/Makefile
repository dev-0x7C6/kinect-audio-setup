CFLAGS ?= -std=c99 -pedantic -Wall -Wextra -O2

# Don't make pedantic checks errors,
# as vanilla libusb-1.0.8 can't live with that
#CFLAGS += -pedantic-errors

# GCC >= 4.6
#CFLAGS += -Wunused-but-set-variable

CFLAGS += -fno-common \
  -Wall \
  -Wextra \
  -Wformat=2 \
  -Winit-self \
  -Winline \
  -Wpacked \
  -Wp,-D_FORTIFY_SOURCE=2 \
  -Wpointer-arith \
  -Wlarger-than-65500 \
  -Wmissing-declarations \
  -Wmissing-format-attribute \
  -Wmissing-noreturn \
  -Wmissing-prototypes \
  -Wnested-externs \
  -Wold-style-definition \
  -Wredundant-decls \
  -Wsign-compare \
  -Wstrict-aliasing=2 \
  -Wstrict-prototypes \
  -Wswitch-enum \
  -Wundef \
  -Wunreachable-code \
  -Wunsafe-loop-optimizations \
  -Wwrite-strings

CFLAGS += $(shell pkg-config --cflags libusb-1.0)
LDLIBS += $(shell pkg-config --libs libusb-1.0)

PREFIX ?= /usr/local
bindir := $(PREFIX)/sbin

all: kinect_upload_fw


endian: endian.o

endian.h: endian
	rm -f endian.h
	./endian > endian.h

kinect_upload_fw.o: endian.h

kinect_upload_fw: kinect_upload_fw.o


install: kinect_upload_fw
	install -d $(DESTDIR)$(bindir)
	install -m 755 kinect_upload_fw $(DESTDIR)$(bindir)

clean:
	rm -rf *~ *.o kinect_upload_fw endian endian.h
