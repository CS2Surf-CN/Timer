#pragma once

#include "common.h"
#include <sdk/protobuf/cs_usercmd.pb.h>

enum InputBitMask_t : uint64_t {
	IN_NONE = 0x0,
	IN_ALL = 0xffffffffffffffff,
	IN_ATTACK = 0x1,
	IN_JUMP = 0x2,
	IN_DUCK = 0x4,
	IN_FORWARD = 0x8,
	IN_BACK = 0x10,
	IN_USE = 0x20,
	IN_TURNLEFT = 0x80,
	IN_TURNRIGHT = 0x100,
	IN_MOVELEFT = 0x200,
	IN_MOVERIGHT = 0x400,
	IN_ATTACK2 = 0x800,
	IN_RELOAD = 0x2000,
	IN_SPEED = 0x10000,
	IN_JOYAUTOSPRINT = 0x20000,
	IN_FIRST_MOD_SPECIFIC_BIT = 0x100000000,
	IN_USEORRELOAD = 0x100000000,
	IN_SCORE = 0x200000000,
	IN_ZOOM = 0x400000000,
	IN_LOOK_AT_WEAPON = 0x800000000,
};

class CSGOUserCmdPB;

class CInButton {
	void** vtable;
	[[maybe_unused]] uint8_t unk[16];

public:
	uint64_t pressing;
	uint64_t changed;
	uint64_t scroll;

public:
	bool Released(InputBitMask_t mask) const {
		return !(pressing & mask) && (changed & mask);
	}

	bool Pressed(InputBitMask_t mask) const {
		return (pressing & mask) && (changed & mask);
	}
};

static_assert(sizeof(CInButton) == sizeof(CInButtonStatePB));

class CUserCmdBase {
public:
	int cmdNum;
	uint8_t unk[4];

	virtual ~CUserCmdBase();

private:
	virtual void unk0();
	virtual void unk1();
	virtual void unk2();
	virtual void unk3();
	virtual void unk4();
	virtual void unk5();
	virtual void unk6();
};

template<typename T>
class CUserCmdBaseHost : public CUserCmdBase, public T {};

class CUserCmd : public CUserCmdBaseHost<CSGOUserCmdPB> {};

class PlayerCommand : public CUserCmd {
public:
	CInButton m_Buttons;

	// Not part of the player message
	uint32_t unknown[4];
	PlayerCommand* unknowncmd;
	PlayerCommand* unknowncmd2;
};
