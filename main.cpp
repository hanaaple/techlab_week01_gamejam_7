﻿#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <Windows.h>
#include <iostream>
#include <string>

#include "ImGui/imgui.h"
#include "Imgui/imgui_internal.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "URenderer.h"
#include "GameObject/CircleObject.h"
#include "GameObject/Player.h"
#include "Manager/GameManager.h"
#include "Manager/ObjectManager.h"
#include "Scene/MainGameScene.h"
#include "Scene/TitleScene.h"
#include "Scene/PresetScene.h"
#include "Math/FVector3.h"

#include "InputSystem.h"
#include <Scene/PresetScene.h>

#include "Weapon/WeaponA.h"
#include "Weapon/WeaponB.h"

enum class EPrimitiveType : UINT8
{
	EPT_Triangle,
	EPT_Cube,
	EPT_Sphere,
	EPT_Max,
};


// ImGui WndProc 정의
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 각종 윈도우 관련 메시지(이벤트)를 처리하는 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // ImGui의 메시지를 처리
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
    {
        return true;
    }

    switch (uMsg)
    {
    // 창이 제거될 때 (창 닫기, Alt+F4 등)
    case WM_DESTROY:
        PostQuitMessage(0); // 프로그램 종료
        break;
	case WM_KEYDOWN:
		InputSystem::GetInstance().KeyDown(static_cast<EKeyCode>(wParam));
		break;
	case WM_KEYUP:
		InputSystem::GetInstance().KeyUp(static_cast<EKeyCode>( wParam ));
		break;
    case WM_LBUTTONDOWN:
        POINTS DownPts = MAKEPOINTS(lParam);
        RECT dRect;
        int dWidth , dHeight;
        if (GetClientRect(hWnd , &dRect)) {
            dWidth = dRect.right - dRect.left;
            dHeight = dRect.bottom - dRect.top;
        }
        InputSystem::GetInstance().MouseKeyDown(FVector3(DownPts.x , DownPts.y , 0), FVector3(dWidth , dHeight , 0));
        std::cout << "MouseDown " << InputSystem::GetInstance().GetMouseDownRatioPos().x << " " << InputSystem::GetInstance().GetMouseDownRatioPos().y << "\n";
        break;
    case WM_LBUTTONUP:
        POINTS UpPts = MAKEPOINTS(lParam);
        RECT uRect;
        int uWidth , uHeight;
        if (GetClientRect(hWnd , &uRect)) {
            uWidth = uRect.right - uRect.left;
            uHeight = uRect.bottom - uRect.top;
        }
        InputSystem::GetInstance().MouseKeyUp(FVector3(UpPts.x , UpPts.y , 0), FVector3(uWidth , uHeight , 0));
        //std::cout << "MouseUp" << InputSystem::GetInstance().GetMouseUpRatioPos().x << " " << InputSystem::GetInstance().GetMouseUpRatioPos().y << "\n";
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

void OpenDebugConsole()
{
	AllocConsole(); // 콘솔 창 생성

	// 표준 출력 및 입력을 콘솔과 연결
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

	std::cout << "Debug Console Opened!" << std::endl;
}

void CloseDebugConsole()
{
	FreeConsole(); // 콘솔 창 닫기
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
#pragma region Init Window
    // 사용 안하는 파라미터들
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    // 윈도우 클래스 이름 및 타이틀 이름
    constexpr WCHAR WndClassName[] = L"DX11 Test Window Class";
    constexpr WCHAR WndTitle[] = L"DX11 Test Window";

    // 윈도우 클래스 등록
    WNDCLASS WndClass = {0, WndProc, 0, 0, hInstance, nullptr, nullptr, nullptr, nullptr, WndClassName};
    RegisterClass(&WndClass);

    // 1024 x 1024로 윈도우 생성
    const HWND hWnd = CreateWindowEx(
        0, WndClassName, WndTitle,
        WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1024, 1024,
        nullptr, nullptr, hInstance, nullptr
    );

	OpenDebugConsole();
#pragma endregion Init Window
	
#pragma region Init Renderer & ImGui
    // 렌더러 초기화
    URenderer Renderer;
    Renderer.Create(hWnd);
    Renderer.CreateShader();
    Renderer.CreateConstantBuffer();

    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // ImGui Backend 초기화
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(Renderer.GetDevice(), Renderer.GetDeviceContext());
#pragma endregion Init Renderer & ImGui

    // FPS 제한
    constexpr int TargetFPS = 60;
    constexpr double TargetDeltaTime = 1000.0f / TargetFPS; // 1 FPS의 목표 시간 (ms)

    // 고성능 타이머 초기화
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);

    LARGE_INTEGER StartTime;
    QueryPerformanceCounter(&StartTime);

    float Accumulator = 0.0; // Fixed Update에 사용되는 값
    constexpr float FixedTimeStep = 1.0f / TargetFPS;

	GameManager::GetInstance().Init(&Renderer);

	ObjectManager& objectManager = ObjectManager::GetInstance();
	objectManager.Initialize(&Renderer);
	
	
    // Main Loop
    bool bIsExit = false;
    while (bIsExit == false)
    {
        // DeltaTime 계산 (초 단위)
        const LARGE_INTEGER EndTime = StartTime;
        QueryPerformanceCounter(&StartTime);

        const float DeltaTime =
            static_cast<float>(StartTime.QuadPart - EndTime.QuadPart) / static_cast<float>(Frequency.QuadPart);

        // 누적 시간 추가
        Accumulator += DeltaTime;

        InputSystem::GetInstance().ExpireOnceMouse();
        // 메시지(이벤트) 처리
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // 키 입력 메시지를 번역
            TranslateMessage(&msg);

            // 메시지를 등록한 Proc에 전달
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

    	// FixedTimeStep 만큼 업데이트
    	while (Accumulator >= FixedTimeStep)
    	{
			//FixedUpdate(FixedTimeStep); //마찬가지로 ObjectManager에 있는 FixedUpdate

    		// 공 충돌 처리 -> 마찬가지로 ObjectManager에서
    		//if (CircleObject::CheckCollision(*Balls[i], *Balls[j]))
    		//{
    		//	Balls[i]->HandleBallCollision(*Balls[j]);
    		//}

    		Accumulator -= FixedTimeStep;
    	}


        // 렌더링 준비 작업
        Renderer.Prepare();
        Renderer.PrepareShader();

    	GameManager::GetInstance().GetCurrentScene()->Update(DeltaTime);
		GameManager::GetInstance().GetCurrentScene()->Render();

		bool isPress = false;

		for(EKeyCode Ek : InputSystem::GetInstance().GetPressedKeys()) {
			//std::cout << static_cast<uint8_t>(Ek) << " ";
			isPress = true;
		}
		
		/*if(isPress)
			std::cout << "\n";*/

        // ImGui Frame 생성
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("DX11 Property Window");
        {
            ImGui::Text("Hello, World!");
            ImGui::Text("FPS: %.3f" , ImGui::GetIO().Framerate);

            ImGui::Text("Current Scene: %s" , GameManager::GetInstance().GetCurrentScene()->GetName().c_str());
            if (ImGui::Button("Title Scene")) {

                GameManager::GetInstance().ChangeScene<TitleScene>();
                GameManager::GetInstance().GetCurrentScene()->SetName("TitleScene");
            }

            if (ImGui::Button("Preset Scene")) {
                GameManager::GetInstance().ChangeScene<PresetScene>();
                GameManager::GetInstance().GetCurrentScene()->SetName("PresetScene");
            }

            if (ImGui::Button("Move to Main Game Scene"))
            {
                GameManager::GetInstance().ChangeScene<MainGameScene>();
                GameManager::GetInstance().GetCurrentScene()->SetName("MainGameScene");
            }

            MainGameScene* mainScene = GameManager::GetInstance().GetCurrentScene<MainGameScene>();
            if (mainScene != nullptr) {

                ImGui::Text("Score  Left: %d, Right: %d" , GameManager::GetInstance().GetLogic()->GetScore(EWorld::First), GameManager::GetInstance().GetLogic()->GetScore(EWorld::Second));


                if (ImGui::Button("Left use 10 Score")) {
                    GameManager::GetInstance().GetLogic()->SpawnMonsterToWorld(EWorld::Second , 100);
                    GameManager::GetInstance().GetLogic()->AddScore(EWorld::First , -10);
                }

                if (ImGui::Button("Right use 10 Score")) {
                        GameManager::GetInstance().GetLogic()->SpawnMonsterToWorld(EWorld::First, 100);
                    GameManager::GetInstance().GetLogic()->AddScore(EWorld::Second , -10);
                }

                ImGui::Text("Right (Second)");
                auto* rightPlayer = mainScene->GetPlayer(EWorld::Second);
                if (rightPlayer != nullptr) {
                    BaseWeapon* baseWeapon = rightPlayer->GetWeapon();
                    int level = GameManager::GetInstance().GetLogic()->GetLv(rightPlayer->GetWorld());
                    if (baseWeapon != nullptr) {
                        WeaponB* weaponB = dynamic_cast< WeaponB* >( baseWeapon );
                        if (weaponB != nullptr) {
                            if (ImGui::SliderFloat("speed" , &weaponB->WeaponData.AngularSpeed , 0.3 , 2)) {
                            }

                            if (ImGui::SliderFloat("bullet radius" , &weaponB->WeaponData.BulletRadius , 0.04 , 0.15)) {
                                // Radius에 맞게 변환
                                weaponB->UpdateRadius();
                            }

                            if (ImGui::SliderFloat("total radius" , &weaponB->WeaponData.TotalRadius , 0.03 , 0.5)) {

                            }
                            if (ImGui::SliderInt("NumOfBullets" , &weaponB->WeaponData.NumOfBullets , 1 , 10)) {
                                // 개수에 맞게 소환
                                weaponB->Clear();
                                weaponB->SpawnBullet(weaponB->WeaponData.NumOfBullets);
                            }

                            if (ImGui::SliderFloat("Force" , &weaponB->WeaponData.Force , 1 , 100)) {

                                // 개수에 맞게 소환
                                weaponB->UpdateForce();
                            }

                            if (ImGui::SliderInt("Level" , &level , 1 , 10)) {

                                // 개수에 맞게 소환
                                GameManager::GetInstance().GetLogic()->SetLevel(rightPlayer->GetWorld(), level);
                            }
                        }
                    }
                }



                ImGui::Text("Left (First)");
                auto* leftPlayer = mainScene->GetPlayer(EWorld::First);
                if (leftPlayer != nullptr) {
                    BaseWeapon* baseWeapon = leftPlayer->GetWeapon();
                    int level = GameManager::GetInstance().GetLogic()->GetLv(leftPlayer->GetWorld());
                    if (baseWeapon != nullptr) {
                        WeaponA* weaponA = dynamic_cast< WeaponA* >( baseWeapon );
                        if (weaponA != nullptr) {
                            if (ImGui::SliderFloat("A speed" , &weaponA->WeaponData.BulletSpeed , 0.3 , 2)) {
                            }

                            if (ImGui::SliderFloat("A bullet radius" , &weaponA->WeaponData.BulletSize , 0.04 , 0.15)) {
                            }

                            if (ImGui::SliderFloat("A Cooldown" , &weaponA->WeaponData.ShootCooldown , 0.03 , 1)) {
                            }

                            if (ImGui::SliderFloat("A Force" , &weaponA->WeaponData.Force , 1 , 100)) {

                            }

                            if (ImGui::SliderInt("A Level" , &level , 1 , 10)) {

                                GameManager::GetInstance().GetLogic()->SetLevel(leftPlayer->GetWorld() , level);
                            }
                        }
                    }
                }

                auto* spawner = mainScene->GetSpawner();
                ImGui::Text("Monster");
                float MonsterSpeed = spawner->GetMonsterSpeed();
                if (ImGui::SliderFloat("Monster Speed" , &MonsterSpeed , 1 , 5)) {
                    spawner->SetMonsterSpeed(MonsterSpeed);
                }

                float MonsterScale = spawner->GetMonsterScale();
                if (ImGui::SliderFloat("Monster Scale" , &MonsterScale , 0.5 , 2)) {
                    spawner->SetMonsterScale(MonsterScale);
                }

                float DefaultMonsterNum = spawner->GetDefaultMonsterNum();
                if (ImGui::SliderFloat("Default Monster Num" , &DefaultMonsterNum, 3 , 10)) {
                    spawner->SetDefaultMonsterNum(DefaultMonsterNum);
                }

                float SpawnRate = spawner->GetSpawnRate();
                if (ImGui::SliderFloat("Monster Spawn Rate" , &SpawnRate , 1 , 7)) {
                    spawner->SetSpawnRate(SpawnRate);
                }

                float MonsterIncreaseTime = spawner->GetMonsterIncreaseTime();
                if (ImGui::SliderFloat("Monster Increase Time" , &MonsterIncreaseTime , 2 , 10)) {
                    spawner->SetMonsterIncreaseTime(MonsterIncreaseTime);
                }

                int MonsterIncreaseNum = spawner->GetMonsterIncreaseNum();
                if (ImGui::SliderInt("Monster Increase Num" , &MonsterIncreaseNum , 1 , 10)) {
                    spawner->SetMonsterIncreaseNum(MonsterIncreaseNum);
                }
            }
        }
        ImGui::End();

        // ImGui 렌더링
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // 현재 화면에 보여지는 버퍼와 그리기 작업을 위한 버퍼를 서로 교환
        Renderer.SwapBuffer();


        // FPS 제한
        double ElapsedTime;
        do
        {
            Sleep(0);

            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);

            ElapsedTime = static_cast<double>(CurrentTime.QuadPart - StartTime.QuadPart) * 1000.0 / static_cast<double>(Frequency.QuadPart);
        } while (ElapsedTime < TargetDeltaTime);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    Renderer.ReleaseConstantBuffer();
    Renderer.ReleaseShader();
    Renderer.Release();

	CloseDebugConsole();
    return 0;
}


