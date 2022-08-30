/// 1.
// Add
#if defined(__SHIP_DEFENSE__)
#	include "ShipDefense.h"
#endif

/// 2.
// Search @ int main
	CDragonLairManager dl_manager;

// Add below
#if defined(__SHIP_DEFENSE__)
	CShipDefenseManager ShipDefenseManager;
#endif

/// 3.
// Search @ int main
	sys_log(0, "<shutdown> Flushing TrafficProfiler...");
	trafficProfiler.Flush();

	destroy();

// Add above
	if (!g_bAuthServer)
	{
#if defined(__SHIP_DEFENSE__)
		sys_log(0, "<shutdown> Destroying ShipDefenseManager.");
		ShipDefenseManager.Destroy();
#endif
	}
