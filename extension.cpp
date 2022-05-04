#include "extension.h"
#include "nextbot.h"
#include "nextbotutil.h"
#include <IStaticPropMgr.h>

CSurvivorBotBlockersFix g_SurvivorBotBlockersFix;

SMEXT_LINK(&g_SurvivorBotBlockersFix);

SH_DECL_MANUALHOOK2(MHook_NextBotTraversableTraceFilter_ShouldHitEntity, 0, 0, 0, bool, IHandleEntity *, int);

int INextBot::m_iVtbl_MySurvivorBotPointer = -1;

ICallWrapper *INextBot::m_pCallWrap_MySurvivorBotPointer = nullptr;

IStaticPropMgrServer *staticpropmgr = nullptr;

static CBaseEntity *EntityFromEntityHandle( IHandleEntity *pHandleEntity )
{
#ifdef CLIENT_DLL
	IClientUnknown *pUnk = (IClientUnknown*)pHandleEntity;
	return pUnk->GetBaseEntity();
#else
#ifndef _X360
	if ( staticpropmgr->IsStaticProp( pHandleEntity ) )
		return NULL;
#else
	if ( !pHandleEntity || pHandleEntity->m_bIsStaticProp )
		return NULL;
#endif

	IServerUnknown *pUnk = (IServerUnknown*)pHandleEntity;
	Assert( !pUnk || pUnk->GetBaseEntity() );
	return pUnk->GetBaseEntity();
#endif
}

bool CSurvivorBotBlockersFix::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	IGameConfig *pGameConfig;

	if (!gameconfs->LoadGameConfigFile(SMEXT_CONF_GAMEDATA_FILE, &pGameConfig, error, sizeof(error)))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to load gamedata file \"" SMEXT_CONF_GAMEDATA_FILE ".txt\"");

		return false;
	}

	if (!pGameConfig->GetOffset("INextBot::MySurvivorBotPointer", &INextBot::m_iVtbl_MySurvivorBotPointer))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to find gamedata offset entry for \"INextBot::MySurvivorBotPointer\"");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	if (!pGameConfig->GetAddress("NextBotTraversableTraceFilter vtable", &m_pfn_NextBotTraversableTraceFilter_vtable))
	{
		ke::SafeStrcpy(error, maxlen, "Unable to find gamedata address entry for \"NextBotTraversableTraceFilter vtable\"");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	if (m_pfn_NextBotTraversableTraceFilter_vtable == nullptr)
	{
		ke::SafeStrcpy(error, maxlen, "Unable to find signature in binary for gamedata entry \"NextBotTraversableTraceFilter vtable\"");

		gameconfs->CloseGameConfigFile(pGameConfig);

		return false;
	}

	gameconfs->CloseGameConfigFile(pGameConfig);

	m_nSHookID = SH_ADD_MANUALDVPHOOK(MHook_NextBotTraversableTraceFilter_ShouldHitEntity, m_pfn_NextBotTraversableTraceFilter_vtable,
		SH_MEMBER(this, &CSurvivorBotBlockersFix::Hook_NextBotTraversableTraceFilter_ShouldHitEntity), false);

	sharesys->AddDependency(myself, "bintools.ext", true, true);

	return true;
}

void CSurvivorBotBlockersFix::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, m_pBinTools);

#if SMINTERFACE_BINTOOLS_VERSION == 4
	PassInfo piReturn(PassType_Basic, PASSFLAG_BYVAL, sizeof(void *), nullptr, 0);
#else
	PassInfo piReturn;
	piReturn.type = PassType_Basic;
	piReturn.flags = PASSFLAG_BYVAL;
	piReturn.size = sizeof(void *);
#endif

	INextBot::m_pCallWrap_MySurvivorBotPointer = m_pBinTools->CreateVCall(INextBot::m_iVtbl_MySurvivorBotPointer, 0, 0, &piReturn, nullptr, 0);

	if (INextBot::m_pCallWrap_MySurvivorBotPointer == nullptr)
	{
		smutils->LogError(myself, "Unable to create vcall wrapper for \"INextBot::MySurvivorBotPointer\"");
	}
}

void CSurvivorBotBlockersFix::SDK_OnUnload()
{
	SH_REMOVE_HOOK_ID(m_nSHookID);
}

bool CSurvivorBotBlockersFix::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, staticpropmgr, IStaticPropMgrServer, INTERFACEVERSION_STATICPROPMGR_SERVER);

	return true;
}

bool CSurvivorBotBlockersFix::Hook_NextBotTraversableTraceFilter_ShouldHitEntity(IHandleEntity *pServerEntity, int fContentsMask)
{
	INextBot *pNextBot = META_IFACEPTR(NextBotTraversableTraceFilter)->m_bot;

	if (pNextBot->MySurvivorBotPointer())
	{
		CBaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);

		if (pEntity && (fContentsMask & CONTENTS_TEAM1))
		{
			const char *pszClassname = gamehelpers->GetEntityClassname(pEntity);

			if (!V_strcmp(pszClassname, "env_physics_blocker"))
			{
				RETURN_META_VALUE(MRES_SUPERCEDE, true);
			}

			if (!V_strcmp(pszClassname, "env_player_blocker"))
			{
				RETURN_META_VALUE(MRES_SUPERCEDE, true);
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, false);
}