/// 1.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search @ void CHARACTER_MANAGER::DestroyCharacter
	if (m_bUsePendingDestroy)
	{
		m_set_pkChrPendingDestroy.insert(ch);
		return;
	}

// Add above
#if defined(__SHIP_DEFENSE__)
	// Delete monsters from the ship defense.
	if (ch->IsPC() == false)
		CShipDefenseManager::Instance().OnKill(ch);
#endif
