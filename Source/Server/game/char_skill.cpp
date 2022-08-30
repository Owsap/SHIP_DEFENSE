/// 1. If you have the knockback skill.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search @ 
			if (IS_SET(m_pkSk->dwFlag, SKILL_FLAG_KNOCKBACK))
			{
				float fKnockbackLength = 300; // Knockback distance.

				[ . . . ]

#if defined(__SHIP_DEFENSE__)
				if (CShipDefenseManager::Instance().IsMiniHydra(pkChrVictim->GetRaceNum()))
					fKnockbackLength = 0;
#endif
				[ . . . ]
			}
