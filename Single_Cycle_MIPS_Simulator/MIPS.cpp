#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>

using namespace std;

#define ADDU (1)
#define SUBU (3)
#define AND (4)
#define OR (5)
#define NOR (7)

// Memory size.
// In reality, the memory size should be 2^32, but for this lab and space reasons,
// we keep it as this large number, but the memory is still 32-bit addressable.
#define MemSize (65536)

class RF
{
public:
  bitset<32> ReadData1, ReadData2;
  RF()
  {
    Registers.resize(32);
    Registers[0] = bitset<32>(0);
  }

  void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
  {
    /**
       * @brief Reads or writes data from/to the Register.
       *
       * This function is used to read or write data from/to the register, depending on the value of WrtEnable.
       * Put the read results to the ReadData1 and ReadData2.
       */
    // TODO: implement!
    ReadData1 = Registers[RdReg1.to_ulong()]; // read from registers
    ReadData2 = Registers[RdReg2.to_ulong()];

    if (WrtEnable.to_ulong() == 1)
    { //if write is enabled then write
      Registers[WrtReg.to_ulong()] = WrtData;
    }
  }

  void OutputRF()
  {
    ofstream rfout;
    rfout.open("RFresult.txt", std::ios_base::app);
    if (rfout.is_open())
    {
      rfout << "A state of RF:" << endl;
      for (int j = 0; j < 32; j++)
      {
        rfout << Registers[j] << endl;
      }
    }
    else
      cout << "Unable to open file";
    rfout.close();
  }

private:
  vector<bitset<32> > Registers;
};

class ALU
{
public:
  bitset<32> ALUresult;
  bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
  {
    /**
       * @brief Implement the ALU operation here.
       *
       * ALU operation depends on the ALUOP, which are definded as ADDU, SUBU, etc. 
       */
    // TODO: implement!
    switch (ALUOP.to_ulong())
    {
    case ADDU:
      return bitset<32>(oprand1.to_ulong() + oprand2.to_ulong());
    case SUBU:
      return bitset<32>(oprand1.to_ulong() - oprand2.to_ulong());
    case AND:
      return bitset<32>(oprand1.to_ulong() & oprand2.to_ulong());
    case OR:
      return bitset<32>(oprand1.to_ulong() | oprand2.to_ulong());
    case NOR:
      return bitset<32>(~(oprand1.to_ulong() | oprand2.to_ulong()));
    }
  }
};

class INSMem
{
public:
  bitset<32> Instruction;
  INSMem()
  {
    IMem.resize(MemSize);
    ifstream imem;
    string line;
    int i = 0;
    imem.open("imem.txt");
    if (imem.is_open())
    {
      while (getline(imem, line))
      {
        IMem[i] = bitset<8>(line.substr(0, 8));
        i++;
      }
    }
    else
      cout << "Unable to open file";
    imem.close();
  }

  bitset<32> ReadMemory(bitset<32> ReadAddress)
  {
    // TODO: implement!
    /**
       * @brief Read Instruction Memory (IMem).
       *
       * Read the byte at the ReadAddress and the following three byte,
       * and return the read result. 
       */
    string result;
    unsigned long index = ReadAddress.to_ulong();
    for (int i = 0; i < 4; i++)
    {
      result += IMem[index].to_string();
      index++;
    }
    Instruction = bitset<32>(result);
    return Instruction;
  }

private:
  vector<bitset<8> > IMem;
};

class DataMem
{
public:
  bitset<32> readdata;
  DataMem()
  {
    DMem.resize(MemSize);
    ifstream dmem;
    string line;
    int i = 0;
    dmem.open("dmem.txt");
    if (dmem.is_open())
    {
      while (getline(dmem, line))
      {
        DMem[i] = bitset<8>(line.substr(0, 8));
        i++;
      }
    }
    else
      cout << "Unable to open file";
    dmem.close();
  }
  bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
  {
    /**
       * @brief Reads/writes data from/to the Data Memory.
       *
       * This function is used to read/write data from/to the DataMem, depending on the readmem and writemem.
       * First, if writemem enabled, WriteData should be written to DMem, clear or ignore the return value readdata,
       * and note that 32-bit WriteData will occupy 4 continious Bytes in DMem. 
       * If readmem enabled, return the DMem read result as readdata.
       */
    // TODO: implement!
    if (readmem[0])
    {
      string s2;
      s2 += DMem[Address.to_ulong()].to_string();
      s2 += DMem[Address.to_ulong() + 1].to_string();
      s2 += DMem[Address.to_ulong() + 2].to_string();
      s2 += DMem[Address.to_ulong() + 3].to_string();
      readdata = bitset<32>(s2);
      return readdata;
    }
    if (writemem[0])
    {
      string s3 = WriteData.to_string();
      DMem[Address.to_ulong()] = bitset<8>(s3.substr(0, 8));
      DMem[Address.to_ulong() + 1] = bitset<8>(s3.substr(8, 8));
      DMem[Address.to_ulong() + 2] = bitset<8>(s3.substr(16, 8));
      DMem[Address.to_ulong() + 3] = bitset<8>(s3.substr(24, 8));
    }
    return readdata;
  }

  void OutputDataMem()
  {
    ofstream dmemout;
    dmemout.open("dmemresult.txt");
    if (dmemout.is_open())
    {
      for (int j = 0; j < 1000; j++)
      {
        dmemout << DMem[j] << endl;
      }
    }
    else
      cout << "Unable to open file";
    dmemout.close();
  }

private:
  vector<bitset<8> > DMem;
};

int main()
{
  RF myRF;
  ALU myALU;
  INSMem myInsMem;
  DataMem myDataMem;

  // Control Registers
  bitset<32> PC;
  bitset<1> wrtEnable;
  bitset<1> isJType;
  bitset<1> isIType;
  bitset<1> isLoad;
  bitset<1> isStore;
  bitset<1> isBranch;
  bitset<3> aluOp;

  while (1)
  {
    // 1. Fetch Instruction
    bitset<32> instruction = myInsMem.ReadMemory(PC);
    cout<<PC<<endl;
    string insString = instruction.to_string();
    // If current insturciton is "11111111111111111111111111111111", then break;
    if (myInsMem.Instruction.to_ulong() == 0xffffffff)
    {
      break;
    }

    // decode(Read RF)
    // Decoder
    isLoad = insString.substr(0, 6) == string("100011");
    isStore = insString.substr(0, 6) == string("101011");
    isJType = insString.substr(0, 6) == string("000010");
    isBranch = insString.substr(0, 6) == string("000100");
    isIType = insString.substr(0, 5) != string("00000") && insString.substr(0, 4) != string("0001");
    //write enable
    wrtEnable = !(isStore.to_ulong() || isBranch.to_ulong() || isJType.to_ulong());

    //alu op
    if (insString.substr(0, 6) == string("100011") ||
        insString.substr(0, 6) == string("101011"))
    {
      aluOp = bitset<3>("001");
    }
    else if (insString.substr(0, 6) == string("000000"))
    {
      aluOp = bitset<3>(insString.substr(29, 3));
    }
    else
    {
      aluOp = bitset<3>(insString.substr(3, 3));
    }

    // 2. Register File Instruction
    myRF.ReadWrite(bitset<5>(insString.substr(6, 5)), bitset<5>(insString.substr(11, 5)),
                   isIType[0] ? bitset<5>(insString.substr(11, 5)) : bitset<5>(insString.substr(16, 5)),
                   bitset<32>(0), wrtEnable);

    // 3. Execuete alu operation
    bitset<32> tmp(insString.substr(16, 16)); // if positive, 0 padded
    if (tmp[15] == true)
    {
      tmp = bitset<32>(string(16, '1') + tmp.to_string().substr(16, 16));
    }
    myALU.ALUresult = myALU.ALUOperation(aluOp, myRF.ReadData1, isIType[0] ? tmp : myRF.ReadData2);

    // 4. Read/Write Mem(Memory Access)
    myDataMem.MemoryAccess(myALU.ALUresult, myRF.ReadData2, isLoad, isStore);

    // 5. Register File Update(Write Back)
    myRF.ReadWrite(bitset<5>(insString.substr(6, 5)), bitset<5>(insString.substr(11, 5)),
                   isIType[0] ? bitset<5>(insString.substr(11, 5)) : bitset<5>(insString.substr(16, 5)),
                   isLoad[0] ? myDataMem.readdata : myALU.ALUresult, wrtEnable);

    // Update PC
    if (isBranch[0] && myRF.ReadData1 == myRF.ReadData2)
    {
      bitset<32> addressExtend;
      if (instruction[15] == true) //msb is 1, sign extend it to 32bits
      {
        addressExtend = bitset<32>(string(14, '1') + insString.substr(16, 16) + string("00"));
      }
      else
      {
        addressExtend = bitset<32>(string(14, '0') + insString.substr(16, 16) + string("00"));
      }
      PC = bitset<32>(PC.to_ulong() + 4 + addressExtend.to_ulong());
    }
    else if (isJType[0])
    {
      PC = bitset<32>(PC.to_string().substr(0, 4) + insString.substr(6, 26) + string("00"));
    }
    else
    {
      PC = bitset<32>(PC.to_ulong() + 4);
    }

    myRF.OutputRF(); // dump RF;
  }
  myDataMem.OutputDataMem(); // dump data mem

  return 0;
}
