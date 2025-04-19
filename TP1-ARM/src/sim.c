#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"

// Define las máscaras de los opcodes
#define ADDS_extended  0b10101011000
#define ADDS_inmediate 0b10110001
#define SUBS_extended_register 0b11101011000
#define SUBS_inmediate 0b11110001
#define HLT 0b11010100010
#define CMP_extended 0b11101011001
#define CMP_inmediate 0b11110001
#define ANDS_shifted_register 0b11101010
#define EOR_shifted_register 0b11001010
#define ORR_shifted_register 0b10101010
#define B 0b000101
#define B_cond 0b01010100
#define BR 0b1101011000011111000000 //raro
#define LSL_inmediate 0b110100110
#define LSR_inmediate 0b110100110
#define STUR 0b11111000000
#define STURB 0b00111000000
#define STURH 0b01111000000
#define LDUR 0b11111000010
#define LDURH 0b01111000010
#define LDURB 0b00111000010
#define MOVZ 0b110100101
#define ADD_extended 0b10001011001
#define ADD_inmediate 0b10010001
#define MUL 0b10011011000
#define CBZ 0b10110100
#define CBNZ 0b10110101


// Define una estructura para almacenar los opcodes y sus longitudes
typedef struct {
    uint32_t opcode;
    int length; // Longitud en bits del opcode
} OpcodeEntry;

// Define un array con los opcodes conocidos
OpcodeEntry opcode_dict[] = {
    {ADDS_extended, 11},
    {ADDS_inmediate, 8},
    {SUBS_extended_register, 11},
    {SUBS_inmediate, 8},
    {HLT, 11},
    {CMP_extended, 11},
    {ANDS_shifted_register, 8},
    {EOR_shifted_register, 8},
    {ORR_shifted_register, 8},
    {B, 6},
    {B_cond, 8},
    {LDURB, 11},
    {MOVZ, 9},
    {ADD_extended, 11},
    {LSL_inmediate, 10},
    {LSR_inmediate, 10},
    {STUR, 11},
    {STURB, 11},
    {STURH, 11},
    {LDUR, 11},
    {MUL, 11},
    {CBZ, 8},
    {CBNZ, 8}
};
const int opcode_dict_size = sizeof(opcode_dict) / sizeof(opcode_dict[0]);



uint32_t get_opcode(uint32_t instruction){
    
    for(int i = 0; i < opcode_dict_size; i++){
        // uint32_t mask = (1 << opcode_dict[i].length) - 1;
        uint32_t extracted_opcode = instruction >> (32 - opcode_dict[i].length);
        if(extracted_opcode == opcode_dict[i].opcode){
            return opcode_dict[i].opcode;
        }
    }
}

// Función para determinar el tipo de instrucción
const char* identify_instruction(uint32_t instruction) {
    uint32_t opcode = get_opcode(instruction);

    printf("Opcode: %x\n", opcode);

    if (opcode == ADDS_extended) {
        return "ADDS_extended";
    } else if (opcode == ADDS_inmediate) {
        return "ADDS_inmediate";
    } else if (opcode == SUBS_extended_register) {
        return "SUBS_extended_register";
    } else if (opcode == SUBS_inmediate) {
        return "SUBS_inmediate";
    } else if (opcode == HLT) {
        return "HLT";
    } else if (opcode == CMP_extended) {
        return "CMP_extended";
    } else if (opcode == ANDS_shifted_register) {
        return "ANDS_shifted_register";
    } else if (opcode == EOR_shifted_register) {
        return "EOR_shifted_register";
    } else if (opcode == ORR_shifted_register) {
        return "ORR_shifted_register";
    } else if (opcode == B) {
        return "B";
    } else if (opcode == B_cond) {
        return "B_cond";
    } else if (opcode == LDURB) {
        return "LDRUB";
    } else if (opcode == MOVZ) {
        return "MOVZ";
    } else if (opcode == ADD_extended) {
        return "ADD_extended";
    } else if (opcode == LSL_inmediate) {
        return "LSL_inmmediate";
    } else if (opcode == LSR_inmediate) {
        return "LSR_inmediate";
    } else if (opcode == STUR) {
        return "STUR";
    } else if (opcode == STURB) {
        return "STURB";
    } else if (opcode == STURH) {
        return "STURH";
    } else if (opcode == LDUR) {
        return "LDUR";
    } else if (opcode == MUL) {
        return "MUL";
    } else if (opcode == CBZ) {
        return "CBZ";
    } else if (opcode == CBNZ) {
        return "CBNZ";
    } else {
        return "Unknown Instruction";
    }
}

void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    printf("Instruction: %x\n", instruction);
    printf("Opcode: %s\n", identify_instruction(instruction));
}





