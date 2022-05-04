#pragma once

#include "ctracefiltersimple.h"
#include "nextbotlocomotion.h"

class INextBot;

/**
 * Trace filter that skips "traversable" entities.  The "when" argument creates
 * a temporal context for asking if an entity is IMMEDIATELY traversable (like thin
 * glass that just breaks as we walk through it) or EVENTUALLY traversable (like a
 * breakable object that will take some time to break through)
 */
struct NextBotTraversableTraceFilter : public CTraceFilterSimple
{
	INextBot *m_bot;
	ILocomotion::TraverseWhenType m_when;
};