#include <stdio.h>
#include "elf.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
void print_phdr_info(Elf32_Phdr *phdr, int num);
void print_flags(uint32_t flags);
void load_phdr(Elf32_Phdr *phdr, int fd);
int startup(int argc, char **argv, void (*start)());

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    int num_phdrs = ehdr->e_phnum;

    for (int i = 0; i < num_phdrs; i++) {
        func(&phdr[i], arg);
    }

    return 0;
}

int get_flag_value(int flags){
    int ret = PROT_NONE;
    if (flags & PF_R) ret |= PROT_READ;
    if (flags & PF_W) ret |= PROT_WRITE;
    if (flags & PF_X) ret |= PROT_EXEC;
}

const char *get_type_str(unsigned int type) {
    switch (type) {
        case PT_NULL: return "NULL";
        case PT_LOAD: return "LOAD";
        case PT_DYNAMIC: return "DYNAMIC";
        case PT_INTERP: return "INTERP";
        case PT_NOTE: return "NOTE";
        case PT_SHLIB: return "SHLIB";
        case PT_PHDR: return "PHDR";
        default: return "UNKNOWN";
    }
}

const char *get_flag_str(unsigned int flags) {
    static char flag_str[5];
    flag_str[0] = (flags & PF_X) ? 'E' : ' ';
    flag_str[1] = (flags & PF_W) ? 'W' : ' ';
    flag_str[2] = (flags & PF_R) ? 'R' : ' ';
    flag_str[3] = '\0';
    return flag_str;
}

void print_phdr_info(Elf32_Phdr *phdr, int num) {
    printf("%-5s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %-3s 0x%02x\n",
           get_type_str(phdr->p_type), phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,
           phdr->p_filesz, phdr->p_memsz, get_flag_str(phdr->p_flags), phdr->p_align);
}

void print_flags(uint32_t flags) 
{
    printf("Protection flags: ");
    if (flags & PF_R) printf("R");
    if (flags & PF_W) printf("W");
    if (flags & PF_X) printf("E");
    printf("\n");

    printf("Mapping flags: ");
    if (flags & PF_R) printf("PROT_READ | ");
    if (flags & PF_W) printf("PROT_WRITE | ");
    if (flags & PF_X) printf("PROT_EXEC | ");
    printf("MAP_PRIVATE");
    if (flags & PF_W) printf(" | MAP_ANONYMUS");
    printf("\n");
}

void load_phdr(Elf32_Phdr* phdr, int fd)
{
  if (phdr->p_type == PT_LOAD)
  {
    int offset_val = phdr->p_offset&0xfffff000;
    int padd_val = phdr->p_vaddr&0xfff;
    int flags_val = get_flag_value(phdr->p_flags);

    print_phdr_info(phdr, fd);
    printf("\n");
    void* map = mmap((void*)(phdr->p_vaddr& 0xfffff000), phdr->p_memsz+padd_val, flags_val, MAP_FIXED | MAP_PRIVATE, fd, offset_val);

    if(map == MAP_FAILED){
    {
      perror("failed to map Program Header to memory :(");
      close(fd);
      exit(1);
    }
    }
  }
}

int main(int argc, char *argv[]) {
    while(1){
        
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <executable>\n", argv[0]);
        return 1;
    }

    // Open the ELF file
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if(fstat(fd, &st) == -1) { 
        perror("stat failed\n");
        exit(-1);
    }
    // Memory map the ELF file
    void *map_start = mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *) map_start;
    foreach_phdr(map_start,load_phdr,fd);
    startup(argc-1, argv+1, (void *)(header->e_entry));
    }
}
