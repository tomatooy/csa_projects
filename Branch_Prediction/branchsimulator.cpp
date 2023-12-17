#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <bitset>

using namespace std;

int main(int argc, char **argv)
{
	ifstream config;
	config.open(argv[1]);

	int m, w, h;
	config >> m;
	config >> h;
	config >> w;

	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());

	ifstream trace;
	trace.open(argv[2]);
	vector<bitset<2> > PHT(1 << m, bitset<2>("10"));
    vector<vector<bool> > BHT(1 << h, vector<bool>(w, false));
	// TODO: Implement a two-level branch predictor
	while (!trace.eof())
	{
		string line;
		getline(trace, line);
		if (line.empty())
		{
			continue;
		}

		string hexStr = line.substr(2, 8);
		int branchAction = stoi(line.substr(11));
		bitset<32> binaryBitset(stoul(hexStr, nullptr, 16));

		// Step 1: Index BHT using h bits from the PC
		string hString = binaryBitset.to_string().substr(32 - (h+2), h);
		int BHTIndex = stoi(hString, nullptr, 2);

		// Step 2: Concatenate w bits from BHT with (m-w) bits from PC
		string wBits;
		for (bool bit : BHT[BHTIndex])
		{
			wBits += bit ? '1' : '0';
		}
		string mwBits = binaryBitset.to_string().substr(32 - (m - w), m - w);
		string concatenatedBits = mwBits + wBits;

		// Step 3: Index PHT using m bits
		int PHTIndex = stoi(concatenatedBits, nullptr, 2);

		if (PHT[PHTIndex].to_ulong() < 2)
		{
			out << 0 << endl; // predict not taken
		}
		else
		{
			out << 1 << endl; // predict taken
		}
		// Step 4: Compare prediction with actual branch action and update PHT
		if (branchAction == 1)
		{
			if (PHT[PHTIndex].to_ulong() < 3)
			{
				PHT[PHTIndex] = PHT[PHTIndex].to_ulong() + 1;
			}
		}
		else
		{
			if (PHT[PHTIndex].to_ulong() > 0)
			{
				PHT[PHTIndex] = PHT[PHTIndex].to_ulong() - 1;
			}
		}

		// Step 5: Update BHT
		BHT[BHTIndex].insert(BHT[BHTIndex].begin(), branchAction);
		BHT[BHTIndex].pop_back();
		

		// Output prediction result
	}

	trace.close();
	out.close();
}

// Path: branchsimulator_skeleton_23.cpp