/// 1.
// Search
	PyModule_AddIntConstant(poModule, "CAMERA_STOP", CPythonApplication::CAMERA_STOP);

// Add below
#if defined(ENABLE_SHIP_DEFENSE)
	PyModule_AddIntConstant(poModule, "ENABLE_SHIP_DEFENSE", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_SHIP_DEFENSE", 0);
#endif
