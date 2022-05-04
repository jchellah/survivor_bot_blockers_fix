#pragma once

#include <IBinTools.h>

class SurvivorBot;
class INextBot;

class INextBot
{
public:
	static int m_iVtbl_MySurvivorBotPointer;

	static ICallWrapper *m_pCallWrap_MySurvivorBotPointer;

	SurvivorBot *MySurvivorBotPointer()
	{
		INextBot *pThis = this;

		SurvivorBot *pSurvivorBot;
		m_pCallWrap_MySurvivorBotPointer->Execute(&pThis, &pSurvivorBot);

		return pSurvivorBot;
	}
};