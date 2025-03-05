﻿#include "GameLogic.h"
#include "Manager/GameManager.h"

GameLogic::GameLogic()
{
}

GameLogic::~GameLogic()
{
}

void GameLogic::Init()
{
	PlayerStates[ First ] = { 0 , 0 , 0 , 1, true };
	PlayerStates[ Second ] = { 0 , 0 , 0 , 1, true };
}

int GameLogic::GetPreset(EWorld World)
{
	return PlayerStates[ World ].Preset;
}

void GameLogic::AddScore(EWorld World , int Score)
{
	PlayerStates[ World ].Score += Score;
}

void GameLogic::AddExp(EWorld World , int exp)
{
	PlayerStates[ World ].Exp += exp;
	if (PlayerStates[ World ].Exp >= LVUP_THRESHOLD)
	{
		Upgrade(World);
		PlayerStates[ World ].Exp -= LVUP_THRESHOLD;
	}
}

void GameLogic::Upgrade(EWorld World)
{
	MainGameScene* mainScene = GameManager::GetInstance().GetCurrentScene<MainGameScene>();

	if (mainScene == nullptr)
		return;
	Player* player = mainScene->GetPlayer(World);

	if (player != nullptr)
	{
		player->LevelUp();
	}
}

void GameLogic::SpawnMonsterToWorld(EWorld World)
{
	MainGameScene* mainScene = GameManager::GetInstance().GetCurrentScene<MainGameScene>();
	if (mainScene == nullptr)
		return;
	MonsterSpawner* spawner = mainScene->GetSpawner();
	if (spawner != nullptr)
	{
		spawner->SpawnToWorld(World);
	}
}

