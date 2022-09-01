/**
* Filename: ShipDefense.h
* Title: Ship Defense
* Description: Ship Defense Instance Manager
* Author: Owsap
* Date: 2022.08.05
*
* Discord: Owsap#7928
* Skype: owsap.
*
* Web: https://owsap.dev/
* GitHub: https://github.com/Owsap
**/

#if defined(__SHIP_DEFENSE__)
namespace ShipDefense
{
	enum EInstance : bool
	{
		NEED_PARTY = true,
		NEED_TICKET = true,
		SPAWN_WOOD_REPAIR = true,
	};

	enum EProbability : WORD
	{
		MIN_PROB = 1,
		MAX_PROB = 1000,
		WOOD_REPAIR_PCT = 1, // 1%
	};

	enum EVNumHelper : DWORD
	{
		FISHER_MAN = 9009, // 어부

		SHIP_MAST = 20434, // 기둥돛
		SHIP_WHEEL = 20436, // 조타대
		SHIP_BARRIER = 3970, // Invisible Wall

		HYDRA1 = 3960, // 히드라1
		HYDRA2 = 3961, // 히드라2
		HYDRA3 = 3962, // 히드라3

		HYDRA_LEFT = 3963, // 히드라(left)
		HYDRA_RIGHT = 3964, // 히드라(right)

		MINI_HYDRA1 = 3957, // 미니히드라1
		MINI_HYDRA2 = 3958, // 미니히드라2
		MINI_HYDRA3 = 3959, // 미니히드라3

		HYDRA_EGG = 20432, // 히드라알
		HYDRA_REWARD = 3965, // 히드라보상

		PORTAL = 3949, // 포탈

		WOOD_REPAIR = 20437, // 수리패킷
	};

	enum EMapIndex : WORD
	{
		PORT_MAP_INDEX = 359,
		SHIP_MAP_INDEX = 358,
		ENTRY_MAP_INDEX = 301,
	};

	enum EClearType : BYTE
	{
		CLEAR_ALL,
		CLEAR_ALL_EXCEPT_BOSS,
		CLEAR_EGG,
		CLEAR_FAKE_BOSS,
	};

	enum EWaves : BYTE
	{
		WAVE0,
		WAVE1,
		WAVE2,
		WAVE3,
		WAVE4,
		WAVES,
	};

	enum EUniqueCharPos : BYTE
	{
		UNIQUE_MAST_POS,
		UNIQUE_FAKE_HYDRA_POS,
		UNIQUE_HYDRA_POS,
		UNIQUE_MINI_HYDRA1_POS,
		UNIQUE_MINI_HYDRA2_POS,
		UNIQUE_MINI_HYDRA3_POS,
		UNIQUE_MINI_HYDRA4_POS,
		UNIQUE_HYDRA_EGG_POS,
	};

	enum EWaveDuration : DWORD
	{
		NEXT_WAVE_FAST_DELAY = 6,
		NEXT_WAVE_DELAY = 10,
		FIRST_WAVE_DELAY = 60,
		FIRST_WAVE_DURATION = 120,
		EXIT_DELAY = 3,
		JUMP_OUT_DELAY = 300,
		FAKE_SPAWN_DURATION = 3,
		EGG_SPAWN_DURATION = 8,
		EFFECT_SHELTER_DURATION = 10,
		WOOD_REPAIR_STUN_DELAY = 5,
	};

	enum EBarrierWall : BYTE
	{
		BACK_BARRIER_LEFT,
		BACK_BARRIER_RIGHT,
		FRONT_BARRIER_LEFT,
		FRONT_BARRIER_RIGHT,
		BARRIERS,
	};

	enum ENoticeType : BYTE
	{
		NOTICE_WAVE1,
		NOTICE_WAVE2,
		NOTICE_WAVE3,
		NOTICE_WAVE4,
		NOTICE_WAVES,
		NOTICE_MAST_HP,
		NOTICE_MAST_PROTECTED,
		NOTICE_MAST_DESTROYED,
	};

	enum EJumpTo : BYTE
	{
		JUMP_HOME,
		JUMP_PORT,
		JUMP_MAX
	};
};

class CShipDefense
{
public:
	CShipDefense(const long c_lMapIndex, const DWORD c_dwLeaderPID);
	virtual ~CShipDefense();
	virtual void Destroy();

public:
	// Events & Handlers
	void CancelEvents();
	void DeadCharacter(const LPCHARACTER c_lpChar);
	void Notice(const LPCHARACTER c_lpChar, const bool c_bBigFont, const char* c_pszBuf, ...);

	void FindAllyCharacter();
	void ClearMonstersByType(ShipDefense::EClearType eClearType = ShipDefense::CLEAR_ALL);
	void CheckLaserPosition();
	void JumpToPosition(const long c_lMapIndex, const long c_lXPos, const long c_lYPos);
	void NoticeByType(const ShipDefense::ENoticeType c_eType);

	// Spawns
	LPCHARACTER Spawn(DWORD dwVNum, int iX, int iY, int iDir, bool bSpawnMotion = false);
	bool SpawnRegen(const char* c_szFileName, bool bOnce = true);
	void SpawnHydra(const DWORD c_dwVNum);
	void SpawnMiniHydra(const DWORD c_dwVNum, const BYTE c_byCount);
	void SpawnEgg();

	// Barriers
	void SpawnBarriers();
	void RemoveBarriers();
	void RemoveFrontBarriers();
	void RemoveBackBarriers();

	// Deck & Wave Related
	void Start();

	void ClearDeck(ShipDefense::EClearType eClearType = ShipDefense::CLEAR_ALL);
	bool PrepareDeck();

	void PrepareWave(const BYTE c_byWave);
	void SetWave(const BYTE c_byWave);
	void SetShipEvent(const BYTE c_byWave);

	// Jumping
	void JumpToQuarterDeck();
	void JumpAll(ShipDefense::EJumpTo eJumpTo);

public:
	std::time_t GetStartTime() const { return m_lStartTime; }

	DWORD GetLeaderPID() const { return m_dwLeaderPID; }

	BYTE GetState() const { return m_byState; }
	BYTE GetWave() const { return m_byWave; }

	long GetMapIndex() const { return m_lMapIndex; };
	LPSECTREE_MAP GetSectreeMap() const { return m_lpSectreeMap; }

	// Unique Characters
	// NOTE: The map contains each character with their unique position.
	using UNIQUE_CHAR_POSITION = BYTE;
	using UniqueCharacterMap = std::map<UNIQUE_CHAR_POSITION, LPCHARACTER>;
	LPCHARACTER GetUniqueCharacter(const BYTE c_byUniqueID);
	bool IsUniqueMiniHydra(const LPCHARACTER c_lpChar);

	LPCHARACTER GetAllianceCharacter();
	LPCHARACTER GetHydraCharacter();

	// Lasser Effect
	using LaserEffectData = struct SLaserEffectData { long x, y; };
	using LaserEffectDataMap = std::map<BYTE, SLaserEffectData>;
	LaserEffectDataMap GetLaserEffectDataMap() { return m_mapLaserEffectData; }
	void SetLaserEffectData(const BYTE c_byPos, const long c_lXPos, const long c_lYPos);

	void SetLastLaserShelterPulse(UINT iPulse) { m_iLastLaserShelterPulse = iPulse; }
	UINT GetLastLaserShelterPulse() const { return m_iLastLaserShelterPulse; }

private:
	std::time_t m_lStartTime;
	BYTE m_byState, m_byWave;

	long m_lMapIndex;
	DWORD m_dwLeaderPID;

	LPSECTREE_MAP m_lpSectreeMap;
	LPCHARACTER m_lpBarrier[ShipDefense::BARRIERS];

	LPEVENT m_lpShipEvent;
	LPEVENT m_lpWaveEvent;
	LPEVENT m_lpSpawnEvent;
	LPEVENT m_lpLaserEffectEvent;
	LPEVENT m_lpClearSpawnEvent;
	LPEVENT m_lpExitEvent;

protected:
	UniqueCharacterMap m_mapUniqueCharacter;
	LaserEffectDataMap m_mapLaserEffectData;
	UINT m_iLastLaserShelterPulse;
};

class CShipDefenseManager : public singleton<CShipDefenseManager>
{
public:
	enum EStates
	{
		STATE_NONE,
		STATE_CREATE,
		STATE_START,
		STATE_STOP,
	};

public:
	CShipDefenseManager();
	virtual ~CShipDefenseManager();

	void Initialize();
	void Remove(const DWORD c_dwLeaderPID);
	void Destroy();

public:
	// Leader Actions
	bool Create(const LPCHARACTER c_lpChar);
	bool Start(const LPCHARACTER c_lpChar);
	void Stop(const LPCHARACTER c_lpChar);

	// States
	BYTE GetState(const DWORD c_dwLeaderPID) const;
	bool IsCreated(const LPCHARACTER c_lpChar);
	bool IsRunning(const LPCHARACTER c_lpChar);

	// Leader
	DWORD GetLeaderPID(const LPCHARACTER c_lpChar, const bool c_bIsLeader);

	// Alliance
	void BroadcastAllianceHP(const LPCHARACTER c_lpAllianceChar, const LPSECTREE_MAP c_lpSectreeMap);
	void SetAllianceHPPct(const LPCHARACTER c_lpRepairChar, const BYTE c_byPct);

	// Single Player Actions
	bool Join(const LPCHARACTER c_lpChar);
	void Leave(const LPCHARACTER c_lpChar);
	void Land(const LPCHARACTER c_lpChar);

	// Triggers
	bool CanAttack(LPCHARACTER lpCharAttacker, LPCHARACTER lpCharVictim);
	bool OnKill(LPCHARACTER lpDeadChar, LPCHARACTER lpKillerChar = nullptr);

	// VNum Helpers
	bool IsHydra(const DWORD c_dwVNum);
	bool IsFakeHydra(const DWORD c_dwVNum);
	bool IsMinion(const DWORD c_dwVNum);
	bool IsMast(const DWORD c_dwVNum);
	bool IsMiniHydra(const DWORD c_dwVNum);
	bool IsDungeon(const long c_lMapIndex);

private:
	using ShipDefenseMap = std::map<DWORD, CShipDefense*>;
	ShipDefenseMap m_mapShipDefense;
};
#endif
