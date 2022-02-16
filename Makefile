ifeq ($(TARGET_DIR_BIN), )
    TARGET_DIR_BIN := /usr/local/bin
endif

all: install

install:
	mkdir -p $(TARGET_DIR_BIN)
	ln -sf $(shell pwd)/bin/lkp-ctl $(TARGET_DIR_BIN)/lkp-ctl

clean:
	rm -rf lkp-tests
	rm -rf workspace
