#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",argv[0]);
        exit(1);
    }

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

    int i = 0;
    char temp[MAXLINELENGTH] = "x";
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) //ลูปสำหรับเก็บข้อมูล label
    {
        sprintf(temp, "%s", label); //นำ label ไปฝากไว้ใน Temp
        if(checkBFInsert(temp)){ //เช็คข้อมูลก่อนที่จะ insert
            insert(i,temp); // insert label พร้อม address ของ label
        }else{
            fprintf(stderr, "Label '%s' has been repeated.\n",label); //แจ้งว่า label มีอยู่แล้ว
            exit(1);
        }
        i++;//เช็คบรรทัดถัดไป
    }
    // displayTable();
    /* this is how to rewind the file ptr so that you start reading from the beginning of the file */
    rewind(inFilePtr); //กลับไปเริ่มอ่านไฟล์ inFilePtr ใหม่
    bool checkUndefine;
    int dex = 0b00000000000000000000000000000000;
    int twoCom = 0b11111111111111111111111111111111;
    int now = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {   
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
                                dex |= hashArray[j]->addr << 0;
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

    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,char *arg1, char *arg2)
{
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
