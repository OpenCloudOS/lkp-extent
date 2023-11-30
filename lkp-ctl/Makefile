.PHONY: default
default: bin/lkp-ctl

bin/lkp-ctl: $(shell find . -type f -name '*.go') go.mod go.sum
	go build -o $@

.PHONY: clean
clean:
	rm -rf bin/

.PHONY: release
release: bin/lkp-ctl install.sh
	tar zcf ../lkp-extent-$(version)-ctl.tar.gz bin/lkp-ctl install.sh