LAUNCH_BUILD=./build
ITEM_LIST = \
	    desktop \
	    gconf \
	    mime

all:
	for item in $(ITEM_LIST); do \
		$(LAUNCH_BUILD) $$item ; \
	done;

clean:
	for item in $(ITEM_LIST); do \
		rm -f $$item-purge ; \
	done;
