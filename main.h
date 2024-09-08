#ifndef __MAIN__
#define __MAIN__
#include "elf.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#define INPUT_FILE_NAME "C:/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define OUTPUT_FILE_NAME "main.bin"
typedef struct 
{
    int index;
    Elf32_Shdr* header;
    int* loadedAddress;
    char*name_;
    bool loaded;
}section_t;
typedef struct 
{
    int numberOfSections;
    section_t* sections;
}sections_t;
typedef struct 
{
    int numberOfSections;
    int* sectionIndex;
    sections_t* sections;
}subSections_t;
extern FILE* inputFile,outputFile;
extern long size;
extern sections_t sections;
#endif