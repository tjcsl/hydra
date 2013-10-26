// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

#ifndef _HYDRASLAVE_CONFIG_H_
#define _HYDRASLAVE_CONFIG_H_

enum value_type { NUMBER, STRING, LIST, UNKNOWN };
typedef enum value_type ValueType;

struct config_entry {
   ValueType value_type;
   char *key;
   char *value;
};
typedef struct config_entry ConfigEntry;

struct config_file {
    ConfigEntry *entry;
    struct config_file *next;
};
typedef struct config_file ConfigFile;

ConfigFile *parse_config(const char *);

#endif
