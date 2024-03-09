#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *infile = NULL;
FILE *outfile = NULL;
FILE *error_output = NULL; 
int debug_mode = 1; // 1 for debug mode, 0 for off
int increase = 1;
char *encoding_key = NULL;

void print_debug(const char *message) {
    if (debug_mode) {
        fprintf(stderr, "%s\n", message);
    }
}

void parse_command_line(int argc, char *argv[]) {
    infile = stdin;
    outfile = stdout;
    error_output = stderr;

    for (int i = 1; i < argc; i++) {
        print_debug(argv[i]);
        if (strcmp(argv[i], "+D") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-D") == 0) {
            debug_mode = 0; 
        } else if (strncmp(argv[i], "+E", 2) == 0 || strncmp(argv[i], "-E", 2) == 0) {
            encoding_key = strdup(argv[i] + 2);
            if(strncmp(argv[i], "-E", 2) == 0)
            {
                increase = 0; // decrease values
            }
        } else if (strncmp(argv[i], "-I", 2) == 0) {
            infile = fopen(argv[i] + 2, "r");
            if (infile == NULL) {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "-O", 2) == 0) {
            outfile = fopen(argv[i] + 2, "w");
            if (outfile == NULL) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
        }       
    }
}

int check_if_digit(char c)
{
    return c >= '0' && c <= '9';
}

int check_if_upper(char c)
{
    return c >= 'A'&& c <= 'Z';
}

void process_input() {
    int c;
    int key_index = 0;
    int key_length = 0;

    while ((c = fgetc(infile)) != EOF) {
        if (check_if_upper(c) || check_if_digit(c)) {
            int key = encoding_key[key_index] - '0';
            if (encoding_key[key_index] == '\0') {
                key_length = key_index;
                key_index = 0;
                key = encoding_key[key_index] - '0';
            }
            if (increase) {
                c += key;
                if (c > 'Z') {
                    c = 'A' + (c - 'Z') - 1;
                }
                else if(c > '9' && isdigit(c - key))
                {
                    c = '0' + (c - '9') - 1;
                }
            } else {
                c -= key;
                if (c < 'A' && isupper(c + key)) {
                    c = 'Z' - ('A' - c) + 1;
                }
                else if(c < '0')
                {
                    c = '9' - ('0' - c) + 1;
                }
            }
        }
        
        if(key_length != 0)
        {
        key_index = (key_index + 1) % key_length;
        }
        else
        {
            key_index++;
        }

        fputc(c, outfile);
    }

    fclose(infile);
    fclose(outfile);
    if (encoding_key != NULL) {
        free(encoding_key);
    }
}

int main(int argc, char *argv[]) {
    parse_command_line(argc, argv);
    process_input();

    return 0;
}