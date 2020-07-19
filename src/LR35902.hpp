#pragma once

#include "Defs.hpp"
#include "Instruction.hpp"

namespace GB {

class Emulator;

class LR35902 final
    : public InstructionStream
    , public InstructionInterpreter {
    public:
        LR35902(Emulator&);
        ~LR35902() {}

        void set_PC(u16 value) { m_PC = value; }
        void set_SP(u16 value) { m_SP = value; }
        u16 PC() const { return m_PC; }
        u16 SP() const { return m_SP; }

        struct Flags {
            enum Flag {
                ZF = 0x80,
                NF = 0x40,
                HF = 0x20,
                CF = 0x10,
            };
        };

        inline u8 reg8(RegisterIndex8 register_index)
        {
            return m_registers[register_index];
        }
        inline void set_reg8(RegisterIndex8 register_index, u8 value)
        {
            m_registers[register_index] = value;
        }
        inline u8 regA() { return reg8(RegisterA); }
        inline u8 regB() { return reg8(RegisterB); }
        inline u8 regC() { return reg8(RegisterC); }
        inline u8 regD() { return reg8(RegisterD); }
        inline u8 regE() { return reg8(RegisterE); }
        inline u8 regF() { return reg8(RegisterF); }
        inline u8 regH() { return reg8(RegisterH); }
        inline u8 regL() { return reg8(RegisterL); }
        inline void setA(u8 value) { set_reg8(RegisterA, value); }
        inline void setB(u8 value) { set_reg8(RegisterB, value); }
        inline void setC(u8 value) { set_reg8(RegisterC, value); }
        inline void setD(u8 value) { set_reg8(RegisterD, value); }
        inline void setE(u8 value) { set_reg8(RegisterE, value); }
        inline void setF(u8 value) { set_reg8(RegisterF, value & 0xf0); }
        inline void setH(u8 value) { set_reg8(RegisterH, value); }
        inline void setL(u8 value) { set_reg8(RegisterL, value); }

        u16 reg16(RegisterIndex16);
        inline u16 regAF() { return reg16(RegisterAF); }
        inline u16 regBC() { return reg16(RegisterBC); }
        inline u16 regDE() { return reg16(RegisterDE); }
        inline u16 regHL() { return reg16(RegisterHL); }
        void set_reg16(RegisterIndex16, u16);
        inline void setAF(u16 value) { set_reg16(RegisterAF, value); }
        inline void setBC(u16 value) { set_reg16(RegisterBC, value); }
        inline void setDE(u16 value) { set_reg16(RegisterDE, value); }
        inline void setHL(u16 value) { set_reg16(RegisterHL, value); }

        inline bool flag(Flags::Flag flag) { return m_registers[RegisterF] & flag; }
        inline bool ZF() { return flag(Flags::ZF); }
        inline bool NF() { return flag(Flags::NF); }
        inline bool HF() { return flag(Flags::HF); }
        inline bool CF() { return flag(Flags::CF); }
        inline void set_flag(Flags::Flag flag, bool value)
        {
            u8 mask = 0xff * value & flag;
            m_registers[RegisterF] &= ~(u8)flag;
            m_registers[RegisterF] |= (u8)(0xff * value) & flag;
        }
        inline void set_ZF(bool value) { set_flag(Flags::ZF, value); }
        inline void set_NF(bool value) { set_flag(Flags::NF, value); }
        inline void set_HF(bool value) { set_flag(Flags::HF, value); }
        inline void set_CF(bool value) { set_flag(Flags::CF, value); }

        u8 pop8();
        u16 pop16();
        void push8(u8);
        void push16(u16);

        bool check_condition(InstructionCondition);
        
        // ^InstructionStream
        virtual u8 read8() override;
        virtual u16 read16() override;

    private:
        void do_cycle();

        void INC(u8&);
        void DEC(u8&);
        void ADC(u8);
        void SBC(u8);
        void XOR(u8);
        void OR(u8);
        void RR(u8&);
        void SRL(u8&);
        void SWAP(u8&);

        // ^InstructionInterpreter
        virtual void illegal_instruction(const Instruction&) override;
        virtual void NOP(const Instruction&) override;
        virtual void STOP(const Instruction&) override;
        virtual void HALT(const Instruction&) override;
        virtual void DI(const Instruction&) override;
        virtual void EI(const Instruction&) override;
        virtual void RST00H(const Instruction&) override;
        virtual void RST10H(const Instruction&) override;
        virtual void RST20H(const Instruction&) override;
        virtual void RST30H(const Instruction&) override;
        virtual void RST08H(const Instruction&) override;
        virtual void RST18H(const Instruction&) override;
        virtual void RST28H(const Instruction&) override;
        virtual void RST38H(const Instruction&) override;
        virtual void LD_r8_r8(const Instruction&) override;
        virtual void LD_r8_imm8(const Instruction&) override;
        virtual void LD_r8_iHL(const Instruction&) override;
        virtual void LD_r16_imm16(const Instruction&) override;
        virtual void LD_ir16_A(const Instruction&) override;
        virtual void LD_iimm16_SP(const Instruction&) override;
        virtual void LD_HLinc_A(const Instruction&) override;
        virtual void LD_HLdec_A(const Instruction&) override;
        virtual void LD_iHL_imm8(const Instruction&) override;
        virtual void LD_iHL_r8(const Instruction&) override;
        virtual void LD_A_ir16(const Instruction&) override;
        virtual void LD_A_HLinc(const Instruction&) override;
        virtual void LD_A_HLdec(const Instruction&) override;
        virtual void LDH_iimm8_A(const Instruction&) override;
        virtual void LDH_A_iimm8(const Instruction&) override;
        virtual void LDH_iC_A(const Instruction&) override;
        virtual void LDH_A_iC(const Instruction&) override;
        virtual void LD_HL_SP_imm8(const Instruction&) override;
        virtual void LD_SP_HL(const Instruction&) override;
        virtual void LD_iimm16_A(const Instruction&) override;
        virtual void LD_A_iimm16(const Instruction&) override;
        virtual void PUSH_r16(const Instruction&) override;
        virtual void POP_r16(const Instruction&) override;
        virtual void INC_r16(const Instruction&) override;
        virtual void DEC_r16(const Instruction&) override;
        virtual void INC_r8(const Instruction&) override;
        virtual void DEC_r8(const Instruction&) override;
        virtual void INC_iHL(const Instruction&) override;
        virtual void DEC_iHL(const Instruction&) override;
        virtual void ADD_HL_r16(const Instruction&) override;
        virtual void ADD_SP_imm8(const Instruction&) override;
        virtual void ADD_r8(const Instruction&) override;
        virtual void ADD_imm8(const Instruction&) override;
        virtual void ADD_iHL(const Instruction&) override;
        virtual void ADC_r8(const Instruction&) override;
        virtual void ADC_imm8(const Instruction&) override;
        virtual void ADC_iHL(const Instruction&) override;
        virtual void SUB_r8(const Instruction&) override;
        virtual void SUB_imm8(const Instruction&) override;
        virtual void SUB_iHL(const Instruction&) override;
        virtual void SBC_r8(const Instruction&) override;
        virtual void SBC_imm8(const Instruction&) override;
        virtual void SBC_iHL(const Instruction&) override;
        virtual void AND_r8(const Instruction&) override;
        virtual void AND_imm8(const Instruction&) override;
        virtual void AND_iHL(const Instruction&) override;
        virtual void XOR_r8(const Instruction&) override;
        virtual void XOR_imm8(const Instruction&) override;
        virtual void XOR_iHL(const Instruction&) override;
        virtual void OR_r8(const Instruction&) override;
        virtual void OR_imm8(const Instruction&) override;
        virtual void OR_iHL(const Instruction&) override;
        virtual void CP_r8(const Instruction&) override;
        virtual void CP_imm8(const Instruction&) override;
        virtual void CP_iHL(const Instruction&) override;
        virtual void RLCA(const Instruction&) override;
        virtual void RLA(const Instruction&) override;
        virtual void DAA(const Instruction&) override;
        virtual void SCF(const Instruction&) override;
        virtual void RRCA(const Instruction&) override;
        virtual void RRA(const Instruction&) override;
        virtual void CPL(const Instruction&) override;
        virtual void CCF(const Instruction&) override;
        virtual void JP(const Instruction&) override;
        virtual void JP_cond(const Instruction&) override;
        virtual void JP_iHL(const Instruction&) override;
        virtual void JR(const Instruction&) override;
        virtual void JR_cond(const Instruction&) override;
        virtual void CALL(const Instruction&) override;
        virtual void CALL_cond(const Instruction&) override;
        virtual void RET(const Instruction&) override;
        virtual void RET_cond(const Instruction&) override;
        virtual void RETI(const Instruction&) override;
        virtual void RLC_r8(const Instruction&) override;
        virtual void RLC_iHL(const Instruction&) override;
        virtual void RRC_r8(const Instruction&) override;
        virtual void RRC_iHL(const Instruction&) override;
        virtual void RL_r8(const Instruction&) override;
        virtual void RL_iHL(const Instruction&) override;
        virtual void RR_r8(const Instruction&) override;
        virtual void RR_iHL(const Instruction&) override;
        virtual void SLA_r8(const Instruction&) override;
        virtual void SLA_iHL(const Instruction&) override;
        virtual void SRA_r8(const Instruction&) override;
        virtual void SRA_iHL(const Instruction&) override;
        virtual void SWAP_r8(const Instruction&) override;
        virtual void SWAP_iHL(const Instruction&) override;
        virtual void SRL_r8(const Instruction&) override;
        virtual void SRL_iHL(const Instruction&) override;
        virtual void BIT_r8(const Instruction&) override;
        virtual void BIT_iHL(const Instruction&) override;
        virtual void RES_r8(const Instruction&) override;
        virtual void RES_iHL(const Instruction&) override;
        virtual void SET_r8(const Instruction&) override;
        virtual void SET_iHL(const Instruction&) override;

    private:
        Emulator& m_emulator;

        u16 m_PC;
        u16 m_SP;
        u8 m_registers[8];
};

}
