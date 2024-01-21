#include "defs.h"
#include <stdio.h>
#include <string.h>

static const char* commands[] = {
    "uci",
};
static char command[128];
static char* to_write = &command[1];
static const char* test = "bruh";

#define READ_INPUT scanf("%127[^\n]", to_write)

static void parse_position_command(){}
static void parse_go_command(){}

void uci_loop(){
    printf("running\n");
    int c;
    while (1) {
        while ((c = getchar()) != '\n' && c != EOF){
            READ_INPUT;
            command[0] = c;
            printf("[READ] %s\n", command);

            switch (c) {
                case 'u':
                    if (strcmp(command, "uci") == 0){
                        printf("id name Leena\nid author Terys\n\n");
                        /* print options... */
                        printf("option name Hash type spin default 2048 min 100 max 1048576\n");
                        init_all();
                        printf("uciok\n");
                    }
                    break;
                default:
                    printf("Error! Couldn't parse the command. Exiting...\n");
                    return;
            }
            memset(command, 0, sizeof(char) * 128);
        }
    }
    
}