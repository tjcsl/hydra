#Generates the hydra string tokenizer

inf = open("hydrapacket.in").read().split("\n")

h = open("hydrapacket.h", "w")
c = open("hydrapacket.c", "w")

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

#write utility functions
c.write(open("hydrapacket.template.c").read())

def gen_write_file(name):
    c.write("    if (write_file(fd, %s) <= 0) {return -1;}\n" %(name))

def gen_read_file(name):
    c.write("    if (read_file(fd, %s) <= 0) {return -1;}\n" %(name))

def gen_read_data(dname):
    c.write("    if (read_data(fd, %s_len, %s_data) <= 0) {return -1;}\n" %(dname, dname))

def gen_write_data(dname):
    c.write("    write_data(fd, %s_len, %s_data);" %
    (dname, dname))

def gen_read_char(name):
    c.write("    if (read(fd, %s, 1) != 1) {return -1;}\n" % (name))

def gen_write_char(name):
    c.write("    if (write(fd, &%s, 1) != 1) {return -1;}\n" % (name))

def gen_read_u32(name):
    c.write("    if (read_u32(fd, %s) < 0) {return -1;}\n" % (name))

def gen_write_u32(name):
    c.write("    if (write_u32(fd, %s) < 0) {return -1;}\n" % (name))

def gen_read_u16(name):
    c.write("    if (read_u16(fd, %s) < 0) {return -1;}\n" %(name))

def gen_write_u16(name):
    c.write("    if (write_u16(fd, %s) < 0) {return -1;}\n" % (name))

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
    packet_args = packettypes[ptype]
    for arg in packet_args:
        print ("Generating argument", arg, "type", packet_args[arg])
        if packet_args[arg] in writes:
            reads[packet_args[arg]](arg)
        else:
            print("Warning: no write function for argtype", packet_args[arg])
    c.write("    return 0;\n")
    c.write("}\n")
    c.write("int hydra_write_%s(%s) {\n" % (ptype, packet_write_astrings[ptype]))
    c.write("    char type;\n");
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
