/// 1.
// Search
	PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE3", CInstanceBase::EFFECT_PERCENT_DAMAGE3);

// Add below
#if defined(ENABLE_SHIP_DEFENSE)
	PyModule_AddIntConstant(poModule, "EFFECT_DEFENSE_WAVE_LASER", CInstanceBase::EFFECT_AFFECT + CInstanceBase::AFFECT_DEFENSE_WAVE_LASER);
#endif
