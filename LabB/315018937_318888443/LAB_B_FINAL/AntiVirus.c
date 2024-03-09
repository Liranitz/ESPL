#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC_LE "VIRL"
#define MAGIC_BE "VIRB"


typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

void list_print(link* virus_list, FILE* output);
link* list_append(link* virus_list, virus* data);
void list_free(link* virus_list);
void detect_virus(char* buffer, unsigned int size, link* virus_list, int * offset_lst, int * counter);
void neutralize_virus(char* fileName, int signatureOffset);

void printVirus(virus* v, FILE* output) {
    fprintf(output, "Virus Name: %s\n", v->virusName);
    fprintf(output, "Signature Length: %u\n", v->SigSize);

    fprintf(output, "Signature (Hex): ");
    for (unsigned short i = 0; i < v->SigSize; ++i) {
        fprintf(output, "%02X ", v->sig[i]);
    }
    fprintf(output, "\n\n");
}


int isValidMagic(const char* magic) {
    return (strcmp(magic, MAGIC_LE) == 0) || (strcmp(magic, MAGIC_BE) == 0);
}

int isLittleEndian(const char* magic) {
    return strcmp(magic, MAGIC_LE) == 0;
}

static int is_little_endian;
static int success_reading_indian;

virus* readVirus(FILE* file) {
    virus* v = (virus*)malloc(sizeof(virus));
    if (v == NULL) {
        perror("Memory allocation error\n");
        return NULL;
    }

    if(success_reading_indian == 1)
    {
        fprintf(stdout,"Start read magic\n");
        char magic[5];
        if (fread(magic, sizeof(char), 4, file) != 4) {
            fprintf(stderr, "Failed to read the magic number.\n");
            free(v);
            return NULL;
        }
        magic[4] = '\0'; // Null-terminate the string

        // Check if the magic number is valid
        if (!isValidMagic(magic)) {
            fprintf(stderr, "Invalid magic number: %s\n", magic);
            free(v);
            return NULL;
        }

        if(isLittleEndian(magic))
        {
            is_little_endian = 1;
        }
        else
        {
           is_little_endian = 0;
        }
             
        success_reading_indian = 0;
        fprintf(stdout,"Read success indian\n");
    }

    if (fread(&(v->SigSize), sizeof(unsigned short), 1, file) != 1) {
        fprintf(stderr, "Failed to read the signature length.\n");
        free(v);
        return NULL;
    }

    if (is_little_endian != 1) {
        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);
    }

    if (v->SigSize <= 0 || v->SigSize > 65535) {
        fprintf(stderr, "Invalid signature length: %hu\n", v->SigSize);
        free(v);
        return NULL;
    }

    if (fread(v->virusName, sizeof(char), 16, file) != 16) {
        fprintf(stderr, "Failed to read the virus name.\n");
        free(v);
        return NULL;
    }

    v->sig = (unsigned char*)malloc(v->SigSize * sizeof(unsigned char));
    if (v->sig == NULL) {
        perror("Memory allocation error\n");
        free(v);
        return NULL;
    }

    if (fread(v->sig, sizeof(unsigned char), v->SigSize, file) != v->SigSize) {
        fprintf(stderr, "Failed to read the signature.\n");
        free(v->sig);
        free(v);
        return NULL;
    }


    printf("Read a signature success\n");
    return v;
}

int main(int argc, char **argv) {
    link* virus_list = NULL;
    int offset_lst[10000];
    int counter_offset_viruses = 0;

    while (1) {
        int choice;
        char fileName[2024];

        printf("\n1) Load signatures\n");
        printf("2) Print signatures\n");
        printf("3) Detect viruses\n");
        printf("4) Fix file\n");
        printf("5) Quit\n");
        printf("Enter your choice: ");

        if (fgets(fileName, sizeof(fileName), stdin) == NULL) {
            fprintf(stderr, "Error reading input.\n");
            return EXIT_FAILURE;
        }

        if (sscanf(fileName, "%d", &choice) != 1) {
            fprintf(stderr, "Invalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1:
                printf("Enter the signature file name: \n");
                if (fgets(fileName, sizeof(fileName), stdin) == NULL) {
                    fprintf(stderr, "Error reading input.\n");
                    return EXIT_FAILURE;
                }
                fileName[strcspn(fileName, "\n")] = '\0';  // Remove the newline character

                FILE* sig_file = fopen(fileName, "r+b");
                if (sig_file == NULL) {
                    perror("Error opening signature file\n");
                    break;
                }

                virus* newVirus;
                success_reading_indian = 1;
                while (!feof(sig_file)) {
                    if((newVirus = readVirus(sig_file)) != NULL)
                        virus_list = list_append(virus_list, newVirus);
                    printf("Signature loaded successfully.\n");
                }


                fclose(sig_file);
                break;

            case 2:
                list_print(virus_list, stdout);
                break;

            case 3:
                printf("Detecting viruses...\n");
                // Assume FILE is provided as command-line argument, read it and detect viruses
                if (argc != 2) {
                    printf("Usage: %s <file_name>\n", argv[0]);
                    break;
                }
                FILE* suspect_file = fopen(argv[1], "r+b");
                if (suspect_file == NULL) {
                    perror("Error opening suspect file\n");
                    break;
                }
                fseek(suspect_file, 0, SEEK_END);
                long file_size = ftell(suspect_file);
                fseek(suspect_file, 0, SEEK_SET);
                char* buffer = (char*)malloc(file_size);
                if (buffer == NULL) {
                    perror("Memory allocation error\n");
                    fclose(suspect_file);
                    break;
                }
                fread(buffer, 1, file_size, suspect_file);
                fclose(suspect_file);
                detect_virus(buffer, file_size, virus_list, offset_lst, &counter_offset_viruses);
                free(buffer);
                break;

            case 4:
                printf("Fixing file...\n");
                if (argc != 2) {
                    printf("Usage: %s <file_name>\n", argv[0]);
                    break;
                }
                for(int i = 0; i < counter_offset_viruses; i++)
                {
                    neutralize_virus(argv[1], offset_lst[i]);
                }
                printf("File fixed successfully.\n");
                break;

            case 5:
                list_free(virus_list);
                printf("Exiting the program.\n");
                return EXIT_SUCCESS;

            default:
                printf("Invalid choice. Please enter a valid option.\n");
        }
    }
}

void list_print(link* virus_list, FILE* output) {
    link* current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

link* list_append(link* virus_list, virus* data) {
    link* newLink = (link*)malloc(sizeof(link));
    if (newLink == NULL) {
        perror("Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    newLink->vir = data;
    newLink->nextVirus = NULL;

    if (virus_list == NULL) {
        return newLink;
    } else {
        link* current = virus_list;
        while (current->nextVirus != NULL) {
            current = current->nextVirus;
        }

        current->nextVirus = newLink;
        return virus_list;
    }
}

void list_free(link* virus_list) {
    link* current = virus_list;
    while (current != NULL) {
        link* next = current->nextVirus;
        free(current->vir->sig);  // Free memory allocated for the virus signature
        free(current->vir);       // Free memory allocated for the virus structure
        free(current);            // Free memory allocated for the link
        current = next;
    }
}

void detect_virus(char* buffer, unsigned int size, link* virus_list, int * offset_lst, int * counter) {
    for (unsigned int i = 0; i < size; ++i) {
        link* current = virus_list;
        while (current != NULL) {
            virus* current_virus = current->vir;
            if (i + current_virus->SigSize <= size) {
                if (memcmp(buffer + i, current_virus->sig, current_virus->SigSize) == 0) {
                    printf("Virus detected at byte %u:\n", i);
                    offset_lst[(*counter)] = i;
                    (*counter)++;
                    printf("Name: %s\n", current_virus->virusName);
                    printf("Signature size: %hu\n", current_virus->SigSize);
                }
            }
            current = current->nextVirus;
        }
    }
}

void neutralize_virus(char *fileName, int signatureOffset) {
    printf("Offset value is : %d\n", signatureOffset);
    FILE* file = fopen(fileName, "r+b");
    if (file == NULL) {
        perror("Error opening file\n");
        return;
    }

    if (fseek(file, signatureOffset, SEEK_SET) != 0) {
        perror("Error seeking file\n");
        fclose(file);
        return;
    }

    char retInstruction = '\xC3';
    if (fwrite(&retInstruction, sizeof(char), 1, file) != 1) {
        perror("Error writing to file\n");
        fclose(file);
        return;
    }

    fclose(file);
}

