// Proste HUD oparte na Canvas (nie UMG) — dla M1 wystarczy.
// Rysuje: pasek górny (pieniądze, czas, stats), interact prompt, peek bubble, carried info.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GastroHUD.generated.h"

class AGastroFPSCharacter;

UCLASS()
class GASTROFPS_API AGastroHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawTopBar(AGastroFPSCharacter* Char);
	void DrawCrosshair();
	void DrawInteractPrompt(AGastroFPSCharacter* Char);
	void DrawPeekBubble(AGastroFPSCharacter* Char);
	void DrawCarriedOrder(AGastroFPSCharacter* Char);
	void DrawMatchEnd();

	FString PizzaName(uint8 Type) const;
};
