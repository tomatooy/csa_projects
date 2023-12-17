#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct
{
    bitset<32> PC;
    bool nop;
};

struct IDStruct
{
    bitset<32> Instr;
    bool nop;
};

struct EXStruct
{
    bitset<32> Read_data1;
    bitset<32> Read_data2;
    bitset<16> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type;
    bool rd_mem;
    bool wrt_mem;
    bool alu_op; //1 for addu, lw, sw, 0 for subu
    bool wrt_enable;
    bool nop;
};

struct MEMStruct
{
    bitset<32> ALUresult;
    bitset<32> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;
    bool nop;
};

struct WBStruct
{
    bitset<32> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct
{
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
    stateStruct()
    {
        IF.nop = 0;
        ID.nop = EX.nop = MEM.nop = WB.nop = 1;
        EX.alu_op = 1;
        EX.is_I_type = 0;
        EX.rd_mem = EX.wrt_mem = EX.wrt_enable = 0;
        MEM.rd_mem = MEM.wrt_enable = MEM.wrt_mem = 0;
        WB.wrt_enable = 0;
    }
};

class RF
{
public:
    bitset<32> Reg_data;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32>(0);
    }

    bitset<32> readRF(bitset<5> Reg_addr)
    {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
    {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open())
        {
            rfout << "State of RF:\t" << endl;
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
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress)
    {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem); //read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8> > IMem;
};

class DataMem
{
public:
    bitset<32> ReadData;
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
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open file";
        dmem.close();
    }

    bitset<32> readDataMem(bitset<32> Address)
    {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        ReadData = bitset<32>(datamem); //read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<32> WriteData)
    {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
    }

    void outputDataMem()
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

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;
    }
    else
        cout << "Unable to open file";
    printstate.close();
}
unsigned long shiftbits(bitset<32> inst, int bits)
{
    return (inst.to_ulong()) >> bits;
}

bitset<32> signextend(bitset<16> inst)
{
    bitset<32> addressExtend;
    if (inst[15] == 1)
    {
        addressExtend = bitset<32>(string(16, '1') + inst.to_string());
    }
    else
    {
        addressExtend = bitset<32>(string(16, '0') + inst.to_string());
    }
    return addressExtend;
}

int main()
{

    RF myRF = RF();
    INSMem myInsMem = INSMem();
    DataMem myDataMem = DataMem();
    stateStruct state = stateStruct();
    stateStruct newState = stateStruct();
    // instruction
    bitset<32> instruction = bitset<32>(0);
    bitset<6> opcode = bitset<6>(0);
    bitset<6> funct = bitset<6>(0);

    //control signals
    bool IType = 0;
    bool RType = 0;
    bool JType = 0;
    bool IsBne = 0;
    bool IsLoad = 0;
    bool IsStore = 0;

    //reg
    bitset<5> RReg1 = bitset<5>(0);
    bitset<5> RReg2 = bitset<5>(0);
    // ALU signals
    bitset<32> signext = bitset<32>(0);

    // branch address
    bitset<32> braddr = bitset<32>(0);

    //cycle signal
    int cycle = 0;

    while (1)
    {

        /* --------------------- WB stage --------------------- */

        if (state.WB.nop == 0)
        {
            if (state.WB.wrt_enable == 1)
            {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
            }
        }

        /* --------------------- MEM stage --------------------- */
        newState.WB.nop = state.MEM.nop;
        if (state.MEM.nop == 0)
        {
            if (state.WB.Wrt_reg_addr == state.MEM.Rt && state.WB.nop == 0) //mem to mem
            {
                state.MEM.Store_data = state.WB.Wrt_data;
            }

            if (state.MEM.wrt_mem == 1) //sw
            {
                myDataMem.writeDataMem(state.MEM.ALUresult, state.MEM.Store_data);
            }
            else if (state.MEM.rd_mem == 1) //lw
            {
                newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
            }
            else //r type
            {
                newState.WB.Wrt_data = state.MEM.ALUresult;
            }
            newState.WB.Rs = state.MEM.Rs;
            newState.WB.Rt = state.MEM.Rt;
            newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
            newState.WB.wrt_enable = state.MEM.wrt_enable;
        }

        /* --------------------- EX stage --------------------- */
        newState.MEM.nop = state.EX.nop;
        if (state.EX.nop == 0)
        {
            if (state.WB.nop == 0 && state.WB.wrt_enable == 1) //mem to ex
            {
                if (state.WB.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = state.WB.Wrt_data;
                }
                if (state.WB.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = state.WB.Wrt_data;
                }
            }
            if (state.MEM.nop == 0 && state.MEM.wrt_enable == 1) //ex to ex
            {
                if (state.MEM.Wrt_reg_addr == state.EX.Rs)
                {
                    state.EX.Read_data1 = state.MEM.ALUresult;
                }
                if (state.MEM.Wrt_reg_addr == state.EX.Rt)
                {
                    state.EX.Read_data2 = state.MEM.ALUresult;
                }
            }

            if (state.EX.is_I_type) //i type
            {
                newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + signextend(state.EX.Imm).to_ulong());
            }
            else if(state.EX.wrt_enable == 1)
            {
                if (state.EX.alu_op == 1)
                {
                    newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());
                }
                else
                {
                    newState.MEM.ALUresult = bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());
                }
            }
            else
            {
                newState.MEM.ALUresult = bitset<32>("00000000000000000000000000000000");
            }
            newState.MEM.Store_data = state.EX.Read_data2;
            newState.MEM.Rt = state.EX.Rt;
            newState.MEM.Rs = state.EX.Rs;
            newState.MEM.rd_mem = state.EX.rd_mem;
            newState.MEM.wrt_mem = state.EX.wrt_mem;
            newState.MEM.wrt_enable = state.EX.wrt_enable;
            newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
        }

        /* --------------------- ID stage --------------------- */
        newState.EX.nop = state.ID.nop;
        if (state.ID.nop == 0)
        {
            opcode = bitset<6>(shiftbits(state.ID.Instr, 26));
            IType = (opcode.to_ulong() != 0 && opcode.to_string().substr(0, 4) != string("0001"));
            JType = opcode.to_ulong() == 2;
            newState.EX.is_I_type = IType;

            IsBne = (opcode.to_ulong() == 5);
            IsLoad = (opcode.to_ulong() == 35);
            IsStore = (opcode.to_ulong() == 43);

            newState.EX.rd_mem = IsLoad;
            newState.EX.wrt_mem = IsStore;
            newState.EX.wrt_enable = (IsStore|| IsBne || JType) ? 0 : 1;
            RReg1 = bitset<5>(shiftbits(state.ID.Instr, 21));
            RReg2 = bitset<5>(shiftbits(state.ID.Instr, 16));
            newState.EX.Rs = RReg1;
            newState.EX.Rt = RReg2;
            funct = bitset<6>(shiftbits(state.ID.Instr, 0));
            if (opcode.to_ulong() == 35 || opcode.to_ulong() == 43 || funct.to_ulong() == 33 || opcode.to_ulong() == 4) //lw sw addu j
            {
                newState.EX.alu_op = 1;
            }
            else
            {
                newState.EX.alu_op = 0;
            }
            bitset<16> imm = bitset<16>(shiftbits(state.ID.Instr, 0));
            newState.EX.Imm = imm;
            if (IType)
            {
                newState.EX.Wrt_reg_addr = RReg2;
            }
            else
            {
                newState.EX.Wrt_reg_addr = bitset<5>(shiftbits(state.ID.Instr, 11));
            }
            newState.EX.Read_data1 = myRF.readRF(RReg1);
            newState.EX.Read_data2 = myRF.readRF(RReg2);
            newState.EX.nop = state.ID.nop;


            // Load-Add stall
            if (state.EX.is_I_type && state.EX.rd_mem)
            {
                if ((state.EX.Rt == RReg1 || state.EX.Rt == RReg2) && state.EX.nop == 0 && opcode == bitset<6>("000000")) //raw 
                {
                    newState.EX.nop = 1;
                }
                
            }
            // Load-Store stall
            if (state.EX.is_I_type && state.EX.rd_mem && IsStore)
            {
                if (state.EX.Rt == RReg1 || state.EX.Rt == RReg2) //raw
                {
                    newState.EX.nop = 1;
                }
            }

            if (IsBne)
            {   
                string insString = state.ID.Instr.to_string();
                if (myRF.readRF(RReg1) != myRF.readRF(RReg2))
                {
                    bitset<32> addressExtend;
                    if (instruction[15] == 1) //msb is 1, sign extend it to 32bits
                    {
                        addressExtend = bitset<32>(string(14, '1') + insString.substr(16, 16) + string("00"));
                    }
                    else
                    {
                        addressExtend = bitset<32>(string(14, '0') + insString.substr(16, 16) + string("00"));
                    }
                    state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + addressExtend.to_ulong());
                }
            }
        }

        /* --------------------- IF stage --------------------- */
        newState.ID.nop = state.IF.nop;
        if (state.IF.nop == 0)
        {
            instruction = myInsMem.readInstr(state.IF.PC);
            if (instruction == 0xffffffff)
            {
                newState.IF.nop = 1;
                newState.ID.nop = 1;
                newState.IF.PC = state.IF.PC;
            }
            else
            {
                newState.IF.PC = state.IF.PC.to_ulong() + 4;
                newState.IF.nop = 0;
            }
            newState.ID.Instr = instruction;
        }

        if (state.EX.is_I_type == 1 && state.EX.rd_mem == 1) //freeze if id if lw add
        {
            if ((state.EX.Rt == RReg1|| state.EX.Rt == RReg2) && state.EX.nop == 0 && opcode == bitset<6>("000000") && state.EX.rd_mem && state.EX.is_I_type)
            {
                newState.IF = state.IF;
                newState.ID = state.ID;
            }
        }
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        
        cout << "alu op" << newState.EX.alu_op << endl;
        state = newState; /*** The end of the cycle and updates the current state with the values calculated in this cycle. csa23 ***/
        cycle++;
    }

    myRF.outputRF();           // dump RF;
    myDataMem.outputDataMem(); // dump data mem

    return 0;
}