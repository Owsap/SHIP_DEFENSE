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
	CShipDefenseManager::Instance().OnKill(this, pkKiller);
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
	if (CShipDefenseManager::Instance().IsMast(GetRaceNum()))
		CShipDefenseManager::Instance().BroadcastAllianceHP(this, GetSectree());
#endif
