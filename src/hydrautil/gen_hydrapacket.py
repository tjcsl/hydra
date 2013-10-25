#Generates the hydra string tokenizer

inf = open("hydrapacket.in").read().split("\n")

h = open("hydrapacket.h", "w")
c = open("hydrapacket.c", "w")

includes = ['arpa/inet.h'
           ,'string.h'
           ,'stdlib.h'
           ,'sys/stat.h'
           ,'unistd.h']

#will be PACKETNAME: {arg1_name:arg1type}
packettypes = {}
#PACKETNAME:integer id
packet_ids = {}
#PACKETNAME:string
packet_read_astrings = {}
#PACKETNAME:string
packet_write_astrings = {}
serial = 0

i = 0
for line in inf:
    if len(line) == 0:
        continue
    if line[0] == '#':
        continue
    if line[0] == ':':
        serial = int(line.split(':')[-1])
        continue
    packet_spec = line.split(' ')
    packet_spec = [x for x in packet_spec if not(x == '')]
    print(packet_spec)
    packet_type = packet_spec[0]
    packet_args = {}
    for arg in packet_spec[1:]:
        if arg[0] == '#':
            break
        arg = arg.split(':')
        packet_args[arg[1]] = arg[0]
    argstring = "int fd,"
    for arg in packet_args:
        paramtype = ""
        if packet_args[arg] == 'u32':
            paramtype = "uint32_t"
        elif packet_args[arg] == 'u16':
            paramtype = "uint16_t"
        elif packet_args[arg] == 'byte':
            paramtype = "char"
        elif packet_args[arg] == 'file':
            paramtype = "int"
        elif packet_args[arg] == 'data':
            argstring += "void*  %s_data,int %s_len," % (arg, arg)
            continue
        argstring += paramtype + "  " + arg + ","
    argstring = argstring[:-1]
    packet_write_astrings[packet_type] = argstring
    argstring = "int fd,"
    for arg in packet_args:
        paramtype = ""
        if packet_args[arg] == 'u32':
            paramtype = "uint32_t"
        elif packet_args[arg] == 'u16':
            paramtype = "uint16_t"
        elif packet_args[arg] == 'byte':
            paramtype = "char"
        elif packet_args[arg] == 'file':
            argstring += "int %s," %(arg)
            continue
        elif packet_args[arg] == 'data':
            argstring += "void** %s_data,int *%s_len," % (arg, arg)
            continue
        argstring += paramtype + " *" + arg + ","
    argstring = argstring[:-1]
    packet_read_astrings[packet_type] = argstring
    packettypes[packet_type] = packet_args
    packet_ids[packet_type] = i
    i += 1
print(packettypes)

h.write("//Automaticly generated by gen_hydrapacket.py\n")
c.write("//Automaticly generated by gen_hydrapacket.py\n")

h.write("#ifndef _HYDRAPACKET_H_\n")
h.write("#define _HYDRAPACKET_H_\n")
h.write("#include <arpa/inet.h>\n")
h.write("#include <sys/types.h>\n")

for ptype in packettypes:
    h.write("#define HYDRA_PACKET_%s %d\n" % (ptype, packet_ids[ptype]))

for ptype in packettypes:
    h.write("extern int hydra_read_%s (%s);\n" % (ptype, packet_read_astrings[ptype]))
    h.write("extern int hydra_write_%s(%s);\n" % (ptype, packet_write_astrings[ptype]))
h.write("extern int hydra_get_next_packettype(int fd);\n")
h.write("#endif")

#includes for hydrapacket.c
for inc in includes:
    c.write("#include <%s>\n" % inc)
c.write("#include \"hydrapacket.h\"\n")
#write utility functions
c.write("\
int read_data(int fd, int *len, void **data) {\n\
    int i;\n\
    if ((i = read(fd, len, 4)) < 4) {\n\
        return i;\n\
    }\n\
    *len = ntohl(*len);\n\
    *data = malloc(*len);\n\
    if ((i = read(fd, *data, *len)) != *len) {\n\
        return i;\n\
    }\n\
}\n\
\n\
int write_data(int fd, int len, void *data) {\n\
    int i;\n\
    uint32_t u32;\n\
    u32 = len;\n\
    u32 = htonl(u32);\n\
    if ((i = write(fd, &u32, 4)) != 4) {return i;} \n\
    if ((i = write(fd, data, len)) != len) {return i;}\n\
}\n\
\n\
int read_file(int fd, int out) {\n\
    uint32_t l;\n\
    int nbytes; \n\
    if (read(fd, &l, sizeof(uint32_t))) {return -1;}\n\
    l = ntohl(l);\n\
    sendfile(out, fd, 0, l);\n\
}\n\
\n\
int write_file(int fd, int in) {\n\
    struct stat info;\n\
    uint32_t w;\n\
    if (fstat(in, &info) < 0) {return -1;}\n\
    w = htonl((uint32_t)(info.st_size));\n\
    if (write(fd, &w, sizeof(uint32_t)) < 0) {return -1;}\n\
    sendfile(fd, in, 0, info.st_size);\n\
}\n\
")

def gen_write_file(name):
    c.write("\nif ((i = write_file(fd, %s)) <= 0) {return i;}\n" %(name))

def gen_read_file(name):
    c.write("\nif ((i = read_file(fd, %s)) <= 0) {return i;}\n" %(name))

def gen_read_data(dname):
    c.write("\n\
    if ((i = read_data(fd, %s_len, %s_data)) <= 0) {return i;}\n" %(dname, dname))

def gen_write_data(dname):
    c.write("\n\
    write_data(fd, %s_len, %s_data);" %
    (dname, dname))

def gen_read_char(name):
    c.write("    if ((i = read(fd, %s, 1)) != 1) {return i;}\n" % (name))

def gen_write_char(name):
    c.write("    if ((i = write(fd, &%s, 1)) != 1) {return i;}\n" % (name))

def gen_read_u32(name):
    c.write("\n\
    if ((i = read(fd, &u32, sizeof(uint32_t))) != sizeof(uint32_t)) {return i;}\n\
    *%s = ntohl(u32);\n"
    % (name))

def gen_write_u32(name):
    c.write("\n\
    u32 = %s; \n\
    u32 = htonl(u32);\n\
    if ((i = write(fd, &u32, sizeof(uint32_t))) != sizeof(uint32_t)) {return i;}\n"
    % (name))

def gen_read_u16(name):
    c.write("\n\
    if ((i = read(fd, &u16, sizeof(uint16_t))) != sizeof(uint16_t)) {return i;}\n\
    *%s = ntohs(u16);\n"
    % (name))

def gen_write_u16(name):
    c.write("\n\
    u16 = %s; \n\
    u16 = htons(u16);\n\
    if ((i = write(fd, &u16, sizeof(uint16_t))) != sizeof(uint16_t)) {return i;}\n"
    % (name))

writes = {
        "byte":gen_write_char,
        "data":gen_write_data,
        "u32": gen_write_u32,
        "u16": gen_write_u16,
        "file": gen_write_file,
        }

reads = {
        "byte":gen_read_char,
        "data":gen_read_data,
        "u32": gen_read_u32,
        "u16": gen_read_u16,
        "file": gen_read_file,
        }
for ptype in packettypes:
    print("Generating ptype", ptype)
    c.write("int hydra_read_%s(%s) {\n" % (ptype, packet_read_astrings[ptype]))
    c.write("    int i; uint16_t u16; uint32_t u32;\n");
    packet_args = packettypes[ptype]
    for arg in packet_args:
        print ("Generating argument", arg, "type", packet_args[arg])
        if packet_args[arg] in writes:
            reads[packet_args[arg]](arg)
        else:
            print("Warning: no write function for argtype", packet_args[arg])
    c.write("   return 0;\n")
    c.write("}\n")
    c.write("int hydra_write_%s(%s) {\n" % (ptype, packet_write_astrings[ptype]))
    c.write("    int i; uint16_t u16; uint32_t u32; char type;\n");
    c.write("    type = %s;\n" % packet_ids[ptype])
    c.write("    if (write(fd, &type, 1) != 1) {return -1;}\n")
    for arg in packet_args:
        print ("Generating argument", arg, "type", packet_args[arg])
        if packet_args[arg] in writes:
            writes[packet_args[arg]](arg)
        else:
            print("Warning: no write function for argtype", packet_args[arg])
    c.write("    return 0;\n")
    c.write("}\n")

c.write("\
int hydra_get_next_packettype(int fd) {\n\
    char c;\n\
    if (read(fd, &c, 1) != 1) {\n\
        return -1;\n\
    }\n\
    return c;\n\
}")
