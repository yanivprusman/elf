#ifndef __MAIN__
#define __MAIN__
#include "elf.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#define INPUT_FILE_NAME "C:/101_coding/102_esp/112_assorted/101_hi/build/esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj"
#define OUTPUT_FILE_NAME "main.bin"
#define SYMBOL_NAMES_SECTION_NAME ".strtab"
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
typedef struct 
{
    char *name;
    int address;
}function_t;
typedef struct 
{
    int numberOfFunctions;
    function_t*functions;
}functions_t;
// typedef struct 
// {
//     int numberOfSymbols;
//     Elf32_Sym *symbol;
// }symbols_t;
typedef struct 
{
    Elf32_Rela *rela;
    section_t *relocationSection;
    section_t *appliedSection;
    section_t *symbolSection;
    Elf32_Sym *symbol;
    char*symbolName;
    char* symbolType;
    int symbolBind;
    int rType;
}relocation_t;
// extern FILE* inputFile,outputFile;
// extern long size;
// extern sections_t sections;
char*symbolTypes[7]={
    "STT_NOTYPE",
    "STT_OBJECT",
    "STT_FUNC",
    "STT_SECTION",
    "STT_FILE",
    "STT_LOPROC",
    "STT_HIPROC"
};
#endif