INC=-I$(CURDIR)/hydrautil
export INC

all: hydramd hydrarun hydrasd

hydramd hydrarun hydrasd: $(OUTDIR)/libhydrautil.a
	@$(MAKE) -C $@

$(OUTDIR)/libhydrautil.a: hydrautil/*.c hydrautil/gen_hydrapacket.py
	echo Rebuilding hydrautil
	@$(MAKE) -C hydrautil

.PHONY: hydramd hydrarun hydrasd
