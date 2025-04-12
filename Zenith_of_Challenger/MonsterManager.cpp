#include "MonsterManager.h"

//MonsterManager::~MonsterManager() {
//    for (auto m : m_monsters)
//        delete m;
//    m_monsters.clear();
//}
//
//void MonsterManager::SpawnMonster(NormalMonsterType type, float x, float y) {
//    Monster* monster = new NormalMonster(type);
//    monster->SetPosition(x, y);
//    m_monsters.push_back(monster);
//}
//
//void MonsterManager::SpawnBoss(float x, float y) {
//    Monster* boss = new BossMonster();
//    boss->SetPosition(x, y);
//    m_monsters.push_back(boss);
//}
//
//void MonsterManager::UpdateMonsters() {
//    for (auto m : m_monsters)
//        m->Update();
//}
//
//void MonsterManager::RenderMonsters() {
//    for (auto m : m_monsters) {
//        auto [x, y] = m->GetPosition();
//        std::cout << "[MONSTER] À§Ä¡: (" << x << ", " << y << ")\n";
//    }
//}
//
//void MonsterManager::RemoveDeadMonsters() {
//    m_monsters.erase(
//        std::remove_if(m_monsters.begin(), m_monsters.end(), [](Monster* m) {
//            if (m->IsDead()) {
//                delete m;
//                return true;
//            }
//            return false;
//            }),
//        m_monsters.end()
//    );
//}