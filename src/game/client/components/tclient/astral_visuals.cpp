// Astral Client - Visuals
#include "astral_visuals.h"

#include <base/math.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <generated/client_data.h>

#include <game/client/components/particles.h>
#include <game/client/gameclient.h>

#include <algorithm>
#include <cstdint>

namespace
{
// Small deterministic hash so a bolt can be re-jittered every frame (giving it a
// "crackling" animated look) while still being reproducible within a single draw call.
float HashNoise(int Seed, int Index)
{
	uint32_t h = (uint32_t)(Seed * 374761393 + Index * 668265263);
	h = (h ^ (h >> 13)) * 1274126177u;
	h ^= (h >> 16);
	return (float)(h % 100000) / 100000.0f; // 0..1
}
} // namespace

void CAstralVisuals::OnReset()
{
	m_vShockwaves.clear();
	m_vFreezeBolts.clear();
	m_vWorldParticles.clear();
	for(bool &Frozen : m_aWasFrozen)
		Frozen = false;
}

// ================================================================================
// Shared helpers
// ================================================================================

void CAstralVisuals::RenderLightningBolt(vec2 From, vec2 To, ColorRGBA Color, int Seed, float Jitter)
{
	const float Len = distance(From, To);
	if(Len < 2.0f)
		return;

	const int Segments = std::clamp((int)(Len / 20.0f), 3, 14);
	const vec2 Dir = (To - From) / Len;
	const vec2 Normal(-Dir.y, Dir.x);

	Graphics()->TextureClear();
	Graphics()->LinesBegin();
	Graphics()->SetColor(Color);

	vec2 PrevPoint = From;
	std::vector<IGraphics::CLineItem> vLines;
	vLines.reserve(Segments);
	for(int i = 1; i <= Segments; i++)
	{
		const float t = (float)i / (float)Segments;
		vec2 Point = mix(From, To, t);
		// taper jitter to zero at both ends so the bolt always connects exactly
		const float Taper = std::sin(pi * t);
		const float Offset = (HashNoise(Seed, i) - 0.5f) * 2.0f * Jitter * Taper;
		if(i != Segments)
			Point += Normal * Offset;
		vLines.emplace_back(PrevPoint, Point);
		PrevPoint = Point;
	}
	Graphics()->LinesDraw(vLines.data(), vLines.size());
	Graphics()->LinesEnd();
}

void CAstralVisuals::RenderRingAt(vec2 Pos, float Radius, ColorRGBA Color, int Thickness)
{
	if(Radius <= 0.0f)
		return;

	const int Segments = std::clamp((int)(Radius / 4.0f), 12, 64);
	Thickness = std::clamp(Thickness, 1, 6);

	Graphics()->TextureClear();
	Graphics()->LinesBegin();
	for(int Layer = 0; Layer < Thickness; Layer++)
	{
		const float R = Radius + Layer * 1.5f;
		const float LayerAlphaMul = 1.0f - (float)Layer / (float)(Thickness + 1);
		Graphics()->SetColor(Color.WithAlpha(Color.a * LayerAlphaMul));
		std::vector<IGraphics::CLineItem> vLines;
		vLines.reserve(Segments);
		vec2 PrevPoint = Pos + direction(0.0f) * R;
		for(int i = 1; i <= Segments; i++)
		{
			const float Angle = 2.0f * pi * (float)i / (float)Segments;
			vec2 Point = Pos + direction(Angle) * R;
			vLines.emplace_back(PrevPoint, Point);
			PrevPoint = Point;
		}
		Graphics()->LinesDraw(vLines.data(), vLines.size());
	}
	Graphics()->LinesEnd();
}

// ================================================================================
// Custom Hook
// ================================================================================

void CAstralVisuals::RenderHookFx(vec2 TeePos, vec2 HookPos, bool HookedTee, int ClientId, float Alpha)
{
	if(!g_Config.m_AstralCustomHook)
		return;

	const float Now = LocalTime();
	const int Mode = g_Config.m_AstralCustomHookMode;
	const float Len = distance(TeePos, HookPos);
	if(Len < 1.0f)
		return;

	// Animation phase: changes a fixed number of times per second so the effect visibly
	// "crackles"/"flickers"/"pulses" instead of looking frozen on the target.
	const int AnimTick = (int)(Now * 18.0f);
	const int Seed = ClientId * 9176 + AnimTick;

	switch(Mode)
	{
	case CAstralVisuals::HOOKFX_LIGHTNING:
	{
		RenderLightningBolt(HookPos, TeePos, ColorRGBA(0.55f, 0.80f, 1.0f, Alpha * 0.85f), Seed, 11.0f);
		RenderLightningBolt(HookPos, TeePos, ColorRGBA(1.0f, 1.0f, 1.0f, Alpha * 0.55f), Seed + 4242, 5.0f);

		if(AnimTick % 2 == 0)
		{
			CParticle p;
			p.SetDefault();
			p.m_Spr = SPRITE_PART_SPARKLE;
			p.m_Pos = HookPos + random_direction() * random_float(8.0f);
			p.m_Vel = random_direction() * random_float(20.0f);
			p.m_LifeSpan = 0.2f;
			p.m_StartSize = random_float(10.0f, 16.0f);
			p.m_EndSize = 0.0f;
			p.m_UseAlphaFading = true;
			p.m_StartAlpha = Alpha;
			p.m_EndAlpha = 0.0f;
			p.m_Color = ColorRGBA(0.7f, 0.9f, 1.0f, 1.0f);
			p.m_Collides = false;
			// NOTE: SPRITE_PART_SPARKLE lives in the "extras" particle atlas, must use GROUP_EXTRA
			GameClient()->m_Particles.Add(CParticles::GROUP_EXTRA, &p);
		}
		if(HookedTee && AnimTick % 4 == 0)
		{
			RenderRingAt(HookPos, 14.0f + 4.0f * std::sin(Now * 20.0f), ColorRGBA(0.6f, 0.85f, 1.0f, Alpha * 0.5f), 2);
		}
		break;
	}
	case CAstralVisuals::HOOKFX_FIRE:
	{
		// flickering flame particles at the tip
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SLICE;
		p.m_Pos = HookPos + random_direction() * random_float(5.0f);
		p.m_Vel = vec2(random_float(-20.0f, 20.0f), -random_float(60.0f, 140.0f));
		p.m_LifeSpan = random_float(0.3f, 0.55f);
		p.m_StartSize = random_float(14.0f, 22.0f);
		p.m_EndSize = 0.0f;
		p.m_Gravity = -random_float(40.0f, 120.0f);
		p.m_Rot = random_angle();
		p.m_Rotspeed = pi;
		p.m_UseAlphaFading = true;
		p.m_StartAlpha = Alpha;
		p.m_EndAlpha = 0.0f;
		p.m_Color = mix(ColorRGBA(1.0f, 0.75f, 0.15f, 1.0f), ColorRGBA(1.0f, 0.25f, 0.05f, 1.0f), random_float());
		p.m_Collides = false;
		GameClient()->m_Particles.Add(CParticles::GROUP_GENERAL, &p);

		// embers scattered along the chain so the rope itself looks ablaze
		if(AnimTick % 2 == 0)
		{
			CParticle Smoke;
			Smoke.SetDefault();
			Smoke.m_Spr = SPRITE_PART_SMOKE;
			Smoke.m_Pos = mix(HookPos, TeePos, random_float());
			Smoke.m_Vel = vec2(0.0f, -random_float(30.0f, 70.0f));
			Smoke.m_LifeSpan = random_float(0.25f, 0.4f);
			Smoke.m_StartSize = random_float(10.0f, 16.0f);
			Smoke.m_EndSize = 0.0f;
			Smoke.m_Gravity = -60.0f;
			Smoke.m_UseAlphaFading = true;
			Smoke.m_StartAlpha = Alpha * 0.8f;
			Smoke.m_EndAlpha = 0.0f;
			Smoke.m_Color = ColorRGBA(1.0f, 0.5f, 0.1f, 1.0f);
			Smoke.m_Collides = false;
			GameClient()->m_Particles.Add(CParticles::GROUP_GENERAL, &Smoke);
		}
		break;
	}
	case CAstralVisuals::HOOKFX_ICE:
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SNOWFLAKE;
		p.m_Pos = HookPos + random_direction() * random_float(16.0f);
		p.m_Vel = random_direction() * random_float(16.0f) + vec2(0.0f, random_float(10.0f, 30.0f));
		p.m_LifeSpan = random_float(0.6f, 1.0f);
		p.m_StartSize = random_float(10.0f, 18.0f);
		p.m_EndSize = p.m_StartSize * 0.4f;
		p.m_Gravity = random_float(20.0f, 60.0f);
		p.m_Rot = random_angle();
		p.m_Rotspeed = pi * 0.5f;
		p.m_UseAlphaFading = true;
		p.m_StartAlpha = Alpha;
		p.m_EndAlpha = 0.0f;
		p.m_Color = ColorRGBA(0.75f, 0.9f, 1.0f, 1.0f);
		p.m_Collides = false;
		// NOTE: SPRITE_PART_SNOWFLAKE lives in the "extras" particle atlas, must use GROUP_EXTRA
		GameClient()->m_Particles.Add(CParticles::GROUP_EXTRA, &p);

		if(AnimTick % 3 == 0)
		{
			CParticle Shard;
			Shard.SetDefault();
			Shard.m_Spr = SPRITE_PART_SPARKLE;
			Shard.m_Pos = mix(HookPos, TeePos, random_float(0.0f, 0.3f));
			Shard.m_LifeSpan = 0.3f;
			Shard.m_StartSize = random_float(6.0f, 12.0f);
			Shard.m_EndSize = 0.0f;
			Shard.m_UseAlphaFading = true;
			Shard.m_StartAlpha = Alpha * 0.7f;
			Shard.m_EndAlpha = 0.0f;
			Shard.m_Color = ColorRGBA(0.6f, 0.85f, 1.0f, 1.0f);
			Shard.m_Collides = false;
			GameClient()->m_Particles.Add(CParticles::GROUP_EXTRA, &Shard);
		}
		break;
	}
	case CAstralVisuals::HOOKFX_BLACKHOLE:
	{
		// particles sucked inwards towards the hook tip
		const float Radius = random_float(28.0f, 60.0f);
		const vec2 Offset = random_direction() * Radius;
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_HECTAGON;
		p.m_Pos = HookPos + Offset;
		p.m_Vel = -Offset * random_float(3.0f, 6.0f);
		p.m_LifeSpan = random_float(0.3f, 0.5f);
		p.m_StartSize = random_float(10.0f, 18.0f);
		p.m_EndSize = 0.0f;
		p.m_Rot = random_angle();
		p.m_Rotspeed = pi * 2.0f;
		p.m_UseAlphaFading = true;
		p.m_StartAlpha = Alpha;
		p.m_EndAlpha = 0.0f;
		p.m_Color = ColorRGBA(0.45f, 0.15f, 0.65f, 1.0f);
		p.m_Collides = false;
		// NOTE: SPRITE_PART_HECTAGON lives in the "extras" particle atlas, must use GROUP_EXTRA
		GameClient()->m_Particles.Add(CParticles::GROUP_EXTRA, &p);

		// pulsing event-horizon ring at the tip
		const float Pulse = 10.0f + 3.0f * std::sin(Now * 10.0f);
		RenderRingAt(HookPos, Pulse, ColorRGBA(0.55f, 0.2f, 0.75f, Alpha * 0.6f), 2);
		break;
	}
	}
}

// ================================================================================
// Hammer Hit shockwave
// ================================================================================

void CAstralVisuals::AddShockwave(vec2 Pos, float RadiusScale, float AlphaScale, float DurationSeconds)
{
	SShockwave Wave;
	Wave.m_Pos = Pos;
	Wave.m_StartTime = LocalTime();
	Wave.m_Duration = std::max(0.05f, DurationSeconds);
	Wave.m_MaxRadius = 90.0f * std::max(0.1f, RadiusScale);
	Wave.m_Alpha = std::clamp(AlphaScale, 0.0f, 1.0f);
	m_vShockwaves.push_back(Wave);
}

void CAstralVisuals::SpawnHammerParticles(vec2 Pos)
{
	const int Count = std::clamp(g_Config.m_AstralHammerHitParticleCount, 0, 128);
	const float SizeScale = g_Config.m_AstralHammerHitParticleSize / 100.0f;
	const float AlphaScale = g_Config.m_AstralHammerHitParticleAlpha / 100.0f;

	for(int i = 0; i < Count; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_BALL;
		p.m_Pos = Pos;
		p.m_Vel = random_direction() * random_float(80.0f, 220.0f);
		p.m_LifeSpan = random_float(0.3f, 0.6f);
		p.m_StartSize = random_float(8.0f, 14.0f) * SizeScale;
		p.m_EndSize = 0.0f;
		p.m_Gravity = random_float(200.0f, 400.0f);
		p.m_Friction = 0.85f;
		p.m_UseAlphaFading = true;
		p.m_StartAlpha = AlphaScale;
		p.m_EndAlpha = 0.0f;
		p.m_Color = ColorRGBA(0.65f, 0.85f, 1.0f, 1.0f);
		GameClient()->m_Particles.Add(CParticles::GROUP_GENERAL, &p);
	}
}

void CAstralVisuals::OnHammerHit(vec2 Pos)
{
	if(!g_Config.m_AstralHammerHit)
		return;

	AddShockwave(Pos, g_Config.m_AstralHammerHitWaveSize / 100.0f, g_Config.m_AstralHammerHitWaveAlpha / 100.0f, g_Config.m_AstralHammerHitWaveTime / 10.0f);
	if(g_Config.m_AstralHammerHitParticles)
		SpawnHammerParticles(Pos);
}

void CAstralVisuals::RenderShockwaves()
{
	const float Now = LocalTime();
	for(auto It = m_vShockwaves.begin(); It != m_vShockwaves.end();)
	{
		const float Age = Now - It->m_StartTime;
		if(Age >= It->m_Duration)
		{
			It = m_vShockwaves.erase(It);
			continue;
		}
		const float Progress = Age / It->m_Duration; // 0..1
		const float Radius = It->m_MaxRadius * (0.15f + 0.85f * Progress);
		const float FadeAlpha = It->m_Alpha * (1.0f - Progress);
		RenderRingAt(It->m_Pos, Radius, ColorRGBA(0.75f, 0.9f, 1.0f, FadeAlpha), 2);
		++It;
	}
}

// ================================================================================
// Freeze lightning
// ================================================================================

void CAstralVisuals::RenderFreezeBolts()
{
	const float Now = LocalTime();
	const float BoltLifeTime = 0.35f;
	for(auto It = m_vFreezeBolts.begin(); It != m_vFreezeBolts.end();)
	{
		const float Age = Now - It->m_StartTime;
		if(Age >= BoltLifeTime)
		{
			It = m_vFreezeBolts.erase(It);
			continue;
		}
		const float Alpha = 1.0f - (Age / BoltLifeTime);
		const vec2 StrikeFrom = It->m_Pos + vec2(0.0f, -900.0f);
		const int Seed = (int)(It->m_StartTime * 1000.0f) + (int)(Now * 30.0f);
		RenderLightningBolt(StrikeFrom, It->m_Pos, ColorRGBA(0.6f, 0.85f, 1.0f, Alpha * 0.9f), Seed, 30.0f);
		RenderLightningBolt(StrikeFrom, It->m_Pos, ColorRGBA(1.0f, 1.0f, 1.0f, Alpha * 0.6f), Seed + 1337, 12.0f);
		++It;
	}
}

// ================================================================================
// World particles
// ================================================================================

void CAstralVisuals::RespawnWorldParticle(SWorldParticle &Particle, bool RandomY)
{
	Particle.m_NormX = random_float();
	Particle.m_NormY = RandomY ? random_float(-0.1f, 1.1f) : (g_Config.m_AstralWorldParticlesGravity == 0 ? -0.08f : random_float());
	Particle.m_SpeedMod = random_float(0.5f, 1.5f);
	Particle.m_SizeMod = random_float(0.6f, 1.4f);
	Particle.m_Hue = random_float();
	Particle.m_Age = 0.0f;
	Particle.m_LifeTime = random_float(3.0f, 7.0f);
}

void CAstralVisuals::UpdateWorldParticles(float Dt)
{
	const size_t WantedCount = (size_t)std::clamp(g_Config.m_AstralWorldParticlesCount, 0, 1000);
	if(m_vWorldParticles.size() < WantedCount)
	{
		const size_t OldSize = m_vWorldParticles.size();
		m_vWorldParticles.resize(WantedCount);
		for(size_t i = OldSize; i < WantedCount; i++)
			RespawnWorldParticle(m_vWorldParticles[i], true);
	}
	else if(m_vWorldParticles.size() > WantedCount)
	{
		m_vWorldParticles.resize(WantedCount);
	}

	const bool Levitate = g_Config.m_AstralWorldParticlesGravity != 0;
	const float FallSpeedScale = g_Config.m_AstralWorldParticlesFallSpeed / 100.0f;

	for(SWorldParticle &Particle : m_vWorldParticles)
	{
		if(!Levitate)
		{
			// falls down the screen and loops back to the top
			Particle.m_NormY += 0.09f * FallSpeedScale * Particle.m_SpeedMod * Dt;
			if(Particle.m_NormY > 1.1f)
				RespawnWorldParticle(Particle, false);
		}
		else
		{
			// drifts gently upward and evaporates (fades out) over its lifetime
			Particle.m_NormY -= 0.025f * Particle.m_SpeedMod * Dt;
			Particle.m_Age += Dt;
			if(Particle.m_Age >= Particle.m_LifeTime || Particle.m_NormY < -0.15f)
				RespawnWorldParticle(Particle, false);
		}
		Particle.m_Hue = std::fmod(Particle.m_Hue + Dt * 0.08f, 1.0f);
	}
}

void CAstralVisuals::RenderWorldParticles()
{
	if(m_vWorldParticles.empty())
		return;

	const CScreenRect ScreenRect = Graphics()->GetScreen();
	const vec2 ScreenSize = ScreenRect.m_BottomRight - ScreenRect.m_TopLeft;
	const bool Levitate = g_Config.m_AstralWorldParticlesGravity != 0;
	const float BaseSize = 6.0f * (g_Config.m_AstralWorldParticlesSize / 100.0f);
	const float BaseAlpha = g_Config.m_AstralWorldParticlesAlpha / 100.0f;

	Graphics()->TextureClear();
	for(const SWorldParticle &Particle : m_vWorldParticles)
	{
		float LifeFade = 1.0f;
		if(Levitate)
		{
			const float t = Particle.m_LifeTime > 0.0f ? Particle.m_Age / Particle.m_LifeTime : 1.0f;
			// fade in quickly, fade out ("evaporate") over the final third of its life
			LifeFade = std::min(t / 0.1f, 1.0f) * std::clamp(1.0f - (t - 0.66f) / 0.34f, 0.0f, 1.0f);
		}
		const float Alpha = BaseAlpha * LifeFade;
		if(Alpha <= 0.001f)
			continue;

		const vec2 Pos = ScreenRect.m_TopLeft + vec2(Particle.m_NormX * ScreenSize.x, Particle.m_NormY * ScreenSize.y);
		const float Size = BaseSize * Particle.m_SizeMod;
		const ColorRGBA Color = color_cast<ColorRGBA>(ColorHSLA(Particle.m_Hue, 0.85f, 0.65f)).WithAlpha(Alpha);

		Graphics()->QuadsBegin();
		Graphics()->SetColor(Color);
		Graphics()->QuadsSetRotation(pi / 4.0f);
		IGraphics::CQuadItem QuadItem(Pos.x, Pos.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
}

// ================================================================================
// Component overrides
// ================================================================================

void CAstralVisuals::OnNewSnapshot()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!GameClient()->m_Snap.m_aCharacters[i].m_Active)
		{
			m_aWasFrozen[i] = false;
			continue;
		}

		const bool Frozen = GameClient()->m_aClients[i].m_FreezeEnd != 0;
		if(Frozen && !m_aWasFrozen[i] && g_Config.m_AstralFreezeLightning)
		{
			const auto &Cur = GameClient()->m_Snap.m_aCharacters[i].m_Cur;
			const vec2 Pos(Cur.m_X, Cur.m_Y);
			m_vFreezeBolts.push_back({Pos, LocalTime()});
			// a couple of ring waves from the strike, like the hammer hit shockwave
			AddShockwave(Pos, 1.0f, 0.8f, 0.5f);
			AddShockwave(Pos, 1.5f, 0.5f, 0.7f);
		}
		m_aWasFrozen[i] = Frozen;
	}
}

void CAstralVisuals::OnRender()
{
	if(!m_vShockwaves.empty())
		RenderShockwaves();

	if(!m_vFreezeBolts.empty())
		RenderFreezeBolts();

	if(g_Config.m_AstralWorldParticles)
	{
		UpdateWorldParticles(Client()->RenderFrameTime());
		RenderWorldParticles();
	}
	else if(!m_vWorldParticles.empty())
	{
		m_vWorldParticles.clear();
	}
}
