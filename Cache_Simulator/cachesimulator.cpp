/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size in bytes)  t=32-s-b
32 bit address (MSB -> LSB): TAG || SET || OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether the block in the cache is valid or not.
Index Bits : the set index field is used to determine which set in the cache the block is stored in.
Offset Bits: the offset field is used to determine which byte in the block is being accessed.
*/

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;
//access state:
#define NA 0         // no action
#define RH 1         // read hit
#define RM 2         // read miss
#define WH 3         // Write hit
#define WM 4         // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
// You need to define your cache class here, or design your own data structure for L1 and L2 cache

/*
A single cache block:
    - valid bit (is the data in the block valid?)
    - dirty bit (has the data in the block been modified by means of a write?)
    - tag (the tag bits of the address)
    - data (the actual data stored in the block, in our case, we don't need to store the data)
*/

/*
A CacheSet:
    - a vector of CacheBlocks
    - a counter to keep track of which block to evict next
*/

// You can design your own data structure for L1 and L2 cache; just an example here
// class cache
// {

// config params;
// unsigned long tag1, tag2, index1, index2, offset1,offset2;
//     // some cache configuration parameters.
//     // cache L1 or L2
// public:
//     cache(config params){
//         this->params = params;
//         offset1 = (unsigned)log2(params.L1blocksize);
//         offset2 = (unsigned)log2(params.L2blocksize);

//         // initialize the cache according to cache parameters
//     }

//     auto write(auto addr){
//         /*
//         step 1: select the set in our L1 cache using set index bits
//         step 2: iterate through each way in the current set
//             - If Matching tag and Valid Bit High -> WriteHit!
//                                                     -> Dirty Bit High
//         step 3: Otherwise? -> WriteMiss!

//         return WH or WM
//         */
//     }

//     auto writeL2(auto addr){
//         /*
//         step 1: select the set in our L2 cache using set index bits
//         step 2: iterate through each way in the current set
//             - If Matching tag and Valid Bit High -> WriteHit!
//                                                  -> Dirty Bit High
//         step 3: Otherwise? -> WriteMiss!

//         return {WM or WH, WRITEMEM or NOWRITEMEM}
//         */
//     }

//     auto readL1(auto addr){
//         /*
//         step 1: select the set in our L1 cache using set index bits
//         step 2: iterate through each way in the current set
//             - If Matching tag and Valid Bit High -> ReadHit!
//         step 3: Otherwise? -> ReadMiss!

//         return RH or RM
//         */
//     }

//     auto readL2(auto addr){
//         /*
//         step 1: select the set in our L2 cache using set index bits
//         step 2: iterate through each way in the current set
//             - If Matching tag and Valid Bit High -> ReadHit!
//                                                  -> copy dirty bit
//         step 3: otherwise? -> ReadMiss! -> need to pull data from Main Memory
//         step 4: find a place in L1 for our requested data
//             - case 1: empty way in L1 -> place requested data
//             - case 2: no empty way in L1 -> evict from L1 to L2
//                     - case 2.1: empty way in L2 -> place evicted L1 data there
//                     - case 2.2: no empty way in L2 -> evict from L2 to memory

//         return {RM or RH, WRITEMEM or NOWRITEMEM}
//         */
//     }
// };

// Tips: another example:
// class CacheSystem
// {
//     Cache L1;
//     Cache L2;
// public:
//     int L1AcceState, L2AcceState, MemAcceState;
//     // auto read(auto addr){};
//     // auto write(auto addr){};
// };

struct CacheBlock
{
    bool valid;
    bool dirty;
    unsigned long tag;
    bitset<32> addr32;
    // we don't actually need to allocate space for data, because we only need to simulate the cache action
    // or else it would have looked something like this: vector<number of bytes> Data;
};
struct Set
{
    int counter;
    int setSize;
    vector<CacheBlock> blocks;

    Set(int associativity) : blocks(associativity)
    {
        setSize = associativity;
        counter = 0;
    }
    CacheBlock evict()
    {
        CacheBlock temp = blocks[counter];
        blocks[counter].valid = false;
        ++counter %= setSize;
        return temp;
    }

    void print()
    {
        for (auto block : blocks)
        {
            cout << block.tag <<" "<<block.dirty << " "<<block.valid;
        }
        cout << endl;
    }
    // tips:
    // Associativity: eg. resize to 4-ways set associative cache
};

class Cache
{
    vector<Set> L1;
    vector<Set> L2;
    int L1IndexSize;
    int L2IndexSize;
    int L1TagSize;
    int L2TagSize;
    int L1offset;
    int L2offset;
    bool L1FA=false; // L1 fully associative
    bool L2FA=false; // L1 fully associative
public:
    Cache(config cfg)
    {

        L1offset = log2(cfg.L1blocksize);
        L2offset = log2(cfg.L2blocksize);

        int L1Length = cfg.L1size * 1024 / cfg.L1blocksize;
        int L2Length = cfg.L2size * 1024 / cfg.L2blocksize;

        // Initialize L1 and L2 caches
        if (cfg.L1setsize == 0)
        { //L1 is FA
            L1FA = true;
            L1.resize(1, Set(L1Length));
        }
        else
        { //DM or SA
            L1.resize(L1Length / cfg.L1setsize, Set(cfg.L1setsize));
        }

        if (cfg.L2setsize == 0)
        { //L2 is FA
            L2FA = true;
            L2.resize(1, Set(L2Length));
        }
        else
        { //DM or SA
            L2.resize(L2Length / cfg.L2setsize, Set(cfg.L2setsize));
        }

        L1IndexSize = L1FA ? log2(L1Length) : log2(L1Length / cfg.L1setsize);
        L2IndexSize = L2FA ? log2(L2Length) : log2(L2Length / cfg.L2setsize);

        L1TagSize = 32 - L1offset - L1IndexSize;
        L2TagSize = 32 - L2offset - L2IndexSize;
    }
    bool read_access_l1(bitset<32> addr)
    {
        unsigned long tag = (addr >> (L1IndexSize + L1offset)).to_ulong();
        unsigned long fulltag = (addr >> L1offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);
        if (L1FA)
        {
            for (int i = 0; i < L1[0].blocks.size();i++)
            {
                CacheBlock block = L1[0].blocks[i];
                if (block.tag == fulltag)
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            for (int i = 0; i < L1[index].blocks.size();i++)
            {
                CacheBlock block = L1[index].blocks[i];
                if (block.tag == tag)
                {
                    return true;
                }
            }
            return false;
        }
        // cout<<"tag size: "<<32-L1IndexSize-L1offset<<endl;
        // cout<<"index size: "<<L1IndexSize<<endl;
        // cout<<"tag: "<<tag<<endl;
        // cout<<"index: "<<index<<endl;
    };

    bool read_access_l2(bitset<32> addr)
    {
        unsigned long tag = (addr >> (L2IndexSize + L2offset)).to_ulong();
        unsigned long fulltag = (addr >> L2offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
        if (L2FA)
        {
            for (int i = 0; i < L2[0].blocks.size();i++)
            {
                CacheBlock block = L2[0].blocks[i];
                if (block.tag == fulltag)
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            for (int i = 0; i < L2[index].blocks.size();i++)
            {
                CacheBlock block = L2[index].blocks[i];
                if (block.tag == tag)
                {
                    return true;
                }
            }
            return false;
        }
    }
    bool write_access_l1(bitset<32> addr)
    {
        unsigned long tag = (addr >> (L1IndexSize + L1offset)).to_ulong();
        unsigned long fulltag = (addr >> L1offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);
        if (L1FA) // if fully associative
        {
            //check available blocks
            for (int i = 0; i < L1[0].blocks.size();i++)
            {
                if (L1[0].blocks[i].valid == false)//empty slot exist, update tag, valid
                { 
                    L1[0].blocks[i].tag = fulltag;
                    L1[0].blocks[i].dirty = false;
                    L1[0].blocks[i].valid = true;
                    L1[0].blocks[i].addr32 = addr;
                    return true;
                }
            }
            // set is full need to perform evict before write
            return false;
        }
        else //DM or SA only check at correspond set index
        {

            for (int i = 0; i < L1[index].blocks.size();i++)
            {
                if (L1[index].blocks[i].valid == false)//empty slot exist, set tag, set valid true
                { 
                    L1[index].blocks[i].tag = tag;
                    L1[index].blocks[i].dirty = false;
                    L1[index].blocks[i].valid = true;
                    L1[index].blocks[i].addr32 = addr;
                    return true;
                }
            }
            return false;
        }
    };

    bool write_access_l2(bitset<32> addr)
    {
        unsigned long tag = (addr >> (L2IndexSize + L2offset)).to_ulong();
        unsigned long fulltag = (addr >> L2offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
        if (L2FA) // if fully associative
        {
            //check available blocks
            for (int i = 0; i < L2[0].blocks.size();i++)
            {
                if (L2[0].blocks[i].valid == false)//empty slot exist, update tag, valid
                { 
                    L2[0].blocks[i].tag = fulltag;
                    L2[0].blocks[i].dirty = false;
                    L2[0].blocks[i].valid = true;
                    L2[0].blocks[i].addr32 = addr;
                    return true;
                }
            }
            // set is full need to perform evict before write
            return false;
        }
        else //DM or SA only check at correspond set index
        {

            for (int i = 0; i < L2[index].blocks.size();i++)
            {
                if (L2[index].blocks[i].valid == false)//empty slot exist, set tag, set valid true
                { 
                    L2[index].blocks[i].tag = tag;
                    L2[index].blocks[i].dirty = false;
                    L2[index].blocks[i].valid = true;
                    L2[index].blocks[i].addr32 = addr;
                    return true;
                }
            }
            return false;
        }
    }
    void L2toL1(bitset<32> addr)
    {
        unsigned long tag1 = (addr >> (L1IndexSize + L1offset)).to_ulong();
        unsigned long fulltag1 = (addr >> L1offset).to_ulong();
        unsigned long index1 = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);

        unsigned long tag2 = (addr >> (L2IndexSize + L2offset)).to_ulong();
        unsigned long fulltag2 = (addr >> L2offset).to_ulong();
        unsigned long index2 = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
        bool L2dirty = false;
        if (L2FA)
        {
            for (int i = 0; i < L2[0].blocks.size();i++)
            {
                CacheBlock &block = L2[0].blocks[i];
                if (block.tag == fulltag2)
                {
                    block.valid = false;
                    L2dirty = block.dirty;
                }
            }
        }
        else
        {
            for (int i = 0; i < L2[index2].blocks.size();i++)
            {
                CacheBlock &block = L2[index2].blocks[i];
                if (block.tag == tag2)
                {
                    block.valid = false;
                    L2dirty = block.dirty;
                }
            }
        }
        if (L1FA)
        {
            for (int i = 0; i < L1[0].blocks.size(); i++)
            {
                CacheBlock &block = L1[0].blocks[i];
                if (block.tag == fulltag1)
                {
                    block.dirty = L2dirty;
                }
            }
        }
        else
        {
            for (int i = 0; i < L1[index1].blocks.size();i++)
            {
                CacheBlock &block = L1[index1].blocks[i];
                if (block.tag == tag1)
                {
                    block.dirty = L2dirty;
                }
            }
        }
    }
    void L1toL2(bitset<32> addr){
        unsigned long tag1 = (addr >> (L1IndexSize + L1offset)).to_ulong();
        unsigned long fulltag1 = (addr >> L1offset).to_ulong();
        unsigned long index1 = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);

        unsigned long tag2 = (addr >> (L2IndexSize + L2offset)).to_ulong();
        unsigned long fulltag2 = (addr >> L2offset).to_ulong();
        unsigned long index2 = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
        bool L1dirty = false;
        if (L1FA)
        {   
            
            for (int i = 0; i < L1[0].blocks.size();i++)
            {
                if (L1[0].blocks[i].tag == fulltag1)
                {
                    L1[0].blocks[i].valid = false;
                    L1dirty = L1[0].blocks[i].dirty;
                }
            }
        }
        else
        {

            for (int i = 0; i < L1[index1].blocks.size();i++)
            {
             
                if (L1[index1].blocks[i].tag == tag1)
                {
                    L1[index1].blocks[i].valid = false;
                    L1dirty = L1[index1].blocks[i].dirty;
                }
            }
        }
        if (L2FA)
        {
            
            for (int i = 0; i < L2[0].blocks.size();i++)
            {
                CacheBlock &block = L2[0].blocks[i];
                if (block.tag == fulltag2)
                {   

                    block.dirty = L1dirty;

                }
            }
        }
        else
        {
            for (int i = 0; i < L2[index2].blocks.size();i++)
            {
              
                CacheBlock &block = L2[index2].blocks[i];
                if (block.tag == tag2)
                {   
                    block.dirty = L1dirty;
                }
            }
        }

    }
    bool writeL1(bitset<32> addr){
        unsigned long tag = (addr >> (L1IndexSize + L1offset)).to_ulong();
        unsigned long fulltag = (addr >> L1offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);
        if (L1FA) // if fully associative
        {
            //check if memory address exist
            for (int i = 0; i < L1[0].blocks.size();i++)
            {
                if (L1[0].blocks[i].tag == fulltag)
                { //tag exist updated dirty to true
                    L1[0].blocks[i].dirty = true;
                    return true;
                }
            }
            // set is full need to perform evict before write
            return false;
        }
        else //DM or SA only check at correspond set index
        {
            //check if tag exsit
            for (int i = 0; i < L1[index].blocks.size();i++)
            {
                if (L1[index].blocks[i].tag == tag)
                {
                    L1[index].blocks[i].dirty = true; //updated set dirty to true
                    return true;
                }
            }
            return false;
        }
    }

    bool writeL2(bitset<32> addr){
        unsigned long tag = (addr >> (L2IndexSize + L2offset)).to_ulong();
        unsigned long fulltag = (addr >> L2offset).to_ulong();
        unsigned long index = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
        if (L2FA) // if fully associative
        {
            //check if memory address exist
            for (int i = 0; i < L2[0].blocks.size();i++)
            {
                if (L2[0].blocks[i].tag == fulltag)
                { //tag exist updated dirty to true
                    L2[0].blocks[i].dirty = true;
                    return true;
                }
            }
            return false;
        }
        else //DM or SA only check at correspond set index
        {
            //check if tag exsit
            for (int i = 0; i < L2[index].blocks.size();i++)
            {
                CacheBlock &block = L2[index].blocks[i];
                if (L2[index].blocks[i].tag == tag)
                {
            
                    L2[index].blocks[i].dirty = true; //updated set dirty to true

                    return true;
                }
            }

            return false;
        }
    }
    

    CacheBlock evict_L1(bitset<32> addr)
    {
        if (L1FA)
        {
            return L1[0].evict();
        }
        else
        {
            unsigned long index = (addr.to_ulong() >> L1offset) & ((1ul << L1IndexSize) - 1);
            return L1[index].evict();
        }
    };
    CacheBlock evict_L2(bitset<32> addr)
    {
        if (L2FA)
        {
            return L2[0].evict();
        }
        else
        {
            unsigned long index = (addr.to_ulong() >> L2offset) & ((1ul << L2IndexSize) - 1);
            return L2[index].evict();
        }
    };
    void print()
    {
        
    }
};
/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

int main(int argc, char *argv[])
{
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof()) // read config file
    {
        cache_params >> dummyLine;               // L1:
        cache_params >> cacheconfig.L1blocksize; // L1 Block size
        cache_params >> cacheconfig.L1setsize;   // L1 Associativity
        cache_params >> cacheconfig.L1size;      // L1 Cache Size
        cache_params >> dummyLine;               // L2:
        cache_params >> cacheconfig.L2blocksize; // L2 Block size
        cache_params >> cacheconfig.L2setsize;   // L2 Associativity
        cache_params >> cacheconfig.L2size;      // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    Cache cache(cacheconfig);
    cache.print();
    if (cacheconfig.L1blocksize != cacheconfig.L2blocksize)
    {
        printf("please test with the same block size\n");
        return 1;
    }
    // cache c1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size,
    //          cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState = 0;  // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = 0;  // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NOWRITEMEM, WRITEMEM;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr))
            {
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);
            //cout<<"32bit address: "<<accessaddr<<endl;
            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0) // a Read request
            {
                if (cache.read_access_l1(addr))
                { //read l1 hit
                    L1AcceState = RH;
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                }
                else//l1 miss
                {
                    if (cache.read_access_l2(addr))//l2 hit
                    {
                        if (cache.write_access_l1(addr)) //l1 has slot to move addr from l2 to l1
                        {
                            cache.L2toL1(addr);
                            L1AcceState = RM;
                            L2AcceState = RH;
                            MemAcceState = NOWRITEMEM;
                        }
                        else //l1 full
                        {
                            CacheBlock evictedL1Block = cache.evict_L1(addr);//evict l1
                            if (cache.write_access_l2(evictedL1Block.addr32)) // l2 has slot to write what is evicted from l1
                            {
                                cache.L1toL2(evictedL1Block.addr32);
                                cache.write_access_l1(addr);
                                cache.L2toL1(addr);
                                L1AcceState = RM;
                                L2AcceState = RH;
                                MemAcceState = NOWRITEMEM;
                            }
                            else // l2 full
                            {

                                CacheBlock evictedL2Block = cache.evict_L2(evictedL1Block.addr32); //evict l2 to mem
                                cache.write_access_l2(evictedL1Block.addr32);
                                cache.L1toL2(evictedL1Block.addr32);//write l1 data to l2 
                                cache.write_access_l1(addr); //write new addr to l1
                                cache.L2toL1(addr); // set dirty and valid
                                L1AcceState = RM;
                                L2AcceState = RH;
                                MemAcceState = evictedL2Block.dirty ? WRITEMEM : NOWRITEMEM;
                            }
                        }
                    }
                    else//l2 miss
                    {
                        L1AcceState = RM;
                        L2AcceState = RM;
                        if (cache.write_access_l1(addr))//write data directly to l1
                        {
                            MemAcceState = NOWRITEMEM;
                        }
                        else//l1 is full evict l1
                        {
                            CacheBlock evictedL1Block = cache.evict_L1(addr);//evict l1
                            if (cache.write_access_l2(evictedL1Block.addr32))//if l2 has slot to store data evicted from l1
                            {
                                cache.L1toL2(evictedL1Block.addr32);
                                cache.write_access_l1(addr);
                                MemAcceState = NOWRITEMEM;
                            }
                            else //l2 is full evict l2
                            {
                                CacheBlock evictedL2Block = cache.evict_L2(evictedL1Block.addr32);//evict l2
                                cache.write_access_l2(evictedL1Block.addr32);//write l1->l2
                                cache.L1toL2(evictedL1Block.addr32); // maintain valid and dirty
                                cache.write_access_l1(addr);//write addr to l1
                                MemAcceState = evictedL2Block.dirty ? WRITEMEM : NOWRITEMEM;
                            }
                        }
                    }
                }
                // Implement by you:
                //   read access to the L1 Cache,
                //   and then L2 (if required),
                //   update the access state variable;
                //   return: L1AcceState L2AcceState MemAcceState

                // For example:
                // L1AcceState = cache.readL1(addr); // read L1
                // if(L1AcceState == RM){
                //     L2AcceState, MemAcceState = cache.readL2(addr); // if L1 read miss, read L2
                // }
                // else{ ... }
            }
            else
            { 
                if(cache.writeL1(addr)){
                    L1AcceState = WH;
                    L2AcceState = NA;
                    MemAcceState = NOWRITEMEM;
                }
                else if(cache.writeL2(addr)){
                    L1AcceState = WM;
                    L2AcceState = WH;
                    MemAcceState = NOWRITEMEM;
                }
                else{
                    L1AcceState = WM;
                    L2AcceState = WM;
                    MemAcceState = WRITEMEM;
                }
                // a Write request
                // Implement by you:
                //   write access to the L1 Cache, or L2 / main MEM,
                //   update the access state variable;
                //   return: L1AcceState L2AcceState

                // For example:
                // L1AcceState = cache.writeL1(addr);
                // if (L1AcceState == WM){
                //     L2AcceState, MemAcceState = cache.writeL2(addr);
                // }
                // else if(){...}
            }
            /*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState << endl; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
