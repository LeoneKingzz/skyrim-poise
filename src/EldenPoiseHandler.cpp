#include "../include/RE/B/BSTEvent.h"
#include "../include/SKSE/Events.h"
#include "RE/M/Misc.h"

class ParryHandler : public RE::BSTEventSink<SKSE::ModCallbackEvent>

{
public:
	static ParryHandler* GetSingleton()
	{
		static ParryHandler singleton;
		return &singleton;
	}

protected:
	using EventResult = RE::BSEventNotifyControl;
	float deliveranceDmgMod = .25;
	

	EventResult ProcessEvent(const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>*) override {
		if (!a_event) return EventResult::kContinue;
		RE::BSFixedString eventName = a_event->eventName;
		if (eventName != "EP_MeleeParryEvent" && eventName != "EP_RangedParryEvent") return EventResult::kContinue;

		const auto player = RE::PlayerCharacter::GetSingleton();
		RE::TESForm* eldenArmorSpell;

		// There are two levels of Elden Armor and we need to know which, if either, we have

		const auto eldenParryPerk_2 = RE::BGSPerk::LookupByEditorID("ORD_Bck20_TimedBlock_Perk_50_OrdASISExclude")->As<RE::BGSPerk>();
		const auto eldenParryPerk_1 = RE::BGSPerk::LookupByEditorID("ORD_Bck20_TimedBlock_Perk_20_OrdASISExclude")->As<RE::BGSPerk>();

		// Each perk has a different proc spell because they have different durations and you can't set duration after a spell is defined
		// If we have neither perk, return early

		if (player->HasPerk(eldenParryPerk_2)) {
			eldenArmorSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("ORD_Bck_TimedBlock_Spell_Proc_2");
		} else if (player->HasPerk(eldenParryPerk_1)) {
			eldenArmorSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("ORD_Bck_TimedBlock_Spell_Proc");
		} else {
			return EventResult::kContinue;
		}

		// Some behavior is shared between ranged and melee parries eg elden armor and deliverance

		doAnyParryProc(a_event, eldenArmorSpell, player);

		// Some behavior is unique to melee parries ie Poke the Dragon and Break Their Teeth (terrible name)

		if (eventName == "EP_MeleeParryEvent") {
			doMeleeParryProc(a_event, eldenArmorSpell, player);
		}

		return EventResult::kContinue;
		
	}

	void doAnyParryProc(const SKSE::ModCallbackEvent* a_event, RE::TESForm* eldenArmorSpell, RE::PlayerCharacter* player) {
		
		const auto caster = player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
		
		// Elden Armor

		recastImmediate(caster, eldenArmorSpell, true, player, 1, false, 45, player);


		// Deliverance?

		const auto deliverance = RE::BGSPerk::LookupByEditorID("ORD_Bck90_Deliverance_Perk_90_OrdASISExclude")->As<RE::BGSPerk>();
		if (player->HasPerk(deliverance)) {
			const auto deliveranceForm = RE::TESForm::LookupByEditorID("ORD_Bck_Deliverance_Spell_Proc");
			const auto deliveranceSpell = deliveranceForm->As<RE::MagicItem>();
			const auto deliveranceKeyword = RE::BGSKeyword::LookupByEditorID("ORD_Bck_Deliverance_Keyword")->As<RE::BGSKeyword>();
			if (player->HasMagicEffectWithKeyword(deliveranceKeyword)) {
				deliveranceDmgMod *= 2;
				recastImmediate(caster, deliveranceForm, true, player, 1, false, deliveranceDmgMod, player);
			} else {
				// set deliverance dmg mod to base (allows us to avoid OnEffectEnd listener)  
				deliveranceDmgMod = .25;
				caster->CastSpellImmediate(deliveranceSpell, true, player, 1, false, deliveranceDmgMod, player);
			}
		}
	}

	void doMeleeParryProc(const SKSE::ModCallbackEvent* a_event, RE::TESForm* eldenArmorSpell, RE::PlayerCharacter* player)
	{
		const auto attacker = a_event->sender->AsReference();
		const auto caster = player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);

		// Poke the Dragon?
		// 
		// (only applies if Deliverance does not)

		const auto pokeTheDragon = RE::BGSPerk::LookupByEditorID("ORD_Bck30_PokeTheDragon_Perk_30_OrdASISExclude")->As<RE::BGSPerk>();
		const auto deliveranceSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("ORD_Bck_Deliverance_Spell_Proc");
		const auto deliveranceKeyword = RE::BGSKeyword::LookupByEditorID("ORD_Bck_Deliverance_Keyword")->As<RE::BGSKeyword>();

		if (player->HasPerk(pokeTheDragon) && !player->HasMagicEffectWithKeyword(deliveranceKeyword)) {
			const auto pokeTheDragonSpell = RE::TESForm::LookupByEditorID("ORD_Bck_PokeTheDragon_Spell");
			recastImmediate(caster, pokeTheDragonSpell, true, attacker, 1, false, 25.0, player);
		}

		// Break Their Teeth?

		const auto breakTheirTeeth = RE::BGSPerk::LookupByEditorID("ORD_Bck80_BreakTheirTeeth_Perk_80_OrdASISExclude")->As<RE::BGSPerk>();
		if (player->HasPerk(breakTheirTeeth)) {
			const auto breakTheirTeethDisarmSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("ORD_Bck_BreakTheirTeeth_Disarm_Spell");
			const auto breakTheirTeethStaggerSpell = RE::TESForm::LookupByEditorID<RE::MagicItem>("ORD_Bck_BreakTheirTeeth_Stagger_Spell");

			caster->CastSpellImmediate(breakTheirTeethDisarmSpell, true, attacker, 1, false, 999, player);
			caster->CastSpellImmediate(breakTheirTeethStaggerSpell, true, player, 1, false, 1, player);
		}
	}

	// Ccast a spell on a target, dispelling any old instances of the spell's effect
	// Effectively refreshing the spell's duration
	void recastImmediate(RE::MagicCaster* caster, RE::TESForm* spellForm, bool a_noHitEffectArt, RE::TESObjectREFR* a_target, float a_effectiveness, bool a_hostileEffectivenessOnly, float a_magnitudeOverride, RE::Actor* a_blameActor) {
		dispelEffect(spellForm, a_target);
		caster->CastSpellImmediate(spellForm->As<RE::MagicItem>(), a_noHitEffectArt, a_target, a_effectiveness, a_hostileEffectivenessOnly, a_magnitudeOverride, a_blameActor);
	}

	void dispelEffect(RE::TESForm* spellForm, RE::TESObjectREFR * a_target)
	{
		const auto formSpell = spellForm->As<RE::MagicItem>();
		const auto targetActor = a_target->As<RE::Actor>();
		if (targetActor->HasMagicEffect(formSpell->effects[0]->baseEffect)) {
			auto activeEffects = targetActor->GetActiveEffectList();
			for (const auto& effect : *activeEffects) {
				if (effect->spell == formSpell)
					effect->Dispel(true);
			}
		}
	}

private:
	ParryHandler() = default;
	ParryHandler(const ParryHandler&) = delete;
	ParryHandler(ParryHandler&&) = delete;
	~ParryHandler() override = default;
	ParryHandler& operator=(const ParryHandler&) = delete;
	ParryHandler& operator=(ParryHandler&&) = delete;
};
