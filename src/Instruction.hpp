#pragma once

#include <string>
#include <cassert>

#include "Defs.hpp"

namespace GB {

class Instruction;
class InstructionInterpreter;
typedef void(InstructionInterpreter::*InstructionHandler)(const Instruction&);

enum InstructionFormat {
    OP_NONE,
    OP_ILLEGAL,

    OP_imm8,
    OP_sr8,
    OP_dr8,
    OP_r8_imm8,
    OP_r8_r8,
    OP_iC_A,
    OP_A_iC,
    OP_iHL,
    OP_iHL_r8,
    OP_r8_iHL,
    OP_iimm8_A,
    OP_A_iimm8,

    OP_cond,
    OP_cond_imm8,
    OP_cond_imm16,

    OP_imm16,
    OP_r16,
    OP_ir16,
    OP_ir16_imm8,
    OP_r16_imm16,
    OP_ir16_A,
    OP_A_ir16,
    OP_ir16inc_A,
    OP_ir16dec_A,
    OP_A_ir16inc,
    OP_A_ir16dec,
    OP_iimm16_A,
    OP_A_iimm16,

    OP_iimm16_SP,
    OP_SP_imm8,
    OP_HL_SP_imm8,
    OP_SP_HL,
};

enum InstructionCondition {
    Z, NZ,
    C, NC,
};

enum RegisterIndex8 {
    RegisterA = 0,
    RegisterB,
    RegisterC,
    RegisterD,
    RegisterE,
    RegisterF,
    RegisterH,
    RegisterL,
};

enum RegisterIndex16 {
    RegisterAF = 0,
    RegisterBC,
    RegisterDE,
    RegisterHL,
    RegisterSP,
};

const char* register_name(RegisterIndex8);
const char* register_name(RegisterIndex16);

class InstructionStream {
    public:
        virtual u8 read8() = 0;
        virtual u16 read16() = 0;
};

class InstructionDescriptor {
    public:
        InstructionHandler handler { nullptr };
        InstructionFormat format { OP_ILLEGAL };
        const char* mnemonic { nullptr };

        bool has_imm8() const;
        bool has_imm16() const;
};

class Instruction {
    public:
        static Instruction from_stream(InstructionStream*);

        InstructionHandler handler() const;

        RegisterIndex8 src_reg8() const;
        RegisterIndex8 dst_reg8() const;
        RegisterIndex16 dst_reg16() const;
        InstructionCondition condition() const;

        bool has_sub_op() const { return m_opcode == 0xcb; }
        inline u8 opcode() const { return m_opcode; }
        inline u8 sub_op() const
        {
            assert(has_sub_op());
            return m_sub_op;
        }
        inline u8 imm8() const
        {
            assert(m_descriptor->has_imm8());
            return m_imm8;
        }
        inline u16 imm16() const
        {
            assert(m_descriptor->has_imm16());
            return m_imm16;
        }

        std::string to_string() const;

    private:
        explicit Instruction(InstructionStream*);

        u8 m_opcode = 0;
        u8 m_sub_op = 0;
        u8 m_imm8 = 0;
        u16 m_imm16 = 0;
        const InstructionDescriptor* m_descriptor;
};

class InstructionInterpreter {
    public:
        virtual void illegal_instruction(const Instruction&) = 0;
        virtual void NOP(const Instruction&) = 0;
        virtual void STOP(const Instruction&) = 0;
        virtual void HALT(const Instruction&) = 0;
        virtual void DI(const Instruction&) = 0;
        virtual void EI(const Instruction&) = 0;

        virtual void RST00H(const Instruction&) = 0;
        virtual void RST10H(const Instruction&) = 0;
        virtual void RST20H(const Instruction&) = 0;
        virtual void RST30H(const Instruction&) = 0;
        virtual void RST08H(const Instruction&) = 0;
        virtual void RST18H(const Instruction&) = 0;
        virtual void RST28H(const Instruction&) = 0;
        virtual void RST38H(const Instruction&) = 0;

        virtual void LD_r8_r8(const Instruction&) = 0;
        virtual void LD_r8_imm8(const Instruction&) = 0;
        virtual void LD_r8_iHL(const Instruction&) = 0;
        virtual void LD_r16_imm16(const Instruction&) = 0;
        virtual void LD_ir16_A(const Instruction&) = 0;
        virtual void LD_iimm16_SP(const Instruction&) = 0;
        virtual void LD_HLinc_A(const Instruction&) = 0;
        virtual void LD_HLdec_A(const Instruction&) = 0;
        virtual void LD_iHL_imm8(const Instruction&) = 0;
        virtual void LD_iHL_r8(const Instruction&) = 0;
        virtual void LD_A_ir16(const Instruction&) = 0;
        virtual void LD_A_HLinc(const Instruction&) = 0;
        virtual void LD_A_HLdec(const Instruction&) = 0;
        virtual void LDH_iimm8_A(const Instruction&) = 0;
        virtual void LDH_A_iimm8(const Instruction&) = 0;
        virtual void LDH_iC_A(const Instruction&) = 0;
        virtual void LDH_A_iC(const Instruction&) = 0;
        virtual void LD_HL_SP_imm8(const Instruction&) = 0;
        virtual void LD_SP_HL(const Instruction&) = 0;
        virtual void LD_iimm16_A(const Instruction&) = 0;
        virtual void LD_A_iimm16(const Instruction&) = 0;

        virtual void PUSH_r16(const Instruction&) = 0;
        virtual void POP_r16(const Instruction&) = 0;

        virtual void INC_r16(const Instruction&) = 0;
        virtual void DEC_r16(const Instruction&) = 0;
        virtual void INC_r8(const Instruction&) = 0;
        virtual void DEC_r8(const Instruction&) = 0;
        virtual void INC_iHL(const Instruction&) = 0;
        virtual void DEC_iHL(const Instruction&) = 0;

        virtual void ADD_HL_r16(const Instruction&) = 0;
        virtual void ADD_SP_imm8(const Instruction&) = 0;

        virtual void ADD_r8(const Instruction&) = 0;
        virtual void ADD_imm8(const Instruction&) = 0;
        virtual void ADD_iHL(const Instruction&) = 0;
        virtual void ADC_r8(const Instruction&) = 0;
        virtual void ADC_imm8(const Instruction&) = 0;
        virtual void ADC_iHL(const Instruction&) = 0;
        virtual void SUB_r8(const Instruction&) = 0;
        virtual void SUB_imm8(const Instruction&) = 0;
        virtual void SUB_iHL(const Instruction&) = 0;
        virtual void SBC_r8(const Instruction&) = 0;
        virtual void SBC_imm8(const Instruction&) = 0;
        virtual void SBC_iHL(const Instruction&) = 0;
        virtual void AND_r8(const Instruction&) = 0;
        virtual void AND_imm8(const Instruction&) = 0;
        virtual void AND_iHL(const Instruction&) = 0;
        virtual void XOR_r8(const Instruction&) = 0;
        virtual void XOR_imm8(const Instruction&) = 0;
        virtual void XOR_iHL(const Instruction&) = 0;
        virtual void OR_r8(const Instruction&) = 0;
        virtual void OR_imm8(const Instruction&) = 0;
        virtual void OR_iHL(const Instruction&) = 0;
        virtual void CP_r8(const Instruction&) = 0;
        virtual void CP_imm8(const Instruction&) = 0;
        virtual void CP_iHL(const Instruction&) = 0;

        virtual void RLCA(const Instruction&) = 0;
        virtual void RLA(const Instruction&) = 0;
        virtual void DAA(const Instruction&) = 0;
        virtual void SCF(const Instruction&) = 0;
        virtual void RRCA(const Instruction&) = 0;
        virtual void RRA(const Instruction&) = 0;
        virtual void CPL(const Instruction&) = 0;
        virtual void CCF(const Instruction&) = 0;

        virtual void JP(const Instruction&) = 0;
        virtual void JP_cond(const Instruction&) = 0;
        virtual void JP_iHL(const Instruction&) = 0;
        virtual void JR(const Instruction&) = 0;
        virtual void JR_cond(const Instruction&) = 0;
        virtual void CALL(const Instruction&) = 0;
        virtual void CALL_cond(const Instruction&) = 0;
        virtual void RET(const Instruction&) = 0;
        virtual void RET_cond(const Instruction&) = 0;
        virtual void RETI(const Instruction&) = 0;

        virtual void RLC_r8(const Instruction&) = 0;
        virtual void RLC_iHL(const Instruction&) = 0;
        virtual void RRC_r8(const Instruction&) = 0;
        virtual void RRC_iHL(const Instruction&) = 0;
        virtual void RL_r8(const Instruction&) = 0;
        virtual void RL_iHL(const Instruction&) = 0;
        virtual void RR_r8(const Instruction&) = 0;
        virtual void RR_iHL(const Instruction&) = 0;
        virtual void SLA_r8(const Instruction&) = 0;
        virtual void SLA_iHL(const Instruction&) = 0;
        virtual void SRA_r8(const Instruction&) = 0;
        virtual void SRA_iHL(const Instruction&) = 0;
        virtual void SWAP_r8(const Instruction&) = 0;
        virtual void SWAP_iHL(const Instruction&) = 0;
        virtual void SRL_r8(const Instruction&) = 0;
        virtual void SRL_iHL(const Instruction&) = 0;
        virtual void BIT_r8(const Instruction&) = 0;
        virtual void BIT_iHL(const Instruction&) = 0;
        virtual void RES_r8(const Instruction&) = 0;
        virtual void RES_iHL(const Instruction&) = 0;
        virtual void SET_r8(const Instruction&) = 0;
        virtual void SET_iHL(const Instruction&) = 0;
};

}
