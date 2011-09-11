CFLAGS  += $(shell pkg-config --cflags libusb-1.0)
LDFLAGS += $(shell pkg-config --libs libusb-1.0)

PREFIX  ?= /usr/local
bindir	= $(PREFIX)/sbin

kinect_upload_fw: kinect_upload_fw.o

install: kinect_upload_fw
	install -d $(DESTDIR)$(bindir)
	install -m 755 kinect_upload_fw $(DESTDIR)$(bindir)

clean:
	rm -rf *~ *.o kinect_upload_fw
