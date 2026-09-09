#pragma once

#include "CoreMinimal.h"

/** Canon palette from docs/art/PALETTE.md (Akitti + Prism). */
namespace FEPalette
{
	// Core atmosphere (dusk / M0)
	inline const FLinearColor NightSlate(0.043f, 0.063f, 0.125f);
	inline const FLinearColor WetAsphalt(0.102f, 0.122f, 0.180f);
	inline const FLinearColor ConcreteDust(0.541f, 0.518f, 0.565f);
	inline const FLinearColor Stucco(0.769f, 0.722f, 0.659f);
	inline const FLinearColor SickAmber(0.910f, 0.659f, 0.290f);
	inline const FLinearColor SodiumDusk(1.000f, 0.420f, 0.173f);
	inline const FLinearColor MagentaSign(1.000f, 0.176f, 0.584f);
	inline const FLinearColor CyanRig(0.176f, 0.886f, 0.902f);
	inline const FLinearColor ToxicLime(0.714f, 1.000f, 0.231f);
	inline const FLinearColor OxidizedTeal(0.122f, 0.435f, 0.416f);
	inline const FLinearColor Rust(0.478f, 0.180f, 0.180f);
	inline const FLinearColor CatCream(0.953f, 0.902f, 0.816f);

	// Neon accents
	inline const FLinearColor HotMagenta(1.000f, 0.063f, 0.565f);
	inline const FLinearColor CyanShock(0.000f, 0.961f, 1.000f);
	inline const FLinearColor IceBlue(0.490f, 0.976f, 1.000f);
	inline const FLinearColor WhiteHot(0.973f, 0.984f, 1.000f);
	inline const FLinearColor WarningAmber(1.000f, 0.820f, 0.000f);
	inline const FLinearColor BloodNeon(1.000f, 0.000f, 0.235f);
	inline const FLinearColor RadioactiveGreen(0.224f, 1.000f, 0.078f);

	// UI / kg
	inline const FLinearColor PanelVoid(0.055f, 0.078f, 0.125f);
	inline const FLinearColor PanelFace(0.102f, 0.133f, 0.200f);
	inline const FLinearColor PanelEdge(0.165f, 0.208f, 0.282f);
	inline const FLinearColor TextPrimary(0.910f, 0.933f, 0.961f);
	inline const FLinearColor TextMuted(0.541f, 0.588f, 0.659f);
	inline const FLinearColor KgOk(0.224f, 1.000f, 0.078f);
	inline const FLinearColor KgWarn(1.000f, 0.820f, 0.000f);
	inline const FLinearColor KgOver(1.000f, 0.000f, 0.235f);
	inline const FLinearColor HandsBusy(0.769f, 0.361f, 0.416f);
	inline const FLinearColor FocusCyan(0.000f, 0.961f, 1.000f);

	// Props
	inline const FLinearColor HerbSap(0.290f, 0.478f, 0.235f);
	inline const FLinearColor PottingSoil(0.231f, 0.165f, 0.118f);
	inline const FLinearColor BalconyGlass(0.659f, 0.769f, 0.831f);
	inline const FLinearColor LampPocket(0.910f, 0.831f, 0.722f);
	inline const FLinearColor Terracotta(0.620f, 0.340f, 0.220f);
	inline const FLinearColor WoodDesk(0.350f, 0.280f, 0.220f);
	inline const FLinearColor MetalRail(0.550f, 0.500f, 0.580f);
	inline const FLinearColor WarmConcrete(0.420f, 0.360f, 0.400f);

	inline FLinearColor EmissionAlbedo(const FLinearColor& C)
	{
		return FLinearColor(C.R * 0.30f, C.G * 0.30f, C.B * 0.30f, C.A);
	}
}
