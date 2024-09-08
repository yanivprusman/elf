#include "main.h"
FILE* inputFile,outputFile;
long size;
char*inputData;
Elf32_Ehdr *startHeader=NULL;
sections_t sections;
Elf32_Off headerNames;
Elf32_Rela *rela=NULL;
Elf32_Sym *symbol=NULL;
void*rodataStart=0;
void setSectionPointers(){
    Elf32_Shdr* section;
    section =(Elf32_Shdr*) (inputData+ startHeader->e_shoff+ startHeader->e_shstrndx*sizeof(Elf32_Shdr));
    headerNames = section->sh_offset;
    sections.sections = (section_t*) malloc(startHeader->e_shnum*sizeof(section_t));
    for(int x=0;x<startHeader->e_shnum;x++){
        sections.sections[x].index =x;
        sections.sections[x].header =(Elf32_Shdr*)(inputData+startHeader->e_shoff+x*sizeof(Elf32_Shdr));
        // int y = sections.sections[x].header->sh_name+inputData+section->sh_offset;
        sections.sections[x].name_ = sections.sections[x].header->sh_name+inputData+headerNames;
        sections.sections[x].loaded=false;
    }
}
void readFile(){
    inputFile = fopen(INPUT_FILE_NAME,"rb");
    fopen(OUTPUT_FILE_NAME,"r+");
    fseek(inputFile, 0, SEEK_END); 
    long size = ftell(inputFile);
    inputData = malloc(size);
    fseek(inputFile, 0, SEEK_SET); 
    fread(inputData,size,1,inputFile);
    startHeader=(Elf32_Ehdr*)inputData;
    // printf("Input file size 0x%x",size);
    setSectionPointers();

}
subSections_t getSectionsBySubstring(char*subString){
    int count=0;
    for (int x = 0; x < startHeader->e_shnum; x++)
    {
        if(strstr(sections.sections[x].name_,subString)!=NULL){
            count++;
        }
    }
    subSections_t subSections;
    subSections.numberOfSections=count;
    subSections.sectionIndex= malloc(count);
    subSections.sections=&sections;
    int subSectionIndex=0;
    for (int x = 0; x < startHeader->e_shnum; x++){
        if(strstr(sections.sections[x].name_,subString)!=NULL){
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
// void*loadSection(section_t loadedSection){
//     loadedSection.loadedAddress = malloc()
//     return NULL;
// }
void loadRodata(){
    subSections_t rodataSections = getSectionsBySubstring("rodata");
    int size=0;
    for(int x=0;x<rodataSections.numberOfSections;x++){
        section_t * section = &sections.sections[rodataSections.sectionIndex[x]];
        size+=section->header->sh_size;
    }
    rodataStart=malloc(size);
    void*nextLocation = rodataStart;
    for(int x=0;x<rodataSections.numberOfSections;x++){
        section_t * section = &sections.sections[rodataSections.sectionIndex[x]];
        void* locationOfSectionData = inputData+section->header->sh_offset;
        memcpy(nextLocation,locationOfSectionData,section->header->sh_size);
        section->loaded=true;
        section->loadedAddress=nextLocation;
        nextLocation+=section->header->sh_size;
    }
}
void main(){
    readFile();
    subSections_t literalSections = getSectionsBySubstring("literal");
    subSections_t textSections = getSectionsBySubstring("text");
    subSections_t symbolsSections = getSectionsBySubstring(".symtab");
    subSections_t relaSections = getSectionsBySubstring(".rela.literal.app_main");
    FILE *fBefore= fopen("beforeRelocation.bin","r+");
    section_t relaSection=sections.sections[relaSections.sectionIndex[0]];
    void*source=relaSection.header->sh_offset+inputData;
    fwrite(source,relaSection.header->sh_size,1,fBefore);
    fclose(fBefore);
    loadRodata();
    section_t *relocationSection,*symbolSection,*appliedSection;
    section_t *loadedSection;
    for(int x=0;x<relaSections.numberOfSections;x++){
        relocationSection = &sections.sections[relaSections.sectionIndex[x]];
        if (!(relocationSection->header->sh_type==SHT_RELA)) continue;
        symbolSection = &sections.sections[relocationSection->header->sh_link];
        appliedSection = &sections.sections[relocationSection->header->sh_info];
        uint32_t * relocationsOffset=(int*)((relocationSection->header->sh_offset) + inputData);
        // int* relocationsOffset2=relocationSection->header->sh_offset + inputData;
        int numberOfRelocations =relocationSection->header->sh_size/sizeof(Elf32_Rela);
        for(int y=0;y<numberOfRelocations;y++){
            rela=(Elf32_Rela*)((char*)relocationsOffset +y*sizeof(Elf32_Rela));
            int type = ELF32_R_TYPE(rela->r_info);
            if (type == 1){//R_XTENSA_32
                int* relocationAddress = (int*)((char*)relocationsOffset+(rela->r_offset));
                int symbolIndex=ELF32_R_SYM(rela->r_info);
                symbol= (Elf32_Sym *)(inputData+ symbolSection->header->sh_offset+symbolIndex*sizeof(Elf32_Sym));
                if(ELF32_ST_TYPE(symbol->st_info)==STT_SECTION){
                    loadedSection=&sections.sections[symbol->st_shndx];
                    if(!loadedSection->loaded){
                        printf("Error section %s not loaded\n",loadedSection->name_);
                        exit(1);
                    }
                    *relocationAddress = (uint32_t)(uintptr_t)(loadedSection->loadedAddress);
                }
            }
        }
    }
    FILE *fAfter= fopen("afterRelocation.bin","r+");
    relaSection=sections.sections[relaSections.sectionIndex[0]];
    source=relaSection.header->sh_offset+inputData;
    fwrite(source,relaSection.header->sh_size,1,fAfter);

}


