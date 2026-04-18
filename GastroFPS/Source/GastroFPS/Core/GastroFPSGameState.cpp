#include "Core/GastroFPSGameState.h"

AGastroFPSGameState::AGastroFPSGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGastroFPSGameState::AddMoney(int32 Delta)
{
	Money += Delta;
}

void AGastroFPSGameState::IncrementOrdersCompleted()
{
	++OrdersCompleted;
}

void AGastroFPSGameState::IncrementOrdersFailed()
{
	++OrdersFailed;
}

void AGastroFPSGameState::StartMatch(float DurationSec)
{
	Money = 0;
	OrdersCompleted = 0;
	OrdersFailed = 0;
	TimeLeftSec = DurationSec;
	bMatchActive = true;
}

void AGastroFPSGameState::EndMatch()
{
	bMatchActive = false;
	TimeLeftSec = 0.f;
}

void AGastroFPSGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bMatchActive)
	{
		TimeLeftSec -= DeltaSeconds;
		if (TimeLeftSec <= 0.f)
		{
			EndMatch();
		}
	}
}
