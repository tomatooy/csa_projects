#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>

using namespace std;

string inputtracename = "trace.txt";
// remove the ".txt" and add ".out.txt" to the end as output name
string outputtracename = inputtracename.substr(0, inputtracename.length() - 4) + ".out.txt";
string hardwareconfigname = "config.txt";

enum Operation
{
	ADD,
	SUB,
	MULT,
	DIV,
	LOAD,
	STORE
};
// The execute cycle of each operation: ADD, SUB, MULT, DIV, LOAD, STORE
const int OperationCycle[6] = {2, 2, 10, 40, 2, 2};

struct HardwareConfig
{
	int LoadRSsize;	 // number of load reservation stations
	int StoreRSsize; // number of store reservation stations
	int AddRSsize;	 // number of add reservation stations
	int MultRSsize;	 // number of multiply reservation stations
	int FRegSize;	 // number of fp registers
};

// We use the following structure to record the time of each instruction
struct InstructionStatus
{
	int cycleIssued;
	int cycleExecuted; // execution completed
	int cycleWriteResult;
};

// Register Result Status structure
struct RegisterResultStatus
{
	string ReservationStationName;
	bool dataReady = false;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
struct Instruction
{
	string operation;
	string destination;
	string source1;
	string source2;
};

class RegisterResultStatuses
{
public:
	RegisterResultStatuses(int size)
	{
		_registers.resize(size);
	}

	void update(string readyResult)
	{
		for (RegisterResultStatus &_register : _registers)
		{
			if (!_register.dataReady && _register.ReservationStationName == readyResult)
			{
				_register.dataReady = true;
			}
		}
	}

	void set(int index, string operation)
	{
		_registers[index].dataReady = false;
		_registers[index].ReservationStationName = operation;
	}

	string checkDependency(int registerNumber)
	{
		string res = _registers[registerNumber].dataReady ? "" : _registers[registerNumber].ReservationStationName;
		return res;
	}

	// ...

	/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/
	/*
	Print all register result status. It is called by PrintRegisterResultStatus4Grade() for grading.
	If you don't want to write such a class, then make sure you call the same function and print the register
	result status in the same format.
	*/
	string _printRegisterResultStatus() const
	{
		std::ostringstream result;
		for (int idx = 0; idx < _registers.size(); idx++)
		{
			result << "F" + std::to_string(idx) << ": ";
			result << _registers[idx].ReservationStationName << ", ";
			result << "dataRdy: " << (_registers[idx].dataReady ? "Y" : "N") << ", ";
			result << "\n";
		}
		return result.str();
	}
	/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
private:
	vector<RegisterResultStatus> _registers;
};

// Define your Reservation Station structure
struct ReservationStation
{
	string ReservationStationID;
	string op;
	int insIndex;
	bool vj;
	bool vk;
	string qj;
	string qk;
	InstructionStatus insStat;
	bool busy;
	bool readyToCompute;
	int remainCycles;
	

	// ...
};
class ReservationStations
{
public:
	ReservationStations(HardwareConfig config)
	{
		int totalRs = config.AddRSsize + config.MultRSsize + config.LoadRSsize + config.StoreRSsize;
		// Initialize reservation stations
		_stations.resize(totalRs);
		// You can further initialize individual stations if needed
		for (int i = 0; i < config.AddRSsize; ++i)
		{
			_stations[i].ReservationStationID = "Add" + std::to_string(i);
			_stations[i].op = "ADD";
		}
		for (int i = config.AddRSsize; i < config.AddRSsize + config.MultRSsize; ++i)
		{
			_stations[i].ReservationStationID = "Mult" + std::to_string(i - config.AddRSsize);
			_stations[i].op = "MULT";
		}
		for (int i = config.AddRSsize + config.MultRSsize; i < config.AddRSsize + config.MultRSsize + config.LoadRSsize; ++i)
		{
			_stations[i].ReservationStationID = "Load" + std::to_string(i - config.AddRSsize - config.MultRSsize);
			_stations[i].op = "LOAD";
			// ... initialize other fields for Load reservation station
		}

		for (int i = config.AddRSsize + config.MultRSsize + config.LoadRSsize; i < totalRs; ++i)
		{
			_stations[i].ReservationStationID = "Store" + std::to_string(i - config.AddRSsize - config.MultRSsize - config.LoadRSsize);
			_stations[i].op = "STORE";
			// ... initialize other fields for Store reservation station
		}
	}
	int checkAvailble(string type)
	{
		for (int i = 0; i < _stations.size(); i++)
		{
			if (_stations[i].busy == false && type == _stations[i].op)
			{
				return i;
			}
		}
		return -1;
	}
	void issue(string type, int insIndex, int cyclesNeeded, string qj, string qk,string fu,int cycle, int i)
	{
		_stations[i].busy = true;
		_stations[i].insIndex = insIndex;
		_stations[i].remainCycles = cyclesNeeded;
		_stations[i].insStat.cycleIssued = cycle;
		if (type == "LOAD" )
		{
			_stations[i].readyToCompute = true;
		}
		else if(type == "STORE"){
			_stations[i].qj = fu;
			_stations[i].vj = _stations[i].qj == "";
			_stations[i].vk = true;
			_stations[i].readyToCompute = _stations[i].vj;
		}
		else
		{
			_stations[i].qj = qj;
			_stations[i].qk = qk;
			_stations[i].vj = qj == "";
			_stations[i].vk = qk == "";
			_stations[i].readyToCompute = _stations[i].vk && _stations[i].vj;
		}
	}
	void execute(int cycle)
	{
		for (ReservationStation &station : _stations)
		{
			if (station.readyToCompute && station.remainCycles > 0)
			{
				station.remainCycles--;
				if (station.remainCycles == 0)
				{
					station.insStat.cycleExecuted = cycle;
				}
			}
			else if(!station.readyToCompute && station.vj && station.vk){
				station.readyToCompute = true;
			}
		}
	}
	string boardcastCandidate()
	{
		int min = numeric_limits<int>::max();
		string candidateId;
		for (ReservationStation &station : _stations)
		{
			if (station.remainCycles == 0 && station.busy == true && station.insStat.cycleIssued < min)
			{
				min = station.insStat.cycleIssued;
				candidateId = station.ReservationStationID;
			}
		}
		return candidateId;
	}

	ReservationStation update(string readyResult, int cycle)
	{
		ReservationStation target;
		for (ReservationStation &station : _stations)
		{
			if (station.busy)
			{
				if (station.qj == readyResult)
				{
					station.vj = true;
				}
				if (station.qk == readyResult)
				{
					station.vk = true;
				}
			}

			if (station.ReservationStationID == readyResult)
			{
				station.insStat.cycleWriteResult = cycle;
				station.busy = false;
				station.readyToCompute = false;
				target = station;
			}
		}
		return target;
	}

	string getStationName(int index)
	{
		return _stations[index].ReservationStationID;
	}

	void print(int cycle)
	{
		std::ofstream outfile("debug.txt", std::ios_base::app); // append result to the end of file
		outfile << "Cycle:"<<cycle<<endl;
		for (ReservationStation s : _stations)
		{
			outfile << "--------------------" << endl;
			outfile << s.ReservationStationID << endl;
			outfile << s.remainCycles << endl;
			outfile << s.busy << endl;
			outfile << s.vj << "|" << s.vk << endl;
			outfile << s.qj << "|" << s.qk << endl;
		}
			outfile.close();
	}

	bool allidle()
	{
		for (ReservationStation s : _stations)
		{
			if (s.busy == true)
				return false;
		}
		return true;
	}

	// ...
private:
	vector<ReservationStation> _stations;
};

class CommonDataBus
{
public:
	bool valid;
	string registerReady;
	CommonDataBus()
	{
		valid = false;
		registerReady = "";
	}
};

// Function to simulate the Tomasulo algorithm
/*
print the register result status each 5 cycles
@param filename: output file name
@param registerResultStatus: register result status
@param thiscycle: current cycle
*/
void PrintRegisterResultStatus4Grade(const string &filename,
									 const RegisterResultStatuses &registerResultStatus,
									 const int thiscycle)
{
	if (thiscycle % 5 != 0)
		return;
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Cycle " << thiscycle << ":\n";
	outfile << registerResultStatus._printRegisterResultStatus() << "\n";
	outfile.close();
}

void simulateTomasulo(HardwareConfig hardwareConfig, vector<Instruction> instructions, vector<InstructionStatus> &instructionsStat)
{

	int thiscycle = 1; // start cycle: 1
	RegisterResultStatuses registerResultStatuses = RegisterResultStatuses(hardwareConfig.FRegSize);
	ReservationStations stations = ReservationStations(hardwareConfig);
	CommonDataBus CDB = CommonDataBus();
	int instructionIndex = 0;
	while (thiscycle < 100000000)
	{

		if (CDB.valid == true)
		{
			registerResultStatuses.update(CDB.registerReady);
			ReservationStation stationInfo = stations.update(CDB.registerReady, thiscycle);
			instructionsStat[stationInfo.insIndex] = stationInfo.insStat;
		}
		// Reservation Stations should be updated every cycle, and broadcast to Common Data Bus
		// ...
		stations.execute(thiscycle);


		string nextToBoardcast = stations.boardcastCandidate();
		//cout << nextToBoardcast << endl;
		if (nextToBoardcast != "")
		{
			CDB.registerReady = nextToBoardcast;
			CDB.valid = true;
		}
		else
			CDB.valid = false;

		if (instructionIndex < instructions.size())
		{
			Instruction newInstruction = instructions[instructionIndex];
			int destinationRegister = stoi(newInstruction.destination.substr(1));
			string op = newInstruction.operation;
			// Issue new instruction in each cycle
			// ...

			if (op == "STORE")
			{
				int stationIndex = stations.checkAvailble("STORE");
				if (stationIndex != -1)
				{
					string source = newInstruction.destination.substr(1);
					string fu = registerResultStatuses.checkDependency(stoi(source));
					stations.issue(op, instructionIndex, 2, "", "", fu,thiscycle, stationIndex);
					instructionIndex++;
				}
			}
			else if (op == "LOAD")
			{

				int stationIndex = stations.checkAvailble("LOAD");
				if (stationIndex != -1)
				{
	
					string reservationName = stations.getStationName(stationIndex);
					stations.issue(op, instructionIndex, 2, "", "", "",thiscycle, stationIndex);
					registerResultStatuses.set(destinationRegister, reservationName);
					instructionIndex++;
				}
			}
			else
			{
				int cycleNeed;
				int stationIndex;
				if (op == "ADD" || op == "SUB")
				{
					cycleNeed = OperationCycle[ADD];
					stationIndex = stations.checkAvailble("ADD");
				}
				else if (op == "MULT")
				{
					cycleNeed = OperationCycle[MULT];
					stationIndex = stations.checkAvailble("MULT");
				}
				else if (op == "DIV")
				{
					cycleNeed = OperationCycle[DIV];
					stationIndex = stations.checkAvailble("MULT");
				}
				if (stationIndex != -1)
				{
					string reservationName = stations.getStationName(stationIndex);
					int source1 = stoi(newInstruction.source1.substr(1));
					int source2 = stoi(newInstruction.source2.substr(1));
					string qj = registerResultStatuses.checkDependency(source1);
					string qk = registerResultStatuses.checkDependency(source2);
					stations.issue(op, instructionIndex, cycleNeed, qj, qk,"",thiscycle, stationIndex);
					registerResultStatuses.set(destinationRegister, reservationName);
					instructionIndex++;
				}
			}
		}
		//stations.print(thiscycle);

		// At the end of this cycle, we need this function to print all registers status for grading
		PrintRegisterResultStatus4Grade(outputtracename, registerResultStatuses, thiscycle);

		// The simulator should stop when all instructions are finished.
		// ...
		if (instructionIndex >= instructions.size() && stations.allidle())
			break;
		thiscycle++;
	}
};

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

/*
print the instruction status, the reservation stations and the register result status
@param filename: output file name
@param instructionStatus: instruction status
*/
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus)
{
	std::ofstream outfile(filename, std::ios_base::app); // append result to the end of file
	outfile << "Instruction Status:\n";
	for (int idx = 0; idx < instructionStatus.size(); idx++)
	{
		outfile << "Instr" << idx << ": ";
		outfile << "Issued: " << instructionStatus[idx].cycleIssued << ", ";
		outfile << "Completed: " << instructionStatus[idx].cycleExecuted << ", ";
		outfile << "Write Result: " << instructionStatus[idx].cycleWriteResult << ", ";
		outfile << "\n";
	}
	outfile.close();
}

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		hardwareconfigname = argv[1];
		inputtracename = argv[2];
	}

	HardwareConfig hardwareConfig;
	std::ifstream config;
	config.open(hardwareconfigname);
	config >> hardwareConfig.LoadRSsize;  // number of load reservation stations
	config >> hardwareConfig.StoreRSsize; // number of store reservation stations
	config >> hardwareConfig.AddRSsize;	  // number of add reservation stations
	config >> hardwareConfig.MultRSsize;  // number of multiply reservation stations
	config >> hardwareConfig.FRegSize;	  // number of fp registers
	config.close();

	/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
	std::ifstream inputtrace(inputtracename);
	string instructionLine;
	vector<Instruction> instructions;
	while (getline(inputtrace, instructionLine))
	{
		// Create an input string stream from the instruction line
		istringstream iss(instructionLine);
		Instruction instruction;

		// Extract variables from the line
		iss >> instruction.operation >> instruction.destination >> instruction.source1 >> instruction.source2;
		instructions.push_back(instruction);

		// Process the variables (you can print them or perform other operations)
	}
	// for (auto instruction : instructions)
	// {
	// 	cout << "Operation: " << instruction.operation << endl;
	// 	cout << "Destination: " << instruction.destination << endl;
	// 	cout << "Source1: " << instruction.source1 << endl;
	// 	cout << "Source2: " << instruction.source2 << endl;
	// 	cout << "----------------------" << endl;
	// }
	// RegisterResultStatuses registerResultStatus();
	// ...

	// Initialize the instruction status table
	vector<InstructionStatus> instructionStatus(instructions.size());
	// ...

	// Simulate Tomasulo:
	simulateTomasulo(hardwareConfig, instructions, instructionStatus);

	/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	PrintResult4Grade(outputtracename, instructionStatus);

	return 0;
}
