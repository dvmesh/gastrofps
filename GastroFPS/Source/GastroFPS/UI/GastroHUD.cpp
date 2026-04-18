#include "UI/GastroHUD.h"
#include "GastroFPSCharacter.h"
#include "Core/GastroFPSGameState.h"
#include "Core/GastroTypes.h"
#include "Stations/Station.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

static void DrawTextBG(UCanvas* C, const FString& Text, float X, float Y, float Scale, FColor Color, FColor BgColor)
{
	UFont* Font = GEngine->GetMediumFont();
	if (!Font) return;
	float W, H;
	C->TextSize(Font, Text, W, H, Scale, Scale);
	if (BgColor.A > 0)
	{
		FCanvasTileItem Bg(FVector2D(X - 6, Y - 4), FVector2D(W + 12, H + 8), BgColor);
		Bg.BlendMode = SE_BLEND_Translucent;
		C->DrawItem(Bg);
	}
	FCanvasTextItem TextItem(FVector2D(X, Y), FText::FromString(Text), Font, Color);
	TextItem.Scale = FVector2D(Scale, Scale);
	C->DrawItem(TextItem);
}

FString AGastroHUD::PizzaName(uint8 Type) const
{
	switch ((EPizzaType)Type)
	{
	case EPizzaType::Margherita: return TEXT("Margherita");
	case EPizzaType::Pepperoni: return TEXT("Pepperoni");
	case EPizzaType::QuattroFormaggi: return TEXT("Quattro");
	case EPizzaType::Funghi: return TEXT("Funghi");
	case EPizzaType::Diavola: return TEXT("Diavola");
	case EPizzaType::Capricciosa: return TEXT("Capricciosa");
	default: return TEXT("?");
	}
}

void AGastroHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	APawn* P = GetOwningPawn();
	AGastroFPSCharacter* Char = Cast<AGastroFPSCharacter>(P);

	DrawTopBar(Char);
	DrawCrosshair();

	if (Char)
	{
		DrawInteractPrompt(Char);
		DrawPeekBubble(Char);
		DrawCarriedOrder(Char);
	}
	DrawMatchEnd();
}

void AGastroHUD::DrawTopBar(AGastroFPSCharacter* Char)
{
	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (!GS) return;

	const int32 TimeInt = FMath::Max(0, (int32)GS->TimeLeftSec);
	const int32 Mins = TimeInt / 60;
	const int32 Secs = TimeInt % 60;

	FString Line = FString::Printf(
		TEXT("$%d   %02d:%02d   OK:%d  FAIL:%d"),
		GS->Money, Mins, Secs, GS->OrdersCompleted, GS->OrdersFailed);

	DrawTextBG(Canvas, Line, 20.f, 20.f, 1.2f, FColor::White, FColor(0, 0, 0, 160));
}

void AGastroHUD::DrawCrosshair()
{
	const float CX = Canvas->SizeX * 0.5f;
	const float CY = Canvas->SizeY * 0.5f;
	FCanvasTileItem Dot(FVector2D(CX - 2.f, CY - 2.f), FVector2D(4.f, 4.f), FColor(255, 255, 255, 180));
	Dot.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Dot);
}

void AGastroHUD::DrawInteractPrompt(AGastroFPSCharacter* Char)
{
	AStation* Target = Char->GetCurrentInteractTarget();
	if (!Target) return;
	FText Prompt = Target->GetPromptText(Char);
	if (Prompt.IsEmpty()) return;

	const float CX = Canvas->SizeX * 0.5f;
	const float CY = Canvas->SizeY * 0.5f + 40.f;
	DrawTextBG(Canvas, Prompt.ToString(), CX - 180.f, CY, 1.1f, FColor::Yellow, FColor(0, 0, 0, 200));
}

void AGastroHUD::DrawPeekBubble(AGastroFPSCharacter* Char)
{
	const TArray<FOrderData>& Queue = Char->GetPeekedOrdersForHUD();
	if (Queue.Num() == 0) return;

	const float X = 20.f;
	const float Y = 70.f;

	// Tło listy
	const float LineH = 26.f;
	const float Height = 36.f + LineH * Queue.Num();
	FCanvasTileItem Bg(FVector2D(X - 10, Y - 10), FVector2D(380.f, Height), FColor(20, 20, 20, 210));
	Bg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Bg);

	// Header
	FString Head = FString::Printf(TEXT("W PAMIĘCI (%d):"), Queue.Num());
	DrawTextBG(Canvas, Head, X, Y, 1.0f, FColor::White, FColor(0, 0, 0, 0));

	// Lista: [T3] Margherita
	float ItemY = Y + 30.f;
	int32 Idx = 0;
	for (const FOrderData& Order : Queue)
	{
		FString PizzaText;
		for (const FOrderItem& It : Order.Items)
		{
			if (!PizzaText.IsEmpty()) PizzaText += TEXT(", ");
			PizzaText += PizzaName((uint8)It.Pizza);
		}
		FString Line = FString::Printf(
			TEXT("%s T%d: %s"),
			(Idx == 0 ? TEXT(">") : TEXT(" ")),  // strzałka na najstarsze (następne do POS)
			Order.TableNumber, *PizzaText);
		FColor C = (Idx == 0) ? FColor::Yellow : FColor(200, 200, 200, 255);
		DrawTextBG(Canvas, Line, X + 5.f, ItemY, 1.1f, C, FColor(0, 0, 0, 0));
		ItemY += LineH;
		++Idx;
	}
}

void AGastroHUD::DrawCarriedOrder(AGastroFPSCharacter* Char)
{
	if (!Char->IsCarryingPizza()) return;
	const FOrderData& Order = Char->GetCarriedOrderForHUD();

	FString PizzaText;
	for (const FOrderItem& It : Order.Items)
	{
		if (!PizzaText.IsEmpty()) PizzaText += TEXT(", ");
		PizzaText += PizzaName((uint8)It.Pizza);
	}

	FString Info = FString::Printf(
		TEXT("[W RĘKACH] → T%d: %s"),
		Order.TableNumber, *PizzaText);
	DrawTextBG(Canvas, Info, 20.f, Canvas->SizeY - 70.f, 1.3f, FColor::Green, FColor(0, 0, 0, 200));
}

void AGastroHUD::DrawMatchEnd()
{
	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (!GS) return;
	if (GS->bMatchActive) return;
	if (GS->TimeLeftSec > 0.f) return;

	const float CX = Canvas->SizeX * 0.5f - 200.f;
	const float CY = Canvas->SizeY * 0.5f - 60.f;
	FCanvasTileItem Bg(FVector2D(CX - 40, CY - 40), FVector2D(480.f, 180.f), FColor(0, 0, 0, 220));
	Bg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Bg);

	DrawTextBG(Canvas, TEXT("KONIEC SERWISU"), CX, CY, 1.6f, FColor::White, FColor(0, 0, 0, 0));
	FString Stats = FString::Printf(TEXT("Zarobki: $%d   OK:%d  FAIL:%d"),
		GS->Money, GS->OrdersCompleted, GS->OrdersFailed);
	DrawTextBG(Canvas, Stats, CX, CY + 50.f, 1.2f, FColor::Yellow, FColor(0, 0, 0, 0));
}
