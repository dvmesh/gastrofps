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
	FCanvasTileItem Bg(FVector2D(X - 6, Y - 4), FVector2D(W + 12, H + 8), BgColor);
	Bg.BlendMode = SE_BLEND_Translucent;
	C->DrawItem(Bg);
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
	case EPizzaType::QuattroFormaggi: return TEXT("Quattro Formaggi");
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
	DrawTextBG(Canvas, Prompt.ToString(), CX - 140.f, CY, 1.1f, FColor::Yellow, FColor(0, 0, 0, 180));
}

void AGastroHUD::DrawPeekBubble(AGastroFPSCharacter* Char)
{
	if (!Char->HasPeekedOrder()) return;

	const FOrderData& Order = Char->GetPeekedOrderForHUD();
	const float TTL = Char->GetPeekedOrderRemainingSec();

	const float X = Canvas->SizeX * 0.5f - 220.f;
	const float Y = Canvas->SizeY * 0.5f - 200.f;

	// Bubble
	FCanvasTileItem Bg(FVector2D(X - 10, Y - 10), FVector2D(440.f, 120.f), FColor(20, 20, 20, 210));
	Bg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Bg);

	// Header
	FString Head = FString::Printf(TEXT("ZAMÓWIENIE #%d  (zapamiętaj: %.1fs)"), Order.OrderId, TTL);
	DrawTextBG(Canvas, Head, X, Y, 1.0f, FColor::White, FColor(0, 0, 0, 0));

	// Items
	float ItemY = Y + 35.f;
	for (const FOrderItem& It : Order.Items)
	{
		FString Line = FString::Printf(TEXT("- %s"), *PizzaName((uint8)It.Pizza));
		DrawTextBG(Canvas, Line, X + 10.f, ItemY, 1.3f, FColor::Yellow, FColor(0, 0, 0, 0));
		ItemY += 30.f;
	}

	// Timer bar
	FCanvasTileItem BarBg(FVector2D(X, Y + 95.f), FVector2D(420.f, 8.f), FColor(60, 60, 60, 255));
	BarBg.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BarBg);

	const float Frac = FMath::Clamp(TTL / 5.f, 0.f, 1.f);
	FCanvasTileItem Bar(FVector2D(X, Y + 95.f), FVector2D(420.f * Frac, 8.f), FColor(255, 200, 50, 255));
	Canvas->DrawItem(Bar);
}

void AGastroHUD::DrawCarriedOrder(AGastroFPSCharacter* Char)
{
	if (!Char->IsCarryingPizza()) return;
	const FOrderData& Order = Char->GetCarriedOrderForHUD();

	FString Info = FString::Printf(TEXT("[W RĘKACH] Zamówienie #%d"), Order.OrderId);
	for (const FOrderItem& It : Order.Items)
	{
		Info += FString::Printf(TEXT("  %s"), *PizzaName((uint8)It.Pizza));
	}
	DrawTextBG(Canvas, Info, 20.f, Canvas->SizeY - 60.f, 1.1f, FColor::Green, FColor(0, 0, 0, 180));
}

void AGastroHUD::DrawMatchEnd()
{
	AGastroFPSGameState* GS = GetWorld()->GetGameState<AGastroFPSGameState>();
	if (!GS) return;
	if (GS->bMatchActive) return;
	if (GS->TimeLeftSec > 0.f) return; // still pre-start

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
