#include "main.h"
FILE* inputFile,outputFile;
long size;
char*elf;
Elf32_Ehdr *startHeader=NULL;
sections_t sections;
// Elf32_Sym *symbols;
Elf32_Off headerNames;
// Elf32_Rela *rela=NULL;
// Elf32_Sym *symbol=NULL;
void*rodataStart=0,*textStart=0,*literalStart=0;
char *symbolNames;
functions_t functions;

void setSectionPointers(){
    Elf32_Shdr* section;
    sections.numberOfSections = startHeader->e_shnum;
    section =(Elf32_Shdr*) (elf+ startHeader->e_shoff+ startHeader->e_shstrndx*sizeof(Elf32_Shdr));
    headerNames = section->sh_offset;
    sections.sections = (section_t*) malloc(startHeader->e_shnum*sizeof(section_t));
    for(int x=0;x<startHeader->e_shnum;x++){
        sections.sections[x].index =x;
        sections.sections[x].header =(Elf32_Shdr*)(elf+startHeader->e_shoff+x*sizeof(Elf32_Shdr));
        // int y = sections.sections[x].header->sh_name+inputData+section->sh_offset;
        sections.sections[x].name_ = sections.sections[x].header->sh_name+elf+headerNames;
        sections.sections[x].loaded=false;
    }
}
void getSymbolNames(){
    bool found=false;
    for(int x=0;x<sections.numberOfSections;x++){
        if(strcmp(sections.sections[x].name_,SYMBOL_NAMES_SECTION_NAME)==0){
            symbolNames=elf + sections.sections[x].header->sh_offset;
            found = true;
            break;
        }
    }
    if(!found){
        printf("Error cant find symbol names section\n");
        exit(1);
    }
}
// void getSymbols(){

// }
void readFile(){
    inputFile = fopen(INPUT_FILE_NAME,"rb");
    fseek(inputFile, 0, SEEK_END); 
    long size = ftell(inputFile);
    elf = malloc(size);
    fseek(inputFile, 0, SEEK_SET); 
    fread(elf,size,1,inputFile);
    startHeader=(Elf32_Ehdr*)elf;
    // printf("Input file size 0x%x",size);
    setSectionPointers();
    // fetSymbols();
    getSymbolNames();

}
subSections_t getSectionsBySubstringAndType(char*subString,Elf32_Word sectionType){
    int count=0;
    for (int x = 0; x < startHeader->e_shnum; x++)
    {
        if((strstr(sections.sections[x].name_,subString)!=NULL)&&((sections.sections[x].header->sh_type&sectionType)==sectionType)){
            count++;
        }
    }
    subSections_t subSections;
    subSections.numberOfSections=count;
    subSections.sectionIndex= malloc(count);
    int subSectionIndex=0;
    for (int x = 0; x < startHeader->e_shnum; x++){
        if((strstr(sections.sections[x].name_,subString)!=NULL)&&((sections.sections[x].header->sh_type&sectionType)==sectionType)){
            subSections.sectionIndex[subSectionIndex++] = x;
        }
    }
    return subSections;
}
void printSubSectionNames(subSections_t subSections){
    for (int x = 0; x < subSections.numberOfSections; x++)
    {
        printf("%s\n",sections.sections[subSections.sectionIndex[x]].name_);
    }
}
void *loadSections(subSections_t subSections){
    int size=0;
    for(int x=0;x<subSections.numberOfSections;x++){
        section_t * section = &sections.sections[subSections.sectionIndex[x]];
        size+=section->header->sh_size;
    }
    void*loadStart=malloc(size);
    void*nextLocation = loadStart;
    for(int x=0;x<subSections.numberOfSections;x++){
        section_t * section = &sections.sections[subSections.sectionIndex[x]];
        void *locationOfSectionData = elf+section->header->sh_offset;
        memcpy(nextLocation,locationOfSectionData,section->header->sh_size);
        section->loaded=true;
        section->loadedAddress=nextLocation;
        nextLocation+=section->header->sh_size;
    }
    return loadStart;
}
void relocationTest(char* fileName){
    subSections_t appliedSections = getSectionsBySubstringAndType(".text.app_main",SHT_PROGBITS);
    FILE *f= fopen(fileName,"w");
    if(f==0){
        printf("Failed opening file %s: %s\n",fileName,strerror(errno));
        
        exit(1);
    }
    section_t appliedSection=sections.sections[appliedSections.sectionIndex[0]];
    void*source=appliedSection.loadedAddress;
    fwrite(source,appliedSection.header->sh_size,1,f);
    fclose(f);
}
void getFunctions(){
    function_t putS,printF;
    putS.name="puts";
    putS.address=0xaaaa1001;
    printF.name="printf";
    printF.address =0xaaaa1002;
    functions.numberOfFunctions=2;
    functions.functions = malloc(functions.numberOfFunctions*sizeof(function_t));
    functions.functions[0] = putS;
    functions.functions[1] = printF;
}
void applyRelocations(subSections_t relocationSections){
    relocation_t relocation;
    section_t *loadedSection;
    for(int x=0;x<relocationSections.numberOfSections;x++){
        relocation.relocationSection=&sections.sections[relocationSections.sectionIndex[x]];
        if (!(relocation.relocationSection->header->sh_type==SHT_RELA)) continue;
        relocation.symbolSection = &sections.sections[relocation.relocationSection->header->sh_link];
        relocation.appliedSection = &sections.sections[relocation.relocationSection->header->sh_info];
        uint32_t * relocationSectionOffset=(int*)((relocation.relocationSection->header->sh_offset) + elf);
        char * appliedSectionOffset=(relocation.appliedSection->loadedAddress);
        int numberOfRelocations =relocation.relocationSection->header->sh_size/sizeof(Elf32_Rela);
        for(int y=0;y<numberOfRelocations;y++){
            relocation.rela=(Elf32_Rela*)((char*)relocationSectionOffset +y*sizeof(Elf32_Rela));
            relocation.rType = ELF32_R_TYPE(relocation.rela->r_info);
            int symbolIndex=ELF32_R_SYM(relocation.rela->r_info);
            relocation.symbol= (Elf32_Sym *)(elf+ relocation.symbolSection->header->sh_offset+symbolIndex*sizeof(Elf32_Sym));
            relocation.symbolName = relocation.symbol->st_name + symbolNames;
            relocation.symbolType = symbolTypes[ELF32_ST_TYPE(relocation.symbol->st_info)];
            relocation.symbolBind = ELF32_ST_BIND(relocation.symbol->st_info);
            char* relocationAddress = (char*)(appliedSectionOffset+(relocation.rela->r_offset));
            if (relocation.rType == 1){//R_XTENSA_32
                if(relocation.symbolType==symbolTypes[STT_SECTION]){
                    loadedSection=&sections.sections[relocation.symbol->st_shndx];
                    if(!loadedSection->loaded){
                        printf("Error section %s not loaded\n",loadedSection->name_);
                        exit(1);
                    }
                    int value = (uint32_t)(uintptr_t)(loadedSection->loadedAddress);//+relocation.symbol->st_value?
                    *(int*)relocationAddress += value;
                }else if(relocation.symbolType==symbolTypes[STT_FUNC]){
                    int address = (uint32_t)(uintptr_t)sections.sections[relocation.symbol->st_shndx].loadedAddress;
                    *relocationAddress=address;
                }else if(ELF32_ST_TYPE(relocation.symbol->st_info)==STT_NOTYPE){
                    bool foud=false;
                    char*symbolName=symbolNames + relocation.symbol->st_name;
                    for(int x=0;x<functions.numberOfFunctions;x++){
                        if(strcmp(symbolName,functions.functions[x].name)){
                            foud=true;
                            *relocationAddress = functions.functions[x].address;
                            break;
                        }
                    }
                    if (!foud){
                        printf("Undefined function- %s\n",symbolName);
                        exit(1);
                    }
                }
            }else if (relocation.rType == 20){//R_XTENSA_SLOT0_OP
                if(relocation.symbolType==symbolTypes[STT_SECTION]){
                    loadedSection=&sections.sections[relocation.symbol->st_shndx];
                    if(!loadedSection->loaded){
                        printf("Error section %s not loaded\n",loadedSection->name_);
                        exit(1);
                    }
                    int destination = (uint32_t)(uintptr_t)(loadedSection->loadedAddress+relocation.rela->r_addend);//+relocation.symbol->st_value?
                    int pc = (uint32_t)(uintptr_t)relocationAddress;
                    uint16_t delta = pc+3-destination;
                    delta = delta>>2;
                    delta = 0x10000 - delta;
                    relocationAddress++;
                    *(uint16_t *)relocationAddress = delta;
                }
                int x =0;
                x++;
                relocation.symbol=relocation.symbol;
                relocation.rela=relocation.rela;
                relocation.symbolType=relocation.symbolType;
                relocation.symbolBind=relocation.symbolBind;
                printf("");
            }else if (relocation.rType == 11){//R XTENSA ASM EXPAN
                int x =0;
                x++;
                relocation.symbol=relocation.symbol;
                printf("");
            }
        }
    }
}

subSections_t combineSubSections(subSections_t sub1,subSections_t sub2){
    subSections_t subSections;
    subSections.numberOfSections=sub1.numberOfSections+sub2.numberOfSections;
    subSections.sectionIndex = malloc(subSections.numberOfSections);
    for(int x=0; x<subSections.numberOfSections;x++){
        subSections.sectionIndex[x]=(x<sub1.numberOfSections)? sub1.sectionIndex[x] : sub2.sectionIndex[x-sub1.numberOfSections];
    }
    return subSections;
}

void main(){
    readFile();
    getFunctions();
    subSections_t rodataSections,literalSections,textSections,combinedLiteralAndTextSections;
    rodataSections = getSectionsBySubstringAndType(".rodata",SHT_PROGBITS);
    rodataStart = loadSections(rodataSections);
    literalSections = getSectionsBySubstringAndType(".literal",SHT_PROGBITS);
    textSections = getSectionsBySubstringAndType(".text",SHT_PROGBITS);
    combinedLiteralAndTextSections=combineSubSections(literalSections,textSections);
    literalStart = loadSections(combinedLiteralAndTextSections);
    relocationTest("beforeRelocation.bin");
    subSections_t relaLiteralSections = getSectionsBySubstringAndType(".rela.literal.app_main",SHT_RELA);
    applyRelocations(relaLiteralSections);
    subSections_t textLiteralSections = getSectionsBySubstringAndType(".rela.text.app_main",SHT_RELA);
    applyRelocations(textLiteralSections);
    relocationTest("afterRelocation.bin");
}


