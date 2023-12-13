.PHONY: default
default: bin/lkp-master

bin/lkp-master: $(shell find . -type f -name '*.go') go.mod go.sum
	go build -o $@

.PHONY: clean
clean:
	rm -rf bin/

.PHONY: release
release: bin/lkp-master install.sh cfg.yaml
	tar zcf ../lkp-extent-$(version)-master.tar.gz bin/lkp-master install.sh cfg.yaml