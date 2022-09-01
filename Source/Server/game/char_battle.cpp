/// 1.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search
void CHARACTER::Dead
{
	[ . . . ]
}

// Add to the bottom of the function
#if defined(__SHIP_DEFENSE__)
	CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
	if (rkShipDefenseMgr.IsDungeon(this->GetMapIndex()))
		rkShipDefenseMgr.OnKill(this, pkKiller);
#endif

/// 3.
// Search @ bool CHARACTER::Damage
	if (IsMonster() && IsStoneSkinner())
	{
		if (GetHPPct() < GetMobTable().bStoneSkinPoint)
			dam /= 2;
	}

// Add below
#if defined(__SHIP_DEFENSE__)
	CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
	if (rkShipDefenseMgr.IsDungeon(this->GetMapIndex()) && rkShipDefenseMgr.IsMast(this->GetRaceNum()))
	{
		const LPSECTREE_MAP c_lpSectreeMap = SECTREE_MANAGER::instance().GetMap(this->GetMapIndex());
		if (c_lpSectreeMap != nullptr)
			CShipDefenseManager::Instance().BroadcastAllianceHP(this, c_lpSectreeMap);
	}
#endif
