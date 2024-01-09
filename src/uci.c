#include "defs.h"
#include <stdio.h>

#define READ_INPUT scanf("%[^\n]", command)

static const char* commands[] = {
    "uci",
};
static char command[128];

static void parse_position_command(){}
static void parse_go_command(){}

void uci_loop(){
    READ_INPUT;
}