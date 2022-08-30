/// 1.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search @ WORD CHARACTER::GetMobAttackRange
	return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE);

// Add above
#if defined(__SHIP_DEFENSE__)
		// MOB (Hydra) attack range.
		if (CShipDefenseManager::Instance().IsHydra(m_pkMobData->m_table.dwVnum))
			return m_pkMobData->m_table.wAttackRange + GetPoint(POINT_BOW_DISTANCE) + 4000;
#endif

/// 3.
// Search @ WORD CHARACTER::GetMobAttackRange
		return m_pkMobData->m_table.wAttackRange;

// Add above
#if defined(__SHIP_DEFENSE__)
		// MOB melee attack range.
		if (CShipDefenseManager::Instance().IsMinion(m_pkMobData->m_table.dwVnum))
			return m_pkMobData->m_table.wAttackRange + 300;
#endif

/// 4.
// Search @ void CHARACTER::StartRecoveryEvent
	char_event_info* info = AllocEventInfo<char_event_info>();

// Add above
#if defined(__SHIP_DEFENSE__)
	if (CShipDefenseManager::Instance().IsMast(this->GetRaceNum()))
		return;
#endif

/// 5.
// Search @ void CHARACTER::ClearTarget
	p.dwVID = 0;

// Add below
#if defined(__SHIP_DEFENSE__)
	p.bAlliance = false;
	p.iAllianceMinHP = 0;
	p.iAllianceMaxHP = 0;
#endif

/// 6.
// Search @ void CHARACTER::SetTarget
	p.header = HEADER_GC_TARGET;

// Add below
#if defined(__SHIP_DEFENSE__)
	p.bAlliance = false;
	p.iAllianceMinHP = 0;
	p.iAllianceMaxHP = 0;
#endif

/// 7.
// Search @ void CHARACTER::BroadcastTargetPacket
	p.dwVID = GetVID();

// Add below
#if defined(__SHIP_DEFENSE__)
	p.bAlliance = false;
	p.iAllianceMinHP = 0;
	p.iAllianceMaxHP = 0;
#endif
