// ���� �÷��� ���� ����
#pragma once
#include "monster.h"

class MonsterManager {
private:
    std::vector<Monster*> m_monsters;

public:
    ~MonsterManager();

    void SpawnMonster(NormalMonsterType type, float x, float y);
    void SpawnBoss(float x, float y);
    void UpdateMonsters();
    void RenderMonsters();
    void RemoveDeadMonsters();

    const std::vector<Monster*>& GetMonsters() const { return m_monsters; }
};