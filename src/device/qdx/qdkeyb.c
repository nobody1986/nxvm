/* Copyright 2012-2014 Neko. */

/* QDKEYB implements quick and dirty keyboard control routines. */

#include "../../utils.h"
#include "../vport.h"
#include "../vcpu.h"
#include "../vpic.h"

#include "qdx.h"
#include "qdkeyb.h"

#define bufptrHead (vramRealWord(Zero16, QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD))
#define bufptrTail (vramRealWord(Zero16, QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL))
#define bufGetSize (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END - \
                    QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1)
#define bufIsEmpty (bufptrHead == bufptrTail)
#define bufIsFull  ((bufptrHead - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START) == \
    (bufptrTail - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize)
#define bufptrAdvance(ptr) ((ptr) = (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + \
        ((ptr) - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize))

static t_bool bufPush(t_nubit16 code) {
    if (bufIsFull) {
        return True;
    }
    vramRealWord(Zero16, bufptrTail) = code;
    bufptrAdvance(bufptrTail);
    return False;
}
static t_nubit16 bufPop() {
    t_nubit16 res = 0;
    if (bufIsEmpty) {
        return res;
    }
    res = vramRealWord(Zero16, bufptrHead);
    bufptrAdvance(bufptrHead);
    return res;
}
static t_nubit16 bufPeek() {
    return vramRealWord(Zero16, bufptrHead);
}

static void qdkeybReadInput() {
    /* TODO: this should have been working with INT 15 */
    while (bufIsEmpty) {
        utilsSleep(10);
    }
    vcpu.data.ax = bufPop();
    vpicSetIRQ(0x01);
}
static void qdkeybGetStatus() {
    t_nubit16 x = bufPeek();
    if (bufIsEmpty) {
        _SetEFLAGS_ZF;
    } else {
        switch (x) {
        case 0x1d00:
        case 0x2a00:
        case 0x3800:
            vcpu.data.ax = Zero16;
            break;
        default:
            vcpu.data.ax = x;
            break;
        }
        _ClrEFLAGS_ZF;
    }
}
static void qdkeybGetShiftStatus() {
    vcpu.data.al = qdkeybVarFlag0;
}
static void qdkeybBufferKey() {
    vcpu.data.al = bufPush((vcpu.data.ch << 8) | vcpu.data.cl);
}

static void INT_09() {
    vport.data.ioByte = 0x20;
    vportExecWrite(0x20);
}
static void INT_16() {
    switch (vcpu.data.ah) {
    case 0x00:
    case 0x10:
        qdkeybReadInput();
        break;
    case 0x01:
    case 0x11:
        qdkeybGetStatus();
        break;
    case 0x02:
        qdkeybGetShiftStatus();
        break;
    case 0x05:
        qdkeybBufferKey();
        break;
    default:
        break;
    }
}

void qdkeybInit() {
    qdxTable[0x09] = (t_faddrcc) INT_09; /* hard keyb */
    qdxTable[0x16] = (t_faddrcc) INT_16; /* soft keyb */
}

int deviceConnectKeyboardGetFlag0CapsLock() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
int deviceConnectKeyboardGetFlag0NumLock()  {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
int deviceConnectKeyboardGetFlag0Shift() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT) || GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}
int deviceConnectKeyboardGetFlag0Alt()  {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
int deviceConnectKeyboardGetFlag0Ctrl() {
    return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void deviceConnectKeyboardClrFlag0() {
    qdkeybVarFlag0 = Zero8;
}
void deviceConnectKeyboardClrFlag1() {
    qdkeybVarFlag1 = Zero8;
}

void deviceConnectKeyboardSetFlag0Insert()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);
}
void deviceConnectKeyboardSetFlag0CapLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
void deviceConnectKeyboardSetFlag0NumLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
void deviceConnectKeyboardSetFlag0ScrLck()     {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);
}
void deviceConnectKeyboardSetFlag0Alt()        {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
void deviceConnectKeyboardSetFlag0Ctrl()       {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void deviceConnectKeyboardSetFlag0LeftShift()  {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);
}
void deviceConnectKeyboardSetFlag0RightShift() {
    SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}

void deviceConnectKeyboardClrFlag0Insert()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);
}
void deviceConnectKeyboardClrFlag0CapLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);
}
void deviceConnectKeyboardClrFlag0NumLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);
}
void deviceConnectKeyboardClrFlag0ScrLck()     {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);
}
void deviceConnectKeyboardClrFlag0Alt()        {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);
}
void deviceConnectKeyboardClrFlag0Ctrl()       {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);
}
void deviceConnectKeyboardClrFlag0LeftShift()  {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);
}
void deviceConnectKeyboardClrFlag0RightShift() {
    ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}

void deviceConnectKeyboardSetFlag1Insert()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);
}
void deviceConnectKeyboardSetFlag1CapLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);
}
void deviceConnectKeyboardSetFlag1NumLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);
}
void deviceConnectKeyboardSetFlag1ScrLck()   {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);
}
void deviceConnectKeyboardSetFlag1Pause()    {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);
}
void deviceConnectKeyboardSetFlag1SysRq()    {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);
}
void deviceConnectKeyboardSetFlag1LeftAlt()  {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);
}
void deviceConnectKeyboardSetFlag1LeftCtrl() {
    SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);
}

void deviceConnectKeyboardClrFlag1Insert()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);
}
void deviceConnectKeyboardClrFlag1CapLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);
}
void deviceConnectKeyboardClrFlag1NumLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);
}
void deviceConnectKeyboardClrFlag1ScrLck()   {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);
}
void deviceConnectKeyboardClrFlag1Pause()    {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);
}
void deviceConnectKeyboardClrFlag1SysRq()    {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);
}
void deviceConnectKeyboardClrFlag1LeftAlt()  {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);
}
void deviceConnectKeyboardClrFlag1LeftCtrl() {
    ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);
}

void deviceConnectKeyboardRecvKeyPress(uint16_t code) {
    /* while(bufPush(code)) {
        utilsSleep(1);
    } */
    bufPush(code);
    vpicSetIRQ(0x01);
}
