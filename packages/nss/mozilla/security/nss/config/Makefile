CORE_DEPTH = ../..
DEPTH      = ../..

include $(CORE_DEPTH)/coreconf/config.mk

NSS_MAJOR_VERSION = `grep "NSS_VMAJOR" ../lib/nss/nss.h | awk '{print $$3}'`
NSS_MINOR_VERSION = `grep "NSS_VMINOR" ../lib/nss/nss.h | awk '{print $$3}'`
NSS_PATCH_VERSION = `grep "NSS_VPATCH" ../lib/nss/nss.h | awk '{print $$3}'`
PREFIX = /usr

all: export libs

export:
	# Create the nss.pc file
	mkdir -p $(DIST)/lib/pkgconfig
	sed -e "s,@prefix@,$(PREFIX)," \
	    -e "s,@exec_prefix@,\$${prefix}," \
	    -e "s,@libdir@,\$${prefix}/lib," \
	    -e "s,@includedir@,\$${prefix}/include/nss," \
	    -e "s,@NSS_MAJOR_VERSION@,$(NSS_MAJOR_VERSION),g" \
	    -e "s,@NSS_MINOR_VERSION@,$(NSS_MINOR_VERSION)," \
	    -e "s,@NSS_PATCH_VERSION@,$(NSS_PATCH_VERSION)," \
	    nss.pc.in > nss.pc
	chmod 0644 nss.pc
	ln -sf ../../../../security/nss/config/nss.pc $(DIST)/lib/pkgconfig

	# Create the nss-config script
	mkdir -p $(DIST)/bin
	sed -e "s,@prefix@,$(PREFIX)," \
	    -e "s,@NSS_MAJOR_VERSION@,$(NSS_MAJOR_VERSION)," \
	    -e "s,@NSS_MINOR_VERSION@,$(NSS_MINOR_VERSION)," \
	    -e "s,@NSS_PATCH_VERSION@,$(NSS_PATCH_VERSION)," \
	    nss-config.in > nss-config
	chmod 0755 nss-config
	ln -sf ../../../security/nss/config/nss-config $(DIST)/bin

libs:

clean:

dummy: all export libs

