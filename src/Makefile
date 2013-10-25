INC=-I$(CURDIR)/hydrautil
export INC

all: hydramd hydrarun hydrasd

hydramd hydrarun hydrasd: $(OUTDIR)/libhydrautil.a
	@$(MAKE) -C $@

$(OUTDIR)/libhydrautil.a: $(wildcard hydrautil/*)
	@$(MAKE) -C hydrautil

.PHONY: hydramd hydrarun hydrasd
