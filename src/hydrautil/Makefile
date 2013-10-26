OBJECTS=$(OUTDIR)/hydracommon.o $(OUTDIR)/hydranet.o  $(OUTDIR)/hydrapacket.o \
		$(OUTDIR)/hydralog.o $(OUTDIR)/config.o

$(OUTDIR)/libhydrautil.a: $(OBJECTS)
	ar rcs $(OUTDIR)/libhydrautil.a $(OBJECTS)
	date > $(OUTDIR)/libhydrautil.a.stamp

$(OUTDIR)/hydracommon.o: hydracommon.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OUTDIR)/hydranet.o : hydranet.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OUTDIR)/hydrapacket.o: hydrapacket.c
	$(CC) $(CFLAGS) -c hydrapacket.c -o $@

$(OUTDIR)/hydralog.o: hydralog.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OUTDIR)/config.o : config.c
	$(CC) $(CFLAGS) -c $^ -o $@

hydrapacket.c : hydrapacket.h hydrapacket.in
hydrapacket.h : hydrapacket.in gen_hydrapacket.py hydrapacket.template.c
	python3 ./gen_hydrapacket.py
hydracommon.c : hydracommon.h
hydranet.c : hydranet.h
hydralog.c : hydralog.h
config.c : config.h
