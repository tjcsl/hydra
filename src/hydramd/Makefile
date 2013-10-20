SRC=main.c hydramaster.c

$(OUTDIR)/hydramd: main.c hydramaster.c $(OUTDIR)/libhydrautil.a
	gcc $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil

main.c hydramaster.c: hydramaster.h 
