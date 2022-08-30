/**
* Filename: ShipDefense.cpp
* Title: Ship Defense
* Description: Ship Defense Instance Manager
* Author: Owsap
* Date: 2022.08.05
*
* Discord: Owsap#7928
* Skype: owsap.
*
* 0x0
*
* Web: https://owsap.dev/
* GitHub: https://github.com/Owsap
**/

#include "stdafx.h"

#if defined(__SHIP_DEFENSE__)
#include "ShipDefense.h"

#include "sectree_manager.h"
#include "char_manager.h"
#include "char.h"
#include "desc.h"
#include "party.h"
#include "mob_manager.h"
#include "regen.h"
#include "config.h"

using namespace ShipDefense;

typedef struct SBroadcastAllianceHP
{
	SBroadcastAllianceHP(const LPCHARACTER c_lpAllianceChar) : m_lpAllianceChar(c_lpAllianceChar) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if ((lpChar->IsPC() == true) && (lpChar->GetDesc()))
			{
				TPacketGCTarget Packet = { 0 };
				Packet.header = HEADER_GC_TARGET;
				Packet.dwVID = m_lpAllianceChar ? m_lpAllianceChar->GetVID() : 0;
				Packet.iMinHP = m_lpAllianceChar ? m_lpAllianceChar->GetHP() : 0;
				Packet.iMaxHP = m_lpAllianceChar ? m_lpAllianceChar->GetMaxHP() : 0;
				Packet.bAlliance = true;
				lpChar->GetDesc()->Packet(&Packet, sizeof(TPacketGCTarget));
			}
		}
	}
	LPCHARACTER m_lpAllianceChar;
} TBroadcastAllianceHP;

typedef struct SExitNotice
{
	SExitNotice(const WORD c_wSec) : m_wSec(c_wSec) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsPC() == true)
				lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("Hurry! You have %d minutes to travel through the portal to reach the mainland."), (m_wSec % 3600) / 60);
		}
	}
	WORD m_wSec;
} TExitNotice;

typedef struct SFindAllianceCharacter
{
	SFindAllianceCharacter(const LPCHARACTER c_lpAllianceChar) : m_lpAllianceChar(c_lpAllianceChar) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsMonster() == true)
			{
				if (m_lpAllianceChar && !m_lpAllianceChar->IsDead())
					lpChar->SetVictim(m_lpAllianceChar);
			}
		}
	}
	LPCHARACTER m_lpAllianceChar;
} TFindAllianceCharacter;

typedef struct SClearMonsters
{
	SClearMonsters(const EClearType c_eType) : m_eType(c_eType) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsMonster() == true || lpChar->IsStone() == true)
			{
				switch (m_eType)
				{
				case EClearType::CLEAR_ALL:
					lpChar->Dead();
					break;

				case EClearType::CLEAR_ALL_EXCEPT_BOSS:
					if (!CShipDefenseManager::Instance().IsHydra(lpChar->GetRaceNum()) &&
						!CShipDefenseManager::Instance().IsFakeHydra(lpChar->GetRaceNum()))
						lpChar->Dead();
					break;

				case EClearType::CLEAR_EGG:
					if (lpChar->GetRaceNum() == ShipDefense::EVNumHelper::HYDRA_EGG)
						CHARACTER_MANAGER::instance().DestroyCharacter(lpChar);
					break;

				case EClearType::CLEAR_FAKE_BOSS:
					if (lpChar->GetRaceNum() == ShipDefense::EVNumHelper::HYDRA_LEFT)
						CHARACTER_MANAGER::instance().DestroyCharacter(lpChar);
					break;
				}
			}
		}
	}
	EClearType m_eType;
} TClearMonsters;

typedef struct SCheckLaserPosition
{
	SCheckLaserPosition(CShipDefense* pShipDefense) : m_pShipDefense(pShipDefense) {}
	void operator() (LPENTITY lpEntity)
	{
		if (m_pShipDefense == nullptr)
			return;

		const LPSECTREE_MAP c_lpSectreeMap = m_pShipDefense->GetSectreeMap();
		if (c_lpSectreeMap == nullptr)
			return;

		CShipDefense::LaserEffectDataMap LaserEffectDataMap = m_pShipDefense->GetLaserEffectDataMap();
		if (LaserEffectDataMap.empty())
			return;

		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsPC() == true)
			{
				for (BYTE byUniquePos = ShipDefense::UNIQUE_MINI_HYDRA1_POS; byUniquePos <= ShipDefense::UNIQUE_MINI_HYDRA4_POS; ++byUniquePos)
				{
					const LPCHARACTER c_lpUniqueChar = m_pShipDefense->GetUniqueCharacter(byUniquePos);
					if (c_lpUniqueChar == nullptr)
						continue;

					CShipDefense::LaserEffectDataMap::iterator it = LaserEffectDataMap.find(byUniquePos);
					if (it != LaserEffectDataMap.end())
					{
						long lXCharPos = (lpChar->GetX() - c_lpSectreeMap->m_setting.iBaseX) / 100;
						long lYCharPos = (lpChar->GetY() - c_lpSectreeMap->m_setting.iBaseY) / 100;

						if (lXCharPos == it->second.x && lYCharPos == it->second.y)
						{
							if (c_lpUniqueChar->FindAffect(AFFECT_DEFENSEWAVE_LASER))
							{
								if (thecore_pulse() - m_pShipDefense->GetLastLaserShelterPulse() < PASSES_PER_SEC(ShipDefense::EFFECT_SHELTER_DURATION))
								{
									if (test_server)
										lpChar->ChatPacket(CHAT_TYPE_INFO, "<TEST_SERVER> You cannot remove this affect during %d sec.", ShipDefense::EFFECT_SHELTER_DURATION);
									return;
								}

								c_lpUniqueChar->RemoveAffect(AFFECT_DEFENSEWAVE_LASER);
								LaserEffectDataMap.erase(it);

								m_pShipDefense->SetLastLaserShelterPulse(thecore_pulse());
							}
						}
					}
				}
			}
		}
	}
	CShipDefense* m_pShipDefense;
} TCheckLaserPosition;

typedef struct SJumpAll
{
	SJumpAll(const long c_lMapIndex, const long c_lXPos, const long c_lYPos)
		: m_lMapIndex(c_lMapIndex), m_lXPos(c_lXPos), m_lYPos(c_lYPos) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsPC() == true)
			{
				if (lpChar->GetMapIndex() == m_lMapIndex)
				{
					lpChar->Show(m_lMapIndex, m_lXPos, m_lYPos, 0);
					lpChar->Stop();
				}
				else
				{
					lpChar->WarpSet(m_lXPos, m_lYPos, m_lMapIndex);
				}
			}
		}
	}
	long m_lMapIndex, m_lXPos, m_lYPos;
} TJumpAll;

typedef struct SWaveNotice
{
	SWaveNotice(const bool c_bNextWave, const BYTE c_byWave, const BYTE c_bySec)
		: m_bNextWave(c_bNextWave), m_byWave(c_byWave), m_bySec(c_bySec) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar->IsPC() == true)
			{
				switch (m_byWave)
				{
				case ShipDefense::EWaves::WAVE1:
				case ShipDefense::EWaves::WAVE2:
				case ShipDefense::EWaves::WAVE3:
				case ShipDefense::EWaves::WAVE4:
				{
					if (m_bNextWave == true)
						lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The next wave begins in %d sec."), m_bySec);
					else
						lpChar->ChatPacket(CHAT_TYPE_INFO, LC_TEXT("The sea battle starts in %d sec."), m_bySec);
				}
				break;
				}
			}
		}
	}
	bool m_bNextWave;
	BYTE m_byWave, m_bySec;
} TWaveNotice;

typedef struct SNotice
{
	SNotice(CShipDefense* pShipDefense, const ENoticeType c_eType)
		: m_pShipDefense(pShipDefense), m_eType(c_eType) {}
	void operator() (LPENTITY lpEntity)
	{
		if (lpEntity->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER lpChar = static_cast<LPCHARACTER>(lpEntity);
			if (lpChar == nullptr)
				return;

			if (lpChar->IsPC() == false)
				return;

			switch (m_eType)
			{
			case NOTICE_WAVE1:
			{
				const LPCHARACTER c_lpAllianceChar = m_pShipDefense->GetAllianceCharacter();
				if (c_lpAllianceChar == nullptr)
					break;

				m_pShipDefense->Notice(lpChar, true, "We're under attack from sea monsters! Get ready to fight!");
				m_pShipDefense->Notice(lpChar, true, "Wave %d: For %d sec. you must protect the %s with all your strength.",
					m_pShipDefense->GetWave() + 1, ShipDefense::FIRST_WAVE_DURATION,
					LC_MOB_NAME(c_lpAllianceChar->GetRaceNum(), lpChar->GetLanguage())
				);
			}
			break;

			case NOTICE_WAVE2:
			case NOTICE_WAVE3:
			case NOTICE_WAVE4:
			{
				const LPCHARACTER c_lpAllianceChar = m_pShipDefense->GetAllianceCharacter();
				const LPCHARACTER c_lpHydraChar = m_pShipDefense->GetHydraCharacter();
				if (c_lpAllianceChar == nullptr || c_lpHydraChar == nullptr)
					break;

				m_pShipDefense->Notice(lpChar, true, "They're attacking again! Ready your weapons!");
				m_pShipDefense->Notice(lpChar, true, "Wave %d: While protecting the %s, defeat %s.",
					m_pShipDefense->GetWave() + 1,
					LC_MOB_NAME(c_lpAllianceChar->GetRaceNum(), lpChar->GetLanguage()),
					LC_MOB_NAME(c_lpHydraChar->GetRaceNum(), lpChar->GetLanguage())
				);
			}
			break;

			case NOTICE_WAVES:
				m_pShipDefense->Notice(lpChar, true, "Land sighted! You have successfully protected the mast and reached the new continent after a long and weary journey. Continue to the portal.");
				break;

			case NOTICE_MAST_HP:
			{
				const LPCHARACTER c_lpAllianceChar = m_pShipDefense->GetAllianceCharacter();
				if (c_lpAllianceChar == nullptr)
					break;

				m_pShipDefense->Notice(lpChar, false, "%s's remaining HP: %d%%",
					LC_MOB_NAME(c_lpAllianceChar->GetRaceNum(), lpChar->GetLanguage()),
					c_lpAllianceChar->GetHPPct()
				);
			}
			break;

			case NOTICE_MAST_PROTECTED:
				m_pShipDefense->Notice(lpChar, true, "You have successfully protected the mast. Keep it up! Get ready for the next wave.");
				break;

			case NOTICE_MAST_DESTROYED:
				m_pShipDefense->Notice(lpChar, true, "SOS! The mast has been smashed and your ship is sinking. You will now be teleported to Cape Dragon Fire.");
				break;
			}
		}
	}
	CShipDefense* m_pShipDefense;
	ENoticeType m_eType;
} TNotice;


EVENTINFO(ClearSpawnEventInfo)
{
	CShipDefense* pShipDefense;
	EClearType eClearType;
};
EVENTFUNC(ClearSpawnEvent)
{
	ClearSpawnEventInfo* pClearSpawnEventInfo = dynamic_cast<ClearSpawnEventInfo*>(event->info);
	if (pClearSpawnEventInfo == nullptr)
	{
		sys_err("ClearSpawnEventInfo nullptr");
		return 0;
	}

	CShipDefense* c_pShipDefense = pClearSpawnEventInfo->pShipDefense;
	if (c_pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	const LPSECTREE_MAP c_lpSectreeMap = c_pShipDefense->GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return 0;

	TClearMonsters ClearMonsters(pClearSpawnEventInfo->eClearType);
	c_lpSectreeMap->for_each(ClearMonsters);

	return 1;
};


EVENTINFO(ExitEventInfo)
{
	CShipDefense* pShipDefense;
	bool bBackHome;
	UINT uiCountSec;
};
EVENTFUNC(ExitEvent)
{
	ExitEventInfo* pExitEventInfo = dynamic_cast<ExitEventInfo*>(event->info);
	if (pExitEventInfo == nullptr)
	{
		sys_err("ExitEventInfo nullptr");
		return 0;
	}

	CShipDefense* pShipDefense = pExitEventInfo->pShipDefense;
	if (pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return 0;

	if (pExitEventInfo->uiCountSec < 1)
	{
		if (pExitEventInfo->bBackHome == true)
			pShipDefense->JumpBackHome();
		else
			pShipDefense->JumpAllOut();
		return 0;
	}

	if (pExitEventInfo->uiCountSec % 60 == 0)
	{
		TExitNotice ExitNotice(pExitEventInfo->uiCountSec);
		c_lpSectreeMap->for_each(ExitNotice);
	}

	--pExitEventInfo->uiCountSec;

	return PASSES_PER_SEC(1);
};

EVENTINFO(ShipEventInfo)
{
	CShipDefense* pShipDefense;
	BYTE byWave, byWaitCount;
};
EVENTFUNC(ShipEvent)
{
	ShipEventInfo* pShipEventInfo = dynamic_cast<ShipEventInfo*>(event->info);
	if (pShipEventInfo == nullptr)
	{
		sys_err("ShipEventInfo nullptr");
		return 0;
	}

	CShipDefense* pShipDefense = pShipEventInfo->pShipDefense;
	if (pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	if (pShipEventInfo->byWaitCount <= 0)
	{
		pShipDefense->PrepareWave(pShipEventInfo->byWave);
		return 1;
	}

	--pShipEventInfo->byWaitCount;

	return PASSES_PER_SEC(1);
}

EVENTINFO(LaserEffectEventInfo)
{
	CShipDefense* pShipDefense;
};
EVENTFUNC(LaserEffectEvent)
{
	LaserEffectEventInfo* pLaserEffectEventInfo = dynamic_cast<LaserEffectEventInfo*>(event->info);
	if (pLaserEffectEventInfo == nullptr)
	{
		sys_err("LaserEffectEventInfo nullptr");
		return 0;
	}

	CShipDefense* pShipDefense = pLaserEffectEventInfo->pShipDefense;
	if (pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return 0;

	// Check if the player is on top of the laser effect.
	TCheckLaserPosition CheckLaserPosition(pShipDefense);
	c_lpSectreeMap->for_each(CheckLaserPosition);

	BYTE byRandomUniqueCharPosition = number(ShipDefense::UNIQUE_MINI_HYDRA1_POS, ShipDefense::UNIQUE_MINI_HYDRA4_POS);

	const LPCHARACTER c_lpUniqueChar = pShipDefense->GetUniqueCharacter(byRandomUniqueCharPosition);
	if (c_lpUniqueChar == nullptr)
		return PASSES_PER_SEC(1);

	if (c_lpUniqueChar->FindAffect(AFFECT_DEFENSEWAVE_LASER))
		return PASSES_PER_SEC(1);

	typedef std::map<BYTE/*pos*/, std::pair<long/*x*/, long/*y*/>> TEffectPositionMap;
	TEffectPositionMap EffectPositionMap = {
		{ ShipDefense::UNIQUE_MINI_HYDRA1_POS, { 390, 405 }, },
		{ ShipDefense::UNIQUE_MINI_HYDRA2_POS, { 379, 405 }, },
		{ ShipDefense::UNIQUE_MINI_HYDRA3_POS, { 379, 394 }, },
		{ ShipDefense::UNIQUE_MINI_HYDRA4_POS, { 390, 394 }, },
	};

	TEffectPositionMap::iterator it = EffectPositionMap.find(byRandomUniqueCharPosition);
	if (it != EffectPositionMap.end())
	{
		// Generate random probability for laser effect.
		std::random_device RandomDevice;
		std::mt19937 Generate(RandomDevice());
		std::uniform_real_distribution<> Distribute(ShipDefense::MIN_PROB, ShipDefense::MAX_PROB);

		UINT uiSpawnProbability = 0;
		switch (pShipDefense->GetWave())
		{
		case ShipDefense::WAVE2:
			uiSpawnProbability = 50; // 5%
			break;
		case ShipDefense::WAVE3:
			uiSpawnProbability = 75; // 7.5%
			break;
		case ShipDefense::WAVE4:
			uiSpawnProbability = 100; // 10%
			break;
		}

		if (static_cast<UINT>(Distribute(Generate)) <= uiSpawnProbability)
		{
			c_lpUniqueChar->AddAffect(AFFECT_DEFENSEWAVE_LASER, POINT_ATT_GRADE, 100, AFF_DEFENSE_WAVE_LASER, INFINITE_AFFECT_DURATION, 0, true);
			pShipDefense->SetLaserEffectData(byRandomUniqueCharPosition, it->second.first, it->second.second);
		}
	}

	return PASSES_PER_SEC(1);
}

EVENTINFO(WaveEventInfo)
{
	CShipDefense* pShipDefense;
	bool bNextWave;
	BYTE byWave, byWaitCount;
};
EVENTFUNC(WaveEvent)
{
	WaveEventInfo* pWaveEventInfo = dynamic_cast<WaveEventInfo*>(event->info);
	if (pWaveEventInfo == nullptr)
	{
		sys_err("WaveEventInfo nullptr");
		return 0;
	}

	CShipDefense* pShipDefense = pWaveEventInfo->pShipDefense;
	if (pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return 0;

	if (pWaveEventInfo->byWaitCount < 1)
	{
		if (pWaveEventInfo->bNextWave == true)
			pShipDefense->PrepareWave(pWaveEventInfo->byWave + 1);
		else
			pShipDefense->SetWave(pWaveEventInfo->byWave);

		return 1;
	}

	switch (pWaveEventInfo->byWave)
	{
	case ShipDefense::WAVE1:
	case ShipDefense::WAVE2:
	case ShipDefense::WAVE3:
	case ShipDefense::WAVE4:
	{
		TWaveNotice WaveNotice(pWaveEventInfo->bNextWave, pWaveEventInfo->byWave, pWaveEventInfo->byWaitCount);
		if (pWaveEventInfo->bNextWave == true)
		{
			if (pWaveEventInfo->byWaitCount <= 10 || (pWaveEventInfo->byWaitCount == 30))
				c_lpSectreeMap->for_each(WaveNotice);
		}
		else
		{
			if (pWaveEventInfo->byWaitCount <= 10 || (pWaveEventInfo->byWaitCount % 10 == 0 && pWaveEventInfo->byWaitCount < 60))
				c_lpSectreeMap->for_each(WaveNotice);
		}
	}
	break;
	}

	--pWaveEventInfo->byWaitCount;

	return PASSES_PER_SEC(1);
}

EVENTINFO(SpawnEventInfo)
{
	CShipDefense* pShipDefense;
	BYTE byWave, byStep;
	UINT uiCountSec;
};
EVENTFUNC(SpawnEvent)
{
	SpawnEventInfo* pSpawnEventInfo = dynamic_cast<SpawnEventInfo*>(event->info);
	if (pSpawnEventInfo == nullptr)
	{
		sys_err("SpawnEventInfo nullptr");
		return 0;
	}

	CShipDefense* pShipDefense = pSpawnEventInfo->pShipDefense;
	if (pShipDefense == nullptr)
	{
		sys_err("CShipDefense nullptr");
		return 0;
	}

	const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return 0;

	// Generate random probability for spawning something.
	std::random_device RandomDevice;
	std::mt19937 Generate(RandomDevice());
	std::uniform_real_distribution<> Distribute(ShipDefense::MIN_PROB, ShipDefense::MAX_PROB);

	typedef std::vector<std::pair<long, long>> TSpawnPositionVector;
	TSpawnPositionVector SpawnPositionVector = {
		{ 396, 426 },
		{ 392, 427 },
		{ 389, 429 },
		{ 384, 431 },
		{ 380, 429 },
		{ 376, 427 },
		{ 373, 426 },
	};
	UINT uiRandomPosition = rand() % SpawnPositionVector.size();

	switch (pSpawnEventInfo->byWave)
	{
	case ShipDefense::WAVE1:
	{
		// Spawn monster each 40 seconds and set the next step.
		if (pSpawnEventInfo->uiCountSec >= 40)
		{
			if (pSpawnEventInfo->uiCountSec % number(1, 3) == 0)
			{
				// Spawn monster each 5 seconds and continue step.
				pShipDefense->Spawn(number(3954, 3955), SpawnPositionVector[uiRandomPosition].first, SpawnPositionVector[uiRandomPosition].second, 0);
			}
		}
	}
	break;

	case ShipDefense::WAVE2:
	{
		if (static_cast<UINT>(Distribute(Generate)) <= 10) // 1%
			pShipDefense->SpawnEgg();

		if (static_cast<UINT>(Distribute(Generate)) <= 30) // 3%
			pShipDefense->SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA1, 1);

		if (pSpawnEventInfo->uiCountSec % 10 == 0 && pSpawnEventInfo->byStep == 0)
		{
			// Spawn regen each 10 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave2_1.txt");
			pSpawnEventInfo->byStep = 1;
		}
		else if (pSpawnEventInfo->uiCountSec % 30 == 0 && pSpawnEventInfo->byStep == 1)
		{
			// Spawn regen each 30 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave2_1.txt");
			pSpawnEventInfo->byStep = 2;
		}
		else if (pSpawnEventInfo->uiCountSec % 60 == 0 && pSpawnEventInfo->byStep == 2)
		{
			// Spawn regen each 60 seconds and reset the steps.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave2.txt");
			pSpawnEventInfo->byStep = 0; // Repeat
		}
	}
	break;

	case ShipDefense::WAVE3:
	{
		if (static_cast<UINT>(Distribute(Generate)) <= 10) // 1%
			pShipDefense->SpawnEgg();

		if (static_cast<UINT>(Distribute(Generate)) <= 40) // 4%
			pShipDefense->SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA2, 2);

		if (pSpawnEventInfo->uiCountSec % 10 == 0 && pSpawnEventInfo->byStep == 0)
		{
			// Spawn regen each 10 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave3_1.txt");
			pSpawnEventInfo->byStep = 1;
		}
		else if (pSpawnEventInfo->uiCountSec % 30 == 0 && pSpawnEventInfo->byStep == 1)
		{
			// Spawn regen each 30 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave3_1.txt");
			pSpawnEventInfo->byStep = 2;
		}
		else if (pSpawnEventInfo->uiCountSec % 60 == 0 && pSpawnEventInfo->byStep == 2)
		{
			// Spawn regen each 60 seconds and reset the steps.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave3.txt");
			pSpawnEventInfo->byStep = 0;
		}
	}
	break;

	case ShipDefense::WAVE4:
	{
		if (static_cast<UINT>(Distribute(Generate)) <= 10) // 1%
			pShipDefense->SpawnEgg();

		if (static_cast<UINT>(Distribute(Generate)) <= 50) // 5%
			pShipDefense->SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA2, 4);

		if (pSpawnEventInfo->uiCountSec % 10 == 0 && pSpawnEventInfo->byStep == 0)
		{
			// Spawn regen each 10 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave4_1.txt");
			pSpawnEventInfo->byStep = 1;
		}
		else if (pSpawnEventInfo->uiCountSec % 30 == 0 && pSpawnEventInfo->byStep == 1)
		{
			// Spawn regen each 30 seconds and set the next step.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave4_1.txt");
			pSpawnEventInfo->byStep = 2;
		}
		else if (pSpawnEventInfo->uiCountSec % 60 == 0 && pSpawnEventInfo->byStep == 2)
		{
			// Spawn regen each 60 seconds and reset the steps.
			pShipDefense->SpawnRegen("data/dungeon/ship_defense/wave4.txt");
			pSpawnEventInfo->byStep = 0;
		}
	}
	break;
	}

	// Set monster victim to alliance character.
	TFindAllianceCharacter FindAllianceCharacter(pShipDefense->GetAllianceCharacter());
	c_lpSectreeMap->for_each(FindAllianceCharacter);

	// Prevent second counter from overflowing.
	if (pSpawnEventInfo->uiCountSec > UINT_MAX)
		return 0;

	// Increase second counter.
	++pSpawnEventInfo->uiCountSec;

	return PASSES_PER_SEC(1);
}

CShipDefense::CShipDefense(const long c_lMapIndex, const DWORD c_dwLeaderPID)
	: m_lMapIndex(c_lMapIndex), m_dwLeaderPID(c_dwLeaderPID)
{
	m_lStartTime = 0;

	m_byState = CShipDefenseManager::STATE_CREATE;
	m_byWave = 0;

	m_lpSectreeMap = nullptr;

	std::memset(&m_lpBarrier, 0, sizeof(m_lpBarrier));

	m_mapUniqueCharacter.clear();
	m_mapLaserEffectData.clear();
	m_iLastLaserShelterPulse = 0;

	PrepareDeck();
}

CShipDefense::~CShipDefense()
{
	Destroy();
}

void CShipDefense::Destroy()
{
	CShipDefenseManager::Instance().Remove(m_dwLeaderPID);
	SECTREE_MANAGER::instance().DestroyPrivateMap(m_lMapIndex);

	m_lStartTime = 0;

	m_byState = CShipDefenseManager::STATE_NONE;
	m_byWave = 0;

	m_lMapIndex = 0;
	m_dwLeaderPID = 0;

	m_lpSectreeMap = nullptr;

	std::memset(&m_lpBarrier, 0, sizeof(m_lpBarrier));

	CancelEvents();

	m_mapUniqueCharacter.clear();
	m_mapLaserEffectData.clear();

	m_iLastLaserShelterPulse = 0;
}

void CShipDefense::CancelEvents()
{
	if (m_lpShipEvent != nullptr)
		event_cancel(&m_lpShipEvent);

	if (m_lpWaveEvent != nullptr)
		event_cancel(&m_lpWaveEvent);

	if (m_lpSpawnEvent != nullptr)
		event_cancel(&m_lpSpawnEvent);

	if (m_lpClearSpawnEvent != nullptr)
		event_cancel(&m_lpClearSpawnEvent);

	if (m_lpLaserEffectEvent != nullptr)
		event_cancel(&m_lpLaserEffectEvent);

	if (m_lpExitEvent != nullptr)
		event_cancel(&m_lpExitEvent);
}

void CShipDefense::DeadCharacter(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return;

	if (c_lpChar->IsPC() == true)
		return;

	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::DeadCharacter(c_lpChar=%p) - c_lpSectreeMap[nullptr]", c_lpChar);
		return;
	}

	if (c_lpChar == GetUniqueCharacter(ShipDefense::UNIQUE_MAST_POS))
	{
		CancelEvents();
		ClearDeck(ShipDefense::EClearType::CLEAR_ALL);

		TNotice Notice(this, NOTICE_MAST_DESTROYED);
		c_lpSectreeMap->for_each(Notice);

		ExitEventInfo* pExitEventInfo = AllocEventInfo<ExitEventInfo>();
		pExitEventInfo->pShipDefense = this;
		pExitEventInfo->bBackHome = true;
		pExitEventInfo->uiCountSec = ShipDefense::EXIT_DELAY;
		m_lpExitEvent = event_create(ExitEvent, pExitEventInfo, PASSES_PER_SEC(1));
	}

	UniqueCharacterMap::iterator it = m_mapUniqueCharacter.begin();
	while (it != m_mapUniqueCharacter.end())
	{
		LPCHARACTER lpUniqueChar = it->second;
		if (lpUniqueChar == c_lpChar)
		{
			m_mapUniqueCharacter.erase(it);
			break;
		}
		++it;
	}
}

void CShipDefense::Notice(const LPCHARACTER c_lpChar, const bool c_bBigFont, const char* c_pszBuf ...)
{
	if (c_lpChar == nullptr)
		return;

	if (c_lpChar->GetDesc() == nullptr)
		return;

	va_list pszArgs;
	va_start(pszArgs, c_pszBuf);
	{
		const char* c_pszLocaleText = LC_TEXT(c_pszBuf);

		// OWSAP_MULTI_LOCALE_SUPPORT
		//const char* c_pszLocaleText = LC_STRING(c_pszBuf, c_lpChar->GetLanguage());
		// OWSAP_END_OF_MULTI_LOCALE_SUPPORT

		char szBuf[CHAT_MAX_LEN + 1] = {};
		int iLen = vsnprintf(szBuf, sizeof(szBuf), c_pszLocaleText, pszArgs);

		c_lpChar->GetDesc()->ChatPacket(c_bBigFont ? CHAT_TYPE_BIG_NOTICE : CHAT_TYPE_NOTICE, "%s", szBuf);
	}
	va_end(pszArgs);
}

LPCHARACTER CShipDefense::Spawn(DWORD dwVNum, int iX, int iY, int iDir, bool bSpawnMotion)
{
	if (dwVNum == 0 || m_lpSectreeMap == nullptr)
		return nullptr;

	long lX = m_lpSectreeMap->m_setting.iBaseX + iX * 100;
	long lY = m_lpSectreeMap->m_setting.iBaseY + iY * 100;
	int iRot = iDir == 0 ? -1 : (iDir - 1) * 45;

	const CMob* c_pMob = CMobManager::instance().Get(dwVNum);
	if (!c_pMob)
	{
		sys_err("CShipDefense::Spawn(...) -  no mob data for vnum %u", dwVNum);
		return nullptr;
	}

	const LPSECTREE c_lpSectree = SECTREE_MANAGER::instance().Get(m_lMapIndex, lX, lY);
	if (!c_lpSectree)
	{
		sys_log(0, "CShipDefense::Spawn(...) - cannot create monster at non-exist sectree %d x %d (map %d)", lX, lY, m_lMapIndex);
		return nullptr;
	}

	LPCHARACTER lpChar = CHARACTER_MANAGER::instance().CreateCharacter(c_pMob->m_table.szLocaleName);
	if (lpChar == nullptr)
	{
		sys_log(0, "CShipDefense::Spawn(...) cannot create new character %u", dwVNum);
		return nullptr;
	}

	if (iRot == -1)
		iRot = number(0, 360);

	lpChar->SetProto(c_pMob);
	lpChar->SetRotation(iRot);

	if (!lpChar->Show(m_lMapIndex, lX, lY, 0, bSpawnMotion))
	{
		CHARACTER_MANAGER::instance().DestroyCharacter(lpChar);
		sys_log(0, "CShipDefense::Spawn(...) - cannot show monster %u", dwVNum);
		return nullptr;
	}

	return lpChar;
}

bool CShipDefense::SpawnRegen(const char* c_szFileName, bool bOnce)
{
	if (!c_szFileName)
	{
		sys_err("CShipDefense::SpawnRegen(c_szFileName=nullptr, bOnce=%d) - m_lMapIndex[%d]", bOnce, m_lMapIndex);
		return false;
	}

	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err("CShipDefense::SpawnRegen(c_szFileName=%s, bOnce=%d) - m_lMapIndex[%d] c_lpSectreeMap[nullptr]", c_szFileName, bOnce, m_lMapIndex);
		return false;
	}

	return regen_do(c_szFileName, m_lMapIndex, c_lpSectreeMap->m_setting.iBaseX, c_lpSectreeMap->m_setting.iBaseY, nullptr, bOnce);
}

void CShipDefense::SpawnHydra(const DWORD c_dwVNum)
{
	if (CShipDefenseManager::Instance().IsHydra(c_dwVNum))
	{
		UniqueCharacterMap::iterator it = m_mapUniqueCharacter.find(ShipDefense::UNIQUE_HYDRA_POS);
		if (it != m_mapUniqueCharacter.end())
			return;

		m_mapUniqueCharacter.emplace(ShipDefense::UNIQUE_HYDRA_POS, Spawn(c_dwVNum, 385, 373, 1));
	}
}

void CShipDefense::SpawnMiniHydra(const DWORD c_dwVNum, const BYTE c_byCount)
{
	for (BYTE byPos = 0; byPos < c_byCount; ++byPos)
	{
		BYTE byUniquePos = ShipDefense::UNIQUE_MINI_HYDRA1_POS + byPos;

		if (m_mapUniqueCharacter.find(byUniquePos) != m_mapUniqueCharacter.end())
			return; // Already spawned.

		typedef std::map<BYTE/*pos*/, std::pair<long/*x*/, long/*y*/>> TMiniHydraPositionMap;
		TMiniHydraPositionMap MiniHydraPositionMap = {
			{ ShipDefense::UNIQUE_MINI_HYDRA1_POS, { 396, 411 }, },
			{ ShipDefense::UNIQUE_MINI_HYDRA2_POS, { 374, 411 }, },
			{ ShipDefense::UNIQUE_MINI_HYDRA3_POS, { 374, 389 }, },
			{ ShipDefense::UNIQUE_MINI_HYDRA4_POS, { 396, 389 }, },
		};

		TMiniHydraPositionMap::iterator it = MiniHydraPositionMap.find(byUniquePos);
		if (it != MiniHydraPositionMap.end())
			m_mapUniqueCharacter.emplace(byUniquePos, Spawn(c_dwVNum, it->second.first, it->second.second, 0));
	}
}

void CShipDefense::SpawnEgg()
{
	UniqueCharacterMap::iterator it = m_mapUniqueCharacter.find(ShipDefense::UNIQUE_HYDRA_EGG_POS);
	if (it != m_mapUniqueCharacter.end())
		return;

	typedef std::vector<std::pair<long/*x*/, long/*y*/>> THydraEggPositionVector;
	THydraEggPositionVector HydraEggPositionVector = {
		{ 379, 394 },
		{ 391, 392 },
		{ 391, 403 },
		{ 380, 406 },
	};
	UINT uiRandomPosition = rand() % HydraEggPositionVector.size();

	m_mapUniqueCharacter.emplace(ShipDefense::UNIQUE_HYDRA_EGG_POS,
		Spawn(ShipDefense::EVNumHelper::HYDRA_EGG,
			HydraEggPositionVector[uiRandomPosition].first,
			HydraEggPositionVector[uiRandomPosition].second,
			0
		)
	);

	if (m_lpClearSpawnEvent != nullptr)
		event_cancel(&m_lpClearSpawnEvent);

	ClearSpawnEventInfo* pClearSpawnEventInfo = AllocEventInfo<ClearSpawnEventInfo>();
	pClearSpawnEventInfo->pShipDefense = this;
	pClearSpawnEventInfo->eClearType = ShipDefense::CLEAR_EGG;
	m_lpClearSpawnEvent = event_create(ClearSpawnEvent, pClearSpawnEventInfo, PASSES_PER_SEC(ShipDefense::EGG_SPAWN_DURATION));
}

void CShipDefense::SpawnBarriers()
{
	if (m_lpBarrier[ShipDefense::BACK_BARRIER_LEFT] == nullptr)
		m_lpBarrier[ShipDefense::BACK_BARRIER_LEFT] = Spawn(ShipDefense::EVNumHelper::SHIP_BARRIER, 400, 372, 1);

	if (m_lpBarrier[ShipDefense::BACK_BARRIER_RIGHT] == nullptr)
		m_lpBarrier[ShipDefense::BACK_BARRIER_RIGHT] = Spawn(ShipDefense::EVNumHelper::SHIP_BARRIER, 369, 372, 1);

	if (m_lpBarrier[ShipDefense::FRONT_BARRIER_LEFT] == nullptr)
		m_lpBarrier[ShipDefense::FRONT_BARRIER_LEFT] = Spawn(ShipDefense::EVNumHelper::SHIP_BARRIER, 400, 435, 1);

	if (m_lpBarrier[ShipDefense::FRONT_BARRIER_RIGHT] == nullptr)
		m_lpBarrier[ShipDefense::FRONT_BARRIER_RIGHT] = Spawn(ShipDefense::EVNumHelper::SHIP_BARRIER, 369, 435, 1);
}

void CShipDefense::RemoveBarriers()
{
	for (BYTE byBarrier = 0; byBarrier < ShipDefense::BARRIERS; ++byBarrier)
	{
		if (m_lpBarrier[byBarrier] != nullptr)
		{
			CHARACTER_MANAGER::Instance().DestroyCharacter(m_lpBarrier[byBarrier]);
			m_lpBarrier[byBarrier] = nullptr;
		}
	}
}

void CShipDefense::RemoveFrontBarriers()
{
	for (BYTE byBarrier = ShipDefense::FRONT_BARRIER_LEFT; byBarrier <= ShipDefense::FRONT_BARRIER_RIGHT; ++byBarrier)
	{
		if (m_lpBarrier[byBarrier] != nullptr)
		{
			CHARACTER_MANAGER::Instance().DestroyCharacter(m_lpBarrier[byBarrier]);
			m_lpBarrier[byBarrier] = nullptr;
		}
	}
}

void CShipDefense::RemoveBackBarriers()
{
	for (BYTE byBarrier = ShipDefense::BACK_BARRIER_LEFT; byBarrier <= ShipDefense::BACK_BARRIER_RIGHT; ++byBarrier)
	{
		if (m_lpBarrier[byBarrier] != nullptr)
		{
			CHARACTER_MANAGER::Instance().DestroyCharacter(m_lpBarrier[byBarrier]);
			m_lpBarrier[byBarrier] = nullptr;
		}
	}
}

void CShipDefense::Start()
{
	m_lStartTime = std::time(nullptr);
	m_byState = CShipDefenseManager::STATE_START;

	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::Start() - c_lpSectreeMap[nullptr]");
		return;
	}

	// Broadcast alliance health point to everyone.
	TBroadcastAllianceHP BroadcastAllianceHP(GetAllianceCharacter());
	c_lpSectreeMap->for_each(BroadcastAllianceHP);

	// Prepare the first wave.
	PrepareWave(ShipDefense::WAVE1);
}

void CShipDefense::ClearDeck(ShipDefense::EClearType eClearType)
{
	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::ClearDeck() - c_lpSectreeMap[nullptr]");
		return;
	}

	TClearMonsters ClearMonsters(eClearType);
	c_lpSectreeMap->for_each(ClearMonsters);
}

bool CShipDefense::PrepareDeck()
{
	m_lpSectreeMap = SECTREE_MANAGER::instance().GetMap(m_lMapIndex);

	if (m_lpSectreeMap != nullptr)
	{
		m_lStartTime = std::time(nullptr);

		SpawnBarriers();

		Spawn(ShipDefense::EVNumHelper::SHIP_WHEEL, 385, 367, 1);
		m_mapUniqueCharacter.emplace(ShipDefense::UNIQUE_MAST_POS, Spawn(ShipDefense::EVNumHelper::SHIP_MAST, 385, 400, 1));

		return true;
	}

	return false;
}

// NOTE: Prepare the wave before it starts.
void CShipDefense::PrepareWave(const BYTE c_byWave)
{
	// NOTE: Cancel all previous events when preparing a new wave.
	CancelEvents();

	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::PrepareWave(c_byWave=%d) - c_lpSectreeMap[nullptr]", c_byWave);
		return;
	}

	// NOTE: Prepare the waves first by sending a notice and cool down
	// before starting the wave.
	WaveEventInfo* pWaveEventInfo = AllocEventInfo<WaveEventInfo>();
	pWaveEventInfo->pShipDefense = this;
	pWaveEventInfo->bNextWave = false;
	pWaveEventInfo->byWave = c_byWave;

	switch (c_byWave)
	{
	case ShipDefense::WAVE1:
	{
		// Intimidate player with fake Hydra.
		{
			m_mapUniqueCharacter.emplace(ShipDefense::UNIQUE_FAKE_HYDRA_POS, Spawn(ShipDefense::EVNumHelper::HYDRA_LEFT, 385, 373, 5));
			ClearSpawnEventInfo* pClearSpawnEventInfo = AllocEventInfo<ClearSpawnEventInfo>();
			pClearSpawnEventInfo->pShipDefense = this;
			pClearSpawnEventInfo->eClearType = ShipDefense::CLEAR_FAKE_BOSS;
			m_lpClearSpawnEvent = event_create(ClearSpawnEvent, pClearSpawnEventInfo, PASSES_PER_SEC(ShipDefense::FAKE_SPAWN_DURATION));
		}

		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 378, 443, 5);
		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 385, 439, 5);
		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 392, 443, 5);

		TNotice Notice(this, NOTICE_WAVE1);
		c_lpSectreeMap->for_each(Notice);

		pWaveEventInfo->byWaitCount = ShipDefense::FIRST_WAVE_DELAY;
	}
	break;

	case ShipDefense::WAVE2:
	{
		ClearDeck(ShipDefense::CLEAR_ALL);
		JumpToQuarterDeck();

		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 385, 439, 5);
		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 392, 443, 5);
		SpawnHydra(ShipDefense::EVNumHelper::HYDRA1);

		TNotice Notice(this, NOTICE_WAVE2);
		c_lpSectreeMap->for_each(Notice);

		pWaveEventInfo->byWaitCount = ShipDefense::NEXT_WAVE_DELAY;
	}
	break;

	case ShipDefense::WAVE3:
	{
		ClearDeck(ShipDefense::CLEAR_ALL);
		JumpToQuarterDeck();

		Spawn(ShipDefense::EVNumHelper::HYDRA_RIGHT, 385, 439, 5);
		SpawnHydra(ShipDefense::EVNumHelper::HYDRA2);

		TNotice Notice(this, NOTICE_WAVE3);
		c_lpSectreeMap->for_each(Notice);

		pWaveEventInfo->byWaitCount = ShipDefense::NEXT_WAVE_DELAY;
	}
	break;

	case ShipDefense::WAVE4:
	{
		ClearDeck(ShipDefense::CLEAR_ALL);
		JumpToQuarterDeck();

		SpawnHydra(ShipDefense::EVNumHelper::HYDRA3);

		TNotice Notice(this, NOTICE_WAVE4);
		c_lpSectreeMap->for_each(Notice);

		pWaveEventInfo->byWaitCount = ShipDefense::NEXT_WAVE_DELAY;
	}
	break;
	}

	m_lpWaveEvent = event_create(WaveEvent, pWaveEventInfo, PASSES_PER_SEC(1));
}

// NOTE: Set wave after PrepareWave is called.
void CShipDefense::SetWave(const BYTE c_byWave)
{
	CancelEvents();

	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::SetWave(c_byWave=%d) - c_lpSectreeMap[nullptr]", c_byWave);
		return;
	}

	m_byWave = c_byWave;

	// Spawns
	switch (c_byWave)
	{
	case ShipDefense::WAVE1:
	case ShipDefense::WAVE2:
	case ShipDefense::WAVE3:
	case ShipDefense::WAVE4:
	{
		// Remove Barries
		RemoveBackBarriers();

		char szRegenFileName[64];
		snprintf(szRegenFileName, sizeof(szRegenFileName), "data/dungeon/ship_defense/wave%d.txt", c_byWave);
		SpawnRegen(szRegenFileName);

		if (c_byWave == ShipDefense::WAVE1)
		{
			// Wave Event (Used for waiting for the first wave)
			WaveEventInfo* pWaveEventInfo = AllocEventInfo<WaveEventInfo>();
			pWaveEventInfo->pShipDefense = this;
			pWaveEventInfo->bNextWave = true;
			pWaveEventInfo->byWave = c_byWave;
			pWaveEventInfo->byWaitCount = ShipDefense::FIRST_WAVE_DURATION;
			m_lpWaveEvent = event_create(WaveEvent, pWaveEventInfo, PASSES_PER_SEC(1));
		}

		if (c_byWave == ShipDefense::WAVE2)
			SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA1, 1);

		if (c_byWave == ShipDefense::WAVE3)
			SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA2, 2);

		if (c_byWave == ShipDefense::WAVE4)
			SpawnMiniHydra(ShipDefense::EVNumHelper::MINI_HYDRA3, 4);

		// Spawn Event
		SpawnEventInfo* pSpawnEventInfo = AllocEventInfo<SpawnEventInfo>();
		pSpawnEventInfo->pShipDefense = this;
		pSpawnEventInfo->byWave = c_byWave;
		pSpawnEventInfo->uiCountSec = 0;
		pSpawnEventInfo->byStep = 0;
		m_lpSpawnEvent = event_create(SpawnEvent, pSpawnEventInfo, PASSES_PER_SEC(1));
	}
	break;
	}

	// Laser Event
	switch (c_byWave)
	{
	case ShipDefense::WAVE2:
	case ShipDefense::WAVE3:
	case ShipDefense::WAVE4:
	{
		LaserEffectEventInfo* pLaserEffectEventInfo = AllocEventInfo<LaserEffectEventInfo>();
		pLaserEffectEventInfo->pShipDefense = this;
		m_lpLaserEffectEvent = event_create(LaserEffectEvent, pLaserEffectEventInfo, PASSES_PER_SEC(1));
	}
	break;
	}

	// Set monster victim to alliance character.
	TFindAllianceCharacter FindAllianceCharacter(GetAllianceCharacter());
	c_lpSectreeMap->for_each(FindAllianceCharacter);
}

void CShipDefense::SetShipEvent(const BYTE c_byWave)
{
	CancelEvents();
	ClearDeck(ShipDefense::CLEAR_ALL);

	switch (c_byWave)
	{
	case ShipDefense::WAVE3:
	case ShipDefense::WAVE4:
	{
		TNotice Notice(this, NOTICE_MAST_PROTECTED);
		GetSectreeMap()->for_each(Notice);

		ShipEventInfo* pShipEventInfo = AllocEventInfo<ShipEventInfo>();
		pShipEventInfo->pShipDefense = this;
		pShipEventInfo->byWave = c_byWave;
		pShipEventInfo->byWaitCount = ShipDefense::NEXT_WAVE_FAST_DELAY;
		m_lpShipEvent = event_create(ShipEvent, pShipEventInfo, PASSES_PER_SEC(1));
	}
	break;

	case ShipDefense::WAVES:
	{
		RemoveBarriers();

		TNotice Notice(this, NOTICE_WAVES);
		GetSectreeMap()->for_each(Notice);

		Spawn(ShipDefense::EVNumHelper::HYDRA_REWARD, 385, 416, 1);
		Spawn(ShipDefense::EVNumHelper::PORTAL, 385, 450, 1);

		ExitEventInfo* pExitEventInfo = AllocEventInfo<ExitEventInfo>();
		pExitEventInfo->pShipDefense = this;
		pExitEventInfo->bBackHome = false;
		pExitEventInfo->uiCountSec = ShipDefense::JUMP_OUT_DELAY;
		m_lpExitEvent = event_create(ExitEvent, pExitEventInfo, PASSES_PER_SEC(1));
	}
	break;
	}
}

void CShipDefense::JumpToQuarterDeck()
{
	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
	{
		sys_err(0, "CShipDefense::JumpToQuarterDeck() - c_lpSectreeMap[nullptr]");
		return;
	}

	PIXEL_POSITION SPos = { 0, 0, 0 };
	SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(m_lMapIndex, 0, SPos);

	TJumpAll JumpAll(m_lMapIndex, SPos.x, SPos.y);
	c_lpSectreeMap->for_each(JumpAll);

	SpawnBarriers();
}

void CShipDefense::JumpBackHome()
{
	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return;

	PIXEL_POSITION SPos = { 0, 0, 0 };
	SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ShipDefense::EMapIndex::ENTRY_MAP_INDEX, 0, SPos);

	TJumpAll JumpAll(ShipDefense::EMapIndex::ENTRY_MAP_INDEX, SPos.x, SPos.y);
	c_lpSectreeMap->for_each(JumpAll);
}

void CShipDefense::JumpAllOut()
{
	const LPSECTREE_MAP c_lpSectreeMap = GetSectreeMap();
	if (c_lpSectreeMap == nullptr)
		return;

	const LPSECTREE_MAP c_lpSectreeTargetMap = SECTREE_MANAGER::instance().GetMap(ShipDefense::EMapIndex::PORT_MAP_INDEX);
	if (c_lpSectreeTargetMap != nullptr)
	{
		TJumpAll JumpAll(ShipDefense::EMapIndex::PORT_MAP_INDEX,
			c_lpSectreeTargetMap->m_setting.iBaseX + 405 * 100,
			c_lpSectreeTargetMap->m_setting.iBaseY + 480 * 100
		);
		c_lpSectreeMap->for_each(JumpAll);

	}
	else
	{
		JumpBackHome();
	}

	Destroy();
}

LPCHARACTER CShipDefense::GetAllianceCharacter()
{
	return GetUniqueCharacter(ShipDefense::UNIQUE_MAST_POS);
}

LPCHARACTER CShipDefense::GetHydraCharacter()
{
	return GetUniqueCharacter(ShipDefense::UNIQUE_HYDRA_POS);
}

bool CShipDefense::IsUniqueMiniHydra(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	if (m_mapUniqueCharacter.empty())
		return false;

	for (BYTE byUniquePos = ShipDefense::UNIQUE_MINI_HYDRA1_POS; byUniquePos <= ShipDefense::UNIQUE_MINI_HYDRA4_POS; ++byUniquePos)
	{
		UniqueCharacterMap::iterator it = m_mapUniqueCharacter.find(byUniquePos);
		if (it != m_mapUniqueCharacter.end())
		{
			if (it->second == c_lpChar)
				return true;
		}
	}

	return false;
}

LPCHARACTER CShipDefense::GetUniqueCharacter(const BYTE c_byUniqueID)
{
	if (m_mapUniqueCharacter.empty())
		return nullptr;

	UniqueCharacterMap::iterator it = m_mapUniqueCharacter.find(c_byUniqueID);
	if (it != m_mapUniqueCharacter.end())
		return it->second;

	return nullptr;
}

void CShipDefense::SetLaserEffectData(const BYTE c_byPos, const long c_lXPos, const long c_lYPos)
{
	LaserEffectData LaserEffectData = { 0, 0 };
	LaserEffectData.x = c_lXPos;
	LaserEffectData.y = c_lYPos;
	m_mapLaserEffectData.insert(std::make_pair(c_byPos, LaserEffectData));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CShipDefenseManager::CShipDefenseManager()
{
	Initialize();
}

CShipDefenseManager::~CShipDefenseManager()
{
	Destroy();
}

void CShipDefenseManager::Initialize()
{
	m_mapShipDefense.clear();
}

void CShipDefenseManager::Remove(const DWORD c_dwLeaderPID)
{
	ShipDefenseMap::iterator it = m_mapShipDefense.find(c_dwLeaderPID);
	if (it != m_mapShipDefense.end())
		m_mapShipDefense.erase(it);
}

void CShipDefenseManager::Destroy()
{
	ShipDefenseMap::iterator it = m_mapShipDefense.begin();
	for (; it != m_mapShipDefense.end(); ++it)
	{
		CShipDefense* pShipDefense = it->second;
		if (pShipDefense)
		{
			pShipDefense->Destroy();
			if (pShipDefense)
				delete pShipDefense;
		}
	}
	m_mapShipDefense.clear();
}

bool CShipDefenseManager::Create(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpChar, true/*isLeader*/);

	// Check if the instance is already prepared.
	switch (GetState(c_dwLeaderPID))
	{
	case STATE_CREATE:
	case STATE_START:
		sys_err("CShipDefenseManager::Create(c_lpChar=%p) - Instance already created or running.", c_lpChar);
		return false;
	}

	// Create Private Map
	long lPrivateMapIndex = SECTREE_MANAGER::instance().CreatePrivateMap(ShipDefense::EMapIndex::SHIP_MAP_INDEX);
	if (!lPrivateMapIndex)
	{
		c_lpChar->ChatPacket(CHAT_TYPE_INFO, "Failed to create private map.");
		return false;
	}

	// Create Instance
	CShipDefense* pShipDefense = new CShipDefense(lPrivateMapIndex, c_dwLeaderPID);
	if (pShipDefense != nullptr)
	{
		// Insert Ship Defense Map
		m_mapShipDefense.insert(std::make_pair(c_dwLeaderPID, pShipDefense));

		const LPPARTY c_lpParty = c_lpChar->GetParty();
		if (c_lpParty != nullptr)
			c_lpParty->ChatPacketToAllMember(CHAT_TYPE_NOTICE, LC_TEXT("The 'Ship Defence' dungeon is ready. The fisherman will let you board now."));

		// Warp to Private Map
		PIXEL_POSITION SPos = { 0, 0, 0 };
		SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ShipDefense::EMapIndex::SHIP_MAP_INDEX, 0, SPos);

		c_lpChar->SaveExitLocation();
		c_lpChar->WarpSet(SPos.x, SPos.y, pShipDefense->GetMapIndex());

		return true;
	}

	return false;
}

bool CShipDefenseManager::Start(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpChar, true/*isLeader*/);

	ShipDefenseMap::const_iterator it = m_mapShipDefense.find(c_dwLeaderPID);
	if (it != m_mapShipDefense.end())
	{
		CShipDefense* pShipDefense = it->second;
		if (pShipDefense == nullptr)
			return false;

		pShipDefense->Start();
		return true;
	}

	return false;
}

void CShipDefenseManager::Stop(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpChar, true/*isLeader*/);

	ShipDefenseMap::const_iterator it = m_mapShipDefense.find(c_dwLeaderPID);
	if (it != m_mapShipDefense.end())
	{
		CShipDefense* pShipDefense = it->second;
		if (pShipDefense != nullptr)
			pShipDefense->JumpBackHome();
	}
}

BYTE CShipDefenseManager::GetState(const DWORD c_dwLeaderPID) const
{
	if (m_mapShipDefense.empty())
		return STATE_NONE;

	ShipDefenseMap::const_iterator it = m_mapShipDefense.find(c_dwLeaderPID);
	if (it != m_mapShipDefense.end())
	{
		const CShipDefense* c_pShipDefense = it->second;
		if (c_pShipDefense != nullptr)
			return c_pShipDefense->GetState();
	}

	return STATE_NONE;
}

bool CShipDefenseManager::IsCreated(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	if (c_lpChar->IsPC() == false)
		return false;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpChar, false/*isLeader*/);
	if (GetState(c_dwLeaderPID) == STATE_CREATE)
		return true;

	return false;
}

bool CShipDefenseManager::IsRunning(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	if (c_lpChar->IsPC() == false)
		return false;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpChar, false/*isLeader*/);
	if (GetState(c_dwLeaderPID) == STATE_START)
		return true;

	return false;
}

// NOTE: Used to check for party leader pid or player pid, based on the instance style.
DWORD CShipDefenseManager::GetLeaderPID(const LPCHARACTER c_lpChar, const bool c_bIsLeader)
{
	if (c_lpChar == nullptr)
		return 0;

	if (c_lpChar->IsPC() == false)
		return 0;

	DWORD dwLeaderPID = c_lpChar->GetPlayerID();

	if (ShipDefense::NEED_PARTY == true)
	{
		const LPPARTY c_pParty = c_lpChar->GetParty();
		if (c_pParty == nullptr)
			return 0;

		if (c_bIsLeader)
			if (c_lpChar != c_pParty->GetLeaderCharacter())
				return 0;

		dwLeaderPID = c_pParty->GetLeaderPID();
	}

	return dwLeaderPID;
}

void CShipDefenseManager::BroadcastAllianceHP(const LPCHARACTER c_lpAllianceChar, const LPSECTREE c_lpSectree)
{
	if (c_lpAllianceChar == nullptr || c_lpSectree == nullptr)
		return;

	TBroadcastAllianceHP BroadcastAllianceHP(c_lpAllianceChar);
	c_lpSectree->ForEachAround(BroadcastAllianceHP);
}

void CShipDefenseManager::SetAllianceHPPct(const LPCHARACTER c_lpRepairChar, const BYTE c_byPct)
{
	if (c_lpRepairChar == nullptr)
		return;

	const DWORD c_dwLeaderPID = GetLeaderPID(c_lpRepairChar, false/*isLeader*/);

	ShipDefenseMap::const_iterator it = m_mapShipDefense.find(c_dwLeaderPID);
	if (it != m_mapShipDefense.end())
	{
		CShipDefense* pShipDefense = it->second;
		if (pShipDefense == nullptr)
			return;

		const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
		if (c_lpSectreeMap == nullptr)
			return;

		LPCHARACTER lpUniqueChar = pShipDefense->GetUniqueCharacter(ShipDefense::UNIQUE_MAST_POS);
		if (lpUniqueChar == nullptr)
			return;

		// Set alliance health points.
		uint64_t iHP = (ShipDefense::WOOD_REPAIR_PCT * lpUniqueChar->GetMaxHP()) / 100;
		lpUniqueChar->SetHP(lpUniqueChar->GetHP() + iHP);

		// Notice players of alliance health points.
		TNotice Notice(pShipDefense, NOTICE_MAST_HP);
		c_lpSectreeMap->for_each(Notice);

		// Add stun affect.
		c_lpRepairChar->AddAffect(AFFECT_STUN, POINT_NONE, 0, AFF_STUN, ShipDefense::WOOD_REPAIR_STUN_DELAY, 0, true);

		// Broadcast alliance health points to players.
		TBroadcastAllianceHP BroadcastAllianceHP(lpUniqueChar);
		c_lpSectreeMap->for_each(BroadcastAllianceHP);
	}
}

bool CShipDefenseManager::Join(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return false;

	const LPPARTY c_lpParty = c_lpChar->GetParty();
	if (c_lpParty == nullptr)
		return false;

	if (m_mapShipDefense.empty())
		return false;

	ShipDefenseMap::const_iterator it = m_mapShipDefense.find(c_lpParty->GetLeaderPID());
	if (it != m_mapShipDefense.end())
	{
		const CShipDefense* c_pShipDefense = it->second;
		if (c_pShipDefense == nullptr)
			return false;

		if (!SECTREE_MANAGER::instance().GetMap(c_pShipDefense->GetMapIndex()))
		{
			sys_err("CShipDefenseManager::Join(c_lpChar=%p) SECTREE_MAP not found for #%ld", c_pShipDefense->GetMapIndex());
			return false;
		}

		PIXEL_POSITION SPos = { 0, 0, 0 };
		SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ShipDefense::EMapIndex::SHIP_MAP_INDEX, 0, SPos);

		c_lpChar->SaveExitLocation();
		c_lpChar->WarpSet(SPos.x, SPos.y, c_pShipDefense->GetMapIndex());
		return true;
	}

	return false;
}

void CShipDefenseManager::Leave(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return;

	PIXEL_POSITION SPos = { 0, 0, 0 };
	SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ShipDefense::EMapIndex::ENTRY_MAP_INDEX, 0, SPos);

	c_lpChar->SaveExitLocation();
	c_lpChar->WarpSet(SPos.x, SPos.y, ShipDefense::EMapIndex::ENTRY_MAP_INDEX);
}

void CShipDefenseManager::Land(const LPCHARACTER c_lpChar)
{
	if (c_lpChar == nullptr)
		return;

	const LPSECTREE_MAP c_lpSectreeTargetMap = SECTREE_MANAGER::instance().GetMap(ShipDefense::EMapIndex::PORT_MAP_INDEX);
	if (c_lpSectreeTargetMap != nullptr)
	{
		c_lpChar->SaveExitLocation();
		c_lpChar->WarpSet(c_lpSectreeTargetMap->m_setting.iBaseX + 405 * 100, c_lpSectreeTargetMap->m_setting.iBaseY + 480 * 100, ShipDefense::EMapIndex::PORT_MAP_INDEX);
	}
	else
	{
		PIXEL_POSITION SPos = { 0, 0, 0 };
		SECTREE_MANAGER::Instance().GetRecallPositionByEmpire(ShipDefense::EMapIndex::ENTRY_MAP_INDEX, 0, SPos);

		c_lpChar->SaveExitLocation();
		c_lpChar->WarpSet(SPos.x, SPos.y, ShipDefense::EMapIndex::ENTRY_MAP_INDEX);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Triggers
bool CShipDefenseManager::CanAttack(LPCHARACTER lpCharAttacker, LPCHARACTER lpCharVictim)
{
	if (lpCharAttacker == nullptr || lpCharVictim == nullptr)
		return false;

	if (CShipDefenseManager::Instance().IsFakeHydra(lpCharVictim->GetRaceNum()))
		return false;

	if (lpCharAttacker->IsMonster() && lpCharAttacker->IsNPC())
		if (lpCharVictim->GetRaceNum() == ShipDefense::EVNumHelper::SHIP_MAST)
			return true;

	return false;
}

bool CShipDefenseManager::OnKill(LPCHARACTER lpDeadChar, LPCHARACTER lpKillerChar)
{
	if (lpDeadChar == nullptr)
		return false;

	if (m_mapShipDefense.empty())
		return false;

	if (lpKillerChar != nullptr)
	{
		const DWORD c_dwLeaderPID = GetLeaderPID(lpKillerChar, false/*isLeader*/);

		ShipDefenseMap::iterator it = m_mapShipDefense.find(c_dwLeaderPID);
		if (it != m_mapShipDefense.end())
		{
			CShipDefense* pShipDefense = it->second;
			if (pShipDefense)
			{
				if (lpDeadChar == pShipDefense->GetUniqueCharacter(ShipDefense::UNIQUE_HYDRA_EGG_POS))
					pShipDefense->ClearDeck(ShipDefense::CLEAR_ALL_EXCEPT_BOSS);

				if (ShipDefense::SPAWN_WOOD_REPAIR == true)
				{
					if (IsMiniHydra(lpDeadChar->GetRaceNum()))
					{
						if (pShipDefense->IsUniqueMiniHydra(lpDeadChar) == true)
						{
							const LPSECTREE_MAP c_lpSectreeMap = pShipDefense->GetSectreeMap();
							if (c_lpSectreeMap != nullptr)
							{
								pShipDefense->Spawn(ShipDefense::EVNumHelper::WOOD_REPAIR,
									(lpDeadChar->GetX() - c_lpSectreeMap->m_setting.iBaseX) / 100,
									(lpDeadChar->GetY() - c_lpSectreeMap->m_setting.iBaseY) / 100,
									0
								);
							}
						}
					}
				}

				if (lpDeadChar == pShipDefense->GetUniqueCharacter(ShipDefense::UNIQUE_HYDRA_POS))
				{
					switch (pShipDefense->GetWave())
					{
					case ShipDefense::WAVE2:
						pShipDefense->SetShipEvent(ShipDefense::WAVE3);
						break;

					case ShipDefense::WAVE3:
						pShipDefense->SetShipEvent(ShipDefense::WAVE4);
						break;

					case ShipDefense::WAVE4:
						pShipDefense->SetShipEvent(ShipDefense::WAVES);
						break;
					}
				}
			}
		}
	}

	ShipDefenseMap::iterator it = m_mapShipDefense.begin();
	while (it != m_mapShipDefense.end())
	{
		CShipDefense* pShipDefense = it->second;
		if (pShipDefense)
			pShipDefense->DeadCharacter(lpDeadChar);
		++it;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// VNum Helpers
bool CShipDefenseManager::IsHydra(const DWORD c_dwVNum)
{
	switch (c_dwVNum)
	{
	case ShipDefense::EVNumHelper::HYDRA1:
	case ShipDefense::EVNumHelper::HYDRA2:
	case ShipDefense::EVNumHelper::HYDRA3:
		return true;
	}
	return false;
}

bool CShipDefenseManager::IsFakeHydra(const DWORD c_dwVNum)
{
	switch (c_dwVNum)
	{
	case ShipDefense::EVNumHelper::HYDRA_LEFT:
	case ShipDefense::EVNumHelper::HYDRA_RIGHT:
		return true;
	}
	return false;
}

bool CShipDefenseManager::IsMinion(const DWORD c_dwVNum)
{
	if (c_dwVNum >= 3401 && c_dwVNum <= 3605)
		return true;

	if (c_dwVNum >= 3950 && c_dwVNum <= 3959)
		return true;

	return false;
}

bool CShipDefenseManager::IsMast(const DWORD c_dwVNum)
{
	if (c_dwVNum == ShipDefense::EVNumHelper::SHIP_MAST)
		return true;
	return false;
}

bool CShipDefenseManager::IsMiniHydra(const DWORD c_dwVNum)
{
	switch (c_dwVNum)
	{
	case ShipDefense::EVNumHelper::MINI_HYDRA1:
	case ShipDefense::EVNumHelper::MINI_HYDRA2:
	case ShipDefense::EVNumHelper::MINI_HYDRA3:
		return true;
	}
	return false;
}

bool CShipDefenseManager::IsDungeon(const long c_lMapIndex)
{
	return c_lMapIndex >= ShipDefense::SHIP_MAP_INDEX * 10000 && c_lMapIndex < (ShipDefense::SHIP_MAP_INDEX + 1) * 10000;
}
#endif
