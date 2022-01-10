#include "MemManager.h"


TLB_table TLB[TLB_SIZE];
_Bool *disk;   //1 for empty
_Bool *memory; // 1 for empty
page_table **pte;
page_table *victim;
int *hit;//TLB
int *miss;//TLB
int *page_hit;
int *page_miss;

void initialize(int num_process, int virtual_page, int frames)
{
    pte = (page_table**) malloc(sizeof(page_table*)* num_process);
    for (int i = 0; i <num_process; i++)
        pte[i] = malloc(sizeof(page_table)* virtual_page);//a 2d array pte[x][y]->page y of process x
    for (int i = 0; i<num_process; i++)
    {
        for (int j = 0; j<virtual_page; j++)
        {
            pte[i][j].location = -1;
            pte[i][j].present = 0;
            pte[i][j].process = i;
        }
    }
    victim = (page_table*) malloc(sizeof(page_table)* frames);
    for (int i = 0; i <frames; i++)
    {
        victim[i].location = -1;
        victim[i].process =-1;
        victim[i].reference= 0;
        victim[i].vpn = -1;
    }
    memory = malloc(sizeof(_Bool)* frames);
    for (int i = 0; i <frames; i++)
        memory[i] = 1;
    disk = malloc(sizeof(_Bool)* num_process * virtual_page);
    for (int i = 0; i < num_process * virtual_page; i++)
        disk[i] = 1;

    miss = malloc(sizeof(int)* num_process);
    hit = malloc(sizeof(int)* num_process);
    page_miss = malloc(sizeof(int)* num_process);
    page_hit = malloc(sizeof(int)* num_process);
    for (int i = 0; i < num_process; i++)
    {
        hit[i] = 0;
        miss[i] = 0;
        page_miss = malloc(sizeof(int)* num_process);
        page_hit = malloc(sizeof(int)* num_process);
    }
    return;
}
int main()
{
    extern const int m;
    extern const int t;
    char TLB_policy[6];//LRU RANDOM
    char page_policy[5];//FIFO CLOCK
    char frame_policy[6];//GLOBAL LOCAL
    int num_process;
    int virtual_page;
    int frames;
    getconfig(TLB_policy,page_policy,frame_policy,&num_process,&virtual_page,&frames);
    initialize(num_process, virtual_page, frames);
    char* path_trace = "./trace.txt";
    //char* path_trace = "./testtrace.txt";
    FILE *trace = fopen(path_trace, "r");
    char* path_trace_output = "./trace_output.txt";
    //char* path_trace_output = "./testout.txt";
    FILE *trace_output = fopen(path_trace_output, "w");


    char command[60];
    int process;
    int vpn;
    int current_process = -1;//if change process, flush TLB
    int file_position;
    while(fgets(command, 60, trace)!= NULL)
    {
        //command format:Reference(B, 44)
        vpn = 0;
        process = command[10] - 'A';
        int temp = 13;
        while(command[temp]<= '9' && command[temp] >= '0')
        {
            vpn = vpn*10 + (int)command[temp]-'0';
            temp++;
        }
        if(process != current_process)
        {
            for (int i=0; i<32; i++)
            {
                TLB[i].physical=-1;
                TLB[i].vir=-1;
            }
            current_process = process;
        }//flush TLB if process switched
        char p =(char) process+'A';
        int pfn = search_TLB(TLB,vpn);

        if(pfn == -1)
        {
            miss[process]++;
            pfn = search_page_table(pte[process], vpn);
            if(pfn == -1)
            {
                page_miss[process]++;
                int source, evict_vpn,des;
                char evict_p;
                page_fault_handler(memory,disk,vpn,frames,pte,victim,process,page_policy,frame_policy, &pfn, &source, &evict_p,&evict_vpn, &des);
                //fprintf(trace_output,"123456\n");
                fprintf(trace_output,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n", p, pfn, evict_vpn,evict_p,des,vpn,source);
                fflush(trace_output);
                //Process A, TLB Miss, Page Fault, 0, Evict -1 of Process A to -1, 71<<-1
            }//page fault
            else
            {
                page_hit[process]++;
                fprintf(trace_output,"Process %c, TLB Miss, Page Hit, %d=>%d\n", p, vpn, pfn);
            }
            pte[process][vpn].location = pfn;
            pte[process][vpn].present = 1;
            update_TLB(TLB,vpn, pfn, TLB_policy,process);
            fseek(trace, file_position,SEEK_SET);
        }//search page
        else
        {
            hit[process]++;
            fprintf(trace_output,"Process %c, TLB Hit, %d=>%d\n", p, vpn, pfn);
            fflush(trace_output);
        }
        file_position = ftell(trace);
    }
    fclose(trace_output);
    fclose(trace);

    FILE* anal = fopen("./analysis.txt","w");
    for(int i =0; i<num_process; i++)
    {
        char a = i+'A';
        double TLB_ratio =(double) hit[i]/(hit[i] + miss[i]);
        double EAT = (double)TLB_ratio*(m+t) + (1-TLB_ratio)*(2*m+t);
        fprintf(anal,"Process:%c, Effective Access Time:%.3f\n", a, EAT);
        double fault_ratio = (double)page_miss[i]/(page_miss[i]+page_hit[i]);
        fprintf(anal,"Process:%c, Page Fault Rate:%.3f\n", a, fault_ratio);
    }
    return 0;

}