// Copyright 2013 Reed Koser, James Forcier, Michael Smith, Fox Wilson

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#define KEY_DELIM "="

static ConfigEntry *parse_line(char *);
static void trim_char(char *, char);
static ValueType get_value_type(const char *);
static int value_is_number(const char *);
static int value_is_list(const char *);
static int value_is_string(const char *);

ConfigFile *parse_config(const char *filename) {
    ConfigFile *parsed_config = malloc(sizeof(ConfigFile));
    ConfigFile *current = NULL;
    ConfigEntry *entry = NULL;
    FILE *config_file = NULL;
    char *line = NULL;
    int status;
    size_t n = 0;

    memset(parsed_config, 0, sizeof(ConfigFile));
    current = parsed_config;
    config_file = fopen(filename, "r");
    if(config_file == NULL) {
        free(parsed_config);
        return (void *)-1;
    }
    while((status = getline(&line, &n, config_file)) > -1) {
        current->next = NULL;
        trim_char(line, ' '); 
        trim_char(line, '\n');
        entry = parse_line(line);
        if(entry == NULL) {
            continue;
        }
        current->next = malloc(sizeof(ConfigFile));
        current = current->next;
        current->entry = entry;
    }
    if(config_file != NULL) {
        fclose(config_file);
        config_file = NULL;
    }
    if(line != NULL) {
        free(line);
        line = NULL;
    }
    return parsed_config;
}

// TODO: Add logging of errors
static ConfigEntry *parse_line(char *line) {
    ConfigEntry *parsed_line;
    char *key, *value, *key_buf, *val_buf;
    // Check for Comments
    if(line[0] == '#') {
        return NULL;
    }
    key = line;
    value = line;
    strsep(&value, KEY_DELIM);
    trim_char(key, ' ');
    trim_char(value, ' ');
    if(strlen(key) < 1) {
        return NULL;
    }
    if(strlen(value) < 1) {
        return NULL;
    }
    key_buf = malloc(sizeof(char) * strlen(key) + 1);
    val_buf = malloc(sizeof(char) * strlen(value) + 1);
    strcpy(key_buf, key);
    strcpy(val_buf, value);
    parsed_line = malloc(sizeof(ConfigEntry));
    parsed_line->key = key_buf;
    parsed_line->value = val_buf;
    parsed_line->value_type = get_value_type(value);
    return parsed_line;
}

static void trim_char(char *str, char ch) {
    char *p, *q;
    for(q = p = str; *p; p++) {
        if(*p != ch) {
            *q++ = *p;
        }
    }
    *q = '\0';
}

static ValueType get_value_type(const char *value) {
    if(value_is_number(value)) {
        return NUMBER;
    } else if(value_is_list(value)) {
        return LIST;
    } else if(value_is_string(value)) {
        return STRING;
    }
    return UNKNOWN;
}

static int value_is_number(const char *value) {
    for(; *value != '\0'; value++) {
        if(!(isdigit(*value) || (*value == '.'))) {
            return 0;
        }
    }
    return 1;
}

static int value_is_list(const char *value) {
    return (value[0] == '[') && (value[strlen(value) - 1] == ']');
}

static int value_is_string(const char *value) {
    return (value[0] == '"') && (value[strlen(value) - 1] == '"');
}
