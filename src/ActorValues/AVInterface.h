#pragma once
#include <functional>
#include <queue>
#include <stdint.h>

class AVInterface
{
public:
    virtual void  DamageAndCheckPoise(RE::Actor* a_target, RE::Actor* a_aggressor, float a_poiseDamage) noexcept = 0;
	virtual float GetBaseActorValue(RE::Actor* a_actor) = 0;
	float         GetBaseAV(RE::Actor* a_actor)
	{
		return GetBaseActorValue(a_actor);
	}

	virtual float GetActorValueMax(RE::Actor* a_actor) = 0;
	float         GetAVMax(RE::Actor* a_actor)
	{
		return GetActorValueMax(a_actor);
	}
};
