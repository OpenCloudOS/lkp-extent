export version = $(shell git describe --tags --always)

sub-projects = lkp-ctl lkp-master lkp-node

define make-target-in-dir
	make $(1) -C $(2);
endef 

.PHONY: default release
default release:
	$(foreach project, $(sub-projects), $(call make-target-in-dir, $@, $(project)))

.PHONY: clean
clean:
	$(foreach project, $(sub-projects), $(call make-target-in-dir, $@, $(project)))
	rm -f *.tar.gz