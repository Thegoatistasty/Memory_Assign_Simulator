# hw4-Memory-Manager
* [Hw4 requirements](https://docs.google.com/presentation/d/1UFuPUwd17Hogh5Vp8GZbnrLRAddGvC1j/edit#slide=id.p3)
## Objective
* Understand how to implement the progress of memory allocating using different policies
## Compile
    make
## Execute
    ./MemManager
# The simulator supports： 
      TLB replacement policy：LRU, RANDOM
      Page replacement policy：FIFO, CLOCK
      Frame alloction policy：GLOBAL, LOCAL
  -->can be modified in sys_config.txt(in UPPER case)
## Input file format
#### Initailize configuration
Users can configure hardware and policies in sys_config
Notice that this simulator does not support more than 26 processes
#### Simulate
To make a memory allocation (modified in trace.txt), you must assign reference in following format
* Reference(A,B) , where A is the name of the process(up to Z) and B is the number of virtual frame number
# Ouput files
### The program gernerate two text files, trace_output.txt and analysis.txt

## trace_output.txt
PFN: frame index that is about to be replaced  
Source: the block number of the page which is page-in from disk  
Destination: the block number where the evicted page page-out 
### Format for a TLB hit:   
Process [X], TLB Hit, [VPN]=>[PFN]  
### Format for a TLB miss:   
Page hit： Process [X], TLB Miss, Page Hit, [VPN]=>[PFN]  
Page fault： Process [X], TLB Miss, Page Fault, [PFN], Evict [VPN] of Process [X] to [Destination], [VPN]<<[Source]  
## analysis.txt
In this file, you get to see the TLB hit ratio and page fault ratio of each process  
![image](https://user-images.githubusercontent.com/39853288/150083201-061f7ba9-67ae-4115-93aa-5a19f3d750d8.png)
