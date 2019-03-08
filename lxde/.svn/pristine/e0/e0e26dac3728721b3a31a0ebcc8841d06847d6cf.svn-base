NULL=

sbin_SCRIPTS = lxdm
CLEANFILES = $(sbin_SCRIPTS)

lxdmdir = $(datadir)/lxdm
lxdm_DATA = \
	lxdm.glade \
	$(NULL)

lxdm_confdir = /etc/lxdm
lxdm_conf_DATA = \
	lxdm.conf \
	Xsession \
	$(NULL)

install-data-hook:
	chmod +x $(DESTDIR)/etc/lxdm/Xsession

themedir=$(datadir)/lxdm/themes/Industrial
theme_DATA= \
	themes/Industrial/index.theme \
	themes/Industrial/gtkrc \
	themes/Industrial/wave.svg \
	themes/Industrial/login.png \
	themes/Industrial/shade.png \
	themes/Industrial/lang.png \
	themes/Industrial/session.png \
	themes/Industrial/greeter.ui \
	$(NULL)

EXTRA_DIST= \
	$(lxdm_DATA) \
	$(lxdm_conf_DATA) \
	$(theme_DATA) \
	$(NULL)
