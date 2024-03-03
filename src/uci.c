#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 128
#define READ_INPUT(to_write) scanf("%127[^\n]", to_write)
#define sizeofString(str) (sizeof(str) / sizeof(char) - 1)


static char command[BUFFER_SIZE];
static char* to_write = &command[1];
static int alreadyInitilized = 0;

static inline int str_compare(const char* base, const char* compareTo, const unsigned int length){
    for (int i = 0; i < length; i++){
        if (base[i] != compareTo[i]){
            return 0;    
        }
    }
    return 1;
}

// static void parse_position_command(){}
// static void parse_go_command(){}

void uci_loop(){
    printf("running\n");
    int c;

    S_Board Board;
    S_Position Pos = {.Board = &Board};
    
    while (1) {
        while ((c = getchar()) != '\n' && c != EOF){
            READ_INPUT(to_write);
            command[0] = c;
            #define DEBUG

            #ifdef DEBUG
            printf("[INFO] Read: [%s]\n", command);
            #endif

            switch (c) {
                case 'u':
                    if (strcmp(command, "uci") == 0){
                        printf("id name Leena\nid author Terys\n\n");
                        /* print options... */
                        printf("option name Hash type spin default 2048 min 100 max 1048576\n");
                        printf("uciok\n");
                    }else if (strcmp(command, "ucinewgame")){
                        load_fen(&Pos, STARTING_POSITION_FEN);
                    }
                    break;
                case 'i':
                    if (strcmp(command, "isready") == 0){
                        init_all();
                        alreadyInitilized = 1;
                        printf("readyok\n");
                    }else{
                        goto default_break;
                    }
                    break;
                case 'p':
                    char* next = command;
                    if (str_compare(command, "position", sizeofString("position"))){
                        next += 9;
                        if (str_compare(next, "fen", sizeofString("fen"))){
                            if (!alreadyInitilized){
                                init_all();
                                alreadyInitilized = 1;
                            }
                            load_fen(&Pos, next + 4);
                        }else if (str_compare(next, "startpos", sizeofString("startpos"))){
                            if (!alreadyInitilized){
                                init_all();
                                alreadyInitilized = 1;
                            }
                            load_fen(&Pos, STARTING_POSITION_FEN);
                        }else {
                            goto default_break;
                        }
                    }else {
                        goto default_break;
                    }
                    break;
                case 'd':
                    print_board(&Pos);
                    break;
                default:
                    default_break:
                    printf("[ERROR] Couldn't parse the command. Exiting...\n");
                    return;
            }
        }
        memset(command, 0, sizeof(char) * BUFFER_SIZE);
    }
    if (alreadyInitilized)
        free(TTable.entries);
}