/// 1.
// Search @ struct packet_target
	DWORD dwVID;

// Add below
#if defined(ENABLE_SHIP_DEFENSE)
	bool bAlliance;
	int64_t iAllianceMinHP, iAllianceMaxHP;
#endif
