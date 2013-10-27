$(OUTDIR)/libini.a.stamp : ini.c
	$(CC) $(CFLAGS) -c ini.c -o $(OUTDIR)/inih.o
	ar rcs $(OUTDIR)/libinih.a $(OUTDIR)/inih.o 
	date > $(OUTDIR)/libini.a.stamp
