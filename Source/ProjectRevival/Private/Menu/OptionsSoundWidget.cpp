// Project Revival. All Rights Reserved


#include "Menu/OptionsSoundWidget.h"

#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"
#include "Menu/MenuLevelTheme.h"

void UOptionsSoundWidget::NativeOnInitialized()
{
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UOptionsSoundWidget::OnMasterVolumeChange);
	}
	if (EffectVolumeSlider)
	{
		EffectVolumeSlider->OnValueChanged.AddDynamic(this, &UOptionsSoundWidget::OnEffectsVolumeChange);
	}
	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &UOptionsSoundWidget::OnMusicVolumeChange);
	}
	if (VoiceVolumeSlider)
	{
		VoiceVolumeSlider->OnValueChanged.AddDynamic(this, &UOptionsSoundWidget::OnVoiceVolumeChange);
	}
}

void UOptionsSoundWidget::OnMasterVolumeChange(float newValue)
{
	MasterVolumeSlider->Value = newValue;
	SetVolume(newValue, "Master");
}

void UOptionsSoundWidget::OnEffectsVolumeChange(float newValue)
{
	EffectVolumeSlider->Value = newValue;
	SetVolume(newValue, "Effects");
}

void UOptionsSoundWidget::OnMusicVolumeChange(float newValue)
{
	MusicVolumeSlider->Value = newValue;
	SetVolume(newValue, "Music");
}


void UOptionsSoundWidget::OnVoiceVolumeChange(float newValue)
{
	VoiceVolumeSlider->Value = newValue;
	SetVolume(newValue, "Voice");
}

void UOptionsSoundWidget::SetVolume(float newValue, FString WhatSound)
{
	TSubclassOf<AMenuLevelTheme> ClassToFind = AMenuLevelTheme::StaticClass();
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ClassToFind, OutActors);
	for (int EveryActor = 0; EveryActor < OutActors.Num(); EveryActor++)
	{
		Cast<AMenuLevelTheme>(OutActors[EveryActor])->ChangeVolume(newValue, WhatSound);
	}
}
