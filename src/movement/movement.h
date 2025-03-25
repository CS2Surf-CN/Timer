#pragma once

#include <pch.h>
#include <sdk/datatypes.h>
#include <core/playermanager.h>

struct SubtickMove {
	float when;
	uint64 button;

	union {
		bool pressed;

		struct {
			float analog_forward_delta;
			float analog_left_delta;
		} analogMove;
	};

	bool IsAnalogInput() {
		return button == 0;
	}
};

class CMoveDataBase {
public:
	CMoveDataBase() = default;

	CMoveDataBase(const CMoveDataBase& source)
		// clang-format off
		: moveDataFlags {source.moveDataFlags}, 
		m_nPlayerHandle {source.m_nPlayerHandle},
		m_vecAbsViewAngles {source.m_vecAbsViewAngles},
		m_vecViewAngles {source.m_vecViewAngles},
		m_vecLastMovementImpulses {source.m_vecLastMovementImpulses},
		m_flForwardMove {source.m_flForwardMove}, 
		m_flSideMove {source.m_flSideMove}, 
		m_flUpMove {source.m_flUpMove},
		m_vecVelocity {source.m_vecVelocity}, 
		m_vecAngles {source.m_vecAngles},
		m_bHasSubtickInputs {source.m_bHasSubtickInputs},
		m_collisionNormal {source.m_collisionNormal},
		m_groundNormal {source.m_groundNormal}, 
		m_vecAbsOrigin {source.m_vecAbsOrigin},
		m_nTickCount {source.m_nTickCount},
		m_nTargetTick {source.m_nTargetTick},
		m_flSubtickEndFraction {source.m_flSubtickEndFraction},
		m_flSubtickStartFraction {source.m_flSubtickStartFraction} // clang-format on
	{
		for (int i = 0; i < source.m_AttackSubtickMoves.Count(); i++) {
			this->m_AttackSubtickMoves.AddToTail(source.m_AttackSubtickMoves[i]);
		}
		for (int i = 0; i < source.m_SubtickMoves.Count(); i++) {
			this->m_SubtickMoves.AddToTail(source.m_SubtickMoves[i]);
		}
		for (int i = 0; i < source.m_TouchList.Count(); i++) {
			auto touch = this->m_TouchList.AddToTailGetPtr();
			touch->deltavelocity = m_TouchList[i].deltavelocity;
			touch->trace.m_pSurfaceProperties = m_TouchList[i].trace.m_pSurfaceProperties;
			touch->trace.m_pEnt = m_TouchList[i].trace.m_pEnt;
			touch->trace.m_pHitbox = m_TouchList[i].trace.m_pHitbox;
			touch->trace.m_hBody = m_TouchList[i].trace.m_hBody;
			touch->trace.m_hShape = m_TouchList[i].trace.m_hShape;
			touch->trace.m_nContents = m_TouchList[i].trace.m_nContents;
			touch->trace.m_BodyTransform = m_TouchList[i].trace.m_BodyTransform;
			touch->trace.m_vHitNormal = m_TouchList[i].trace.m_vHitNormal;
			touch->trace.m_vHitPoint = m_TouchList[i].trace.m_vHitPoint;
			touch->trace.m_flHitOffset = m_TouchList[i].trace.m_flHitOffset;
			touch->trace.m_flFraction = m_TouchList[i].trace.m_flFraction;
			touch->trace.m_nTriangle = m_TouchList[i].trace.m_nTriangle;
			touch->trace.m_nHitboxBoneIndex = m_TouchList[i].trace.m_nHitboxBoneIndex;
			touch->trace.m_eRayType = m_TouchList[i].trace.m_eRayType;
			touch->trace.m_bStartInSolid = m_TouchList[i].trace.m_bStartInSolid;
			touch->trace.m_bExactHitPoint = m_TouchList[i].trace.m_bExactHitPoint;
		}
	}

public:
	uint8_t moveDataFlags;
	CHandle<CCSPlayerPawn> m_nPlayerHandle;
	QAngle m_vecAbsViewAngles;
	QAngle m_vecViewAngles;
	Vector m_vecLastMovementImpulses;
	float m_flForwardMove;
	float m_flSideMove; // Warning! Flipped compared to CS:GO, moving right gives negative value
	float m_flUpMove;
	Vector m_vecVelocity;
	Vector m_vecAngles;
	CUtlVector<SubtickMove> m_SubtickMoves;
	CUtlVector<SubtickMove> m_AttackSubtickMoves;
	bool m_bHasSubtickInputs;
	float unknown; // Set to 1.0 during SetupMove, never change during gameplay.
	CUtlVector<touchlist_t> m_TouchList;
	Vector m_collisionNormal;
	Vector m_groundNormal; // unsure
	Vector m_vecAbsOrigin;
	int32_t m_nTickCount;
	int32_t m_nTargetTick;
	float m_flSubtickEndFraction;
	float m_flSubtickStartFraction;
};

class CMoveData : public CMoveDataBase {
public:
	Vector m_outWishVel;
	Vector m_vecOldAngles;
	float m_flMaxSpeed;
	float m_flClientMaxSpeed;
	float m_flFrictionDecel;     // Related to ground acceleration subtick stuff with sv_stopspeed and friction
	bool m_bJumpedThisTick;      // something to do with basevelocity and the tick the player jumps
	bool m_bGameCodeMovedPlayer; // true if usercmd cmd number == (m_nGameCodeHasMovedPlayerAfterCommand + 1)
};

static_assert(sizeof(CMoveData) == 256, "Class didn't match expected size");

class CMovementPlayer : public CPlayer {
public:
	using CPlayer::CPlayer;

public:
	virtual void Reset() override;

public:
	virtual CCSPlayer_MovementServices* GetMoveServices();
	virtual void GetBBoxBounds(bbox_t& bounds);

	virtual void RegisterTakeoff(bool jumped);
	virtual void RegisterLanding(const Vector& landingVelocity, bool distbugFix = true);
	virtual void GetOrigin(Vector& origin);
	virtual void SetOrigin(const Vector& origin);
	virtual void GetVelocity(Vector& velocity);
	virtual void SetVelocity(const Vector& velocity);
	virtual void GetAngles(QAngle& angles);
	virtual void SetAngles(const QAngle& angles);

public:
	// General
	bool processingMovement {};
	CMoveData* currentMoveData {};
	CMoveData moveDataPre;
	CMoveData moveDataPost;
	Vector landingVelocity;
	bool jumped {};

	float m_fCurrentMaxSpeed = 260.0f;
};

class CMovementPlayerManager : public CPlayerManager {
public:
	CMovementPlayerManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CMovementPlayer>(i);
		}
	}

	virtual CMovementPlayer* ToPlayer(CServerSideClientBase* pClient) const override;
	virtual CMovementPlayer* ToPlayer(CPlayerPawnComponent* component) const override;
	virtual CMovementPlayer* ToPlayer(CBasePlayerController* controller) const override;
	virtual CMovementPlayer* ToPlayer(CBasePlayerPawn* pawn) const override;
	virtual CMovementPlayer* ToPlayer(CPlayerSlot slot) const override;
	virtual CMovementPlayer* ToPlayer(CEntityIndex entIndex) const override;
	virtual CMovementPlayer* ToPlayer(CPlayerUserId userID) const override;
	virtual CMovementPlayer* ToPlayer(CSteamID steamid, bool validate = false) const override;

	// Dont pass by global playermanager
	CMovementPlayer* ToMovementPlayer(CPlayer* player) {
		return static_cast<CMovementPlayer*>(player);
	}
};

class CMovementForward : public CBaseForward<CMovementForward> {
public:
	virtual bool OnPlayerRunCmd(CCSPlayerPawn* pPawn, CInButtonState& buttons, float (&vec)[3], QAngle& viewAngles, int& weapon, int& cmdnum, int& tickcount, int& seed, int (&mouse)[2]) {
		return true;
	}

	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {}

	virtual bool OnTryPlayerMove(CCSPlayer_MovementServices* ms, CMoveData* mv, Vector* pFirstDest, trace_t* pFirstTrace) {
		return true;
	}

	virtual void OnTryPlayerMovePost(CCSPlayer_MovementServices* ms, const CMoveData* mv, const Vector* pFirstDest, const trace_t* pFirstTrace) {}

	virtual bool OnCategorizePosition(CCSPlayer_MovementServices* ms, CMoveData* mv, bool bStayOnGround) {
		return true;
	}

	virtual void OnCategorizePositionPost(CCSPlayer_MovementServices* ms, const CMoveData* mv, const bool bStayOnGround) {}

	virtual bool OnJump(CCSPlayer_MovementServices* ms, CMoveData* mv) {
		return true;
	}

	virtual void OnJumpPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) {}

	virtual bool OnProcessMovement(CCSPlayer_MovementServices* ms, CMoveData* mv) {
		return true;
	}

	virtual void OnProcessMovementPost(CCSPlayer_MovementServices* ms, const CMoveData* mv) {}

	virtual bool OnPhysicsSimulate(CCSPlayerController* pController) {
		return true;
	}

	virtual void OnPhysicsSimulatePost(CCSPlayerController* pController) {}

public:
	virtual void OnStartTouchGround(CMovementPlayer* pPlayer) {}

	virtual void OnStopTouchGround(CMovementPlayer* pPlayer) {}
};

namespace MOVEMENT {
	extern CMovementPlayerManager* GetPlayerManager();

	namespace TRAMPOLINE {
		inline void* g_fnMovementServicesRunCmds;
		inline void* g_fnTryPlayerMove;
		inline void* g_fnCategorizePosition;
		inline void* g_fnJump;
		inline void* g_fnProcessMovement;
		inline void* g_fnPhysicsSimulate;
		inline void* g_fnGetMaxSpeed;
	} // namespace TRAMPOLINE

	void SetupHooks();

	void ClipVelocity(Vector& in, Vector& normal, Vector& out);
	bool IsValidMovementTrace(trace_t& tr, bbox_t& bounds, CTraceFilterPlayerMovementCS* filter);
} // namespace MOVEMENT
