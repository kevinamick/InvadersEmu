#include <stdio.h>
#include <stdlib.h>

typedef struct ConditionCodes {
    uint8_t z:1;
    uint8_t s:1;
    uint8_t p:1;
    uint8_t cy:1;
    uint8_t ac:1;
    uint8_t pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint16_t sp;
    uint16_t pc;
    uint8_t *memory;
    struct ConditionCodes cc;
    uint8_t int_enable;
} State8080;

int parity(int x, int size) {
    int i;
    int p = 0;
    x = (x & ((1<<size)-1));
    for (i=0; i<size; i++) {
        if (x & 0x1) p++;
        x = x >> 1;
    }

    return (0 == (p & 0x1));
}

int Disassemble8080Op(unsigned char *codebuffer, int pc) {
    unsigned char *code = &codebuffer[pc];
    int opbytes = 1;
    printf("%04x ", pc);

    switch (*code) {
        /*
         * & = And operation
         * ^ = Exclusive or operation
         * | = or operation
         * (HL) = Memory
         */
        case 0x00: printf("NOP"); break; // No operation
        case 0x01: printf("LXI    B,#$%02x%02x", code[2], code[1]); opbytes=3; break; // Load immediate register Pair B & C
        case 0x02: printf("STAX    B"); break; // Store A indirect
        case 0x03: printf("INX    B"); break; // Increment B & C registers
        case 0x04: printf("INR    B"); break; // Increment B register
        case 0x05: printf("DCR    B"); break; // Decrement B register
        case 0x06: printf("MVI   B,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> B
        case 0x07: printf("RLC"); break; // Rotate A left
        case 0x08: printf("NOP"); break; // No operation
        case 0x09: printf("DAD    B"); break; // Add B & C to H & L
        case 0x0A: printf("LDAX    B"); break; // Load A indirect
        case 0x0B: printf("DCX    B"); break; // Decrement B & C
        case 0x0C: printf("INR    C"); break; // Increment C
        case 0x0D: printf("DCR    C"); break; // Decrement C
        case 0x0E: printf("MVI    C,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> C
        case 0x0F: printf("RRC"); break; // Rotate A right

        case 0x10: printf("NOP"); break; // No operation
        case 0x11: printf("LXI    D,#$%02x%02x", code[2], code[1]); opbytes=3; break; // Load immediate register Pair D & E
        case 0x12: printf("STAX    D"); break; // Store A indirect
        case 0x13: printf("INX    D"); break; // Increment D & E registers
        case 0x14: printf("INR    D"); break; // Increment D register
        case 0x15: printf("DCR    D"); break; // Decrement D register
        case 0x16: printf("MVI    D,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> D
        case 0x17: printf("RAL"); break; // Rotate A left through carry
        case 0x18: printf("NOP"); break;
        case 0x19: printf("DAD    D"); break; // Add D & E to H & L
        case 0x1A: printf("LDAX    D"); break; // Load A indirect
        case 0x1B: printf("DCX    D"); break; // Decrement D register
        case 0x1C: printf("INR    E"); break; // Increment E register
        case 0x1D: printf("DCR    E"); break; // Decrement E register
        case 0x1E: printf("MVI    E,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> E
        case 0x1F: printf("RAR"); break; // Rotate A right through carry

        case 0x20: printf("NOP"); break; // No operation
        case 0x21: printf("LXI    H,#$%02x%02x", code[2], code[1]); opbytes=3; break; // Load immediate register Pair H & L
        case 0x22: printf("SHLD    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Store register Pair H & L to memory
        case 0x23: printf("INX    H"); break; // Increment H & L registers
        case 0x24: printf("INR    H"); break; // Increment H register
        case 0x25: printf("DCR    H"); break; // Decrement H register
        case 0x26: printf("MVI    H,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> H
        case 0x27: printf("DAA"); break; // Decimal Adjust accumulator
        case 0x28: printf("NOP"); break; // No operation
        case 0x29: printf("DAD    H"); break; // Add H & L to H & L
        case 0x2A: printf("LHLD    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Load H & L registers to memory
        case 0x2B: printf("DCX   H"); break; // Decrement H register
        case 0x2C: printf("INR    L"); break; // Increment L
        case 0x2D: printf("DCR    L"); break; // Decrement L
        case 0x2E: printf("MVI    L,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> L
        case 0x2F: printf("CMA"); break; // Compliment A

        case 0x30: printf("NOP"); break; // No operation
        case 0x31: printf("LXI    SP,#$%02x%02x", code[2], code[1]); opbytes=3; break; // Load immediate register Stack pointer
        case 0x32: printf("STA    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Store register A direct to memory
        case 0x33: printf("INX    SP"); break; // Increment Stack Pointer
        case 0x34: printf("INR    M"); break; // Increment  (HL+1)
        case 0x35: printf("DCR    M"); break; // Decrement  (HL+1)
        case 0x36: printf("MVI    M,#$%02x", code[1]); opbytes=2; break; //Move byte 2 to  (HL)
        case 0x37: printf("STC"); break; // Set carry flag
        case 0x38: printf("NOP"); break; // No operation
        case 0x39: printf("DAD    SP"); break; // Add stack pointer to H & L
        case 0x3A: printf("LDA    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Load A from  (HL)
        case 0x3B: printf("DCX    SP"); break; // Decrement stack pointer
        case 0x3C: printf("INR    A"); break; // Increment A
        case 0x3D: printf("DCR    A"); break; // Decrement A
        case 0x3E: printf("MVI    A,#$%02x", code[1]); opbytes=2; break; // Move byte 2 -> A
        case 0x3F: printf("CMC"); break; // Compliment carry flag

        case 0x40: printf("MOV    B,B"); break; // B -> B
        case 0x41: printf("MOV    B,C"); break; // B -> C
        case 0x42: printf("MOV    B,D"); break; // B -> D
        case 0x43: printf("MOV    B,E"); break; // B -> E
        case 0x44: printf("MOV    B,H"); break; // B -> H
        case 0x45: printf("MOV    B,L"); break; // B -> L
        case 0x46: printf("MOV    B,M"); break; // B ->  (HL)
        case 0x47: printf("MOV    B,A"); break; // B -> A
        case 0x48: printf("MOV    C,B"); break; // C -> B
        case 0x49: printf("MOV    C,C"); break; // C -> C
        case 0x4A: printf("MOV    C,D"); break; // C -> D
        case 0x4B: printf("MOV    C,E"); break; // C -> E
        case 0x4C: printf("MOV    C,H"); break; // C -> H
        case 0x4D: printf("MOV    C,L"); break; // C -> L
        case 0x4E: printf("MOV    C,M"); break; // C ->  (HL)
        case 0x4F: printf("MOV    C,A"); break; // C -> A

        case 0x50: printf("MOV    D,B"); break; // D -> B
        case 0x51: printf("MOV    D,C"); break; // D -> C
        case 0x52: printf("MOV    D,D"); break; // D -> D
        case 0x53: printf("MOV    D,E"); break; // D -> E
        case 0x54: printf("MOV    D,H"); break; // D -> H
        case 0x55: printf("MOV    D,L"); break; // D -> L
        case 0x56: printf("MOV    D,M"); break; // D ->  (HL)
        case 0x57: printf("MOV    D,A"); break; // D -> A
        case 0x58: printf("MOV    E,B"); break; // E -> B
        case 0x59: printf("MOV    E,C"); break; // E -> C
        case 0x5A: printf("MOV    E,D"); break; // E -> D
        case 0x5B: printf("MOV    E,E"); break; // E -> E
        case 0x5C: printf("MOV    E,H"); break; // E -> H
        case 0x5D: printf("MOV    E,L"); break; // E -> L
        case 0x5E: printf("MOV    E,M"); break; // E ->  (HL)
        case 0x5F: printf("MOV    E,A"); break; // E -> A

        case 0x60: printf("MOV    H,B"); break; // H -> B
        case 0x61: printf("MOV    H,C"); break; // H -> C
        case 0x62: printf("MOV    H,D"); break; // H -> D
        case 0x63: printf("MOV    H,E"); break; // H -> E
        case 0x64: printf("MOV    H,H"); break; // H -> H
        case 0x65: printf("MOV    H,L"); break; // H -> L
        case 0x66: printf("MOV    H,M"); break; // H ->  (HL)
        case 0x67: printf("MOV    H,A"); break; // H -> A
        case 0x68: printf("MOV    L,B"); break; // L -> B
        case 0x69: printf("MOV    L,C"); break; // L -> C
        case 0x6A: printf("MOV    L,D"); break; // L -> D
        case 0x6B: printf("MOV    L,E"); break; // L -> E
        case 0x6C: printf("MOV    L,H"); break; // L -> H
        case 0x6D: printf("MOV    L,L"); break; // L -> L
        case 0x6E: printf("MOV    L,M"); break; // L ->  (HL)
        case 0x6F: printf("MOV    L,A"); break; // L -> A

        case 0x70: printf("MOV    M,B"); break; //  (HL) -> B
        case 0x71: printf("MOV    M,C"); break; //  (HL) -> C
        case 0x72: printf("MOV    M,D"); break; //  (HL) -> D
        case 0x73: printf("MOV    M,E"); break; //  (HL) -> E
        case 0x74: printf("MOV    M,H"); break; //  (HL) -> H
        case 0x75: printf("MOV    M,L"); break; //  (HL) -> L
        case 0x76: printf("HLT"); break; // Halt processor
        case 0x77: printf("MOV    M,A"); break; //  (HL) -> A
        case 0x78: printf("MOV    A,B"); break; // A -> B
        case 0x79: printf("MOV    A,C"); break; // A -> C
        case 0x7A: printf("MOV    A,D"); break; // A -> D
        case 0x7B: printf("MOV    A,E"); break; // A -> E
        case 0x7C: printf("MOV    A,H"); break; // A -> H
        case 0x7D: printf("MOV    A,L"); break; // A -> L
        case 0x7E: printf("MOV    A,M"); break; // A ->  (HL)
        case 0x7F: printf("MOV    A,A"); break; // A -> A

        case 0x80: printf("ADD    B"); break; // A -> A + B
        case 0x81: printf("ADD    C"); break; // A -> A + C
        case 0x82: printf("ADD    D"); break; // A -> A + D
        case 0x83: printf("ADD    E"); break; // A -> A + E
        case 0x84: printf("ADD    H"); break; // A -> A + H
        case 0x85: printf("ADD    L"); break; // A -> A + L
        case 0x86: printf("ADD    M"); break; // A -> A +  (HL)
        case 0x87: printf("ADD    A"); break; // A -> A + A
        case 0x88: printf("ADC    B"); break; // A -> A + B w/ carry
        case 0x89: printf("ADC    C"); break; // A -> A + C w/ carry
        case 0x8A: printf("ADC    D"); break; // A -> A + D w/ carry
        case 0x8B: printf("ADC    E"); break; // A -> A + E w/ carry
        case 0x8C: printf("ADC    H"); break; // A -> A + H w/ carry
        case 0x8D: printf("ADC    L"); break; // A -> A + L w/ carry
        case 0x8E: printf("ADC    M"); break; // A -> A +  (HL) w/ carry
        case 0x8F: printf("ADC    A"); break; // A -> A + A w/ carry

        case 0x90: printf("SUB    B"); break; // A -> A - B
        case 0x91: printf("SUB    C"); break; // A -> A - C
        case 0x92: printf("SUB    D"); break; // A -> A - D
        case 0x93: printf("SUB    E"); break; // A -> A - E
        case 0x94: printf("SUB    H"); break; // A -> A - H
        case 0x95: printf("SUB    L"); break; // A -> A - L
        case 0x96: printf("SUB    M"); break; // A -> A -  (HL)
        case 0x97: printf("SUB    A"); break; // A -> A - A
        case 0x98: printf("SBB    B"); break; // A -> A - B w/ carry
        case 0x99: printf("SBB    C"); break; // A -> A - C w/ carry
        case 0x9A: printf("SBB    D"); break; // A -> A - D w/ carry
        case 0x9B: printf("SBB    E"); break; // A -> A - E w/ carry
        case 0x9C: printf("SBB    H"); break; // A -> A - H w/ carry
        case 0x9D: printf("SBB    L"); break; // A -> A - L w/ carry
        case 0x9E: printf("SBB    M"); break; // A -> A -  (HL) w/ carry
        case 0x9F: printf("SBB    A"); break; // A -> A - A w/ carry

        case 0xA0: printf("ANA    B"); break; // A & B
        case 0xA1: printf("ANA    C"); break; // A & C
        case 0xA2: printf("ANA    D"); break; // A & D
        case 0xA3: printf("ANA    E"); break; // A & E
        case 0xA4: printf("ANA    H"); break; // A & H
        case 0xA5: printf("ANA    L"); break; // A & L
        case 0xA6: printf("ANA    M"); break; // A & (HL)
        case 0xA7: printf("ANA    A"); break; // A & A
        case 0xA8: printf("XRA    B"); break; // A ^ B
        case 0xA9: printf("XRA    C"); break; // A ^ C
        case 0xAA: printf("XRA    D"); break; // A ^ D
        case 0xAB: printf("XRA    E"); break; // A ^ E
        case 0xAC: printf("XRA    H"); break; // A ^ H
        case 0xAD: printf("XRA    L"); break; // A ^ L
        case 0xAE: printf("XRA    M"); break; // A ^ (HL)
        case 0xAF: printf("XRA    A"); break; // A ^ A

        case 0xB0: printf("ORA    B"); break; // A | B
        case 0xB1: printf("ORA    C"); break; // A | C
        case 0xB2: printf("ORA    D"); break; // A | D
        case 0xB3: printf("ORA    E"); break; // A | E
        case 0xB4: printf("ORA    H"); break; // A | H
        case 0xB5: printf("ORA    L"); break; // A | L
        case 0xB6: printf("ORA    M"); break; // A | (HL)
        case 0xB7: printf("ORA    A"); break; // A | A
        /*
         * CMP - The specified byte is compared to the
         * contents of the accumulator. The comparison is performed
         * by internally subtracting the contents of REG from the accumulator
         * (leaving both unchanged) and setting the condition
         * bits according to the result. In particular, the Zero bit is
         * set if the quantities are equal, and reset if they are unequal.
         * Since a subtract operation is performed, the Carry bit will be
         * set if there is no carry out of bit 7, indicati ng that the
         * contents of REG are greater than the contents of the accumulator,
         * and reset otherwise.
         */
        case 0xB8: printf("CMP    B"); break;
        case 0xB9: printf("CMP    C"); break;
        case 0xBA: printf("CMP    D"); break;
        case 0xBB: printf("CMP    E"); break;
        case 0xBC: printf("CMP    H"); break;
        case 0xBD: printf("CMP    L"); break;
        case 0xBE: printf("CMP    M"); break;
        case 0xBF: printf("CMP    A"); break;

        /* Return if Not Zero : If the Zero bit is zero, a return operation is performed */
        case 0xC0: printf("RNZ"); break;
        case 0xC1: printf("POP    B"); break; // Pop register Pair B & C off stack
        /* Jump if Not Zero : If the Zero bit is zero, program execution continues at the memory address */
        case 0xC2: printf("JNZ    #$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0xC3: printf("JMP    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump to memory address
        /* Call if Not Zero : If the Zero bit is one, a call operation is performed to subroutine */
        case 0xC4: printf("CNZ    #$%02x%02x", code[2], code[1]); opbytes=3; break;
        case 0xC5: printf("PUSH    B"); break; // Push register Pair B & C on stack
        case 0xC6: printf("ADI    #$%02x", code[1]); opbytes=2; break; // A -> A + byte
        /*
         * RST - The contents of the program counter
         * are pushed onto the stack, providing a return address for
         * later use by a RETURN instruction.
         */
        case 0xC7: printf("RST"); break; // Restart
        /* Return if Zero : If the Zero bit is one, a return operation is performed */
        case 0xC8: printf("RZ"); break;
        case 0xC9: printf("RET"); break; // Unconditional return from subroutine
        case 0xCA: printf("JZ    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Zero
        case 0xCB: printf("JMP    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump to memory address
        case 0xCC: printf("CZ    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Zero
        case 0xCD: printf("CALL    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call address
        case 0xCE: printf("ACI    #$%02x", code[1]); opbytes=2; break; // A -> A + byte w/ carry
        case 0xCF: printf("RST"); break; // Restart

        /* Return if No Carry - If the carry bit is zero, a return operation is performed */
        case 0xD0: printf("RNC"); break;
        case 0xD1: printf("POP    D"); break; // Pop register Pair D & E off stack
        case 0xD2: printf("JNC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if No Carry
        case 0xD3: printf("OUT    #$%02x", code[1]); opbytes=2; break; // Write A to Output Port
        case 0xD4: printf("CNC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if No Carry
        case 0xD5: printf("PUSH    D"); break; // Push register Pair D & E on stack
        case 0xD6: printf("SUI    #$%02x", code[1]); opbytes=2; break; // A -> A - byte
        case 0xD7: printf("RST"); break; // Restart
        case 0xD8: printf("RC"); break; // Return if Carry
        case 0xD9: printf("RET"); break; // Unconditional return from subroutine
        case 0xDA: printf("JC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Carry
        case 0xDB: printf("IN    #$%02x", code[1]); opbytes=2; break; // Write Input Port to A
        case 0xDC: printf("CC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Carry
        case 0xDD: printf("CALL    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call address
        case 0xDE: printf("ACI    #$%02x", code[1]); opbytes=2; break; // A -> A - byte w/ borrow
        case 0xDF: printf("RST"); break; // Restart

        /* Return if Parity Odd - If the Parity bit is zero (indicating odd parity), a return operation is performed */
        case 0xE0: printf("RPO"); break;
        case 0xE1: printf("POP    H"); break; // Pop register Pair H & L off stack
        case 0xE2: printf("JNC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Parity Odd
        /*
         * Exchange Stack - The contents of the L register are exchanged
         * with the contents of the memory byte whose address
         * is held in the stack pointer SP. The contents of the H
         * register are exchanged with the contents of the memory
         * byte whose address is one greater than that held in the stack
         * pointer
         */
        case 0xE3: printf("XTHL"); break;
        case 0xE4: printf("CPO    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Parity Odd
        case 0xE5: printf("PUSH    H"); break; // Push register Pair H & L on stack
        case 0xE6: printf("ANI    #$%02x", code[1]); opbytes=2; break; // A -> A & byte
        case 0xE7: printf("RST"); break; // Restart
        case 0xE8: printf("RPE"); break; // Return if Parity Even
        /*
         * Load Program Counter - The contents of the H register replace the
         * most significant 8 bits of the program counter, and the contents
         * of the L register replace the least significant 8 bits of
         * the program counter. This causes program execution to continue
         * at the address contained in the H and L registers
         */
        case 0xE9: printf("PCHL"); break;
        case 0xEA: printf("JPE,#$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Parity Even
        /*
         * Exchange Registers - The 16 bits of data held in the Hand L
         * registers are exchanged with the 16 bits of data held in the
         * D and E registers
         */
        case 0xEB: printf("XCHG"); break;
        case 0xEC: printf("CC    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Parity Even
        case 0xED: printf("CALL    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call address
        case 0xEE: printf("XRI    #$%02x", code[1]); opbytes=2; break; // A ^ byte
        case 0xEF: printf("RST"); break; // Restart

        case 0xF0: printf("RP"); break; // Return on Positive
        /*
         * Register pair PSW (Program Status Word) refers to register
         * A (7) and a special byte which reflects the current status of
         * the machine flags
         */
        case 0xF1: printf("POP    PSW"); break; // Pop A and Flags off stack
        case 0xF2: printf("JP    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Positive
        case 0xF3: printf("DI"); break; // Disable Interrupt
        case 0xF4: printf("CP    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Positive
        case 0xF5: printf("PUSH    PSW"); break; // Push A and Flags on stack
        case 0xF6: printf("ORI    #$%02x", code[1]); opbytes=2; break; // A -> A | byte
        case 0xF7: printf("RST"); break; // Restart
        case 0xF8: printf("RM"); break; // Return on Minus
        case 0xF9: printf("SPHL"); break; // Load Stack Pointer from H & L
        case 0xFA: printf("JM    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Jump if Minus
        case 0xFB: printf("EI"); break; // Enable Interrupts
        case 0xFC: printf("CM    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call if Minus
        case 0xFD: printf("CALL    #$%02x%02x", code[2], code[1]); opbytes=3; break; // Call address
        case 0xFE: printf("CPI     #$%02x", code[1]); opbytes=2; break; // A compared with byte
        case 0xFF: printf("RST"); break; // Restart
    }
    printf("\n");
    return opbytes;
}

void UnimplementedInstruction(State8080* state) {
    //pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    state->pc--;
    Disassemble8080Op(state->memory, state->pc);
    exit(1);
}

int Emulate8080p(State8080* state) {
    unsigned char *opcode = &state->memory[state->pc];

    switch(*opcode) {
        case 0x00: break; // NOP
        case 0x01: // LXI B
            state->c = opcode[1];
            state->b = opcode[2];
            state->pc += 2;
            break;
        case 0x02: // STAX B
            state->a = state->b;
            break;
        case 0x03: // INX B
            state->c++;
            if (state->c == 0) {
                state->b++;
            }
            break;
        case 0x0F: { // RRC
            uint8_t x = state->a;
            state->a = ((x & 1) << 7) | (x >> 1);
            state->cc.cy = (1 == (x & 1));
        } break;
        case 0x1F: { // RAR
            uint8_t x = state->a;
            state->a = ((state->cc.cy) << 7) | (x >> 1);
            state->cc.cy = (1 == (x & 1));
        }
        case 0x2F: //CMA
            state->a = ~state->a;
            break;
        case 0x80: { // ADD B
            uint16_t a = (u_int16_t) state->a + (uint16_t) state->b;
            state->cc.z = ((a & 0xff) == 0);
            state->cc.s = ((a & 0x80) != 0);
            state->cc.cy = (a > 0xff);
            state->cc.p = parity(a & 0xff, 8);
            state->a = a & 0xff;
        } break;
        case 0x81: { // ADD C
            uint16_t a = (u_int16_t) state->a + (uint16_t) state->c;
            state->cc.z = ((a & 0xff) == 0);
            state->cc.s = ((a & 0x80) != 0);
            state->cc.cy = (a > 0xff);
            state->cc.p = parity(a & 0xff, 8);
            state->a = a & 0xff;
        } break;
        case 0x86: { // ADD M
            uint16_t offset = (state->h<<8) | (state->l);
            uint16_t a = (u_int16_t) state->a + (uint16_t) state->memory[offset];
            state->cc.z = ((a & 0xff) == 0);
            state->cc.s = ((a & 0x80) != 0);
            state->cc.cy = (a > 0xff);
            state->cc.p = parity(a & 0xff, 8);
            state->a = a & 0xff;
        } break;
        case 0xC2: // JNZ address
            if (state->cc.z == 0) {
                state->pc = (opcode[2] << 8) | opcode[1];
            } else {
                state->pc += 2;
            }
            break;
        case 0xC3: // JMP address
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        case 0xC6: { // ADI byte
            uint16_t a = (u_int16_t) state->a + (uint16_t) opcode[1];
            state->cc.z = ((a & 0xff) == 0);
            state->cc.s = ((a & 0x80) != 0);
            state->cc.cy = (a > 0xff);
            state->cc.p = parity(a & 0xff, 8);
            state->a = a & 0xff;
        } break;
        case 0xC9: // RET
            state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8);
            state->sp += 2;
            break;
        case 0xCD: { // CALL address
            uint16_t ret = state->pc+2;
            state->memory[state->sp-1] = (ret >> 8) & 0xff;
            state->memory[state->sp-2] = (ret & 0xff);
            state->sp = state->sp-2;
            state->pc = (opcode[2] << 8) | opcode[1];
        } break;
        case 0xE6: { // ANI
            uint8_t x = state->a & opcode[1];
            state->cc.z = (x == 0);
            state->cc.s = (0x80 == (x & 0x80));
            state->cc.p = parity(x, 8);
            state->cc.cy = 0;
            state->a = x;
            state->pc++;
        } break;
        default:
            UnimplementedInstruction(state);
            break;
    }
}

int main (int argc, char **argv) {
    FILE *f = fopen(argv[1], "rb");

    if (f==NULL) {
        printf("error: Couldn't open %s\n", argv[1]);
        exit(1);
    }

    //Get the file size and read it into a buffer
    fseek(f, 0L, SEEK_END); // Position to end of file
    int fsize = ftell(f); // File size = current position (last character)
    fseek(f, 0L, SEEK_SET); // Position to start of file

    unsigned char *buffer = malloc(fsize);

    fread(buffer, fsize, 1, f);
    fclose(f);

    int pc = 0;

    while (pc < fsize) {
        pc += Disassemble8080Op(buffer, pc);
    }

    return 0;
}