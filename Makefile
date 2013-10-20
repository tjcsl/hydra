export MAKEFLAGS := --no-print-directory
export CFLAGS := -Wall -Wextra
export OUTDIR := $(CURDIR)/build

all: $(OUTDIR)
	@$(MAKE) -C src
clean:
	rm -rf $(OUTDIR)

$(OUTDIR):
	mkdir -p $(OUTDIR)

