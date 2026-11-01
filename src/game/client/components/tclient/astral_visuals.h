// Astral Client - Visuals
// Custom hook effects, hammer hit shockwave, freeze lightning and world particles.
#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_ASTRAL_VISUALS_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_ASTRAL_VISUALS_H

#include <base/color.h>
#include <base/vmath.h>

#include <engine/shared/protocol.h>

#include <game/client/component.h>

#include <vector>

class CAstralVisuals : public CComponent
{
public:
	enum
	{
		HOOKFX_LIGHTNING = 0,
		HOOKFX_FIRE,
		HOOKFX_ICE,
		HOOKFX_BLACKHOLE,
		NUM_HOOKFX,
	};

	CAstralVisuals() = default;
	int Sizeof() const override { return sizeof(*this); }

	void OnReset() override;
	void OnRender() override;
	void OnNewSnapshot() override;

	// Called by CPlayers::RenderHook() every frame a hook is active (ClientId<0 for ghosts/other render paths is skipped by caller).
	void RenderHookFx(vec2 TeePos, vec2 HookPos, bool HookedTee, int ClientId, float Alpha);

	// Called from CGameClient when a NETEVENTTYPE_HAMMERHIT event is processed (real or predicted).
	void OnHammerHit(vec2 Pos);

private:
	// ---------------- shared shockwave ring effect (hammer hit & freeze lightning) ----------------
	struct SShockwave
	{
		vec2 m_Pos;
		float m_StartTime;
		float m_Duration;
		float m_MaxRadius;
		float m_Alpha;
	};
	std::vector<SShockwave> m_vShockwaves;
	void AddShockwave(vec2 Pos, float RadiusScale, float AlphaScale, float DurationSeconds);
	void RenderShockwaves();
	void SpawnHammerParticles(vec2 Pos);

	// ---------------- freeze lightning ----------------
	bool m_aWasFrozen[MAX_CLIENTS] = {};
	struct SFreezeBolt
	{
		vec2 m_Pos;
		float m_StartTime;
	};
	std::vector<SFreezeBolt> m_vFreezeBolts;
	void RenderFreezeBolts();

	// ---------------- world particles ----------------
	struct SWorldParticle
	{
		float m_NormX = 0.0f;
		float m_NormY = 0.0f;
		float m_SpeedMod = 1.0f;
		float m_SizeMod = 1.0f;
		float m_Hue = 0.0f;
		float m_Age = 0.0f;
		float m_LifeTime = 1.0f;
	};
	std::vector<SWorldParticle> m_vWorldParticles;
	void RespawnWorldParticle(SWorldParticle &Particle, bool RandomY);
	void UpdateWorldParticles(float Dt);
	void RenderWorldParticles();

	// ---------------- drawing helpers ----------------
	void RenderLightningBolt(vec2 From, vec2 To, ColorRGBA Color, int Seed, float Jitter);
	void RenderRingAt(vec2 Pos, float Radius, ColorRGBA Color, int Thickness);
};

#endif
