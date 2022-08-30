/// 1.
// Search
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DRAGON_SOUL_DECK2", CInstanceBase::NEW_AFFECT_DRAGON_SOUL_DECK2);

// Add below
#if defined(ENABLE_SHIP_DEFENSE)
	PyModule_AddIntConstant(poModule, "NEW_AFFECT_DEFENSEWAVE_LASER", CInstanceBase::NEW_AFFECT_DEFENSEWAVE_LASER);
#endif
