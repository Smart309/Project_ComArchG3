#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

//เอาไว้เรียกใช้ function ต่างๆ
void printState(stateType *);
void Add(stateType *, int);
void Nand(stateType *, int);
void LW(stateType *,int);
void SW(stateType *, int);
void Beq(stateType *, int);
void Jalr(stateType *, int);


int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) { //ถ้าใส่ไฟล์ไม่ครบ 2 files
        printf("error: usage: %s <machine-code file>\n", argv[0]); //จะแจ้ง error
        exit(1);
    }
    
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) { //ถ้าไม่มีอะไรใน file จะแจ้ง error
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
	    printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    for(int i = 0; i < NUMREGS; i++){ //set reg[] 7 ตัว
        state.reg[i] = 0;
    }
    
    state.pc = 0; //set program counter = 0
    int count = 0; // สร้างตัวแปร count เพื่อใช้นับ instructions executed
    for(int i = 0; 1 ; i++){ //สร้าง loop เพื่อวนแต่ละ instruction
       
        int opcode = (state.mem[i] >> 22) & 0x7; //เอาส่วน opcode ออกมาจาก machine code
        
        count++; //ทำ instructions executed แต่ละครั้ง count จะเพิ่มทีละ 1   
            printState(&state); // printstate ก่อนทำ instruction ต่างๆ
            
            if(opcode == 2){   // ถ้า opcode = 2 ทำ lw instruction
                LW(&state,i);
            }
            else if(opcode == 0){ // ถ้า opcode = 0 ทำ Add instruction
                Add(&state,i);
            }
            else if(opcode == 4){ // ถ้า opcode = 4 ทำ Beq instruction
                Beq(&state,i);
                i = state.pc; //PC คือ address ของ beq instruction
            }else if(opcode == 6){ // ถ้า opcode = 6 ทำ halt instruction
                printf("machine halted\n");
                printf("Total of %d instructions executed\n",count); //แสดงจำนวน instructions executed ทั้งหมด
                printf("final state of machine:\n");
                state.pc++; // pc+1
                printState(&state);
                break;
            }else if(opcode == 7){ // ถ้า opcode = 7 ไม่ต้องทำอะไร
                
            }else if(opcode == 1){ // ถ้า opcode = 1 ทำ Nand instruction
                Nand(&state,i);
            }else if(opcode == 3){ // ถ้า opcode = 3 ทำ sw instruction
                SW(&state,i);
            }else if(opcode == 5){ // ถ้า opcode = 5 ทำ Jalr instruction
                Jalr(&state,i);
                i = state.pc-1;
                state.pc--;
            }
            
            state.pc++; // pc+1

    }
    return(0);
}

void printState(stateType *statePtr) //print state
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("end state\n");
}


void Add(stateType *AddPtr , int i){
    int rs = (AddPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA
    int rt = (AddPtr->mem[i] >> 16) & 0x7; // rt = ค่าใน RegB
    int rd = (AddPtr->mem[i]) & 0x7; //rd = ค่าใน destReg
    int sum = AddPtr->reg[rs] + AddPtr->reg[rt]; //ผลลัพธ์ได้จาก rs + rt 
    AddPtr->reg[rd] = sum; //นำคำตอบเก็บใน destReg
}
void Nand(stateType *NandPtr, int i){
    int rs = (NandPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA
    int rt = (NandPtr->mem[i] >> 16) & 0x7; // rt = ค่าใน RegB
    int rd = (NandPtr->mem[i]) & 0x7; //rd = ค่าใน destReg
    int sum = !(NandPtr->reg[rs] & NandPtr->reg[rt]);
    NandPtr->reg[rd] = sum; //นำคำตอบเก็บใน destReg
}
void LW(stateType *LWPtr, int i){
    int rs = (LWPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA 
    int rt = (LWPtr->mem[i] >> 16) & 0x7; // rt = ค่าใน RegB
    int OFF = (LWPtr->mem[i]) & 0xFFFF; // ค่า offsetField เป็นเลข16-bit และเป็น 2’s complement
    int sum = LWPtr->reg[rs] + OFF; //ผลลัพธ์ได้จาก offsetField บวกกับค่าใน regA
    // printf("%d\n",sum); //เอาไว้ check
    LWPtr->reg[rt] = LWPtr->mem[sum]; //Load RegB จาก memory
    // printf("\n %d %d %d\n",rs,rt,OF);
}
void SW(stateType *SWPtr, int i){
    int rs = (SWPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA
    int rt = (SWPtr->mem[i] >> 16) & 0x7; // rt = ค่าใน RegB
    int OFF = (SWPtr->mem[i]) & 0xFFFF; // ค่า offsetField เป็นเลข16-bit และเป็น 2’s complement
    int sum = SWPtr->reg[rs] + OFF; //ผลลัพธ์ได้จาก offsetField บวกกับค่าใน RegA
    SWPtr->mem[sum] = SWPtr->reg[rt]; //Store RegB ใน memory
}
void Beq(stateType *BeqPtr, int i){
    int rs = (BeqPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA
    int rt = (BeqPtr->mem[i] >> 16) & 0x7; // rt = ค่าใน RegB
    short OFF = (BeqPtr->mem[i]) & 0xFFFF; // ค่า offsetField เป็นเลข16-bit และเป็น 2’s complement
    if(BeqPtr->reg[rs] == BeqPtr->reg[rt]){ //ถ้า RegA เท่ากับค่าใน RegB ให้กระโดดไปที่ address PC+1+offsetField
        BeqPtr->pc += OFF;
    }

}
void Jalr(stateType *JalrPtr, int i){
    int rs = (JalrPtr->mem[i] >> 19) & 0x7; // rs = ค่าใน RegA
    int rd = (JalrPtr->mem[i] >> 16) & 0x7; // rd = ค่าใน RegB
    JalrPtr->reg[rd] = JalrPtr->pc+1; //เก็บค่า PC+1 ไว้ใน RegB
    if(rs != rd){
        JalrPtr->pc = JalrPtr->reg[rs]; //ถ้า rs ไม่เท่ากับ rd กระโดดไปที่ address ที่ถูกเก็บไว้ใน regA
    }else{
        JalrPtr->pc = JalrPtr->reg[rd]; //ถ้า rs เท่ากับ rd กระโดดไปที่ PC+1       
    }
}