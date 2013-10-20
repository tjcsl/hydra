$(OUTDIR)/libhydrautil.a: $(OUTDIR)/hydracommon.o $(OUTDIR)/hydranet.o
	ar rcs $(OUTDIR)/libhydrautil.a $(OUTDIR)/hydranet.o $(OUTDIR)/hydracommon.o

$(OUTDIR)/hydracommon.o: hydracommon.c
	gcc -c $^ -o $@

$(OUTDIR)/hydranet.o : hydranet.c
	gcc -c $^ -o $@
	

hydracommon.c : hydracommon.h
hydranet.c : hydranet.h
