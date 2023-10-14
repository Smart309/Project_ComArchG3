#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// ReadAssembly
#define MAXLINELENGTH 1000
#define SIZE 100

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int hashCode(int key); //Use the key to make hashcode.
bool checkBFInsert(char vals[MAXLINELENGTH]); //Check for duplicate data before inserting.
void insert(int key, char val[MAXLINELENGTH]);//Insert data.
struct address* delete(struct address* item);//Delete data.
void displayTable();

struct address
{
    int addr;
    char nameLabel[MAXLINELENGTH];
};

struct address* hashArray[SIZE]; 
struct address* dummyItem;
struct address* item;
void ReadAssembly(int argc , char * argv[]);

//Simulator
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
void simulator(int argc , char * argv[]);

int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",argv[0]);
        exit(1);
    }
    // ReadAssembly
    ReadAssembly(argc,argv);
    //Simulator
    simulator(argc,argv);
    return(0);
}

//Assembly
void ReadAssembly(int argc , char * argv[]){
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    
    dummyItem = (struct address*) malloc(sizeof(struct address));
    dummyItem->nameLabel[MAXLINELENGTH] = -1;  
    dummyItem->addr = -1;
    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening inFileStrin %s\n", inFileString);
        exit(1);
    }

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening outFileString %s\n", outFileString);
        exit(1);
    }

    int j = 0;
    char temp[MAXLINELENGTH] = "x";
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) //ลูปสำหรับเก็บข้อมูล label
    {
        sprintf(temp, "%s", label); //นำ label ไปฝากไว้ใน Temp
        if(checkBFInsert(temp)){ //เช็คข้อมูลก่อนที่จะ insert
            insert(j,temp); // insert label พร้อม address ของ label
        }else{
            fprintf(stderr, "Label '%s' has been repeated.\n",label); //แจ้งว่า label มีอยู่แล้ว
            exit(1);
        }
        j++;//เช็คบรรทัดถัดไป
    }
    // displayTable();
    /* this is how to rewind the file ptr so that you start reading from the beginning of the file */
    rewind(inFilePtr); //กลับไปเริ่มอ่านไฟล์ inFilePtr ใหม่
    bool checkUndefine;
    int dex = 0b00000000000000000000000000000000;
    int twoCom = 0b11111111111111111111111111111111;
    int now = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){   
        checkUndefine = false;
        dex = 0b00000000000000000000000000000000;
        twoCom = 0b0000000000000000;
        if (!strcmp(opcode, "add")) { //ถ้า opcode = add
            dex |= 0b000 << 22; //Bits 24-22 opcode is 000
            if(isNumber(arg0)&&isNumber(arg1)&&isNumber(arg2)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rt)
                dex |= atoi(arg2) << 0; //Bits 2-0 reg2(rd)
                
            } 
        }else if(!strcmp(opcode, "nand")){ //ถ้า opcode = nand
            dex |= 0b001 << 22; //Bits 24-22 opcode is 001
            if(isNumber(arg0)&&isNumber(arg1)&&isNumber(arg2)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rt)
                dex |= atoi(arg2) << 0; //Bits 2-0 reg2(rd)
            }
        }else if(!strcmp(opcode, "lw")){ //ถ้า opcode = lw
            dex |= 0b010 << 22; //Bits 24-22 opcode is 010
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rt)
            }
            if (atoi(arg2) > 0xFFFF){ //ถ้า arg2(OffSetField) มีจำนวนมากกว่า 16 bits   
                fprintf(stderr, "OffSetField > 0xFFFF\n"); //แจ้งว่า arg2(OffSetField) เกิน 16 bits
                exit(1);
            }
            if (isNumber(arg2)) //ถ้า arg2 เป็นตัวเลข
            {
                dex |= atoi(arg2) << 0; //Bits 15-0 arg2(OffSetField)
            }else{ //ถ้า arg2 ไม่เป็นตัวเลข
                for (int j = 0; j < SIZE; j++){ //วิ่งหาข้อมูลใน hashArray ที่เก็บข้อมูล label
                    if(hashArray[j] != NULL){ //เช็คก่อนว่า hashArray[j] != Null
                        if(!strcmp(arg2, hashArray[j]->nameLabel)){ //เช็คว่า arg2(label) ตรงกับ nameLabel ใน hashArray[j]
                            dex |= hashArray[j]->addr << 0; //นำเลข address ไปเก็บที่ตำแหน่งของ offsetField
                            checkUndefine = true; //รู้จัก label
                            break;
                        }
                    }
                }
                if (checkUndefine != true){ //ไม่รู้จัก label
                    fprintf(stderr, "Label '%s' is Undefine.\n",arg2); //แจ้งว่าไม่รู้จัก label
                    exit(1);
                }
            }
        }else if(!strcmp(opcode, "sw")){ //ถ้า opcode = sw
            dex |= 0b011 << 22; //Bits 24-22 opcode is 011
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rt)
            }
            if (atoi(arg2) > 0xFFFF){ //เช็ค arg2(OffSetField) มีจำนวนมากกว่า 16 bits
                fprintf(stderr, "OffSetField > 0xFFFF\n"); //แจ้งว่า arg2(OffSetField) เกิน 16 bits
                exit(1);
            }
            if (isNumber(arg2)){ //ถ้า arg2 เป็นตัวเลข
                dex |= atoi(arg2) << 0; //Bits 15-0 arg2(OffSetField)
            }else{ //ถ้า arg2 ไม่เป็นตัวเลข
                for (int j = 0; j < SIZE; j++){ //วิ่งหาข้อมูลใน hashArray ที่เก็บข้อมูล label
                    if(hashArray[j] != NULL){ //เช็คก่อนว่า hashArray[j] != Null
                        if(!strcmp(arg2, hashArray[j]->nameLabel)){ //เช็คว่า arg2(label) ตรงกับ nameLabel ใน hashArray[j]
                            dex |= hashArray[j]->addr << 0; //นำเลข address ไปเก็บที่ตำแหน่งของ offsetField
                            checkUndefine = true; //รู้จัก label
                            break;
                        }
                    }
                }
                if (checkUndefine != true){ //ไม่รู้จัก label
                    fprintf(stderr, "Label '%s' is Undefine.\n",arg2); //แจ้งว่าไม่รู้จัก label
                    exit(1);
                }
            }
        }else if(!strcmp(opcode, "beq")){ //ถ้า opcode = beq
            dex |= 0b100 << 22; //Bits 24-22 opcode is 100
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rt)
            }
            if (atoi(arg2) > 0xFFFF){ //เช็ค arg2(OffSetField) มีจำนวนมากกว่า 16 bits
                fprintf(stderr, "OffSetField > 0xFFFF\n"); //แจ้งว่า arg2(OffSetField) เกิน 16 bits
                exit(1);
            }
            if (isNumber(arg2)){ //ถ้า arg2 เป็นตัวเลข
                dex |= atoi(arg2) << 0; //Bits 15-0 arg2(OffSetField)
            }else{ //ถ้า arg2 ไม่เป็นตัวเลข
                for (int j = 0; j < SIZE; j++){ //วิ่งหาข้อมูลใน hashArray ที่เก็บข้อมูล label
                    if(hashArray[j] != NULL){ //เช็คก่อนว่า hashArray[j] != Null
                        if(!strcmp(arg2, hashArray[j]->nameLabel)){ //เช็คว่า arg2(label) ตรงกับ nameLabel ใน hashArray[j]
                            if(now > hashArray[j]->addr){ //ถ้าบรรทัด address ปัจจุบัน มากกว่า address ของ label
                                twoCom = (1 << 16) + (~(now-hashArray[j]->addr+1)+1);
                                dex |= twoCom;
                            }else{ //ถ้าบรรทัด address ปัจจุบัน น้อยกว่า address ของ label
                                dex |= hashArray[j]->addr-now-1 << 0;
                            }
                            checkUndefine = true; //รู้จัก label
                            break;
                        }
                    }
                }
                if (checkUndefine != true){ //ไม่รู้จัก label
                    fprintf(stderr, "Label '%s' is Undefine.\n",arg2); //แจ้งว่าไม่รู้จัก label
                    exit(1);
                }
            }
        }else if(!strcmp(opcode, "jalr")){ //ถ้า opcode = jalr
            dex |= 0b101 << 22; //Bits 24-22 opcode is 101
            if(isNumber(arg0)&&isNumber(arg1)){
                dex |= atoi(arg0) << 19; //Bits 21-19 reg0(rs)
                dex |= atoi(arg1) << 16; //Bits 18-16 reg1(rd)
            }
        }else if(!strcmp(opcode, "halt")){ //ถ้า opcode = halt
            dex |= 0b110 << 22; //Bits 24-22 opcode is 110
        }else if(!strcmp(opcode, "noop")){ //ถ้า opcode = noop
            dex |= 0b111 << 22; //Bits 24-22 opcode is 111
        }else if(!strcmp(opcode, ".fill")){ //ถ้า opcode = .fill
            if(isNumber(arg0)){ //ถ้า arg0 เป็นตัวเลข
                dex |= atoi(arg0) << 0; //Bits 15-0 arg2(OffSetField)
            }else{ //ถ้า arg2 ไม่เป็นตัวเลข
                for (int k = 0; k < SIZE; k++){ //วิ่งหาข้อมูลใน hashArray ที่เก็บข้อมูล label
                    if(hashArray[k] != NULL){ //เช็คก่อนว่า hashArray[k] != Null
                        if(!strcmp(arg0, hashArray[k]->nameLabel)){ //เช็คว่า arg0(label) ตรงกับ nameLabel ใน hashArray[k]
                            dex |= hashArray[k]->addr << 0; //เก็บค่าของ label
                            break;
                        }
                    }
                    
                }
            }
        }else{
            fprintf(stderr, "Opcode '%s' is Undefine.\n",opcode); //แจ้งว่าไม่รู้จัก Opcode
            exit(1);
        }
        fprintf(outFilePtr,"%d\n",dex);
        now++;
    }
    rewind(outFilePtr);
}

int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,char *arg1, char *arg2){
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
	    printf("error: line too long\n");
	    exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int hashCode(int key){
    return key % SIZE;
}

bool checkBFInsert(char vals[MAXLINELENGTH]){
    for(int i = 0; i<SIZE; i++) {
        if(hashArray[i] != NULL){
            if(!strcmp(hashArray[i]->nameLabel,vals) && hashArray[i] != NULL){
                return false;
            }
        }
    }
    return true;   
}

void insert(int key,char vals[MAXLINELENGTH]){
    if(strcmp(vals,"")){
        struct address *item = (struct address*) malloc(sizeof(struct address));
        
        strcpy( item->nameLabel, vals);
        item->addr = key;

        //get the hash 
        int hashIndex = hashCode(key);

        //move in array until an empty or deleted cell
        while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->addr != -1) {
            //go to next cell
            ++hashIndex;
            
            //wrap around the table
            hashIndex %= SIZE;
        }

        hashArray[hashIndex] = item;
    }
}

struct address* delete(struct address* item) {
   int key = item->addr;

   //get the hash 
   int hashIndex = hashCode(key);

   //move in array until an empty 
   while(hashArray[hashIndex] !=NULL) {
	
      if(hashArray[hashIndex]->addr == key) {
         struct address* temp = hashArray[hashIndex]; 
			
         //assign a dummy item at deleted position
         hashArray[hashIndex] = dummyItem; 
         return temp;
      } 
		
      //go to next cell
      ++hashIndex;
		
      //wrap around the table
      hashIndex %= SIZE;
   }  
	
   return NULL;        
}

void displayTable() {
   int i = 0;
	
   for(i = 0; i<SIZE; i++) {
      if(hashArray[i] != NULL)
         printf(" (%d,%s)",hashArray[i]->addr,hashArray[i]->nameLabel);
      else
         printf(" ~~ ");
   }
	
   printf("\n");
}


//Simulator
void simulator(int argc , char * argv[]){
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    
    filePtr = fopen(argv[2], "r");
    if (filePtr == NULL) { //ถ้าไม่มีอะไรใน file จะแจ้ง error
        printf("error: can't open file %s", argv[2]);
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
}

void printState(stateType *statePtr){ //print state
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