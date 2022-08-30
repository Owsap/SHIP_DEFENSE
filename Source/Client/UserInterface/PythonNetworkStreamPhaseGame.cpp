/// 1.
// Search @ bool CPythonNetworkStream::RecvTargetPacket
	CInstanceBase* pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();

// Add above
#if defined(ENABLE_SHIP_DEFENSE)
	if (TargetPacket.bAlliance)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPAllianceTargetBoard", Py_BuildValue("(iLL)", TargetPacket.dwVID, TargetPacket.iMinHP, TargetPacket.iMaxHP));
		return true;
	}
#endif
