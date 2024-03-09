#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */

  for(int i=0; i<array_length; ++i)
  {
    mapped_array[i] = f(array[i]);
  }

  return mapped_array;
}


char my_get(char c)
{
  int char_int = fgetc(stdin);

  return (char)char_int;
}


char cprt(char c)
{
  if(c >= 0x20 && c <= 0x7E){
    printf("%c\n", c);
  }
  else
  {
    printf(".\n");
  }
  
  
  return c;
}

char encrypt(char c)
{
  if(c >= 0x20 && c <= 0x7E)
  {
      return c+1;
  }
  
  return c;
}

char decrypt(char c)
{
  if(c >= 0x20 && c <= 0x7E)
  {
      return c-1;
  }
  
  return c;
}

char xprt(char c)
{
  if(c >= 0x20 && c <= 0x7E){
    printf("%02X\n",c);
  } 
  else
  {
    printf(".\n");
  }
   
  
  return c;
}

struct fun_desc
{
  char *name;
  char (*fun)(char);
};

void print_the_menu(struct fun_desc menu_items[], size_t menu_size)
{
  printf("Select from menu :\n");
  for(int i = 0 ; i < menu_size ; ++i)
  {
    printf("%zu. %s\n", i, menu_items[i].name);
  }
};

 
int main(int argc, char **argv)
{
  char *carray = (char[5]){};
  
  struct fun_desc menu[] =
  {
    {"Get String", my_get},
    {"Print String", cprt},
    {"Print Hex", xprt},
    {"Encrpting string", encrypt},
    {"Decrpting string", decrypt},
    {NULL, NULL}
  };
  
  int menu_size = sizeof(menu) / sizeof(menu[0]) -1;

  while(1)
  {
    print_the_menu(menu, menu_size);

    char input[256];

    char* input_data = fgets(input, sizeof(input), stdin);
    if(input_data == NULL)
    {
      printf("Exit.\n");
      break;
    }

    while(input_data[0] == '\n')
    {
      input_data = fgets(input, sizeof(input), stdin);
    }

    int menu_choice;
    int scanf_val = sscanf(input, "%d", &menu_choice);
    if(scanf_val != 1 || menu_choice < 0 || (size_t)menu_choice >= menu_size)
    {
      printf("Out of bound.\n");
      break;
    }

    printf("Within bounds\n");
    
    char* map_carray = map(carray, 5, menu[menu_choice].fun);
    memcpy(carray, map_carray, 5);
    printf("DONE.\n");

    free(map_carray);
  }

  return 0;
}