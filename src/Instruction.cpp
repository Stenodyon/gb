#include <cassert>
#include <sstream>
#include <iomanip>

#include <fmt/core.h>

#include "Instruction.hpp"

namespace GB {

const char* register_name(RegisterIndex8 reg)
{
    switch (reg) {
        case RegisterA:
            return "A";
        case RegisterB:
            return "B";
        case RegisterC:
            return "C";
        case RegisterD:
            return "D";
        case RegisterE:
            return "E";
        case RegisterF:
            return "F";
        case RegisterH:
            return "H";
        case RegisterL:
            return "L";

        default:
            assert(false); // unreachable
    }
}

const char* register_name(RegisterIndex16 reg)
{
    switch (reg) {
        case RegisterAF:
            return "AF";
        case RegisterBC:
            return "BC";
        case RegisterDE:
            return "DE";
        case RegisterHL:
            return "HL";
        case RegisterSP:
            return "SP";

        default:
            assert(false); // unreachable
    }
}

const char* condition_name(InstructionCondition condition)
{
    switch (condition) {
        case Z:  return "Z";
        case NZ: return "NZ";
        case C:  return "C";
        case NC: return "NC";

        default:
                 assert(false); // unreachable
    }
}

const char* instruction_format_name(InstructionFormat format)
{
    switch (format) {
        case OP_NONE:       return "OP_NONE";
        case OP_ILLEGAL:    return "OP_ILLEGAL";
        case OP_imm8:       return "OP_imm8";
        case OP_sr8:        return "OP_sr8";
        case OP_dr8:        return "OP_dr8";
        case OP_r8_imm8:    return "OP_r8_imm8";
        case OP_r8_r8:      return "OP_r8_r8";
        case OP_iC_A:       return "OP_iC_A";
        case OP_A_iC:       return "OP_A_iC";
        case OP_iHL:        return "OP_iHL";
        case OP_iHL_r8:     return "OP_iHL_r8";
        case OP_r8_iHL:     return "OP_r8_iHL";
        case OP_iimm8_A:    return "OP_iimm8_A";
        case OP_A_iimm8:    return "OP_A_iimm8";
        case OP_cond:       return "OP_cond";
        case OP_cond_imm8:  return "OP_cond_imm8";
        case OP_cond_imm16: return "OP_cond_imm16";
        case OP_imm16:      return "OP_imm16";
        case OP_r16:        return "OP_r16";
        case OP_ir16:       return "OP_ir16";
        case OP_ir16_imm8:  return "OP_ir16_imm8";
        case OP_r16_imm16:  return "OP_r16_imm16";
        case OP_ir16_A:     return "OP_ir16_A";
        case OP_A_ir16:     return "OP_A_ir16";
        case OP_ir16inc_A:  return "OP_ir16inc_A";
        case OP_ir16dec_A:  return "OP_ir16dec_A";
        case OP_A_ir16inc:  return "OP_A_ir16inc";
        case OP_A_ir16dec:  return "OP_A_ir16dec";
        case OP_iimm16_A:   return "OP_iimm16_A";
        case OP_A_iimm16:   return "OP_A_iimm16";
        case OP_iimm16_SP:  return "OP_iimm16_SP";
        case OP_SP_imm8:    return "OP_SP_imm8";
        case OP_HL_SP_imm8: return "OP_HL_SP_imm8";
        case OP_SP_HL:      return "OP_SP_HL";

        default:
            assert(false); // unreachable
    }
}

InstructionDescriptor instruction_table[256];
InstructionDescriptor special_instruction_table[256];

static void build(
        InstructionDescriptor* table,
        u8 opcode,
        const char* mnemonic,
        InstructionFormat format,
        InstructionHandler handler
        ) {
    InstructionDescriptor descriptor { handler, format, mnemonic };
    table[opcode] = descriptor;
}

static void build(
        u8 opcode,
        const char* mnemonic,
        InstructionFormat format,
        InstructionHandler handler
        ) {
    build(&instruction_table[0], opcode, mnemonic, format, handler);
}

static void build_cb(
        u8 opcode,
        const char* mnemonic,
        InstructionFormat format,
        InstructionHandler handler
        ) {
    build(&special_instruction_table[0], opcode, mnemonic, format, handler);
}

[[gnu::constructor]] static void build_instruction_table()
{
    build(0x00, "NOP", OP_NONE, &InstructionInterpreter::NOP);
    build(0x10, "STOP", OP_imm8, &InstructionInterpreter::STOP);
    build(0x20, "JR", OP_cond_imm8, &InstructionInterpreter::JR_cond);
    build(0x30, "JR", OP_cond_imm8, &InstructionInterpreter::JR_cond);

    build(0x01, "LD", OP_r16_imm16, &InstructionInterpreter::LD_r16_imm16);
    build(0x11, "LD", OP_r16_imm16, &InstructionInterpreter::LD_r16_imm16);
    build(0x21, "LD", OP_r16_imm16, &InstructionInterpreter::LD_r16_imm16);
    build(0x31, "LD", OP_r16_imm16, &InstructionInterpreter::LD_r16_imm16);

    build(0x02, "LD", OP_ir16_A, &InstructionInterpreter::LD_ir16_A);
    build(0x12, "LD", OP_ir16_A, &InstructionInterpreter::LD_ir16_A);
    build(0x22, "LD", OP_ir16inc_A, &InstructionInterpreter::LD_HLinc_A);
    build(0x32, "LD", OP_ir16dec_A, &InstructionInterpreter::LD_HLdec_A);

    build(0x03, "INC", OP_r16, &InstructionInterpreter::INC_r16);
    build(0x13, "INC", OP_r16, &InstructionInterpreter::INC_r16);
    build(0x23, "INC", OP_r16, &InstructionInterpreter::INC_r16);
    build(0x33, "INC", OP_r16, &InstructionInterpreter::INC_r16);
    build(0x0b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x1b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x2b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x3b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);

    build(0x04, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x14, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x24, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x34, "INC", OP_iHL, &InstructionInterpreter::INC_iHL);
    build(0x05, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x15, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x25, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x35, "DEC", OP_iHL, &InstructionInterpreter::DEC_iHL);

    build(0x06, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x16, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x26, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x36, "LD", OP_ir16_imm8, &InstructionInterpreter::LD_iHL_imm8);

    build(0x07, "RLCA", OP_NONE, &InstructionInterpreter::RLCA);
    build(0x17, "RLA", OP_NONE, &InstructionInterpreter::RLA);
    build(0x27, "DAA", OP_NONE, &InstructionInterpreter::DAA);
    build(0x37, "SCF", OP_NONE, &InstructionInterpreter::SCF);

    build(0x08, "LD", OP_iimm16_SP, &InstructionInterpreter::LD_iimm16_SP);
    build(0x18, "JR", OP_imm8, &InstructionInterpreter::JR);
    build(0x28, "JR", OP_cond_imm8, &InstructionInterpreter::JR_cond);
    build(0x38, "JR", OP_cond_imm8, &InstructionInterpreter::JR_cond);

    build(0x09, "ADD HL,", OP_r16, &InstructionInterpreter::ADD_HL_r16);
    build(0x19, "ADD HL,", OP_r16, &InstructionInterpreter::ADD_HL_r16);
    build(0x29, "ADD HL,", OP_r16, &InstructionInterpreter::ADD_HL_r16);
    build(0x39, "ADD HL,", OP_r16, &InstructionInterpreter::ADD_HL_r16);

    build(0x0a, "LD", OP_A_ir16, &InstructionInterpreter::LD_ir16_A);
    build(0x1a, "LD", OP_A_ir16, &InstructionInterpreter::LD_ir16_A);
    build(0x2a, "LD", OP_A_ir16inc, &InstructionInterpreter::LD_A_HLinc);
    build(0x3a, "LD", OP_A_ir16dec, &InstructionInterpreter::LD_A_HLdec);

    build(0x0b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x1b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x2b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);
    build(0x3b, "DEC", OP_r16, &InstructionInterpreter::DEC_r16);

    build(0x0c, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x1c, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x2c, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x3c, "INC", OP_dr8, &InstructionInterpreter::INC_r8);
    build(0x0d, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x1d, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x2d, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);
    build(0x3d, "DEC", OP_dr8, &InstructionInterpreter::DEC_r8);

    build(0x0e, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x1e, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x2e, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);
    build(0x3e, "LD", OP_r8_imm8, &InstructionInterpreter::LD_r8_imm8);

    build(0x0f, "RRCA", OP_NONE, &InstructionInterpreter::RRCA);
    build(0x1f, "RRA", OP_NONE, &InstructionInterpreter::RRA);
    build(0x2f, "CPL", OP_NONE, &InstructionInterpreter::CPL);
    build(0x3f, "CCF", OP_NONE, &InstructionInterpreter::CCF);

    for (u8 opcode = 0x40; opcode <= 0x7f; ++opcode) {
        if (opcode == 0x76) {
            build(0x76, "HALT", OP_NONE, &InstructionInterpreter::HALT);
            continue;
        }

        if ((opcode & 0xf) == 6 || (opcode & 0xf8) == 0x70) // LD with (HL)
            continue;

        build(opcode, "LD", OP_r8_r8, &InstructionInterpreter::LD_r8_r8);
    }

    build(0x46, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x56, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x66, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x4e, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x5e, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x6e, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);
    build(0x7e, "LD", OP_r8_iHL, &InstructionInterpreter::LD_r8_iHL);

    build(0x70, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x71, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x72, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x73, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x74, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x75, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);
    build(0x77, "LD", OP_iHL_r8, &InstructionInterpreter::LD_iHL_r8);

    build(0x80, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x81, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x82, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x83, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x84, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x85, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);
    build(0x86, "ADD", OP_iHL, &InstructionInterpreter::ADD_iHL);
    build(0x87, "ADD", OP_sr8, &InstructionInterpreter::ADD_r8);

    build(0x88, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x89, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x8a, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x8b, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x8c, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x8d, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);
    build(0x8e, "ADC", OP_iHL, &InstructionInterpreter::ADC_iHL);
    build(0x8f, "ADC", OP_sr8, &InstructionInterpreter::ADC_r8);

    build(0x90, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x91, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x92, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x93, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x94, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x95, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);
    build(0x96, "SUB", OP_iHL, &InstructionInterpreter::SUB_iHL);
    build(0x97, "SUB", OP_sr8, &InstructionInterpreter::SUB_r8);

    build(0x98, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x99, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x9a, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x9b, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x9c, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x9d, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);
    build(0x9e, "SBC", OP_iHL, &InstructionInterpreter::SBC_iHL);
    build(0x9f, "SBC", OP_sr8, &InstructionInterpreter::SBC_r8);

    build(0xa0, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa1, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa2, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa3, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa4, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa5, "AND", OP_sr8, &InstructionInterpreter::AND_r8);
    build(0xa6, "AND", OP_iHL, &InstructionInterpreter::AND_iHL);
    build(0xa7, "AND", OP_sr8, &InstructionInterpreter::AND_r8);

    build(0xa8, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xa9, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xaa, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xab, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xac, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xad, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);
    build(0xae, "XOR", OP_iHL, &InstructionInterpreter::XOR_iHL);
    build(0xaf, "XOR", OP_sr8, &InstructionInterpreter::XOR_r8);

    build(0xb0, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb1, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb2, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb3, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb4, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb5, "OR", OP_sr8, &InstructionInterpreter::OR_r8);
    build(0xb6, "OR", OP_iHL, &InstructionInterpreter::OR_iHL);
    build(0xb7, "OR", OP_sr8, &InstructionInterpreter::OR_r8);

    build(0xb8, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xb9, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xba, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xbb, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xbc, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xbd, "CP", OP_sr8, &InstructionInterpreter::CP_r8);
    build(0xbe, "CP", OP_iHL, &InstructionInterpreter::CP_iHL);
    build(0xbf, "CP", OP_sr8, &InstructionInterpreter::CP_r8);

    build(0xc0, "RET", OP_cond, &InstructionInterpreter::RET_cond);
    build(0xd0, "RET", OP_cond, &InstructionInterpreter::RET_cond);
    build(0xe0, "LDH", OP_iimm8_A, &InstructionInterpreter::LDH_iimm8_A);
    build(0xf0, "LDH", OP_A_iimm8, &InstructionInterpreter::LDH_A_iimm8);

    build(0xc1, "POP", OP_r16, &InstructionInterpreter::POP_r16);
    build(0xd1, "POP", OP_r16, &InstructionInterpreter::POP_r16);
    build(0xe1, "POP", OP_r16, &InstructionInterpreter::POP_r16);
    build(0xf1, "POP", OP_r16, &InstructionInterpreter::POP_r16);

    build(0xc2, "JP", OP_cond_imm16, &InstructionInterpreter::JP_cond);
    build(0xd2, "JP", OP_cond_imm16, &InstructionInterpreter::JP_cond);
    build(0xe2, "LD", OP_iC_A, &InstructionInterpreter::LDH_iC_A);
    build(0xf2, "LD", OP_A_iC, &InstructionInterpreter::LDH_A_iC);

    build(0xc3, "JP", OP_imm16, &InstructionInterpreter::JP);
    build(0xf3, "DI", OP_NONE, &InstructionInterpreter::DI);

    build(0xc4, "CALL", OP_cond_imm16, &InstructionInterpreter::CALL_cond);
    build(0xd4, "CALL", OP_cond_imm16, &InstructionInterpreter::CALL_cond);

    build(0xc5, "PUSH", OP_r16, &InstructionInterpreter::PUSH_r16);
    build(0xd5, "PUSH", OP_r16, &InstructionInterpreter::PUSH_r16);
    build(0xe5, "PUSH", OP_r16, &InstructionInterpreter::PUSH_r16);
    build(0xf5, "PUSH", OP_r16, &InstructionInterpreter::PUSH_r16);

    build(0xc6, "ADD", OP_imm8, &InstructionInterpreter::ADD_imm8);
    build(0xd6, "SUB", OP_imm8, &InstructionInterpreter::SUB_imm8);
    build(0xe6, "AND", OP_imm8, &InstructionInterpreter::AND_imm8);
    build(0xf6, "OR", OP_imm8, &InstructionInterpreter::OR_imm8);

    build(0xc7, "RST 00H", OP_NONE, &InstructionInterpreter::RST00H);
    build(0xd7, "RST 10H", OP_NONE, &InstructionInterpreter::RST10H);
    build(0xe7, "RST 20H", OP_NONE, &InstructionInterpreter::RST20H);
    build(0xf7, "RST 30H", OP_NONE, &InstructionInterpreter::RST30H);

    build(0xc8, "RET", OP_cond, &InstructionInterpreter::RET_cond);
    build(0xd8, "RET", OP_cond, &InstructionInterpreter::RET_cond);
    build(0xe8, "ADD", OP_SP_imm8, &InstructionInterpreter::ADD_SP_imm8);
    build(0xf8, "LD", OP_HL_SP_imm8, &InstructionInterpreter::LD_HL_SP_imm8);

    build(0xc9, "RET", OP_NONE, &InstructionInterpreter::RET);
    build(0xd9, "RETI", OP_NONE, &InstructionInterpreter::RETI);
    build(0xe9, "JP (HL)", OP_NONE, &InstructionInterpreter::JP_iHL);
    build(0xf9, "LD SP, HL", OP_NONE, &InstructionInterpreter::LD_SP_HL);

    build(0xca, "JP", OP_cond_imm16, &InstructionInterpreter::JP_cond);
    build(0xda, "JP", OP_cond_imm16, &InstructionInterpreter::JP_cond);
    build(0xea, "LD", OP_iimm16_A, &InstructionInterpreter::LD_iimm16_A);
    build(0xfa, "LD", OP_A_iimm16, &InstructionInterpreter::LD_A_iimm16);

    build(0xfb, "EI", OP_NONE, &InstructionInterpreter::EI);

    build(0xcc, "CALL", OP_cond_imm16, &InstructionInterpreter::CALL_cond);
    build(0xdc, "CALL", OP_cond_imm16, &InstructionInterpreter::CALL_cond);

    build(0xcd, "CALL", OP_imm16, &InstructionInterpreter::CALL);

    build(0xce, "ADC", OP_imm8, &InstructionInterpreter::ADC_imm8);
    build(0xde, "SBC", OP_imm8, &InstructionInterpreter::SBC_imm8);
    build(0xee, "XOR", OP_imm8, &InstructionInterpreter::XOR_imm8);
    build(0xfe, "CP", OP_imm8, &InstructionInterpreter::CP_imm8);

    build(0xcf, "RST 08H", OP_NONE, &InstructionInterpreter::RST08H);
    build(0xdf, "RST 18H", OP_NONE, &InstructionInterpreter::RST18H);
    build(0xef, "RST 28H", OP_NONE, &InstructionInterpreter::RST28H);
    build(0xff, "RST 38H", OP_NONE, &InstructionInterpreter::RST38H);

    for (usize opcode = 0x00; opcode < 0x08; ++opcode) {
        build_cb((u8)opcode, "RLC", OP_sr8, &InstructionInterpreter::RLC_r8);
    }
    build_cb(0x06, "RLC", OP_iHL, &InstructionInterpreter::RLC_iHL);

    for (usize opcode = 0x08; opcode < 0x10; ++opcode) {
        build_cb((u8)opcode, "RRC", OP_sr8, &InstructionInterpreter::RRC_r8);
    }
    build_cb(0x06, "RRC", OP_iHL, &InstructionInterpreter::RRC_iHL);

    for (usize opcode = 0x10; opcode < 0x18; ++opcode) {
        build_cb((u8)opcode, "RL", OP_sr8, &InstructionInterpreter::RL_r8);
    }
    build_cb(0x06, "RL", OP_iHL, &InstructionInterpreter::RL_iHL);

    for (usize opcode = 0x18; opcode < 0x20; ++opcode) {
        build_cb((u8)opcode, "RR", OP_sr8, &InstructionInterpreter::RR_r8);
    }
    build_cb(0x06, "RR", OP_iHL, &InstructionInterpreter::RR_iHL);

    for (usize opcode = 0x20; opcode < 0x28; ++opcode) {
        build_cb((u8)opcode, "SLA", OP_sr8, &InstructionInterpreter::SLA_r8);
    }
    build_cb(0x06, "SLA", OP_iHL, &InstructionInterpreter::SLA_iHL);

    for (usize opcode = 0x28; opcode < 0x30; ++opcode) {
        build_cb((u8)opcode, "SRA", OP_sr8, &InstructionInterpreter::SRA_r8);
    }
    build_cb(0x06, "SRA", OP_iHL, &InstructionInterpreter::SRA_iHL);

    for (usize opcode = 0x30; opcode < 0x38; ++opcode) {
        build_cb((u8)opcode, "SWAP", OP_sr8, &InstructionInterpreter::SWAP_r8);
    }
    build_cb(0x06, "SWAP", OP_iHL, &InstructionInterpreter::SWAP_iHL);

    for (usize opcode = 0x38; opcode < 0x40; ++opcode) {
        build_cb((u8)opcode, "SRL", OP_sr8, &InstructionInterpreter::SRL_r8);
    }
    build_cb(0x06, "SRL", OP_iHL, &InstructionInterpreter::SRL_iHL);

    for (usize opcode = 0x40; opcode < 0x80; ++opcode) {
        build_cb((u8)opcode, "BIT", OP_sr8, &InstructionInterpreter::BIT_r8);

        if ((opcode & 0x7) == 6)
            build_cb((u8)opcode, "BIT", OP_iHL, &InstructionInterpreter::BIT_iHL);
    }

    for (usize opcode = 0x80; opcode < 0xc0; ++opcode) {
        build_cb((u8)opcode, "RES", OP_sr8, &InstructionInterpreter::RES_r8);

        if ((opcode & 0x7) == 6)
            build_cb((u8)opcode, "RES", OP_iHL, &InstructionInterpreter::RES_iHL);
    }

    for (usize opcode = 0xc0; opcode < 0x100; ++opcode) {
        build_cb((u8)opcode, "SET", OP_sr8, &InstructionInterpreter::SET_r8);

        if ((opcode & 0x7) == 6)
            build_cb((u8)opcode, "SET", OP_iHL, &InstructionInterpreter::SET_iHL);
    }
}

bool InstructionDescriptor::has_imm8() const
{
    switch (format) {
        case OP_NONE:
        case OP_ILLEGAL:
        case OP_sr8:
        case OP_dr8:
        case OP_r8_r8:
        case OP_iC_A:
        case OP_A_iC:
        case OP_iHL:
        case OP_iHL_r8:
        case OP_r8_iHL:
        case OP_cond:
        case OP_cond_imm16:
        case OP_imm16:
        case OP_r16:
        case OP_ir16:
        case OP_r16_imm16:
        case OP_ir16_A:
        case OP_A_ir16:
        case OP_ir16inc_A:
        case OP_ir16dec_A:
        case OP_A_ir16inc:
        case OP_A_ir16dec:
        case OP_iimm16_A:
        case OP_A_iimm16:
        case OP_iimm16_SP:
        case OP_SP_HL:
            return false;

        case OP_imm8:
        case OP_r8_imm8:
        case OP_iimm8_A:
        case OP_A_iimm8:
        case OP_cond_imm8:
        case OP_ir16_imm8:
        case OP_SP_imm8:
        case OP_HL_SP_imm8:
            return true;

        default:
            assert(false); // unreachable
    }
}

bool InstructionDescriptor::has_imm16() const
{
    switch (format) {
        case OP_NONE:
        case OP_ILLEGAL:
        case OP_imm8:
        case OP_sr8:
        case OP_dr8:
        case OP_r8_imm8:
        case OP_r8_r8:
        case OP_iC_A:
        case OP_A_iC:
        case OP_iHL:
        case OP_iHL_r8:
        case OP_r8_iHL:
        case OP_iimm8_A:
        case OP_A_iimm8:
        case OP_cond:
        case OP_cond_imm8:
        case OP_r16:
        case OP_ir16:
        case OP_ir16_imm8:
        case OP_ir16_A:
        case OP_A_ir16:
        case OP_ir16inc_A:
        case OP_ir16dec_A:
        case OP_A_ir16inc:
        case OP_A_ir16dec:
        case OP_SP_imm8:
        case OP_HL_SP_imm8:
        case OP_SP_HL:
            return false;

        case OP_cond_imm16:
        case OP_imm16:
        case OP_r16_imm16:
        case OP_iimm16_A:
        case OP_A_iimm16:
        case OP_iimm16_SP:
            return true;

        default:
            assert(false); // unreachable
    }
}

Instruction Instruction::from_stream(InstructionStream* instruction_stream)
{
    return Instruction(instruction_stream);
}

InstructionHandler Instruction::handler() const
{
    if (m_descriptor->format == OP_ILLEGAL)
        return &InstructionInterpreter::illegal_instruction;
    return m_descriptor->handler;
}

Instruction::Instruction(InstructionStream* instruction_stream)
{
    m_opcode = instruction_stream->read8();

    if (has_sub_op()) {
        m_sub_op = instruction_stream->read8();
        m_descriptor = &special_instruction_table[m_sub_op];
    } else {
        m_descriptor = &instruction_table[m_opcode];
    }

    if (m_descriptor->has_imm8()) {
        m_imm8 = instruction_stream->read8();
    }

    if (m_descriptor->has_imm16()) {
        m_imm16 = instruction_stream->read16();
    }
}

static RegisterIndex8 reg8_map(u8 code)
{
    switch (code) {
        case 0x0:
            return RegisterB;
        case 0x1:
            return RegisterC;
        case 0x2:
            return RegisterD;
        case 0x3:
            return RegisterE;
        case 0x4:
            return RegisterH;
        case 0x5:
            return RegisterL;
        case 0x7:
            return RegisterA;
        default:
            assert(false); // unreachable
    }
}

RegisterIndex8 Instruction::src_reg8() const
{
    u8 opcode = has_sub_op() ? m_sub_op : m_opcode;
    return reg8_map(opcode & 7);
}

RegisterIndex8 Instruction::dst_reg8() const
{
    u8 opcode = has_sub_op() ? m_sub_op : m_opcode;
    return reg8_map((opcode & (7 << 3)) >> 3);
}

RegisterIndex16 Instruction::dst_reg16() const
{
    auto register_code = (m_opcode & (3 << 4)) >> 4;
    switch (register_code) {
        case 0:
            return RegisterBC;
        case 1:
            return RegisterDE;
        case 2:
            return RegisterHL;
        case 3:
            if (m_opcode & 0x80)
                return RegisterAF;
            else
                return RegisterSP;

        default:
            assert(false); // unreachable
    }
}

InstructionCondition Instruction::condition() const
{
    auto condition_code = (m_opcode & (3 << 3)) >> 3;
    switch (condition_code) {
        case 0:
            return NZ;
        case 1:
            return Z;
        case 2:
            return NC;
        case 3:
            return C;

        default:
            assert(false); // unreachable
    }
}

std::string Instruction::to_string() const
{
    if (m_descriptor->format == OP_ILLEGAL) {
        if (has_sub_op()) {
            return fmt::format("illegal instruction 0xcb {:#04x}", m_sub_op);
        } else {
            return fmt::format("illegal instruction {:#04x}", m_opcode);
        }
    }

    std::stringstream out;

    auto format_u8 = [] (u8 value) {
        return fmt::format("{:#04x}", value);
    };
    auto format_u16 = [] (u16 value) {
        return fmt::format("{:#06x}", value);
    };

    out << m_descriptor->mnemonic;

    switch (m_descriptor->format) {
        case OP_NONE:
            break;
        case OP_ILLEGAL:
            assert(false); // unreachable

        case OP_imm8:
            out << " " << format_u8(imm8());
            break;
        case OP_sr8:
            out << " " << register_name(src_reg8());
            break;
        case OP_dr8:
            out << " " << register_name(dst_reg8());
            break;
        case OP_r8_imm8:
            out << " " << register_name(dst_reg8())
                << ", " << format_u8(imm8());
            break;
        case OP_r8_r8:
            out << " " << register_name(dst_reg8())
                << ", "  << register_name(src_reg8());
            break;
        case OP_imm16:
            out << " " << format_u16(imm16());
            break;
        case OP_r16_imm16:
            out << " " << register_name(dst_reg16())
                << ", " << format_u16(imm16());
            break;
        case OP_ir16dec_A:
            out << " (HL-), A";
            break;
        case OP_cond_imm8:
            out << " " << condition_name(condition())
                << ", " << format_u8(imm8());
            break;
        case OP_cond_imm16:
            out << " " << condition_name(condition())
                << ", " << format_u16(imm16());
            break;
        case OP_iimm8_A:
            out << " (" << format_u8(imm8())
                << "), A";
            break;
        case OP_A_iimm8:
            out << " A, (" << format_u8(imm8()) << ")";
            break;
        case OP_A_ir16inc:
            out << " A, (HL+)";
            break;
        case OP_ir16_A:
            out << " (" << register_name(dst_reg16()) << "), A";
            break;
        case OP_iimm16_A:
            out << " (" << format_u16(imm16()) << "), A";
            break;
        case OP_A_iimm16:
            out << " A, (" << format_u16(imm16()) << ")";
            break;
        case OP_r16:
            out << " " << register_name(dst_reg16());
            break;
        case OP_iHL_r8:
            out << " (HL), " << register_name(src_reg8());
            break;
        case OP_A_ir16:
            out << " A, (" << register_name(dst_reg16()) << ")";
            break;
        case OP_ir16inc_A:
            out << " (HL+), A";
            break;
        case OP_r8_iHL:
            out << " " << register_name(dst_reg8()) << ", (HL)";
            break;
        case OP_iHL:
            out << " (HL)";
            break;
        case OP_cond:
            out << " " << condition_name(condition());
            break;
        case OP_ir16:
            out << " (" << register_name(dst_reg16()) << ")";
            break;

        default:
            fprintf(
                    stderr,
                    "unimplemented instruction format %s to_string()\n",
                    instruction_format_name(m_descriptor->format)
                   );
            exit(-1);
    }

    return std::string(out.str());
}

}
