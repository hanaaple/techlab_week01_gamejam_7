﻿#include "MainGameScene.h"

#include "Manager/GameManager.h"
#include "GameObject/Player.h"
#include "MonsterSpawner.h"
#include "Weapon/WeaponA.h"
#include "Weapon/WeaponB.h"
#include "InputSystem.h"

void MainGameScene::LoadScene()
{

	InputHandlerInstance = std::make_unique<InputHandler>();
  
	LeftPlayer = ObjectManager::GetInstance().RegistObject<Player>(First);
	WeaponA * leftWeapon = new WeaponA(LeftPlayer);

	LeftPlayer->SetWeapon(leftWeapon);
	
	RightPlayer = ObjectManager::GetInstance().RegistObject<Player>(Second);
	WeaponB* rightWeapon = new WeaponB(RightPlayer);

	RightPlayer->SetWeapon(rightWeapon);

	SpawnerInfo Info;

	Info.MonsterSpeed = 1.f;
	Info.MonsterScale = 1.f;
	Info.DefaultMonsterNum = 5;
	Info.SpawnRate = 10.0f;
	Info.MonsterIncreaseTime = 5.0f;
	Info.MonsterIncreaseNum = 1;

	Spawner = std::make_unique<MonsterSpawner>(Info);
}

void MainGameScene::ExitScene()
{
	ObjectManager::GetInstance().Destroy(LeftPlayer);
	ObjectManager::GetInstance().Destroy(RightPlayer);

	Spawner.reset();
}

void MainGameScene::Update(float DeltaTime)
{
	InputHandlerInstance->InputUpdate();
	Spawner->Update(DeltaTime);
	BaseScene::Update(DeltaTime);
}

void MainGameScene::Render()
{
	RenderWall(GameManager::GetInstance().GetRenderer());
}

Player* MainGameScene::GetPlayer(EWorld WorldType) const
{
	if (WorldType == EWorld::First)
	{
		return LeftPlayer;
	}
	else
	{
		return RightPlayer;
	}
}


FVertexSimple LineVertices[ ] = {
	{1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
	{1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
};

void MainGameScene::RenderWall(URenderer* Renderer) {

	ID3D11Buffer* VertexBufferLine = Renderer->CreateVertexBuffer(LineVertices , sizeof(LineVertices));
	int NumOfLineVertices = ARRAYSIZE(LineVertices);

	Renderer->PrepareViewport(EWorld::First);
	Renderer->UpdateConstant(FVector3() , 1.0f , 0.0f);
	Renderer->PrepareLine();
	Renderer->RenderPrimitive(VertexBufferLine , NumOfLineVertices);
}
