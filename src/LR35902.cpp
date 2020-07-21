#include <cassert>

#include "LR35902.hpp"
#include "Emulator.hpp"

//#define TRACE

namespace GB {

LR35902::LR35902(Emulator& emulator)
    : m_emulator(emulator)
{
}

void LR35902::cycle()
{
    if (PC() >= 0x8000 && PC() < 0xa000) {
        fprintf(stderr, "trying to execute code from VRAM\n");
        exit(-1);
    }

    if (handle_interrupt())
        return;

    if (halted()) {
        do_cycle();
        return;
    }

#ifdef TRACE
    auto saved_PC = PC();
#endif

    auto ins = Instruction::from_stream(this);

#ifdef TRACE
    if (ins.has_sub_op()) {
        printf(
                "\033[0;36m%#06x\033[0m: \033[0;33m0xcb %#04x\033[0m %s\n",
                saved_PC,
                ins.sub_op(),
                ins.to_string().c_str()
              );
    } else {
        printf(
                "\033[0;36m%#06x\033[0m: \033[0;33m%#04x\033[0m      %s\n",
                saved_PC,
                ins.opcode(),
                ins.to_string().c_str()
              );
    }
#endif
    (this->*ins.handler())(ins);
}

void LR35902::dump_registers() const
{
    printf("A=%02x, F=%02x, B=%02x, C=%02x\n",
            regA(),
            regF(),
            regB(),
            regC()
          );
    printf("D=%02x, E=%02x, H=%02x, L=%02x\n",
            regD(),
            regE(),
            regH(),
            regL()
          );
    printf("PC=%04x, SP=%04x\n", PC(), SP());
}

void LR35902::start_dma(u8 source_sector)
{
    m_doing_dma = true;
    m_dma_source_sector = source_sector;
    m_dma_progress = 0;
}

void LR35902::cycle_dma()
{
    if (m_dma_progress >= 0xa0) {
        m_doing_dma = false;
        return;
    }

    auto low_byte = (u16)(m_dma_progress & 0xff);
    m_emulator.mmu().dma_copy(m_dma_source_sector, low_byte);
    ++m_dma_progress;
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
    u8 result = m_emulator.mmu().read8(PC());
    m_PC++;
    do_cycle();
    return result;
}

u16 LR35902::read16()
{
    u16 result = (u16)m_emulator.mmu().read8(PC());
    do_cycle();
    result |= (u16)m_emulator.mmu().read8(PC() + 1) << 8;
    m_PC += 2;
    do_cycle();
    return result;
}

u16 LR35902::reg16(RegisterIndex16 register_index)
{
    if (register_index == RegisterSP)
        return SP();

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
        setSP(value);
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
    u8 value = m_emulator.mmu().read8(SP());
    setSP(SP() + 1);
    return value;
}

u16 LR35902::pop16()
{
    u16 value = (u16)m_emulator.mmu().read8(SP());
    setSP(SP() + 1);
    value |= (u16)m_emulator.mmu().read8(SP()) << 8;
    setSP(SP() + 1);

    return value;
}

void LR35902::push8(u8 value)
{
    setSP(SP() - 1);
    m_emulator.mmu().write8(SP(), value);
}

void LR35902::push16(u16 value)
{
    setSP(SP() - 1);
    m_emulator.mmu().write8(SP(), value >> 8);
    setSP(SP() - 1);
    m_emulator.mmu().write8(SP(), value & 0xff);
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
    if (doing_dma())
        cycle_dma();

    m_emulator.ppu().cycle();
    m_emulator.joypad().cycle();
    m_emulator.timer().cycle();
    m_emulator.apu().cycle();
}

bool LR35902::handle_interrupt()
{
    if (!m_interrupts_enabled)
        return false;

    u8 triggerable_interrupts = m_interrupt_enable_reg & m_interrupt_flag_reg;

    if ((triggerable_interrupts & 0x01) > 0) { // VBLANK
        m_halted = false;
        m_interrupts_enabled = false;
        m_interrupt_flag_reg &= ~0x01;
        push16(PC());
        setPC(0x0040);

        return true;
    }

    if ((triggerable_interrupts & 0x02) > 0) { // LCD STAT
        m_halted = false;
        m_interrupts_enabled = false;
        m_interrupt_flag_reg &= ~0x02;
        push16(PC());
        setPC(0x0048);

        return true;
    }

    if ((triggerable_interrupts & 0x04) > 0) { // Timer
        m_halted = false;
        m_interrupts_enabled = false;
        m_interrupt_flag_reg &= ~0x04;
        push16(PC());
        setPC(0x0050);

        return true;
    }

    if ((triggerable_interrupts & 0x08) > 0) { // Serial
        m_halted = false;
        m_interrupts_enabled = false;
        m_interrupt_flag_reg &= ~0x08;
        push16(PC());
        setPC(0x0058);

        return true;
    }

    if ((triggerable_interrupts & 0x10) > 0) { // Joypad
        m_halted = false;
        m_interrupts_enabled = false;
        m_interrupt_flag_reg &= ~0x10;
        push16(PC());
        setPC(0x0060);

        return true;
    }

    return false;
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
    m_halted = true;
}

void LR35902::DI(const Instruction&)
{
    m_interrupts_enabled = false;
}

void LR35902::EI(const Instruction&)
{
    m_interrupts_enabled = true;
}

void LR35902::RST00H(const Instruction&)
{
    push16(PC());
    setPC(0x0000);
}
void LR35902::RST10H(const Instruction&)
{
    push16(PC());
    setPC(0x0010);
}

void LR35902::RST20H(const Instruction&)
{
    push16(PC());
    setPC(0x0020);
}

void LR35902::RST30H(const Instruction&)
{
    push16(PC());
    setPC(0x0030);
}

void LR35902::RST08H(const Instruction&)
{
    push16(PC());
    setPC(0x0008);
}

void LR35902::RST18H(const Instruction&)
{
    push16(PC());
    setPC(0x0018);
}

void LR35902::RST28H(const Instruction&)
{
    push16(PC());
    setPC(0x0028);
}

void LR35902::RST38H(const Instruction&)
{
    push16(PC());
    setPC(0x0038);
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

void LR35902::LD_iimm16_SP(const Instruction& ins)
{
    u16 address = ins.imm16();
    m_emulator.mmu().write8(address, SP() & 0xff);
    m_emulator.mmu().write8(address + 1, SP() >> 8);
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

void LR35902::LD_A_ir16(const Instruction& ins)
{
    u16 address = reg16(ins.dst_reg16());
    u8 value = m_emulator.mmu().read8(address);
    setA(value);
}

void LR35902::LD_A_HLinc(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    setA(value);
    setHL(regHL() + 1);
}

void LR35902::LD_A_HLdec(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    setA(value);
    setHL(regHL() - 1);
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
    u16 address = 0xff00 | (u16)regC();
    m_emulator.mmu().write8(address, regA());
}

void LR35902::LDH_A_iC(const Instruction&)
{
    u16 address = 0xff00 | (u16)regC();
    u8 value = m_emulator.mmu().read8(address);
    setA(value);
}

void LR35902::LD_HL_SP_imm8(const Instruction& ins)
{
    u8 imm8 = ins.imm8();
    i8 as_signed = *reinterpret_cast<i8*>(&imm8);
    u16 value = SP() + as_signed;
    setHL(value);

    set_ZF(false);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    // FIXME: compute carry flag
    set_CF(false);
}

void LR35902::LD_SP_HL(const Instruction&)
{
    setSP(regHL());
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
    u8 value = m_emulator.mmu().read8(regHL());
    INC(value);
    m_emulator.mmu().write8(regHL(), value);
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

void LR35902::DEC_r16(const Instruction& ins)
{
    u16 value = reg16(ins.dst_reg16()) - 1;
    set_reg16(ins.dst_reg16(), value);
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

void LR35902::ADD_SP_imm8(const Instruction& ins)
{
    auto imm8 = ins.imm8();
    auto signed_value = *reinterpret_cast<i8*>(&imm8);
    setSP(SP() + signed_value);

    set_ZF(false);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    // FIXME: compute carry flag
    set_CF(false);
}

void LR35902::ADD(u8 value)
{
    u8 prev_A = regA();
    u8 result = regA() + value;
    setA(result);

    set_ZF(result == 0);
    set_NF(false);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(result < prev_A);
}

void LR35902::ADD_r8(const Instruction& ins)
{
    ADD(reg8(ins.src_reg8()));
}

void LR35902::ADD_imm8(const Instruction& ins)
{
    ADD(ins.imm8());
}

void LR35902::ADD_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    ADD(value);
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

void LR35902::ADC_r8(const Instruction& ins)
{
    ADC(reg8(ins.src_reg8()));
}

void LR35902::ADC_imm8(const Instruction& ins)
{
    ADC(ins.imm8());
}

void LR35902::ADC_iHL(const Instruction&)
{
    ADC(m_emulator.mmu().read8(regHL()));
}

void LR35902::SUB(u8 value)
{
    u8 prevA = regA();
    u8 result = regA() - value;
    setA(result);

    set_ZF(result == 0);
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(value > prevA);
}

void LR35902::SUB_r8(const Instruction& ins)
{
    SUB(reg8(ins.src_reg8()));
}

void LR35902::SUB_imm8(const Instruction& ins)
{
    SUB(ins.imm8());
}

void LR35902::SUB_iHL(const Instruction&)
{
    SUB(m_emulator.mmu().read8(regHL()));
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
    set_CF((value + carry) > prevA);
}

void LR35902::SBC_r8(const Instruction& ins)
{
    SBC(reg8(ins.src_reg8()));
}

void LR35902::SBC_imm8(const Instruction& ins)
{
    SBC(ins.imm8());
}

void LR35902::SBC_iHL(const Instruction&)
{
    SBC(m_emulator.mmu().read8(regHL()));
}

void LR35902::AND(u8 value)
{
    u8 result = regA() & value;
    setA(result);

    set_ZF(result == 0);
    set_NF(false);
    set_HF(true);
    set_CF(false);
}

void LR35902::AND_r8(const Instruction& ins)
{
    AND(reg8(ins.src_reg8()));
}

void LR35902::AND_imm8(const Instruction& ins)
{
    AND(ins.imm8());
}

void LR35902::AND_iHL(const Instruction&)
{
    AND(m_emulator.mmu().read8(regHL()));
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

void LR35902::CP(u8 value)
{
    set_ZF(regA() == value);
    set_NF(true);
    // FIXME: proper half carry flag
    set_HF(false);
    set_CF(regA() < value);
}

void LR35902::CP_r8(const Instruction& ins)
{
    CP(reg8(ins.src_reg8()));
}

void LR35902::CP_imm8(const Instruction& ins)
{
    CP(ins.imm8());
}

void LR35902::CP_iHL(const Instruction&)
{
    CP(m_emulator.mmu().read8(regHL()));
}

void LR35902::RLCA(const Instruction&)
{
    u8 value = regA();
    RLC(value);
    setA(value);

    set_ZF(false);
}

void LR35902::RLA(const Instruction&)
{
    u8 value = regA();
    RL(value);
    setA(value);

    set_ZF(false);
}

void LR35902::DAA(const Instruction&)
{
    if (!NF()) {
        if (CF() || regA() > 0x99) {
            setA(regA() + 0x60);
            set_CF(true);
        }
        if (HF() || (regA() & 0x0f) > 0x09) {
            setA(regA() + 0x06);
        }
    } else {
        if (CF()) {
            setA(regA() - 0x60);
        }
        if (HF()) {
            setA(regA() - 0x06);
        }
    }

    set_ZF(regA() == 0);
    set_HF(false);
}

void LR35902::SCF(const Instruction&)
{
    set_NF(false);
    set_HF(false);
    set_CF(true);
}

void LR35902::RRCA(const Instruction&)
{
    u8 value = regA();
    RRC(value);
    setA(value);

    set_ZF(false);
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
    setA(~regA());

    set_NF(true);
    set_HF(true);
}

void LR35902::CCF(const Instruction&)
{
    set_NF(false);
    set_HF(false);
    set_CF(!CF());
}

void LR35902::JP(const Instruction& ins)
{
    setPC(ins.imm16());
}

void LR35902::JP_cond(const Instruction& ins)
{
    if (check_condition(ins.condition()))
        setPC(ins.imm16());
}

void LR35902::JP_iHL(const Instruction&)
{
    setPC(regHL());
}

void LR35902::JR(const Instruction& ins)
{
    auto imm8 = ins.imm8();
    auto jump = *reinterpret_cast<i8*>(&imm8);
    m_PC += jump;
    do_cycle();

    if (imm8 == 0xfe) { // infinite loop
        m_halted = true;
    }
}

void LR35902::JR_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        auto imm8 = ins.imm8();
        auto jump = *reinterpret_cast<i8*>(&imm8);
        m_PC += jump;
        do_cycle();
    }
}

void LR35902::CALL(const Instruction& ins)
{
    push16(PC());
    setPC(ins.imm16());
}

void LR35902::CALL_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        push16(PC());
        setPC(ins.imm16());
    }
}

void LR35902::RET(const Instruction&)
{
    setPC(pop16());
}

void LR35902::RET_cond(const Instruction& ins)
{
    if (check_condition(ins.condition())) {
        setPC(pop16());
    }
}

void LR35902::RETI(const Instruction&)
{
    m_interrupts_enabled = true;
    setPC(pop16());
}

void LR35902::RLC(u8& value)
{
    bool carry = (value & 0x80) > 0;
    value = (value << 1) | (carry ? 1 : 0);

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::RLC_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    RLC(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::RLC_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    RLC(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::RRC(u8& value)
{
    bool carry = (value & 1) > 0;
    value = ((value >> 1) & 0x7f) | (carry ? 0x80 : 0);

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::RRC_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    RRC(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::RRC_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    RRC(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::RL(u8& value)
{
    bool carry = (value & 0x80) != 0;
    value = (value << 1) | (CF() ? 1 : 0);

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::RL_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    RL(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::RL_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    RL(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::RR(u8& value)
{
    bool carry = (value & 1) != 0;
    value = ((value >> 1) & 0x7f) | (CF() ? 0x80 : 0);

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

void LR35902::RR_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    RL(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::SLA(u8& value)
{
    bool carry = (value & 0x80) != 0;
    value <<= 1;

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::SLA_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    SLA(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::SLA_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    SLA(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::SRA(u8& value)
{
    u8 sign = value & 0x80;
    bool carry = (value & 1) != 0;
    value = (value >> 1) | sign;

    set_ZF(value == 0);
    set_NF(false);
    set_HF(false);
    set_CF(carry);
}

void LR35902::SRA_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    SRA(value);
    set_reg8(ins.src_reg8(), value);
}

void LR35902::SRA_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    SRA(value);
    m_emulator.mmu().write8(regHL(), value);
}

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

void LR35902::SWAP_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    SWAP(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::SRL(u8& value)
{
    bool carry = (value & 1) != 0;
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

void LR35902::SRL_iHL(const Instruction&)
{
    u8 value = m_emulator.mmu().read8(regHL());
    SRL(value);
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::BIT(u8 value, u8 bit)
{
    bool result = (value & (1 << bit)) == 0;

    set_ZF(result);
    set_NF(false);
    set_HF(true);
}

void LR35902::BIT_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    BIT(value, ins.special_bit());
    set_reg8(ins.src_reg8(), value);
}

void LR35902::BIT_iHL(const Instruction& ins)
{
    u8 value = m_emulator.mmu().read8(regHL());
    BIT(value, ins.special_bit());
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::RES(u8& value, u8 bit)
{
    u8 mask = ~(1 << bit);
    value &= mask;
}

void LR35902::RES_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    RES(value, ins.special_bit());
    set_reg8(ins.src_reg8(), value);
}

void LR35902::RES_iHL(const Instruction& ins)
{
    u8 value = m_emulator.mmu().read8(regHL());
    RES(value, ins.special_bit());
    m_emulator.mmu().write8(regHL(), value);
}

void LR35902::SET(u8& value, u8 bit)
{
    value |= (1 << bit);
}

void LR35902::SET_r8(const Instruction& ins)
{
    u8 value = reg8(ins.src_reg8());
    SET(value, ins.special_bit());
    set_reg8(ins.src_reg8(), value);
}

void LR35902::SET_iHL(const Instruction& ins)
{
    u8 value = m_emulator.mmu().read8(regHL());
    SET(value, ins.special_bit());
    m_emulator.mmu().write8(regHL(), value);
}

} // namespace GB
