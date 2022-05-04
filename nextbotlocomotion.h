#pragma once

struct ILocomotion
{
	enum TraverseWhenType
	{
		IMMEDIATELY,		// the entity will not block our motion - we'll carry right through
		EVENTUALLY			// the entity will block us until we spend effort to open/destroy it
	};
};