// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject.h"
#include "../../ThirdParty/BiobraceMagic/Includes/serial.h"
#include "../../ThirdParty/BiobraceMagic/Includes/v8stdint.h"
#include "../../ThirdParty/BiobraceMagic/Includes/win.h"

#include <string>
#include <iostream>
#include <cstdio>

UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = PlayerMusicSkill)
int32 MusicSkillLevel;
class FMyProjectGameModule : public FDefaultGameModuleImpl
{
	
	serial::Serial *PointerToBiobraceMagic;
	//string port = "3";
	//string baud = "9600";
	//String

	/**
	* Called right after the module DLL has been loaded and the module object has been created
	*/
	virtual void StartupModule() override
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Some debug message!"));
		FString port;
		
		PointerToBiobraceMagic = new serial::Serial();

		PointerToBiobraceMagic->read();
	}

	/**
	* Called before the module is unloaded, right before the module object is destroyed.
	*/
	virtual void ShutdownModule()
	{
		PointerToBiobraceMagic->close();
		PointerToBiobraceMagic = NULL;
	}
};

// Override the default implementation with ours implementation =)
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, MyProject, "MyProject");