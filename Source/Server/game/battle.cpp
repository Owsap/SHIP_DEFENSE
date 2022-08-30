/// 1.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search @ bool battle_is_attackable
	return CPVPManager::instance().CanAttack(ch, victim);

// Add above
#if defined(__SHIP_DEFENSE__)
	if (CShipDefenseManager::Instance().CanAttack(ch, victim))
		return true;

	if (CShipDefenseManager::Instance().IsFakeHydra(victim->GetRaceNum()))
		return false;
#endif

/// 3.
// Search @ int battle_melee_attack
		if (distance > max)

// Add above
#if defined(__SHIP_DEFENSE__)
		// PC melee attack on HUGE_RACE Hydra
		if (ch->IsPC() && CShipDefenseManager::Instance().IsHydra(victim->GetRaceNum()))
			max += 600;
#endif
