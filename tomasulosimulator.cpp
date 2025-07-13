#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <limits>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::istringstream;

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

// Akshath's Helper Functions Start
string regToInt(string regName) {
	if(!isdigit(regName[0]))
		return regName.substr(1, regName.length());
	return regName;
}
int operationMap(string op) {
	if(op == "ADD") {return 0;}
	if(op == "SUB") {return 1;}
	if(op == "MULT") {return 2;}
	if(op == "DIV")  {return 3;}
	if(op == "LOAD") {return 4;}
	if(op == "STORE") {return 5;}
	return -1;
}
// Akshath's Helper Functions End

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
	bool dataReady;
};

/*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/
struct Instruction {
	string op;  // "LOAD" / "STORE" / etc
	bool is_i_type;
	int src_1;
	int src_2;
	int dest;
};

class RegisterResultStatuses
{
public:
	RegisterResultStatuses(int FRegSize) {
		_registers.resize(FRegSize);
		for(int i = 0; i < FRegSize; i++) {
			_registers[i].dataReady = false;
			_registers[i].ReservationStationName = "";
		}
	}
	void _setRegisterResult(int idx, bool dataReady, string stationName, string expected_stn) {
		if(_registers[idx].ReservationStationName == expected_stn){
			_registers[idx].dataReady = dataReady;
		}
		if(stationName != "" && stationName[0] != 'S') {
			_registers[idx].ReservationStationName = stationName;
		}
	}

	void setRegisterResult(int idx, bool dataReady, string stationName) {
		if(stationName[0] == 'S') { return; }

		_registers[idx].dataReady = dataReady;
		if(stationName != "") {
			_registers[idx].ReservationStationName = stationName;
		}
	}
	string getReservationStationName(int idx) {
		// Index is the F0, F1, F2... Fn's numeric part
		// This is the same as source/destination
		// cout << "IDX: " << idx << endl;
		// cout << "REG LEN:" << _registers.size() << endl;
		if(_registers[idx].dataReady) { return ""; }
		return _registers[idx].ReservationStationName;
	}

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
	string station_name;
	string Qj, Qk;
	bool busy;
	int inst_idx;
	int cycles_remaining;
};
class ReservationStations
{
public:
	ReservationStations(int n_add, int n_mult, int n_ld, int n_store) {
		_stations.resize(n_add+n_ld+n_mult+n_store);
		add_start = 0;
		mult_start = add_start + n_add;
		ld_start = mult_start + n_mult;
		store_start = ld_start + n_ld;
		end = store_start + n_store;
		for(int i = 0; i < end; i++) {
			_stations[i].inst_idx= -1;
		}

		for(int i = add_start; i < mult_start; i++)
			_stations[i].station_name = "Add"+std::to_string(i);
		for(int i = mult_start; i < ld_start; i++)
			_stations[i].station_name = "Mult"+std::to_string(i-mult_start);
		for(int i = ld_start; i < store_start; i++)
			_stations[i].station_name = "Load"+std::to_string(i-ld_start);
		for(int i = store_start; i < end; i++)
			_stations[i].station_name = "Store"+std::to_string(i-store_start);
	}
	ReservationStation issue(string op, int instr_idx, string qj, string qk) {
		// Returns stn name if success
		int start_idx, end_idx;
		int cycles = OperationCycle[operationMap(op)];
		if(qk != "" || qj != "") {cycles++;}
		// cout << op << "\t" << cycles << endl;
		// Start exec 1 cycle after both have been readied
		if(op == "LOAD") {
			start_idx = ld_start;
			end_idx   = store_start;
		}
		if(op == "STORE") {
			start_idx = store_start;
			end_idx   = end;
		}
		if(op == "ADD" || op == "SUB"){
			start_idx = add_start;
			end_idx   = mult_start;
		}
		if(op == "MULT" || op == "DIV"){
				start_idx = mult_start;
				end_idx   = ld_start;
			}
		// Here
		// Now we have which type of operation we're trying to issue.
		// Now we will assign instruction to reservation station if it is free
		for(int i = start_idx; i < end_idx; i++) {
			// cout << _stations[i].station_name << "\t" << _stations[i].busy << endl;
			if(!_stations[i].busy){
				// Found empty station...
				// cout << "Found Empty" << endl;
				_stations[i].busy = true;
				_stations[i].inst_idx = instr_idx;
				_stations[i].Qj = qj;
				_stations[i].Qk = qk;
				_stations[i].cycles_remaining = cycles;
				return _stations[i];
			}
		}
		return ReservationStation({""});
	}
	vector<int> update() {
		// Subtracting
		vector<int> idxs;
		// cout << "Updating:\n";
		for(int i = 0; i < end; i++) {
			if(_stations[i].busy && _stations[i].Qj == "" && _stations[i].Qk == "") {
				if(_stations[i].cycles_remaining > 0){
					_stations[i].cycles_remaining -= 1;
				}
				// cout << _stations[i].station_name << "\t" << _stations[i].cycles_remaining << "\n";
				if(_stations[i].cycles_remaining == 0){
					// cout << "Cycles become 0 for instr: " << _stations[i].station_name <<"\n";
					idxs.push_back(_stations[i].inst_idx);
				}
			}
		}
		return idxs;
	}
	string free_station_with_instr_idx(int instr_idx) {
		// cout << "Freeing stn with instr_idx: " << instr_idx << endl;
		// Add logic to update _stations having qk or qj = this station
		int station_idx = -1;
		for(int i = 0; i < end; i++) {
			if(_stations[i].inst_idx == instr_idx) {
				// Find station (stn[i]) with instr_idx = instr_idx
				station_idx = i;
				break;
			}
		}
		if(station_idx == -1){return "";}
		// cout << "Here\n";
		if(_stations[station_idx].cycles_remaining == 0){
			_stations[station_idx].busy = false;
			// cout << "FREED STATION NAME: " << _stations[station_idx].station_name << "\t" << (_stations[station_idx].busy?"busy":"free") << endl;
		} else {
			// cout << "CYCLES REMAINING " << _stations[station_idx].cycles_remaining << endl;
		}
		for(int i = 0; i < end; i++){
			if(_stations[i].Qj != "" && _stations[i].Qj == _stations[station_idx].station_name) {
				// cout << "FREED QJ FOR STATION: " << _stations[i].station_name << endl;
				_stations[i].Qj = "";
			}
			if(_stations[i].Qk != "" && _stations[i].Qk == _stations[station_idx].station_name) {
				// cout << "FREED Qk FOR STATION: " << _stations[i].station_name << endl;
				_stations[i].Qk = "";
			}
		}
		return _stations[station_idx].station_name;
	}
private:
	int add_start, mult_start, ld_start, store_start, end;
	vector<ReservationStation> _stations;
};

class CommonDataBus
{
public:
	// ...
};

// Function signatures
void PrintRegisterResultStatus4Grade(const string &filename, const RegisterResultStatuses &registerResultStatus, const int thiscycle);
void PrintResult4Grade(const string &filename, const vector<InstructionStatus> &instructionStatus);


// Function to simulate the Tomasulo algorithm
void simulateTomasulo(vector<Instruction>instructions, RegisterResultStatuses registerResultStatus, vector<InstructionStatus> instructionStatus, ReservationStations reservationStations)
{
	// for(int i = 0; i < instructions.size(); i++) {
	// 	Instruction current_instruction = instructions[i];
	// 	cout << current_instruction.op << "\t" << current_instruction.is_i_type << "\t" << current_instruction.dest << "\t" << current_instruction.src_1 << "\t" << current_instruction.src_2 << "\n";	
	// }
	// cout << "HERE " << instructionStatus.size() << endl;
	int thiscycle = 1; // start cycle: 1
	int instruction_idx = 0;
	bool issue_more = true;
	
	while (thiscycle < 100000000)
	{
		// cout << "\nCYCLE: " << thiscycle << "\n";
		
		// WRITE BACK (CDB) and FREE RESERVATION STATIONS
		int wb_instr_idx = -1;
		// cout << "In wb\n";
		for(int i = 0; i < instructionStatus.size(); i++) {
			// cout << "Instr " << i << " cycleExecuted:"<< instructionStatus[i].cycleExecuted << " finished:" << instructionStatus[i].cycleWriteResult;
			if(instructionStatus[i].cycleExecuted != 0 && instructionStatus[i].cycleWriteResult == 0) {
				if(wb_instr_idx == -1 || instructionStatus[i].cycleIssued < instructionStatus[wb_instr_idx].cycleIssued) {
					// If wb_instr_idx is not assigned or if instr i's issued before wb_instr
					wb_instr_idx = i;
				}
			}
		}
		if(wb_instr_idx != -1) {
			// cout << "WB FOR INSTR: " << wb_instr_idx << endl;
			// cout << "Written instr: " << wb_instr_idx << endl;
			instructionStatus[wb_instr_idx].cycleWriteResult = thiscycle;
			// FREE RESERVATION STATIONS AFTER CDB
			string s = reservationStations.free_station_with_instr_idx(wb_instr_idx);
			registerResultStatus._setRegisterResult(instructions[wb_instr_idx].dest, true, "", s);
			
		}
		// UPDATE RESERVATION STATION
		
		// cout << "In update" << endl;
		vector<int> exec_complete_idxs = reservationStations.update();
		// cout << "NEED TO FREE: ";
		for(int i = 0; i < exec_complete_idxs.size(); i++) {
			int ind = exec_complete_idxs[i];
			// cout << ind << "\t";
			if(instructionStatus[ind].cycleExecuted == 0) {
				instructionStatus[ind].cycleExecuted = thiscycle;
			}
		}
		// cout << endl;
		
		// ISSUE NEW INSTRUCTION
		
		// cout << "In issue" << endl;
		if(issue_more) {
			Instruction current_instruction = instructions[instruction_idx];
			// cout << "In issue stn" << endl;
			ReservationStation issued_stn;
			if(current_instruction.op == "STORE") {
				// cout << "HERE!!\t" << current_instruction.op << "\t" << instruction_idx << "\t" << current_instruction.dest << endl;
				issued_stn = reservationStations.issue(
				current_instruction.op,
				instruction_idx,
				registerResultStatus.getReservationStationName(current_instruction.dest),
				""
				);
				// cout << "ISSUED STN: " << issued_stn.station_name << endl; 
			} else {
				issued_stn = reservationStations.issue(
					current_instruction.op,
					instruction_idx,
					current_instruction.is_i_type?"":registerResultStatus.getReservationStationName(current_instruction.src_1), //Qj
					current_instruction.is_i_type?"":registerResultStatus.getReservationStationName(current_instruction.src_2)  //Qk
				);
			}
			// cout << "Out of issue stn" << endl;
			if(issued_stn.station_name != "") {
				// If successfully issued
				instructionStatus[instruction_idx].cycleIssued = thiscycle;
				// cout << "Issue success at cycle: " << thiscycle << " issued to stn: " << issued_stn.station_name << " Qk: " << issued_stn.Qk << " Qj: " << issued_stn.Qj << endl;
				registerResultStatus.setRegisterResult(current_instruction.dest, false, issued_stn.station_name);
				// Update issue more fn:
				if(instruction_idx < instructionStatus.size() - 1){
					instruction_idx++;
				} else {
					issue_more = false;
				}
			}
		}


		// At the end of this cycle, we need this function to print all registers status for grading
		PrintRegisterResultStatus4Grade(outputtracename, registerResultStatus, thiscycle);
		// PrintResult4Grade(outputtracename, instructionStatus);
		++thiscycle;

		// The simulator should stop when all instructions are finished.
		bool exit_loop = true;
		for(int m = 0; m < instructionStatus.size(); m++){
			if(instructionStatus[m].cycleWriteResult == 0) {
				exit_loop = false;
			}
		}
		if(exit_loop) { break; }
	}

	PrintResult4Grade(outputtracename, instructionStatus);
	
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

	std::ifstream trace;
	trace.open(inputtracename);
	string line;
	string op, src1, src2, dest;

	vector<Instruction> instructions;
	while (getline(trace, line)) {
		Instruction current_instruction;
		
		istringstream iss(line);
		if (!(iss >> op >> dest >> src1 >> src2)){
			break;
		}
		string _dest, _src_1, _src_2, _op;
		istringstream(op) >> _op; 
		istringstream(dest) >> _dest;
		istringstream(src1) >> _src_1; 
		istringstream(src2) >> _src_2;

		current_instruction.op 		=  _op;
		current_instruction.is_i_type = (_op == "LOAD" || _op == "STORE");
		current_instruction.dest 	= stoi(regToInt(_dest));
		current_instruction.src_1	= stoi(regToInt(_src_1));
		current_instruction.src_2 = stoi(regToInt(_src_2));
		instructions.push_back(current_instruction);
	}

	RegisterResultStatuses registerResultStatus(hardwareConfig.FRegSize);
	ReservationStations reservationStations(hardwareConfig.AddRSsize, hardwareConfig.MultRSsize, hardwareConfig.LoadRSsize, hardwareConfig.StoreRSsize);
	vector<InstructionStatus> instructionStatus;
	instructionStatus.resize(instructions.size());

	// Simulate Tomasulo:
	simulateTomasulo(instructions, registerResultStatus, instructionStatus, reservationStations);

/*********************************** ↑↑↑ Todo: Implement by you ↑↑↑ ******************************************/

	// At the end of the program, print Instruction Status Table for grading
	// PrintResult4Grade(outputtracename, instructionStatus);

	return 0;
}
