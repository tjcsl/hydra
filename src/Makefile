INC=-I$(CURDIR)/hydrautil
export INC

all: hydramd hydrarun hydrasd libhydrautil.a

hydramd hydrarun hydrasd: libhydrautil.a
	@$(MAKE) -C $@

libhydrautil.a:
	@$(MAKE) -C hydrautil

.PHONY: hydramd hydrarun hydrasd
