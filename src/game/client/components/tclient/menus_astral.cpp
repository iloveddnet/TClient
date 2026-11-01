// Astral Client - Settings menu (Visuals tab)
#include <engine/shared/config.h>

#include <game/client/components/menus.h>
#include <game/client/gameclient.h>
#include <game/client/ui.h>
#include <game/localization.h>

#include <vector>

namespace
{
enum
{
	ASTRAL_TAB_VISUALS = 0,
	NUMBER_OF_ASTRAL_TABS
};

constexpr float FontSize = 14.0f;
constexpr float LineSize = 20.0f;
constexpr float HeadlineFontSize = 20.0f;
constexpr float HeadlineHeight = HeadlineFontSize;
constexpr float Margin = 10.0f;
constexpr float MarginSmall = 5.0f;
constexpr float MarginBetweenViews = 30.0f;
} // namespace

void CMenus::RenderSettingsAstral(CUIRect MainView)
{
	static int s_CurAstralTab = ASTRAL_TAB_VISUALS;

	CUIRect TabBar, Button;
	MainView.HSplitTop(LineSize, &TabBar, &MainView);
	const float TabWidth = TabBar.w / NUMBER_OF_ASTRAL_TABS;
	static CButtonContainer s_aAstralTabs[NUMBER_OF_ASTRAL_TABS] = {};
	const char *apTabNames[] = {
		TCLocalize("Visuals")};

	for(int Tab = 0; Tab < NUMBER_OF_ASTRAL_TABS; ++Tab)
	{
		TabBar.VSplitLeft(TabWidth, &Button, &TabBar);
		const int Corners = Tab == 0 ? IGraphics::CORNER_L : Tab == NUMBER_OF_ASTRAL_TABS - 1 ? IGraphics::CORNER_R :
										     IGraphics::CORNER_NONE;
		if(DoButton_MenuTab(&s_aAstralTabs[Tab], apTabNames[Tab], s_CurAstralTab == Tab, &Button, Corners, nullptr, nullptr, nullptr, nullptr, 4.0f))
			s_CurAstralTab = Tab;
	}

	MainView.HSplitTop(Margin, nullptr, &MainView);

	if(s_CurAstralTab == ASTRAL_TAB_VISUALS)
		RenderSettingsAstralVisuals(MainView);
}

void CMenus::RenderSettingsAstralVisuals(CUIRect MainView)
{
	CUIRect LeftView, RightView, Button, Label;
	MainView.VSplitMid(&LeftView, &RightView, MarginBetweenViews);
	LeftView.VSplitLeft(MarginSmall, nullptr, &LeftView);
	RightView.VSplitRight(MarginSmall, &RightView, nullptr);

	// ============================== Left column ==============================

	// ---- Custom Hook ----
	LeftView.HSplitTop(HeadlineHeight, &Label, &LeftView);
	Ui()->DoLabel(&Label, TCLocalize("Custom Hook"), HeadlineFontSize, TEXTALIGN_ML);
	LeftView.HSplitTop(MarginSmall, nullptr, &LeftView);

	DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_AstralCustomHook, TCLocalize("Enable custom hook effects"), &g_Config.m_AstralCustomHook, &LeftView, LineSize);

	if(g_Config.m_AstralCustomHook)
	{
		static std::vector<CButtonContainer> s_vHookModeButtons = {{}, {}, {}, {}};
		int Mode = g_Config.m_AstralCustomHookMode;
		if(DoLine_RadioMenu(LeftView, TCLocalize("Hook effect"),
			   s_vHookModeButtons,
			   {TCLocalize("Lightning"), TCLocalize("Fire"), TCLocalize("Ice"), TCLocalize("Black Hole")},
			   {0, 1, 2, 3},
			   Mode))
		{
			g_Config.m_AstralCustomHookMode = Mode;
		}
	}

	LeftView.HSplitTop(MarginBetweenViews - Margin, nullptr, &LeftView);

	// ---- Hammer Hit ----
	LeftView.HSplitTop(HeadlineHeight, &Label, &LeftView);
	Ui()->DoLabel(&Label, TCLocalize("Hammer Hit"), HeadlineFontSize, TEXTALIGN_ML);
	LeftView.HSplitTop(MarginSmall, nullptr, &LeftView);

	DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_AstralHammerHit, TCLocalize("Enable hammer hit shockwave"), &g_Config.m_AstralHammerHit, &LeftView, LineSize);

	if(g_Config.m_AstralHammerHit)
	{
		LeftView.HSplitTop(LineSize, &Button, &LeftView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitWaveSize, &g_Config.m_AstralHammerHitWaveSize, &Button, TCLocalize("Wave size"), 10, 400, &CUi::ms_LinearScrollbarScale, 0, "%");
		LeftView.HSplitTop(LineSize, &Button, &LeftView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitWaveAlpha, &g_Config.m_AstralHammerHitWaveAlpha, &Button, TCLocalize("Wave opacity"), 0, 100, &CUi::ms_LinearScrollbarScale, 0, "%");
		LeftView.HSplitTop(LineSize, &Button, &LeftView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitWaveTime, &g_Config.m_AstralHammerHitWaveTime, &Button, TCLocalize("Wave duration"), 5, 300, &CUi::ms_LinearScrollbarScale, 0, " (x0.1s)");

		LeftView.HSplitTop(MarginSmall, nullptr, &LeftView);
		DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_AstralHammerHitParticles, TCLocalize("Enable hammer hit particles"), &g_Config.m_AstralHammerHitParticles, &LeftView, LineSize);

		if(g_Config.m_AstralHammerHitParticles)
		{
			LeftView.HSplitTop(LineSize, &Button, &LeftView);
			Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitParticleCount, &g_Config.m_AstralHammerHitParticleCount, &Button, TCLocalize("Particle count"), 0, 128);
			LeftView.HSplitTop(LineSize, &Button, &LeftView);
			Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitParticleSize, &g_Config.m_AstralHammerHitParticleSize, &Button, TCLocalize("Particle size"), 10, 400, &CUi::ms_LinearScrollbarScale, 0, "%");
			LeftView.HSplitTop(LineSize, &Button, &LeftView);
			Ui()->DoScrollbarOption(&g_Config.m_AstralHammerHitParticleAlpha, &g_Config.m_AstralHammerHitParticleAlpha, &Button, TCLocalize("Particle opacity"), 0, 100, &CUi::ms_LinearScrollbarScale, 0, "%");
		}
	}

	LeftView.HSplitTop(MarginBetweenViews - Margin, nullptr, &LeftView);

	// ---- Freeze Lightning ----
	LeftView.HSplitTop(HeadlineHeight, &Label, &LeftView);
	Ui()->DoLabel(&Label, TCLocalize("Freeze Lightning"), HeadlineFontSize, TEXTALIGN_ML);
	LeftView.HSplitTop(MarginSmall, nullptr, &LeftView);

	DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_AstralFreezeLightning, TCLocalize("Strike frozen players with lightning"), &g_Config.m_AstralFreezeLightning, &LeftView, LineSize);

	// ============================== Right column ==============================

	RightView.HSplitTop(HeadlineHeight, &Label, &RightView);
	Ui()->DoLabel(&Label, TCLocalize("World Particles"), HeadlineFontSize, TEXTALIGN_ML);
	RightView.HSplitTop(MarginSmall, nullptr, &RightView);

	DoButton_CheckBoxAutoVMarginAndSet(&g_Config.m_AstralWorldParticles, TCLocalize("Enable floating world particles"), &g_Config.m_AstralWorldParticles, &RightView, LineSize);

	if(g_Config.m_AstralWorldParticles)
	{
		RightView.HSplitTop(LineSize, &Button, &RightView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralWorldParticlesCount, &g_Config.m_AstralWorldParticlesCount, &Button, TCLocalize("Particle count"), 0, 1000);
		RightView.HSplitTop(LineSize, &Button, &RightView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralWorldParticlesSize, &g_Config.m_AstralWorldParticlesSize, &Button, TCLocalize("Particle size"), 10, 400, &CUi::ms_LinearScrollbarScale, 0, "%");
		RightView.HSplitTop(LineSize, &Button, &RightView);
		Ui()->DoScrollbarOption(&g_Config.m_AstralWorldParticlesAlpha, &g_Config.m_AstralWorldParticlesAlpha, &Button, TCLocalize("Particle opacity"), 0, 100, &CUi::ms_LinearScrollbarScale, 0, "%");

		RightView.HSplitTop(MarginSmall, nullptr, &RightView);
		static std::vector<CButtonContainer> s_vGravityButtons = {{}, {}};
		int Gravity = g_Config.m_AstralWorldParticlesGravity;
		if(DoLine_RadioMenu(RightView, TCLocalize("Gravity"),
			   s_vGravityButtons,
			   {TCLocalize("Falling"), TCLocalize("Levitate")},
			   {0, 1},
			   Gravity))
		{
			g_Config.m_AstralWorldParticlesGravity = Gravity;
		}

		if(g_Config.m_AstralWorldParticlesGravity == 0)
		{
			RightView.HSplitTop(LineSize, &Button, &RightView);
			Ui()->DoScrollbarOption(&g_Config.m_AstralWorldParticlesFallSpeed, &g_Config.m_AstralWorldParticlesFallSpeed, &Button, TCLocalize("Fall speed"), 10, 400, &CUi::ms_LinearScrollbarScale, 0, "%");
		}
	}
}
