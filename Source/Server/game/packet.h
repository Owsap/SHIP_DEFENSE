/// 1.
// Search @ struct packet_target
	DWORD dwVID;

// Add below
#if defined(__SHIP_DEFENSE__)
	bool bAlliance;
	int64_t iAllianceMinHP, iAllianceMaxHP;
#endif
