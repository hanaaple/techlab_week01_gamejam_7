﻿#pragma once

#include "GameObject/Bullet/Bullet.h"

// 기본 탄환
// 몬스터 피격시 삭제
// 데미지 및 반동?

class BulletA : public Bullet {
public:

    BulletA(EWorld selectedWorld);
    ~BulletA() = default;

    virtual void Update(float DeltaTime) override;
    virtual void FixedUpdate(float Fixed) override;

    virtual void HandleWallCollision(const FVector3& WallNormal) override;
    virtual void HandleBallCollision(CircleObject* OtherBall) override;

    virtual void Move(float DeltaTime) override;
    virtual void OnDestroy() override;

    // Bullet을(를) 통해 상속됨
    virtual void OnHit(FVector3 HitForce , int Damage) override;
};