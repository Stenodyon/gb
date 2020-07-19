#include <cassert>

#include "LR35902.hpp"
#include "Emulator.hpp"

namespace GB {

LR35902::LR35902(Emulator& emulator)
    : m_emulator(emulator)
{
}

void LR35902::illegal_instruction(const Instruction& ins)
{
    std::string instruction_name = ins.to_string();

    fprintf(stderr, "encountered illegal instruction ");
    if (ins.has_sub_op()) {
        fprintf(stderr, "0xcb %#04x", ins.sub_op());
    } else {
        fprintf(stderr, "%#04x", ins.opcode());
    }
    fprintf(stderr, " \"%s\"\n", instruction_name.c_str());

    exit(-1);
}

u8 LR35902::read8()
{
    u8 result = m_emulator.mmu().read8(m_PC);
    m_PC++;
    do_cycle();
    return result;
}

u16 LR35902::read16()
{
    u16 result = (u16)m_emulator.mmu().read8(m_PC);
    do_cycle();
    result |= (u16)m_emulator.mmu().read8(m_PC + 1) << 8;
    m_PC += 2;
    do_cycle();
    return result;
}

u16 LR35902::reg16(RegisterIndex16 register_index)
{
    if (register_index == RegisterSP)
        return m_SP;

    switch (register_index) {
        case RegisterAF:
            return (regA() << 8) | regF();
        case RegisterBC:
            return (regB() << 8) | regC();
        case RegisterDE:
            return (regD() << 8) | regE();
        case RegisterHL:
            return (regH() << 8) | regL();

        default:
            assert(false); // unreachable
    }
}

void LR35902::set_reg16(RegisterIndex16 register_index, u16 value)
{
    if (register_index == RegisterSP) {
        m_SP = value;
        return;
    }

    u8 top = (value & 0xff00) >> 8;
    u8 bottom = value & 0xff;
    switch (register_index) {
        case RegisterAF:
            setA(top);
            setF(bottom);
            break;
        case RegisterBC:
            setB(top);
            setC(bottom);
            break;
        case RegisterDE:
            setD(top);
            setE(bottom);
            break;
        case RegisterHL:
            setH(top);
            setL(bottom);
            break;

        default:
            assert(false); // unreachable
    }
}

u8 LR35902::pop8()
{
    u8 value = m_emulator.mmu().read8(m_SP);
    ++m_SP;
    return value;
}

u16 LR35902::pop16()
{
    u16 value = (u16)m_emulator.mmu().read8(m_SP);
    ++m_SP;
    value |= (u16)m_emulator.mmu().read8(m_SP) << 8;
    ++m_SP;

    return value;
}

void LR35902::push8(u8 value)
{
    --m_SP;
    m_emulator.mmu().write8(m_SP, value);
}

void LR35902::push16(u16 value)
{
    --m_SP;
    m_emulator.mmu().write8(m_SP, value >> 8);
    --m_SP;
    m_emulator.mmu().write8(m_SP, value & 0xff);
}

bool LR35902::check_condition(InstructionCondition condition)
{
    switch (condition) {
        case Z:
            return ZF();
        case NZ:
            return !ZF();
        case C:
            return CF();
        case NC:
            return !CF();

        default:
            assert(false); // unreachable
    }
}

void LR35902::do_cycle()
{
    m_emulator.ppu().cycle();
}

void LR35902::NOP(const Instruction&)
{
}

void LR35902::STOP(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::HALT(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::DI(const Instruction&)
{
    fprintf(stderr, "TODO: DI Disable Interrputs\n");
}

void LR35902::EI(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::RST00H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST10H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST20H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST30H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST08H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST18H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST28H(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RST38H(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::LD_r8_r8(const Instruction& ins)
{
    set_reg8(ins.dst_reg8(), reg8(ins.src_reg8()));
}

void LR35902::LD_r8_imm8(const Instruction& ins)
{
    set_reg8(ins.dst_reg8(), ins.imm8());
}

void LR35902::LD_r8_iHL(const Instruction& ins)
{
    u8 value = m_emulator.mmu().read8(regHL());
    set_reg8(ins.dst_reg8(), value);
}

void LR35902::LD_r16_imm16(const Instruction& ins)
{
    set_reg16(ins.dst_reg16(), ins.imm16());
}

void LR35902::LD_ir16_A(const Instruction& ins)
{
    u16 address = reg16(ins.dst_reg16());
    m_emulator.mmu().write8(address, regA());
}

void LR35902::LD_iimm16_SP(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::LD_HLinc_A(const Instruction&)
{
    m_emulator.mmu().write8(regHL(), regA());
    setHL(regHL() + 1);
    do_cycle();
}

void LR35902::LD_HLdec_A(const Instruction&)
{
    m_emulator.mmu().write8(regHL(), regA());
    setHL(regHL() - 1);
    do_cycle();
}

void LR35902::LD_iHL_imm8(const Instruction& ins)
{
    m_emulator.mmu().write8(regHL(), ins.imm8());
}

void LR35902::LD_iHL_r8(const Instruction& ins)
{
    m_emulator.mmu().write8(regHL(), reg8(ins.src_reg8()));
}

void LR35902::LD_A_ir16(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::LD_A_HLinc(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    setA(value);
    setHL(regHL() + 1);
}

void LR35902::LD_A_HLdec(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::LDH_iimm8_A(const Instruction& ins)
{
    u16 address = 0xff00 | (u16)ins.imm8();
    m_emulator.mmu().write8(address, regA());
}

void LR35902::LDH_A_iimm8(const Instruction& ins)
{
    u16 address = 0xff00 | (u16)ins.imm8();
    setA(m_emulator.mmu().read8(address));
}

void LR35902::LDH_iC_A(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::LDH_A_iC(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::LD_HL_SP_imm8(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::LD_SP_HL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::LD_iimm16_A(const Instruction& ins)
{
    m_emulator.mmu().write8(ins.imm16(), regA());
}

void LR35902::LD_A_iimm16(const Instruction& ins)
{
    setA(m_emulator.mmu().read8(ins.imm16()));
}

void LR35902::PUSH_r16(const Instruction& ins)
{
    push16(reg16(ins.dst_reg16()));
}

void LR35902::POP_r16(const Instruction& ins)
{
    set_reg16(ins.dst_reg16(), pop16());
}

void LR35902::INC(u8& value)
{
    ++value;

    set_ZF(value == 0);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
}

void LR35902::INC_r8(const Instruction& ins)
{
    u8 value = reg8(ins.dst_reg8());
    INC(value);
    set_reg8(ins.dst_reg8(), value);
}

void LR35902::INC_iHL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::INC_r16(const Instruction& ins)
{
    set_reg16(ins.dst_reg16(), reg16(ins.dst_reg16()) + 1);
}

void LR35902::DEC(u8& value)
{
    --value;

    set_ZF(value == 0);
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
}

void LR35902::DEC_r8(const Instruction& ins)
{
    u8 value = reg8(ins.dst_reg8());
    DEC(value);
    set_reg8(ins.dst_reg8(), value);
}

void LR35902::DEC_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    DEC(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::DEC_r16(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::ADD_HL_r16(const Instruction& ins)
{
    u16 prevHL = regHL();
    u16 result = regHL() + reg16(ins.dst_reg16());
    setHL(result);

    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(result < prevHL);
}
void LR35902::ADD_SP_imm8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::ADD_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::ADD_imm8(const Instruction& ins)
{
    u8 prev_A = regA();
    u8 value = regA() + ins.imm8();
    setA(value);

    set_ZF(value == 0);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(value < prev_A);
}

void LR35902::ADD_iHL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::ADC(u8 value)
{
    u8 prev_A = regA();
    u8 carry = CF() ? 1 : 0;
    u8 result = regA() + value + carry;
    setA(result);

    set_ZF(result == 0);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(result < prev_A);
}

void LR35902::ADC_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::ADC_imm8(const Instruction& ins)
{
    ADC(ins.imm8());
}

void LR35902::ADC_iHL(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::SUB_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::SUB_imm8(const Instruction& ins)
{
    u8 prev_A = regA();
    u8 value = regA() - ins.imm8();
    setA(value);

    set_ZF(value == 0);
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(value > prev_A);
}

void LR35902::SUB_iHL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::SBC(u8 value)
{
    u8 prevA = regA();
    u8 carry = CF() ? 1 : 0;
    u8 result = regA() - value - carry;
    setA(result);

    set_ZF(result == 0);
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(result > prevA);
}

void LR35902::SBC_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::SBC_imm8(const Instruction& ins)
{
    SBC(ins.imm8());
}

void LR35902::SBC_iHL(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::AND_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::AND_imm8(const Instruction& ins)
{
    u8 value = regA() & ins.imm8();
    setA(value);

    set_ZF(value == 0);
    set_NF(false);
    set_HF(true);
    set_CF(false);
}

void LR35902::AND_iHL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::XOR(u8 value)
{
    u8 result = regA() ^ value;
    setA(result);

    set_ZF(result == 0);
    set_NF(false);
    set_HF(false);
    set_CF(false);
}

void LR35902::XOR_r8(const Instruction& ins)
{
    XOR(reg8(ins.src_reg8()));
}

void LR35902::XOR_imm8(const Instruction& ins)
{
    XOR(ins.imm8());
}

void LR35902::XOR_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    XOR(value);
}

void LR35902::OR(u8 value)
{
    u8 result = regA() | value;
    setA(result);

    set_ZF(result == 0);
    set_NF(false);
    set_HF(false);
    set_CF(false);
}

void LR35902::OR_r8(const Instruction& ins)
{
    OR(reg8(ins.src_reg8()));
}

void LR35902::OR_imm8(const Instruction& ins)
{
    OR(ins.imm8());
}

void LR35902::OR_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    OR(value);
}

void LR35902::CP_r8(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::CP_imm8(const Instruction& ins)
{
    set_ZF(regA() == ins.imm8());
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(regA() < ins.imm8());
}

void LR35902::CP_iHL(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::RLCA(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RLA(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::DAA(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::SCF(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::RRCA(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::RRA(const Instruction&)
{
    u8 value = regA();
    RR(value);
    setA(value);

    set_ZF(false);
}

void LR35902::CPL(const Instruction&)
{
    assert(false); // TODO
}
void LR35902::CCF(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::JP(const Instruction& ins)
{
    set_PC(ins.imm16());
}
void LR35902::JP_cond(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::JP_iHL(const Instruction&)
{
    set_PC(regHL());
}

void LR35902::JR(const Instruction& ins)
{
    auto imm8 = ins.imm8();
    auto jump = *reinterpret_cast<i8*>(&imm8);
    m_PC += jump;
}

void LR35902::JR_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        auto imm8 = ins.imm8();
        auto jump = *reinterpret_cast<i8*>(&imm8);
        m_PC += jump;
    }
}

void LR35902::CALL(const Instruction& ins)
{
    push16(m_PC);
    m_PC = ins.imm16();
}

void LR35902::CALL_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        push16(m_PC);
        m_PC = ins.imm16();
    }
}

void LR35902::RET(const Instruction&)
{
    m_PC = pop16();
}

void LR35902::RET_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        m_PC = pop16();
    }
}
void LR35902::RETI(const Instruction&)
{
    assert(false); // TODO
}

void LR35902::RLC_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::RLC_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::RRC_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::RRC_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::RL_r8(const Instruction&)    { assert(false); /* TODO */ }
void LR35902::RL_iHL(const Instruction&)   { assert(false); /* TODO */ }

void LR35902::RR(u8& value)
{
    bool carry = (value & 1) > 0;
    u8 top_bit = (CF() ? 1 : 0) * 0x80;
    value = ((value >> 1) & 0x7f) | top_bit;

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::RR_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    RR(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::RR_iHL(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::SLA_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::SLA_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::SRA_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::SRA_iHL(const Instruction&)  { assert(false); /* TODO */ }

void LR35902::SWAP(u8& value)
{
    u8 bottom = (value & 0xf0) >> 4;
    u8 top = (value & 0x0f) << 4;
    value = top | bottom;

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(false);
}

void LR35902::SWAP_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    SWAP(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::SWAP_iHL(const Instruction&) { assert(false); /* TODO */ }

void LR35902::SRL(u8& value)
{
    bool carry = (value & 1) > 0;
    value = (value >> 1) & 0x7f;

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::SRL_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    SRL(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::SRL_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::BIT_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::BIT_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::RES_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::RES_iHL(const Instruction&)  { assert(false); /* TODO */ }
void LR35902::SET_r8(const Instruction&)   { assert(false); /* TODO */ }
void LR35902::SET_iHL(const Instruction&)  { assert(false); /* TODO */ }

} // namespace GB
