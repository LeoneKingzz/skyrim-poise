#include "PoiseAVHUD.h"

float PoiseAVHUD::GetMaxSpecial([[maybe_unused]] RE::Actor* a_actor)
{
	if (a_actor) {
		auto manager = AVManager::GetSingleton();
		return manager->GetActorValueMax("Poise", a_actor);
	}
	return 1.0f;
}

float PoiseAVHUD::GetCurrentSpecial([[maybe_unused]] RE::Actor* a_actor)
{
	if (a_actor) {
		auto manager = AVManager::GetSingleton();
		return manager->GetActorValue("Poise", a_actor) + FLT_MIN;
	}
	return 1.0f;
}
