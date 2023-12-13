.PHONY: default
default: bin/lkp-node

bin/lkp-node: $(shell find . -type f -name '*.go') go.mod go.sum
	go build -o $@

.PHONY: clean
clean:
	rm -rf bin/

.PHONY: release
release: bin/lkp-node install.sh node.yaml run.sh
	tar zcf ../lkp-extent-$(version)-node.tar.gz bin/lkp-node install.sh node.yaml run.sh