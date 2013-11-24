################################################################################
#             Generates the hydra packet handling code                         #
################################################################################
import sys

inf = open("hydrapacket.in").read().split("\n")

h = open("hydrapacket.h", "w")
c = open("hydrapacket.c", "w")

def print_flower_box(out, message):
    num_spaces = (80 - len(message)) // 2
    out.write("/*" + ("*" * 78) + "\\\n")
    out.write("*")
    out.write(" " * num_spaces)
    out.write(message)
    out.write(" " * num_spaces)
    out.write("*\n")
    out.write("\\" + ("*" * 78) + "*/\n")
################################################################################
#                               Scanning                                       #
################################################################################

class Packet:
    packet_name = ""
    packet_id = -1
    """ of format (type, name)"""
    packet_args = []

    def __init__ (self, packet_name, packet_id, packet_args):
        self.packet_name = packet_name
        self.packet_id = packet_id
        self.packet_args = packet_args

packet_specs = []

last_id = -1
for line in inf:
    line = line.strip()
    if len(line) == 0 or line[0] == '#':
        continue
    #remove end of line comments
    line = line.split('#')
    line = line[0]
    #split up the line into individual tokens
    line = line.split()
    line = [x.strip() for x in line]
    #The name of the packet must be the first thing in the line
    last_id += 1
    packet_name = line[0]
    packet_id = last_id
    packet_args = []
    line = line[1:]
    for spec in line:
        spec = spec.split(':')
        packet_args.append((spec[0], spec[1]))
    packet_specs.append(Packet(packet_name, packet_id, packet_args))

################################################################################
#                               Generation                                     #
################################################################################

print_flower_box(h, "Automaticly generated by gen_hydrapacket.py")
print_flower_box(c, "Automaticly generated by gen_hydrapacket.py")

h.write("\n")
c.write("\n")

h.write("#ifndef _HYDRAPACKET_H_\n")
h.write("#define _HYDRAPACKET_H_\n")
h.write("#include <arpa/inet.h>\n")
h.write("#include <sys/types.h>\n")

argtype_mappings = {
        'u32': 'uint32_t',
        'u16': 'uint16_t',
        'byte': 'char',
        'file': 'int',
}

#layout of hydra packet sturctures:
#struct hydra_packet_pname {
#   uint32_t u32_arg;
#   uint16_t u16_arg;
#   char     byte_arg;
#   int      file_arg;
#   size_t   data_arg_length;
#   void *   data_arg;
#   
#}
################################################################################
#                          Header File Generation                              #
################################################################################

for packet_spec in packet_specs:
    h.write("#define HYDRA_PACKET_%s %d\n" % (packet_spec.packet_name.upper(), packet_spec.packet_id))
h.write("\n")
for packet_spec in packet_specs:
    h.write("struct hydra_packet_%s {\n" % (packet_spec.packet_name.lower()))
    for arg in packet_spec.packet_args:
        if arg[0] != 'data' and arg[0] in argtype_mappings:
            h.write("    %s %s;\n" % (argtype_mappings[arg[0]], arg[1]))
        elif arg[0] == 'data':
            h.write("    size_t %s_length;\n" % (arg[1]))
            h.write("    void *%s;\n" % (arg[1]))
        else:
            print("Warning: tried to generate for unknown argument type", arg[0])
            sys.exit(-1)
    h.write("};\n\n")
h.write("typedef struct hydra_packet {\n")
h.write("    unsigned char id;\n")
h.write("    union {\n")
for packet_spec in packet_specs:
    h.write("        struct hydra_packet_{0} {0};\n".format(packet_spec.packet_name.lower()))
h.write("    };\n")
h.write("} HydraPacket;\n\n")

print_flower_box(h, "INTERFACE")
h.write("//if reading things with type 'file' the fd field coresponding to the file argument\n")
h.write("//must be set to the output file descriptor\n")
h.write("extern int hydra_read_packet(int from_fd, HydraPacket *into);\n")
h.write("//if writting things with type 'file' the fd field coresponding to the file argument\n")
h.write("//must be set to a valid file desciptor\n")
h.write("extern int hydra_write_packet(int to_fd, HydraPacket *from);\n")
h.write("//Get the next packet type from fd\n")
h.write("extern int hydra_get_next_packettype(int fd);\n")
print_flower_box(h, "PACKET SPECIFICATIONS")

h.write("#endif")

################################################################################
#                          Source File Generation                              #
################################################################################

#utility functions
c.write(open("hydrapacket.template.c").read())

def gen_read_char(name):
    c.write("if (read(fd, &(%s), 1) != 1) {return -1;}\n" % (name))

def gen_read_u16(name):
    c.write("if (read_u16(fd, &(%s)) < 0) {return -1;}\n" %(name))

def gen_read_u32(name):
    c.write("if (read_u32(fd, &(%s)) < 0) {return -1;}\n" % (name))

def gen_read_file(name):
    c.write("if (read_file(fd, %s) < 0) {return -1;}\n" %(name))

def gen_read_data(dname):
    c.write("if (read_data(fd, &(%s_length), &(%s)) < 0) {return -1;}\n" %(dname, dname))

def gen_write_char(name):
    c.write("if (write(fd, &%s, 1) != 1) {return -1;}\n" % (name))

def gen_write_u16(name):
    c.write("if (write_u16(fd, %s) < 0) {return -1;}\n" % (name))

def gen_write_u32(name):
    c.write("if (write_u32(fd, %s) < 0) {return -1;}\n" % (name))

def gen_write_file(name):
    c.write("if (write_file(fd, %s) < 0) {return -1;}\n" %(name))

def gen_write_data(dname):
    c.write("if (write_data(fd, %s_length, %s) < 0) {return -1;}\n" % (dname, dname))

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

c.write("int hydra_read_packet(int fd, HydraPacket *out) {\n");
c.write("    switch (out->id) {\n")

for packet_spec in packet_specs:
    c.write((" " * 8))
    c.write("case HYDRA_PACKET_%s:\n" % packet_spec.packet_name)
    for arg in packet_spec.packet_args:
        c.write(" " * 12)
        reads[arg[0]]("out->{0}.{1}".format(packet_spec.packet_name.lower(), arg[1]))
    c.write("\n")
    c.write((" " * 12))
    c.write("break;\n")

c.write("    };\n")
c.write("    return 0;\n")
c.write("}\n\n")

c.write("int hydra_write_packet(int fd, HydraPacket *in) {\n");
gen_write_char('in->id')
c.write("    switch (in->id) {\n")

for packet_spec in packet_specs:
    c.write((" " * 8))
    c.write("case HYDRA_PACKET_%s:\n" % packet_spec.packet_name)
    for arg in packet_spec.packet_args:
        c.write(" " * 12)
        writes[arg[0]]("in->{0}.{1}".format(packet_spec.packet_name.lower(), arg[1]))
    c.write((" " * 12))
    c.write("break;\n")

c.write("    };\n")
c.write("    return 0;\n")
c.write("}\n\n")
