#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    // Any additional fields you deem necessary
    int display_flag;
} state;

void toggle_debug_mode(state *s) {
    if (s->debug_mode) {
        printf("Debug flag now off\n");
        s->debug_mode = 0;
    } else {
        printf("Debug flag now on\n");
        s->debug_mode = 1;
    }
}

void set_file_name(state *s) {
    printf("Enter file name: ");
    scanf("%s", s->file_name);
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state *s) {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            printf("Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size. Please enter 1, 2, or 4.\n");
    }
}

void quit(state *s) {
    if (s->debug_mode) {
        printf("Quitting\n");
    }
    exit(0);
}

void print_menu() {
    printf("Choose action:\n");
    printf("0-Toggle Debug Mode\n");
    printf("1-Set File Name\n");
    printf("2-Set Unit Size\n");
    printf("3-Load Into Memory\n");
    printf("4-Toggle Display Mode\n");
    printf("5-Memory Display\n");
    printf("6-Save Into File\n");
    printf("7-Memory Modify\n");
    printf("8-Quit\n");
}

void load_into_memory(state *s) {
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: File name not set\n");
        return;
    }

    FILE *file = fopen(s->file_name, "r");
    if (file == NULL) {
        printf("Error: Unable to open file for reading\n");
        return;
    }
    unsigned int location;
    size_t length;
    printf("Enter location and length: ");
    scanf("%X %d", &location, &length);

    if (s->debug_mode) {
        printf("Debug: file name: %s, location: %x, length: %zu\n", s->file_name, location, length);
    }

    fseek(file, location, SEEK_SET);
    if(fread(s->mem_buf, s->unit_size, length, file) < 0)
    {
        perror("Count not load any data to memory");
        return;
    }
    s->mem_count = length * s->unit_size;
    fclose(file);

    printf("Loaded %zu units into memory\n", length);
}

void toggle_display_mode(state *s) {
    static char *display_modes[] = {"Decimal", "Hexadecimal"};
    static int current_mode = 0;

    if (s->debug_mode) {
        if (current_mode == 0) {
            printf("Display flag now on, hexadecimal representation\n");
        } else {
            printf("Display flag now off, decimal representation\n");
        }
    }    
    current_mode = (current_mode + 1) % 2;
    s->display_flag = current_mode;
}

static char* hex_formats[] = {"%hhx\n", "%hx\n", "No such unit", "%x\n"};
static char* dec_formats[] = {"%hhd\n", "%hd\n", "No such unit", "%d\n"};

void print_units(int u, unsigned int val, int is_hexa) {
    if(is_hexa)
    {
        printf(hex_formats[u-1], val);
    }
    else
    {
        printf(dec_formats[u-1], val);
    }
}

void memory_display(state *s) {
    printf("Enter address and length: ");
    unsigned int address;
    size_t length;
    scanf("%x %zu", &address, &length);

    printf("%s\n", s->display_flag ? "Hexadecimal" : "Decimal");
    printf("=======\n");

    void* actual_address = address + s->mem_buf;

    void* end_mem = address + s->mem_buf + s->unit_size*length;

    while (actual_address < (unsigned int) end_mem){
        unsigned char buffer[s->unit_size];
        memcpy(buffer, actual_address, s->unit_size);
        

        unsigned int value = 0;
        for (int i = 0; i < s->unit_size; i++) {
            value |= buffer[i] << (i * 8); // Assuming each byte is 8 bits
        }

        print_units(s->unit_size, value, s->display_flag);
        
        actual_address = actual_address + s->unit_size;
    }
}

void save_into_file(state* s){
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: File name not set\n");
        return;
    }

    FILE *file = fopen(s->file_name, "r+b");
    if (file == NULL) {
        printf("Error: Unable to open file for writing\n");
        return;
    }

    printf("Enter source address, target location, and length (in hex and decimal): ");
    unsigned int source_address, target_location;
    int length;
    scanf("%X %X %d", &source_address, &target_location, &length);
    
    fseek(file, 0, SEEK_END);

    long current = ftell(file);
    if(current < 0){
        printf("location is not valid");
    }

    if (s->debug_mode) {
        printf("Debug: source address: %x, target location: %x, length: %zu\n", source_address, target_location, length);
    }

    fseek(file, target_location, SEEK_SET);

    // if(source_address == 0)
    // {
    //     printf("Got source_addres 0 , update it to mem_buf\n");
    //     source_address = (unsigned int)(s->mem_buf);
    // }
    
    fwrite(s->mem_buf + source_address,s->unit_size,length,file);
    
    fclose(file);
}

void memory_modify(state *s) {
    printf("Enter location and value (in hex): ");
    unsigned int location, value;
    scanf("%x %x", &location, &value);

    if (s->debug_mode) {
        printf("Debug: location: %x, value: %x\n", location, value);
    }
    if(s->mem_count < location){
        printf("Memory count is less than value of location");
        return; 
    }
    else{
    *((unsigned int *)(s->mem_buf + location * s->unit_size)) = value;
    }
}

int main() {
    state s;
    s.debug_mode = 0; // Debug mode off by default
    s.unit_size = 1;  // Default unit size
    s.mem_count = 0;
    

    while (1) {
        if (s.debug_mode) {
            printf("unit_size: %d, file_name: %s, mem_count: %zu\n", s.unit_size, s.file_name, s.mem_count);
        }

        print_menu();

        int option;
        scanf("%d", &option);

        switch (option) {
            case 0:
                toggle_debug_mode(&s);
                break;
            case 1:
                set_file_name(&s);
                break;
            case 2:
                set_unit_size(&s);
                break;
            case 3:
                load_into_memory(&s);
                break;
            case 4:
                toggle_display_mode(&s);
                break;
            case 5:
                memory_display(&s);
                break;
            case 6:
                save_into_file(&s);
                break;
            case 7:
                memory_modify(&s);
                break;
            case 8:
                quit(&s);
                break;
            default:
                printf("Invalid option\n");
        }
    }

    return 0;
}