#include<stdio.h>
#include<stdlib.h>
#include<malloc.h>
#include<string.h>
#include<time.h>
#define TLB_SIZE 32
static const int m = 100;
static const int t = 40;

typedef struct page_table
{
    int process;//for victim list; although I prefer create another victim struct but HW requires reference bit in page table
    int vpn;

    int location;
    _Bool reference;
    _Bool present;
} page_table;


typedef struct TLB_table
{
    int vir;
    int physical;
} TLB_table;

int search_TLB(TLB_table* TLB,int vpn)
{
    for (int i =0; i<TLB_SIZE; i++)
    {
        if (vpn == TLB[i].vir)
        {
            int ret = TLB[i].physical;
            TLB_table temp = TLB[i];
            for(int j=i; j<TLB_SIZE-1; j++)
                TLB[j] = TLB[j+1];
            TLB[TLB_SIZE-1] = temp;
            return ret;
        }
    }
    return -1;
}//return pfn if hit, -1 if miss
int search_page_table(page_table* table, int vpn)
{

    if(table[vpn].present == 1)
        return table[vpn].location;
    else
        return -1;
}
void update_TLB(TLB_table* TLB, int vpn, int pfn, char* TLB_policy, int process)
{
    for(int i=0; i<TLB_SIZE; i++)
    {
        if(TLB[i].vir==-1)
        {
            TLB[i].vir = vpn;
            TLB[i].physical = pfn;
            return;
        }
    }
    //if full
    if(strcmp(TLB_policy,"LRU")==0)
    {
        for(int i=0; i<TLB_SIZE-1; i++)
        {
            TLB[i] = TLB[i+1];
        }
        TLB[TLB_SIZE-1].vir = vpn;
        TLB[TLB_SIZE-1].physical = pfn;
    }
    else
    {
        srand(time(NULL));
        int index;
        index = rand()%TLB_SIZE;
        TLB[index].vir = vpn;
        TLB[index].physical = pfn;
    }
    return;
}
void remove_victim(page_table* victim,page_table* page,_Bool* disk,int frames, int index, int* des)
{
    for(int i=0;; i++)
    {
        if(disk[i] == 1)
        {
            page[0].location = i;
            page[0].present=0;
            disk[i] = 0;
            *des = i;
            break;
        }
    }//save to disk
    for(int i = index; i< frames-1; i++)
    {
        victim[i] = victim[i+1];
    }//move forward
    return;
}
void page_fault_handler(_Bool* memory,_Bool* disk,int vpn,int frames,page_table** pte, page_table* victim, int process,char* page_policy,char* frame_policy, int* pfn, int* source, char* evict_p, int* evict_vpn, int* des)
{
    //if memory avalable
    for(int i=0; i<frames; i++)
    {

        if(memory[i] == 1)
        {
            *source = pte[process][vpn].location;
            for(int j=0; j<frames; j++)
            {
                if(victim[j].process == -1)
                {
                    victim[j].process = process;
                    victim[j].location= pte[process][vpn].location = i;
                    victim[j].reference= pte[process][vpn].reference = 0;
                    victim[j].vpn = vpn;
                    break;
                }
            }

            memory[i] = 0;
            *evict_p =(char) process + 'A';
            *evict_vpn = -1;
            *pfn = i;
            *des=-1;
            pte[process][vpn].present=1;
            /*if(i==63){
                for(int j=0;j<frames;j++){
                    printf("victim:%d process:%d location:%d\n",j,victim[j].process,victim[j].location);
                }
            }*/
            return;
        }
    }
    //if no memory available
    while(1)
    {
        for(int i=0; i<frames; i++)
        {
            if(strcmp(frame_policy,"LOCAL") == 0 && victim[i].process!=process)
                continue;
            if(strcmp(page_policy, "FIFO") == 0)
            {
                *evict_p =(char) victim[i].process + 'A';
                *evict_vpn = victim[i].vpn;
                *pfn = victim[i].location;
                //printf("pfn=%d\n", *pfn);
                *source = pte[process][vpn].location;

                remove_victim(victim,&pte[victim[i].process][victim[i].vpn],disk,frames,i,des);
                if(pte[process][vpn].location != -1)
                    disk[pte[process][vpn].location] = 1;//if used page will free disk
                victim[frames-1].process = process;
                victim[frames-1].reference=0;
                victim[frames-1].vpn = vpn;
                victim[frames-1].location= *pfn;

                return;
            }
            else //clock
            {
                if(pte[process][vpn].reference == 0)
                {
                    pte[process][vpn].reference = 1;
                }
                else
                {
                    *evict_p =(char) victim[i].process + 'A';
                    *evict_vpn = victim[i].vpn;
                    *pfn = victim[i].location;
                    *source = pte[process][vpn].location;
                    if(pte[process][vpn].location != -1)
                        disk[pte[process][vpn].location] = 1;//if used page will free disk
                    pte[process][vpn].reference = 0;
                    remove_victim(victim,&pte[victim[i].process][vpn],disk,frames,i,des);
                    victim[frames-1].process = process;
                    victim[frames-1].reference=0;
                    victim[frames-1].vpn = vpn;
                    victim[frames-1].location= *pfn;
                    return;
                }
            }
        }
    }

}//return pfn

void getconfig(char* TLB_policy, char* page_policy, char *frame_policy, int *num_process, int *virtual_page, int *frames)
{
    char* path_config = "./sys_config.txt";
    char temp[15];
    FILE* config = fopen(path_config,"r");
    int useless;//to ignore warning
    useless = fscanf(config, "%*[^:]%*c%*c%[^\n]s", TLB_policy);
    useless = fscanf(config, "%*[^:]%*c%*c%[^\n]s", page_policy);
    useless = fscanf(config, "%*[^:]%*c%*c%[^\n]s", frame_policy);
    useless = fscanf(config, "%*[^:]%*c%*c%[^'\n']s", temp);
    *num_process = atoi(temp);
    useless =fscanf(config, "%*[^:]%*c%*c%[^'\n']s", temp);
    *virtual_page = atoi(temp);
    useless = fscanf(config, "%*[^:]%*c%*c%s", temp);
    *frames = atoi(temp);
    fclose(config);
    useless = useless+t+m;//useless line
    return;
}