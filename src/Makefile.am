
bin_PROGRAMS=sshlprd sshlpr_client sshlpr
dist_bin_SCRIPTS=sshlpr_helper
#bin_SCRIPTS=sshlprd

#sshlprd_helper_SOURCES=sshlprd.cpp data.h data.cpp
sshlprd_SOURCES=sshlprd.cpp data.h data.cpp

sshlpr_client_SOURCES=sshlpr_client.cpp data.h data.cpp
sshlpr_SOURCES=sshlpr.cpp data.h data.cpp

#sshlprd: sshlprd_template.sh ../config.h
#	echo "#!/bin/sh" > sshlprd
#	echo "SSHLPRD_SOCKDIR="$(SSHLPRD_SOCKDIR) >> sshlprd
#	cat sshlprd_template.sh >> sshlprd

install-exec-hook:
	echo    "#### Setting SUID for my_bin. ####"
	chown $(LPUSER) $(DESTDIR)$(bindir)/sshlpr_client
	chmod 4755 $(DESTDIR)$(bindir)/sshlpr_client
	echo    "####-------------------------------####"
	-mkdir $(SSHLPRD_SOCKDIR)
	chown $(LPUSER) $(SSHLPRD_SOCKDIR)

install-exec-local:
	echo "#### INSTALLING sshlpr ####"
	$(INSTALL_PROGRAM) sshlpr $(DESTDIR)$(CUPS_BACKEND)/sshlpr


