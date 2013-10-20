$(OUTDIR)/libhydrautil.a: hydracommon.c
	gcc -c $^ -o $(OUTDIR)/hydrautil.o
	ar rcs $(OUTDIR)/libhydrautil.a $(OUTDIR)/hydrautil.o

hydracommon.c : hydracommon.h
