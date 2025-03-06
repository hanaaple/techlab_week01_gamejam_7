﻿#pragma once
#include <enum.h>
#include <unordered_map>
#include "Scene/MainGameScene.h"
#include "GameObject/Player.h"
#include "Manager/ObjectManager.h"
#include "MonsterSpawner.h"

const int LVUP_THRESHOLD = 100;

class GameManager;

struct PlayerState
{
	int Preset;
	int Score;
	int Exp;
	int Lv;
	int Hp;

	bool bIsAlive = true;
};

class GameLogic
{
private:
	std::unordered_map<EWorld , PlayerState> PlayerStates;

public:
	GameLogic();
	~GameLogic();

// Preset씬에서 실행
public:
	void Init();
	void SetPreset(EWorld World , int Preset)
	{
		PlayerStates[ World ].Preset = Preset;
	}

// MainGameScene에서 실행
public:

	int GetPreset(EWorld World);
	int GetLv(EWorld World);
	int GetScore(EWorld World);
	int GetExp(EWorld World);

	void AddScore(EWorld World , int Score);

	void AddExp(EWorld World , int exp);

	void SetLevel(EWorld World, int lv);

	void SpawnMonsterToWorld(EWorld World, int NumOfMonster);

	void EndGame(EWorld DeadPlayerWorld);

	void OnPlayerHit(EWorld World, int Damage);

	bool CanUseSpecialSkill(EWorld World);

	void UseSpecialSkill(EWorld World);

};

