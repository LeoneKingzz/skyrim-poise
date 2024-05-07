#include "../include/RE/B/BSTEvent.h"
#include "../include/SKSE/Events.h"
#include "RE/M/Misc.h"
#include "Hooks/PoiseAV.h"

class EldenPoiseHandler : public RE::BSTEventSink<SKSE::ModCallbackEvent>

{
public:
	static EldenPoiseHandler* GetSingleton()
	{
		static EldenPoiseHandler singleton;
		return &singleton;
	}

protected:
	using EventResult = RE::BSEventNotifyControl;

	

	EventResult ProcessEvent(const SKSE::ModCallbackEvent* a_event, RE::BSTEventSource<SKSE::ModCallbackEvent>*) override {
		if (!a_event) return EventResult::kContinue;
		RE::BSFixedString eventName = a_event->eventName;
		if (eventName != "EP_RipostePoiseEvent") return EventResult::kContinue;

		auto attacker = a_event->sender->As<RE::Actor>();
		auto EPpoiseDamage = a_event->numArg;
		

		if (eventName == "EP_RipostePoiseEvent") {
			auto poiseAV = PoiseAV::GetSingleton();
			poiseAV->DamageAndCheckPoise(attacker, nullptr, EPpoiseDamage);
		}

		return EventResult::kContinue;
	}


private:
	EldenPoiseHandler() = default;
	EldenPoiseHandler(const EldenPoiseHandler&) = delete;
	EldenPoiseHandler(EldenPoiseHandler&&) = delete;
	~EldenPoiseHandler() override = default;
	EldenPoiseHandler& operator=(const EldenPoiseHandler&) = delete;
	EldenPoiseHandler& operator=(EldenPoiseHandler&&) = delete;
};
