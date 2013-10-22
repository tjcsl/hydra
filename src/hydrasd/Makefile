SRC=main.c hydraslave.c system.c network.c

$(OUTDIR)/hydrasd: $(SRC) $(OUTDIR)/libhydrautil.a 
	gcc $(CFLAGS) $(INC) $(SRC) -o $@ -L$(OUTDIR) -lhydrautil -lm

main.c hydramaster: hydraslave.h system.h network.h
