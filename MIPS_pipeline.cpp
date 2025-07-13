#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab csa23, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
	bitset<32>  PC;
	bool        nop;
};

struct IDStruct {
	bitset<32>  Instr;
	bool        nop;
};

struct EXStruct {
	// bitset<32> 	Instr;
	bitset<32>  Read_data1;
	bitset<32>  Read_data2;
	bitset<16>  Imm;
	bitset<5>   Rs;
	bitset<5>   Rt;
	bitset<5>   Wrt_reg_addr;
	bool        is_I_type;
	bool        rd_mem;
	bool        wrt_mem; 
	bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
	bool        wrt_enable;
	bool        nop;  
};

struct MEMStruct {
	// bitset<32> 	Instr;
	bitset<32>  ALUresult;
	bitset<32>  Store_data;
	bitset<5>   Rs;
	bitset<5>   Rt;    
	bitset<5>   Wrt_reg_addr;
	bool        rd_mem;
	bool        wrt_mem; 
	bool        wrt_enable;    
	bool        nop;    
};

struct WBStruct {
	// bitset<32> 	Instr;
	bitset<32>  Wrt_data;
	bitset<5>   Rs;
	bitset<5>   Rt;     
	bitset<5>   Wrt_reg_addr;
	bool        wrt_enable;
	bool        nop;     
};

struct stateStruct {
	IFStruct    IF;
	IDStruct    ID;
	EXStruct    EX;
	MEMStruct   MEM;
	WBStruct    WB;
};

class RF
{
	public: 
		bitset<32> Reg_data;
	 	RF()
		{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
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
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
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
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
			else cout<<"Unable to open file";
			imem.close();                     
		}
				  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
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
			int i=0;
			dmem.open("dmem.txt");
			if (dmem.is_open())
			{
				while (getline(dmem,line))
				{      
					DMem[i] = bitset<8>(line);
					i++;
				}
			}
			else cout<<"Unable to open file";
				dmem.close();          
		}
		
		bitset<32> readDataMem(bitset<32> Address)
		{	
			string datamem;
			datamem.append(DMem[Address.to_ulong()].to_string());
			datamem.append(DMem[Address.to_ulong()+1].to_string());
			datamem.append(DMem[Address.to_ulong()+2].to_string());
			datamem.append(DMem[Address.to_ulong()+3].to_string());
			ReadData = bitset<32>(datamem);		//read data memory
			return ReadData;               
		}
			
		void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
		{
			DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
			DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
			DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
			DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
		}   
					 
		void outputDataMem()
		{
			ofstream dmemout;
			dmemout.open("dmemresult.txt");
			if (dmemout.is_open())
			{
				for (int j = 0; j< 1000; j++)
				{     
					dmemout << DMem[j]<<endl;
				}
					 
			}
			else cout<<"Unable to open file";
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
		printstate<<"State after executing cycle:\t"<<cycle<<endl; 
		
		printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
		printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
		
		
		printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
		printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
		
		// printstate<<"EX.Instr:\t"<<state.EX.Instr<<endl;
		printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
		printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
		printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
		printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
		printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
		printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
		printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
		printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
		printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
		printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
		printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
		printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

		// printstate<<"MEM.Instr:\t"<<state.MEM.Instr<<endl;
		printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
		printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
		printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
		printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
		printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
		printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
		printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
		printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
		printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

		// printstate<<"WB.Instr:\t"<<state.WB.Instr<<endl;
		printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
		printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
		printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
		printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
		printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
		printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
	}
	else cout<<"Unable to open file";
	printstate.close();
}
 

int main()
{
	
	RF myRF;
	INSMem myInsMem;
	DataMem myDataMem;

	stateStruct state;
	state.IF.PC=bitset<32> (0);
	state.IF.nop = false;
	state.ID.nop = state.EX.nop = state.MEM.nop = state.WB.nop = true;

	state.ID.Instr=bitset<32>(0);
	state.EX.Read_data1=bitset<32> (0);
	state.EX.Read_data2=bitset<32> (0);
	state.EX.Rs=bitset<5> (0);
	state.EX.alu_op = true;
	state.EX.Imm=bitset<16> (0);
	state.EX.Rt=bitset<5> (0);
	state.EX.Wrt_reg_addr=bitset<5> (0);
	state.EX.is_I_type=0;
	state.EX.rd_mem=0;
	state.EX.wrt_mem=0;
	state.EX.wrt_enable=0;
	state.MEM.ALUresult=bitset<32> (0);
	state.MEM.Rs=bitset<5> (0);
	state.MEM.Rt=bitset<5> (0);
	state.MEM.Store_data=bitset<32> (0);
	state.MEM.Wrt_reg_addr=bitset<5> (0);
	state.MEM.rd_mem=0;
	state.MEM.wrt_mem=0;
	state.MEM.wrt_enable= 0;
	state.WB.Rs=bitset<5> (0);
	state.WB.Rt=bitset<5> (0);
	state.WB.Wrt_reg_addr=bitset<5> (0);
	state.WB.Wrt_data=bitset<32> (0);
	state.WB.wrt_enable=0;

	// state.EX.Instr = state.MEM.Instr = state.WB.Instr = bitset<32>(0);

	int cycle = 0;
	bitset<32> branchAddr;
	
	bool exit = false;
	while (1) {
		stateStruct newState = state;
		bool take_branch = false;
		/* --------------------- WB stage --------------------- */
		if(!state.WB.nop) {
			if(state.WB.wrt_enable) {
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
			}
		}
		newState.WB.nop = state.MEM.nop;
		/* --------------------- MEM stage --------------------- */
		if(!state.MEM.nop) {
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;

			newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			
			// Do mem operation
			if(state.MEM.rd_mem) {
				// LW
				newState.WB.Wrt_data = myDataMem.readDataMem(
					state.MEM.ALUresult
				);
			} else if(state.MEM.wrt_mem){
				// SW
				myDataMem.writeDataMem (
					state.MEM.ALUresult,
					state.MEM.Store_data
				);
			} else {
				newState.WB.Wrt_data=state.MEM.ALUresult;
			}
			// newState.WB.Instr = state.MEM.Instr;
		}
		newState.MEM.nop = state.EX.nop;
		/* --------------------- EX stage --------------------- */
		if(!state.EX.nop) {
			// Inputs - Rs, Rt, ReadData1, ReadData2, is_i_type
			// If it is R-type: Wrt_reg_addr, wrt_enable, alu_op => True is AddU

			// Forwarding control
			// if(state.EX.Rs == state.WB.Wrt_reg_addr && !state.WB.nop) {
			// 	state.EX.Read_data1 = newState.WB.Wrt_data;
			// 	// MEM-EX forward
			// }
			// if(state.EX.Rt == state.WB.Wrt_reg_addr && !state.WB.nop) {
			// 	state.EX.Read_data2 = newState.WB.Wrt_data;
			// 	// MEM-EX forward
			// }
			// // If ExRs = MemRd && not read/write
			// if(state.EX.Rs == state.MEM.Wrt_reg_addr && !state.MEM.rd_mem && !state.MEM.wrt_mem && !state.MEM.nop) {
			// 	state.EX.Read_data1 = newState.MEM.ALUresult;
			// 	// EX-EX Forward
			// }
			// if(state.EX.Rt == state.MEM.Wrt_reg_addr && !state.MEM.rd_mem && !state.MEM.wrt_mem && !state.MEM.nop) {
			// 	state.EX.Read_data2 = newState.MEM.ALUresult;
			// 	// newState.MEM.ALUresult is state.EX.R1 +- state.EX.R2
			// 	// EX-EX Forward
			// }
			
			
			// Generate output FFs
			newState.MEM.Rs = state.EX.Rs;
			newState.MEM.Rt = state.EX.Rt;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr; //Rd
			newState.MEM.wrt_enable = state.EX.wrt_enable;
			newState.MEM.wrt_mem = state.EX.wrt_mem;
			newState.MEM.rd_mem =  state.EX.rd_mem;
			
			
			newState.MEM.Store_data = state.EX.Read_data2; // Was in SW command

			// Changed all newState.EX -> state.EX
			// Run original test
			if(!state.EX.is_I_type) { 			// R-Type
				if(state.EX.alu_op) { 				// AddU
					newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() 
																 + state.EX.Read_data2.to_ulong();
				} else { 											// SubU
					newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() 
																 - state.EX.Read_data2.to_ulong();	
				}
			} else {
			// If it is I-type:  Imm, rd_mem, wrt_mem, wrt_enable
				bitset<32> signExtendImm;
				if(state.EX.Imm[15]) {// if MSB is 1 -
					signExtendImm = (0xFFFF << 16) | state.EX.Imm.to_ulong();
				} else {
					signExtendImm = (0x0000 << 16) | state.EX.Imm.to_ulong();
				}
				// cout << signExtendImm << "\n";
				
				if(state.EX.rd_mem) {
					// LW
					// Do nothing since rd_mem already updated
					newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() 
																 + signExtendImm.to_ulong();
				} else if (state.EX.wrt_mem) {
					// SW
					// Do nothing since wrt_mem already updated
					newState.MEM.ALUresult = state.EX.Read_data1.to_ulong() 
																 + signExtendImm.to_ulong();
				} else {
					// BNE
					// Try to handle in IF/ID stage
				}	
			}
			// newState.MEM.Instr = state.EX.Instr;
		}
		newState.EX.nop = state.ID.nop;
		/* --------------------- ID stage --------------------- */
		if(!state.ID.nop) {
			// Inputs - ID.Instr, ID.nop
			int instruction_int = (int)state.ID.Instr.to_ulong();
			bitset<6> opcode(instruction_int >> 26);

			// Generate output FFs for EX stage
			newState.EX.Rs = bitset<5>((instruction_int >> 21) & 0b11111);
			newState.EX.Rt = bitset<5>((instruction_int >> 16) & 0b11111);
			newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
			newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);

			newState.EX.is_I_type = false;
			newState.EX.rd_mem = false;
			newState.EX.wrt_mem = false;

			// newState.EX.Instr = state.ID.Instr;

			// newState.EX.wrt_enable = false; // Default to false

			newState.EX.Imm = bitset<16>((instruction_int) & 0xFFFF);
			// Generate more specific output FFs for EX stage
			if(opcode.none()) {
				// R-Type Instruction 
				newState.EX.Wrt_reg_addr = bitset<5>((instruction_int >> 11) & 0b11111);
				newState.EX.wrt_enable = true;

				bitset<6> funct(instruction_int & 0b111111);
				if(funct == 0x21) { 											// AddU
					newState.EX.alu_op = true;
				} else if(funct == 0x23) {								// SubU
					newState.EX.alu_op = false;
				}
			} else {
				// I-Type
				newState.EX.is_I_type = true;	
				// cout << "Instruction: " << state.ID.Instr << "\n" << "Imm: " << newState.EX.Imm << "\n";
				if(opcode.to_ulong() == 0x23) { 					// LW
					newState.EX.rd_mem = true;
					newState.EX.wrt_mem = false;
					newState.EX.Wrt_reg_addr = bitset<5>((instruction_int >> 16) & 0b11111); // = Rt
					newState.EX.wrt_enable = true;
				} else if (opcode.to_ulong() == 0x2B) { 	// SW
					newState.EX.rd_mem = false;
					newState.EX.wrt_mem = true;
					newState.EX.wrt_enable = false;
				} else { 																	// BNE
					// cout << "Here\n";
					newState.EX.rd_mem = false;
					newState.EX.wrt_mem = false;
					newState.EX.wrt_enable = false;
					if(newState.EX.Read_data1 != newState.EX.Read_data2) {
						newState.ID.nop = true;
						bitset<32> signExtendImm;
						// cout << newState.EX.Imm << "\n";
						if(newState.EX.Imm[15]) {// if MSB is 1 -
							signExtendImm = (0xFFFF << 16) | newState.EX.Imm.to_ulong();
						} else {
							signExtendImm = (0x0000 << 16) | newState.EX.Imm.to_ulong();
						}
						branchAddr = (signExtendImm << 2).to_ulong() & 0xFFFFFFFF;
						take_branch = true;
						// cout << "Here1: " << branchAddr << "\n";
					}
				}
			}
		}
		newState.ID.nop = state.IF.nop;
		if(take_branch) { newState.ID.nop = true; } // Squash out branched instruction
		/* --------------------- IF stage --------------------- */
		if(!state.IF.nop) {
			// Inputs - IF.PC, IF.nop
			// cout << "PC:" << state.IF.PC.to_ulong() << " In cycle:"<< cycle << "\n";
			// Generate output FFs for ID stage
			newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
			// if(state.EX.is_I_type && !state.EX.rd_mem && !state.EX.wrt_mem)
			// Update PC for next IF stage
			if(newState.ID.Instr.all()) {
				newState.IF.nop = newState.ID.nop = true;
				exit = true;
				// Need to set ID to nop too, because otherwise newState.ID.Instr will be fetched and 1 extra cycle
			} else {
				newState.IF.PC = state.IF.PC.to_ulong() + 4;
			}
			if(take_branch){
					newState.IF.PC = state.IF.PC.to_ulong() + branchAddr.to_ulong(); 
													 //state.IF.PC has already become state.IF.PC(beforeBNE) + 4
					// cout << "PC Original:"  << state.IF.PC.to_ulong() << " New: " << newState.IF.PC.to_ulong() << "\n";
					take_branch = false;
					exit = false;
					branchAddr = 0;
				}
		}
		
		  // Handle Forwarding Control Logic
			if(newState.EX.Rs == newState.WB.Wrt_reg_addr && !newState.WB.nop) {
				newState.EX.Read_data1 = newState.WB.Wrt_data;
				// MEM-EX forward
			}
			if(newState.EX.Rt == newState.WB.Wrt_reg_addr && !newState.WB.nop) {
				newState.EX.Read_data2 = newState.WB.Wrt_data;
				// MEM-EX forward
			}
			// If ExRs = MemRd && not read/write
			if(newState.EX.Rs == newState.MEM.Wrt_reg_addr && !newState.MEM.rd_mem && !newState.MEM.wrt_mem && !newState.MEM.nop) {
				newState.EX.Read_data1 = newState.MEM.ALUresult;
				// EX-EX Forward
			}
			if(newState.EX.Rt == newState.MEM.Wrt_reg_addr && !newState.MEM.rd_mem && !newState.MEM.wrt_mem && !newState.MEM.nop) {
				newState.EX.Read_data2 = newState.MEM.ALUresult;
				// newState.MEM.ALUresult is state.EX.R1 +- state.EX.R2
				// EX-EX Forward
			}
			if(newState.MEM.wrt_enable && newState.MEM.rd_mem && !newState.MEM.nop) {
				if(newState.MEM.Wrt_reg_addr == newState.EX.Rs || newState.MEM.Wrt_reg_addr == newState.EX.Rt) {
					// Stall pipeline if Rs or Rt match.
					newState.ID = state.ID;
					newState.IF = state.IF;		
					newState.EX.nop = true; 
				}
			}

		if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
			break;
		if(!exit)
			newState.IF.nop = false;
		
		printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
	   
		state = newState; /*** The end of the cycle and updates the current state with the values calculated in this cycle. csa23 ***/ 
		cycle+=1;
	}
	myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}
