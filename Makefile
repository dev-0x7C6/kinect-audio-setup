all:
	$(MAKE) -C kinect_upload_fw 

install:
	$(MAKE) -C kinect_upload_fw install
	install -d $(DESTDIR)/lib/udev/rules.d
	install -m 644 contrib/55-kinect_audio.rules.in $(DESTDIR)/lib/udev/rules.d/55-kinect_audio.rules

clean: 
	$(MAKE) -C kinect_upload_fw clean

changelog:
	git log --pretty="format:%ai  %aN  <%aE>%n%n%x09* %s%d%n" > ChangeLog
