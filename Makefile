export MAKEFLAGS := --no-print-directory
export CFLAGS := -Wall
export OUTDIR := $(CURDIR)/build

all: $(OUTDIR)
	@$(MAKE) -C src
clean:
	rm -rf $(OUTDIR)
	rm src/hydrautil/hydrapacket.h
	rm src/hydrautil/hydrapacket.c

$(OUTDIR):
	mkdir -p $(OUTDIR)

