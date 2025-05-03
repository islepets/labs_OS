#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define PAGE_SIZE 256
#define PHYSICAL_MEMORY_SIZE 65536 
#define PAGE_TABLE_SIZE 256


typedef struct{
    int page_num;
    int frame_num;
}TLB;


static int next_index = 0;                     //индекс для замены в TLB
TLB tlb[16];                                    
char physicalMemory[PHYSICAL_MEMORY_SIZE];     //физическая память
int pageTable[PAGE_TABLE_SIZE];                //таблица страниц
int fault = 0;                                 //счетчик ошибок
int next_available_frame = 0;                  //следующий свободный кадр в физической памяти
int total = 0;                                 //счетчик обращений
int tlb_hits = 0;                              //счетчик попаданий


void loadPageFromBackingStore(int page_num, int frame) {
    FILE* backing_store_fp = fopen("BACKING_STORE.bin", "rb");
    if (backing_store_fp == NULL) {
        perror("Error opening backing store file");
        exit(1);
    }

    fseek(backing_store_fp, page_num * PAGE_SIZE, SEEK_SET);//перемещаемся к месту в файле, соответствующему нужной странице
    fread(physicalMemory + (frame * PAGE_SIZE), sizeof(char), PAGE_SIZE, backing_store_fp);
    fclose(backing_store_fp);
}

int get_pagenumber(int num) {
    return (num >> 8) & 0xFF;
}


int get_offset(int num) {
    return num & 0xFF;
}

int search_tlb(int page_num){
    for(int i = 0; i < 16; i++){
        if (tlb[i].page_num == page_num){
            return tlb[i].frame_num;
        }
    }
    return -1; 
}


void add_tlb(int page_num, int frame_num){
    tlb[next_index].frame_num = frame_num;
    tlb[next_index].page_num = page_num;
    next_index = (next_index + 1) % 16; 
}


int translate(int address) {
    total++;
    int page_num = get_pagenumber(address);
    int offset_num = get_offset(address);
    int frame = search_tlb(page_num);

    if (frame == -1){
        frame = pageTable[page_num];
        if(frame == -1){
            fault++;
            frame = next_available_frame;
            loadPageFromBackingStore(page_num, frame);
            pageTable[page_num] = frame;
            next_available_frame++; 
        }
        add_tlb(page_num, frame);        
    }
    else{
        tlb_hits++;
    }

    return (frame * 256) + offset_num;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        return EXIT_FAILURE;
    }

    FILE* infile = fopen(argv[1], "r");
    if (infile == NULL) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
        pageTable[i] = -1; 
    }

    for(int i = 0; i < 16; i++){
        tlb[i].frame_num = -1; 
        tlb[i].page_num = -1;
    }

    int virtualAddress;
    while (fscanf(infile, "%d", &virtualAddress) != EOF) {
        int physicalAddress = translate(virtualAddress);
        int val = physicalMemory[physicalAddress];
        printf("VA: %10d  PA: %10d  Val: %10d\n", virtualAddress, physicalAddress, val);
    }

    printf("page fault rate: %.3f\n", ((double)fault / total) * 100.0);
    printf("tlb hit rate: %.3f\n", ((double)tlb_hits / total) * 100.0);

    fclose(infile);
    return EXIT_SUCCESS;
}
