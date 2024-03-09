#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <elf.h>
#include <string.h> // Include for strcmp
#include <stddef.h> // Include for offsetof

#define MAX_FILE_NAME_LENGTH 256

// Structure for menu options
struct menu_option {
    char *description;
    void (*function)(void);
};

// Global variables
int fd1 = -1, fd2 = -1;
int debug_mode = 0;
void *map_start1 = NULL, *map_start2 = NULL;
Elf32_Ehdr *elf_header1 = NULL, *elf_header2 = NULL;

// Function prototypes
void toggle_debug_mode();
void examine_elf_file();
void print_section_names();
void print_symbols();
void check_files_for_merge();
void merge_elf_files();
void quit();
char* get_section_name(Elf32_Ehdr* elf_header, int index);
int is_mergable_section(Elf32_Shdr *section);
void merge_sections(int fd_out, void *map_start1, void *map_start2, Elf32_Shdr section);
void copy_section(int fd_out, void *map_start1, Elf32_Shdr section);

// Function to read ELF header information
void read_elf_header(int fd, void *map_start);

// Main menu options
struct menu_option options[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine ELF File", examine_elf_file},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", check_files_for_merge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit}
};

int main() {
    int choice;
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        printf("Choose action:\n");
        for (int i = 0; i < num_options; i++) {
            printf("%d-%s\n", i, options[i].description);
        }
        scanf("%d", &choice);
        getchar(); // Clear newline character from input buffer

        if (choice >= 0 && choice < num_options) {
            options[choice].function();
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

void toggle_debug_mode() {
    if(debug_mode == 1){
        printf("Toggle Debug Mode off\n");
        debug_mode--;
    }
    else {
        debug_mode++;
        printf("Toggle Debug mode is on\n");
    }
}

void examine_elf_file() {
    char filename[MAX_FILE_NAME_LENGTH];

    printf("Enter ELF file name: ");
    fgets(filename, MAX_FILE_NAME_LENGTH, stdin);

    // Remove newline character from filename
    filename[strcspn(filename, "\n")] = 0;

    

    // Map ELF file into memory
    if(map_start1 == NULL)
    {
        // Open ELF file
        fd1 = open(filename, O_RDONLY);
        if (fd1 == -1) {
            perror("Error opening file");
            return;
        }
        map_start1 = mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ, MAP_PRIVATE, fd1, 0);
        printf("I WANT TO PRINT ADRESS - %p\n", map_start1);
        if (map_start1 == MAP_FAILED) {
            perror("Error mapping file");
            return;
        }

        // Read ELF header
        elf_header1 = (Elf32_Ehdr *)map_start1;
        read_elf_header(fd1, map_start1);
    }
    else
    {
            // Open ELF file
        fd2 = open(filename, O_RDONLY);
        if (fd2 == -1) {
            perror("Error opening file");
            return;
        }
        map_start2 = mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ, MAP_PRIVATE, fd2, 0);
        printf("I WANT TO PRINT ADRESS - %p\n", map_start2);
        if (map_start2 == MAP_FAILED) {
            perror("Error mapping file");
            return;
        }

        // Read ELF header
        elf_header2 = (Elf32_Ehdr *)map_start2;
        read_elf_header(fd2, map_start2);
    }
    // Unmap file and close file descriptor
    //munmap(map, sizeof(Elf32_Ehdr));
    
}

void read_elf_header(int fd, void *map_start) {
    Elf32_Ehdr *elf_header = map_start;
    printf("------------------------------------------------------\n");
    // Print magic number
    printf("Magic number (bytes 1, 2, 3): %c%c%c\n",
           elf_header->e_ident[EI_MAG0], elf_header->e_ident[EI_MAG1], elf_header->e_ident[EI_MAG2]);

    // Check if it's an ELF file
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 || elf_header->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header->e_ident[EI_MAG2] != ELFMAG2) {
        printf("Not an ELF file.\n");
        return;
    }

    // Print entry point
    printf("Entry point: 0x%x\n", elf_header->e_entry);

    // Print other relevant information from the header

    printf("Data encoding scheme: %d-bit\n", elf_header->e_ident[EI_CLASS] == ELFCLASS32 ? 32 : 64);
    printf("Section header table offset: %d\n", elf_header->e_shoff);
    printf("Number of section header entries: %d\n", elf_header->e_shnum);
    printf("Size of each section header entry: %d\n", elf_header->e_shentsize);
    printf("Program header table offset: %d\n", elf_header->e_phoff);
    printf("Number of program header entries: %d\n", elf_header->e_phnum);
    printf("Size of each program header entry: %d\n", elf_header->e_phentsize);
    printf("------------------------------------------------------\n");
}

// Function prototypes
void print_section_names_helper(int fd, void *map_start, Elf32_Ehdr *elf_header);
void print_symbols_helper(int fd, void *map_start, Elf32_Ehdr *elf_header);

void print_section_names() {
    if (fd1 == -1 && fd2 == -1) {
        printf("No ELF file opened. Please open an ELF file first.\n");
        return;
    }

    if (fd1 != -1) {
        print_section_names_helper(fd1, map_start1, elf_header1);
    }

    if (fd2 != -1) {
        print_section_names_helper(fd2, map_start2, elf_header2);
    }
}

void print_symbols() {
    if (fd1 == -1 && fd2 == -1) {
        printf("No ELF file opened. Please open an ELF file first.\n");
        return;
    }

    if (fd1 != -1) {
        print_symbols_helper(fd1, map_start1, elf_header1);
    }

    if (fd2 != -1) {
        print_symbols_helper(fd2, map_start2, elf_header2);
    }
}

void print_section_names_helper(int fd, void *map_start, Elf32_Ehdr *elf_header) {
    Elf32_Off sh_offset = elf_header->e_shoff;

    // Calculate section header start address
    Elf32_Shdr *section_start = (Elf32_Shdr *)((char *)map_start + sh_offset);

    // Get section names offset
    Elf32_Shdr *section_names_header = &section_start[elf_header->e_shstrndx];
    char *section_names = (char *)map_start + section_names_header->sh_offset;

    // Print section header information
    printf(" Section Headers:\n");
    printf("  [Nr]    Name                 Addr       Off       Size      Type\n");

    for (int i = 0; i < elf_header->e_shnum; i++) {
        // Get section name
        char *section_name = &section_names[section_start[i].sh_name];

        // Print section details
        printf("  [%2d]    %-20s 0x%08x  0x%08x  0x%08x  %d\n", i, section_name,
               section_start[i].sh_addr, section_start[i].sh_offset, section_start[i].sh_size, section_start[i].sh_type);
    }
}

void print_symbols_helper(int fd, void *map_start, Elf32_Ehdr *elf_header) {
    Elf32_Shdr *section_header = (Elf32_Shdr *)((char *)map_start + elf_header->e_shoff);
    Elf32_Shdr *symbol_table = NULL;
    Elf32_Shdr *string_table = NULL;

    // Find symbol table and string table sections
    for (int i = 0; i < elf_header->e_shnum; i++) {
        if (section_header[i].sh_type == SHT_SYMTAB) {
            symbol_table = &section_header[i];
        } else if (section_header[i].sh_type == SHT_STRTAB && i != elf_header->e_shstrndx) {
            string_table = &section_header[i];
        }
    }

    if (symbol_table == NULL || string_table == NULL) {
        printf("No symbol table or string table found.\n");
        return;
    }

    // Get symbol table and string table data
    Elf32_Sym *symbols = (Elf32_Sym *)((char *)map_start + symbol_table->sh_offset);
    char *strtab = (char *)map_start + string_table->sh_offset;

    printf("File ELF-file0name\n"); // Replace "ELF-file0name" with actual filename
    printf("[index] value section_index section_name symbol_name\n");

    // Iterate over symbols and print information
    for (int i = 0; i < symbol_table->sh_size / sizeof(Elf32_Sym); i++) {
        printf("[%d] 0x%08x %d %s %s\n", i, symbols[i].st_value,
               symbols[i].st_shndx, get_section_name(elf_header, symbols[i].st_shndx),
               &strtab[symbols[i].st_name]);
    }
}

// Function to get the name of the section from its index

char* get_section_name(Elf32_Ehdr* elf_header, int index) {
    Elf32_Shdr *section_header = (Elf32_Shdr *)((char *)elf_header + elf_header->e_shoff);
    if (index >= 0 && index < elf_header->e_shnum) {
        return (char *)((char *)elf_header + section_header[index].sh_name);
    }
    return "N/A";
}



void check_files_for_merge() {
    // Check if both ELF files have been opened and mapped
    if (fd1 == -1 || fd2 == -1 || map_start1 == NULL || map_start2 == NULL) {
        printf("Error: Both ELF files must be opened and mapped before checking for merge.\n");
        return;
    }

    // Check if each ELF file contains exactly one symbol table
    Elf32_Shdr *section_header1 = (Elf32_Shdr *)((char *)map_start1 + elf_header1->e_shoff);
    Elf32_Shdr *section_header2 = (Elf32_Shdr *)((char *)map_start2 + elf_header2->e_shoff);
    int num_symbol_tables1 = 0, num_symbol_tables2 = 0;
    for (int i = 0; i < elf_header1->e_shnum; i++) {
        if (section_header1[i].sh_type == SHT_SYMTAB)
            num_symbol_tables1++;
    }
    for (int i = 0; i < elf_header2->e_shnum; i++) {
        if (section_header2[i].sh_type == SHT_SYMTAB)
            num_symbol_tables2++;
    }
    if (num_symbol_tables1 != 1 || num_symbol_tables2 != 1) {
        printf("Error: Each ELF file must contain exactly one symbol table.\n");
        return;
    }

    // Find symbol table sections
    Elf32_Shdr *symbol_table1 = NULL, *symbol_table2 = NULL;
    for (int i = 0; i < elf_header1->e_shnum; i++) {
        if (section_header1[i].sh_type == SHT_SYMTAB) {
            symbol_table1 = &section_header1[i];
            break;
        }
    }
    for (int i = 0; i < elf_header2->e_shnum; i++) {
        if (section_header2[i].sh_type == SHT_SYMTAB) {
            symbol_table2 = &section_header2[i];
            break;
        }
    }

    // Get symbol tables and string tables
    Elf32_Sym *symbols1 = (Elf32_Sym *)((char *)map_start1 + symbol_table1->sh_offset);
    Elf32_Sym *symbols2 = (Elf32_Sym *)((char *)map_start2 + symbol_table2->sh_offset);
    char *strtab1 = (char *)map_start1 + section_header1[symbol_table1->sh_link].sh_offset;
    char *strtab2 = (char *)map_start2 + section_header2[symbol_table2->sh_link].sh_offset;

    // Perform checks on symbols
    printf("Checking for merge:\n");
    for (int i = 1; i < symbol_table1->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym symbol = symbols1[i];
        if (ELF32_ST_BIND(symbol.st_info) == STB_GLOBAL) {
            if (symbol.st_shndx == SHN_UNDEF) {
                // Symbol is UNDEFINED, check if it exists in the second file
                int found = 0;
                for (int j = 1; j < symbol_table2->sh_size / sizeof(Elf32_Sym); j++) {
                    if (strcmp(&strtab1[symbol.st_name], &strtab2[symbols2[j].st_name]) == 0 &&
                        symbols2[j].st_shndx != SHN_UNDEF) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("Error: Symbol %s undefined.\n", &strtab1[symbol.st_name]);
                }
            } else {
                // Symbol is DEFINED, check if it's multiply defined in the second file
                for (int j = 1; j < symbol_table2->sh_size / sizeof(Elf32_Sym); j++) {
                    if (strcmp(&strtab1[symbol.st_name], &strtab2[symbols2[j].st_name]) == 0 &&
                        symbols2[j].st_shndx != SHN_UNDEF) {
                        printf("Error: Symbol %s multiply defined.\n", &strtab1[symbol.st_name]);
                        break;
                    }
                }
            }
        }
    }
    printf("Check completed.\n");
}

void merge_elf_files() {
    // Open the output file "out.ro"
    int fd_out = open("out.ro", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("Error opening output file");
        return;
    }

    printf("hey1\n");
    // Create a new ELF header for the output file by copying the ELF header of the first input file
    Elf32_Ehdr elf_header_out = *elf_header1;

    // Modify the e_shoff field
    elf_header_out.e_shoff = sizeof(Elf32_Ehdr); // Assuming no program headers
    printf("hey2\n");

    // Write the ELF header to the output file
    write(fd_out, &elf_header_out, sizeof(Elf32_Ehdr));
    printf("hey3\n");

    // Create an initial version of the section header table for the output file by copying that of the first ELF file
    Elf32_Shdr *section_headers_out = (Elf32_Shdr *)((char *)map_start1 + elf_header1->e_shoff);
    printf("hey31\n");
    write(fd_out, section_headers_out, elf_header1->e_shnum * sizeof(Elf32_Shdr));
    int x = elf_header1->e_shnum;
    printf("Value of elf1->eshnum is %d\n",x);
    printf("hey4\n");


    // Loop over the section header table entries
    for (int i = 0; i < x; i++) {
        // Check if the section is mergable (e.g., ".text", ".data", ".rodata")
        Elf32_Shdr section = section_headers_out[i]; // Retrieve the current section header
        if (is_mergable_section(&section)) {
            // Concatenate the section contents from both input files and update the section header accordingly
            printf("hey8888\n");
            merge_sections(fd_out, map_start1, map_start2, section);
        } else {
            // Copy the section from the first ELF file and update the section header accordingly
            copy_section(fd_out, map_start1, section);
        }
        printf("hey5 , %d\n",i);
    }

    // Fix the e_shoff field in the ELF header of the output file
    
    lseek(fd_out, offsetof(Elf32_Ehdr, e_shoff), SEEK_SET);
    Elf32_Off e_shoff = elf_header1->e_shoff; // Assuming no program headers
    write(fd_out, &e_shoff, sizeof(Elf32_Off));
    printf("hey6\n");

    // Close the output file
    close(fd_out);
    printf("hey7\n");

    printf("Merging completed. Output file: out.ro\n");
}



void quit() {
    if (map_start1 != NULL)
        munmap(map_start1, sizeof(Elf32_Ehdr));
    if (map_start2 != NULL)
        munmap(map_start2, sizeof(Elf32_Ehdr));
    if (fd1 != -1)
        close(fd1);
    if (fd2 != -1)
        close(fd2);
   
    printf("Exiting program.\n");
    exit(0);
}

	
int is_mergable_section(Elf32_Shdr *section) {
    // Check if the section name is ".text", ".data", or ".rodata"
    // You can adjust this condition based on your requirements
    //TODO : sh_name here is null
    printf("sh_name : %s\n",section->sh_name);
    return (strcmp(section->sh_name, ".text") == 0 ||
            strcmp(section->sh_name, ".data") == 0 ||
            strcmp(section->sh_name, ".rodata") == 0);
}

void merge_sections(int fd_out, void *map_start1, void *map_start2, Elf32_Shdr section) {
    // Get the section contents from both files and concatenate them into the output file
    printf("HEY512");
    off_t offset1 = section.sh_offset;
    off_t offset2 = section.sh_offset;

    // Calculate the total size of the merged section
    size_t total_size = section.sh_size;

    // Allocate buffer for merged section
    char *merged_section = (char *)malloc(total_size);
    if (merged_section == NULL) {
        perror("Error allocating memory for merged section");
        exit(EXIT_FAILURE);
    }

    // Copy contents from the first file
    memcpy(merged_section, (char *)map_start1 + offset1, section.sh_size);

    // Move the file pointer to the position of the second section
    lseek(fd_out, offset2, SEEK_SET);

    // Write the contents from the second file to the end of the merged section
    read(fd_out, merged_section + section.sh_size, section.sh_size);

    // Write the merged section to the output file
    lseek(fd_out, section.sh_offset, SEEK_SET);
    write(fd_out, merged_section, total_size);

    // Free allocated memory
    free(merged_section);
}

void copy_section(int fd_out, void *map_start1, Elf32_Shdr section) {
    // Copy the section contents from the first file to the output file
    off_t offset = section.sh_offset;
    size_t size = section.sh_size;
    printf("HEY513");
    // Allocate buffer for section contents
    char *section_contents = (char *)malloc(size);
    if (section_contents == NULL) {
        perror("Error allocating memory for section contents");
        exit(EXIT_FAILURE);
    }

    // Copy contents from the first file
    memcpy(section_contents, (char *)map_start1 + offset, size);

    // Write the section contents to the output file
    lseek(fd_out, offset, SEEK_SET);
    write(fd_out, section_contents, size);

    // Free allocated memory
    free(section_contents);
}