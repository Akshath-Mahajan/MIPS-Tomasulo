# MIPS Pipeline with Tomasulo Algorithm Simulator

This repository contains two C++ files developed as part of NYU's *Computer Systems Architecture* course. Both make a simulator that is cycle-accurate and was designed to model real-world microarchitectural behavior with a focus on instruction-level parallelism and control hazard handling.

**Passed All Cases**

## 🛠 Projects

### 1. MIPS 5-Stage Pipeline Simulator

Simulates a classic 5-stage pipelined MIPS processor:
- **Stages:** IF, ID, EX, MEM, WB
- **Hazard Handling:** RAW hazards via forwarding/stalling; Control hazards via 2-level dynamic branch prediction
- **Output:** Logs architectural state per cycle (`stateresult.txt`), and dumps final register file and memory states (`RFresult.txt`, `dmemresult.txt`)
- **Inputs:** Hidden test cases using `imem.txt` and `dmem.txt` (not included here)

📄 **Details:** [MIPS_Simulator_Details.pdf](./MIPS_Simulator_Details.pdf)

---

### 2. Tomasulo's Algorithm Simulator

Implements out-of-order execution using Tomasulo's algorithm:
- **Reservation Stations** for ADD, MULT, LOAD, STORE
- **Common Data Bus (CDB)** for broadcasting results
- **Register Status Table** for dependency tracking
- **Instruction Latency Modeling:** ADD/SUB/LOAD/STORE = 2 cycles, MULT = 10, DIV = 40
- **Output:** Tracks issue, execution, and write-back cycles for each instruction in `trace.out.txt`

📄 **Details:** [Tomasulo_Details.pdf](./Tomasulo_Details.pdf)

---

## 📦 Repository Structure

```text
.
├── MIPS_pipeline.cpp            # MIPS 5-stage pipeline simulator
├── tomasulosimulator.cpp        # Tomasulo's algorithm simulator
├── MIPS_Simulator_Details.pdf   # Lab 2 specification
├── Tomasulo_Details.pdf         # Lab 6 specification
└── README.md                    # This file
```

> 🧪 **Note:** Input files (`imem.txt`, `dmem.txt`, `trace.txt`, `config.txt`) are hidden test cases provided as part of the coursework and are not included here.

---

## 📍Author

**Akshath Mahajan**  
[GitHub](https://github.com/Akshath-Mahajan) • [LinkedIn](https://www.linkedin.com/in/akshath-mahajan-713989192/)
