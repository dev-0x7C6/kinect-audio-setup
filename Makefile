all:
	make -C kinect_upload_fw 

install:
	make -C kinect_upload_fw install
	install -d $(DESTDIR)/lib/udev/rules.d
	install -m 644 contrib/55-kinect_audio.rules.in $(DESTDIR)/lib/udev/rules.d/55-kinect_audio.rules
	
	# We prare the firmware location, but it should be downloaded at
	# package installation time
	install -d $(DESTDIR)/lib/firmware/kinect

clean: 
	make -C kinect_upload_fw clean
