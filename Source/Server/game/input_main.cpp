/// 1.
// Search @ void CInputMain::Target
		pckTarget.dwVID = p->dwVID;

// Add below
#if defined(__SHIP_DEFENSE__)
		pckTarget.bAlliance = false;
#endif
