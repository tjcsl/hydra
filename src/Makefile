INC=-I$(CURDIR)/hydrautil
export INC

all: hydramd hydrarun hydrasd

$(OUTDIR)/libhydrautil.a: hydrautil/*.c hydrautil/gen_hydrapacket.py
	@$(MAKE) -C hydrautil

hydramd hydrarun hydrasd: $(OUTDIR)/libhydrautil.a
	@$(MAKE) -C $@

.PHONY: hydramd hydrarun hydrasd
