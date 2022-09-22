/**
* Filename: questlua_shipdefense_mgr.cpp
* Title: Ship Defense
* Description: Quest Ship Defense Manager
* Author: Owsap
* Date: 2022.08.05
*
* Discord: Owsap#7928
* Skype: owsap.
*
* Web: https://owsap.dev/
* GitHub: https://github.com/Owsap
**/

#include "stdafx.h"

#if defined(__SHIP_DEFENSE__)

#include "questlua.h"
#include "questmanager.h"
#include "ShipDefense.h"

namespace quest
{
	int ship_defense_mgr_create(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		if (!rkShipDefenseMgr.Create(c_lpChar))
		{
			sys_err("Failed to create ship defense instance.");
			return 0;
		}

		return 0;
	}

	int ship_defense_mgr_start(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		if (rkShipDefenseMgr.IsRunning(c_lpChar))
		{
			sys_err("Cannot start ship defense, already started.");
			return 0;
		}

		if (!rkShipDefenseMgr.Start(c_lpChar))
		{
			sys_err("Cannot start ship defense, instance not created.");
			return 0;
		}

		return 0;
	}

	int ship_defense_mgr_join(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		rkShipDefenseMgr.Join(c_lpChar);
		return 0;
	}

	int ship_defense_mgr_leave(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		rkShipDefenseMgr.Leave(c_lpChar);
		return 0;
	}

	int ship_defense_mgr_land(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		rkShipDefenseMgr.Land(c_lpChar);
		return 0;
	}

	int ship_defense_mgr_is_created(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		lua_pushboolean(L, rkShipDefenseMgr.IsCreated(c_lpChar));
		return 1;
	}

	int ship_defense_mgr_is_running(lua_State* L)
	{
		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		lua_pushboolean(L, rkShipDefenseMgr.IsRunning(c_lpChar));
		return 1;
	}

	int ship_defense_mgr_need_party(lua_State* L)
	{
		lua_pushboolean(L, ShipDefense::NEED_PARTY);
		return 1;
	}

	int ship_defense_mgr_need_ticket(lua_State* L)
	{
		lua_pushboolean(L, ShipDefense::NEED_TICKET);
		return 1;
	}

	int ship_defense_mgr_spawn_wood_repair(lua_State* L)
	{
		lua_pushboolean(L, ShipDefense::SPAWN_WOOD_REPAIR);
		return 1;
	}

	int ship_defense_mgr_require_cooldown(lua_State* L)
	{
		lua_pushboolean(L, ShipDefense::REQUIRE_COOLDOWN);
		return 1;
	}

	int ship_defense_mgr_set_alliance_hp_pct(lua_State* L)
	{
		if (!lua_isnumber(L, 1))
		{
			sys_err("Wrong argument.");
			return 0;
		}

		const LPCHARACTER c_lpChar = CQuestManager::instance().GetCurrentCharacterPtr();
		if (c_lpChar == nullptr)
			return 0;

		const BYTE c_byPct = static_cast<BYTE>(lua_tonumber(L, 1));

		CShipDefenseManager& rkShipDefenseMgr = CShipDefenseManager::Instance();
		rkShipDefenseMgr.SetAllianceHPPct(c_lpChar, c_byPct);
		return 0;
	}

	void RegisterShipDefenseManagerFunctionTable()
	{
		luaL_reg functions[] =
		{
			{ "create", ship_defense_mgr_create },
			{ "start", ship_defense_mgr_start },
			{ "join", ship_defense_mgr_join },
			{ "leave", ship_defense_mgr_leave },
			{ "land", ship_defense_mgr_land },
			{ "is_created", ship_defense_mgr_is_created },
			{ "is_running", ship_defense_mgr_is_running },
			{ "need_party", ship_defense_mgr_need_party },
			{ "need_ticket", ship_defense_mgr_need_ticket },
			{ "spawn_wood_repair", ship_defense_mgr_spawn_wood_repair },
			{ "require_cooldown", ship_defense_mgr_require_cooldown },
			{ "set_alliance_hp_pct", ship_defense_mgr_set_alliance_hp_pct },
			{ nullptr, nullptr }
		};

		CQuestManager::instance().AddLuaFunctionTable("ship_defense_mgr", functions);
	}
}
#endif
