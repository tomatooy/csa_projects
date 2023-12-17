#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

#define MemSize (65536)

class PhyMem    
{
  public:
    bitset<32> readdata;  
    PhyMem()
    {
      DMem.resize(MemSize); 
      ifstream dmem;
      string line;
      int i=0;
      dmem.open("pt_initialize.txt");
      if (dmem.is_open())
      {
        while (getline(dmem,line))
        {      
          DMem[i] = bitset<8>(line);
          i++;
        }
      }
      else cout<<"Unable to open page table init file";
      dmem.close();

    }  
    bitset<32> outputMemValue (bitset<12> Address) 
    {    
    unsigned long index = Address.to_ulong();
    // cout << "index" << index <<"\n\n";
    bitset<32> readdata;
    if (index < MemSize - 4) {
        // Combine four 8-bit values to form a 32-bit word
        for (int i = 0; i < 4; i++) {
            bitset<8> byte = DMem[index + i];
            cout<< byte << "\n";
            for (int j = 7; j >= 0; j--) {
                readdata[(3 - i) * 8 + j] = byte[j];
            }
        }
    }
      /**TODO: implement!
       * Returns the value stored in the physical address 
       */


      return readdata;     
    }              

  private:
    vector<bitset<8> > DMem;

};  

int main(int argc, char *argv[])
{
    PhyMem myPhyMem;

    ifstream traces;
    ifstream PTB_file;
    ofstream tracesout;

    string outname;
    outname = "pt_results.txt";

    traces.open(argv[1]);
    PTB_file.open(argv[2]);
    tracesout.open(outname.c_str());

    //Initialize the PTBR
    bitset<12> PTBR;
    PTB_file >> PTBR;

    string line;
    bitset<14> virtualAddr;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

    // Read a virtual address form the PageTable and convert it to the physical address - CSA23
    if(traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        {
          virtualAddr = bitset<14>(line);

          // bitset<32> outertemp = myPhyMem.outputMemValue(72);
          // cout << "temp" << outertemp << "\n";
          bitset<4> outerIndex(virtualAddr.to_string().substr(0, 4));
          bitset<4> innerIndex(virtualAddr.to_string().substr(4, 4));
          bitset<6> offset(virtualAddr.to_string().substr(8, 6));
          bitset<12> outerAddr = PTBR.to_ulong() + (outerIndex.to_ulong() << 2);
          bitset<32> outerEntry = myPhyMem.outputMemValue(outerAddr);
          bool outerValid = outerEntry[0];
          // cout << "outeraddr" << outerAddr;
          // cout << "outervalid" << outerValid;
          // cout << "outerentry" << outerEntry <<"\n";

          // If outer page table entry is valid, access the inner page table
         bitset<12> innerBase(outerEntry.to_string().substr(0, 12));
         bitset<12> innerAddr = innerBase.to_ulong() + (innerIndex.to_ulong() << 2);
        //  innerAddr = innerAddr << 2;
         bitset<32> innerEntry = myPhyMem.outputMemValue(innerAddr);
         bool innerValid = innerEntry[0];
        //  cout << "inner" << innerValid;

          // If inner page table entry is valid, calculate the physical address
         bitset<12> frame(innerEntry.to_string().substr(0, 6));
         bitset<12> physicalAddr = (frame.to_ulong() << 6) + offset.to_ulong();
        //  cout << "physical" << physicalAddr;
         bitset<32> value = myPhyMem.outputMemValue(physicalAddr);
         tracesout << outerValid  <<"," << " " << innerValid << ", ";
         if (outerValid && innerValid) {
           tracesout << "0x" << hex << setw(3) << setfill('0') << physicalAddr.to_ulong() << ",";
           tracesout << " 0x" << hex << setw(8) << setfill('0') << value.to_ulong();
         } else {
           tracesout << "0x000, 0x00000000";
        }
         tracesout << endl;
            //TODO: Implement!
            // Access the outer page table 

            // If outer page table valid bit is 1, access the inner page table 

            //Return valid bit in outer and inner page table, physical address, and value stored in the physical memory.
            // Each line in the output file for example should be: 1, 0, 0x000, 0x00000000

        }
        traces.close();
        tracesout.close();
    }

    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
