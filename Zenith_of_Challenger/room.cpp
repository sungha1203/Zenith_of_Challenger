#include "room.h"

void Room::Init(int room_id)
{
	m_room_id = room_id;
	m_clients.clear();
}

void Room::AddClient(int client_id)
{
	m_clients.push_back(client_id);
}

void Room::RemoveClient(int client_id)
{
	m_clients.erase(remove(m_clients.begin(), m_clients.end(), client_id), m_clients.end());
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	--m_enterClientNum;
}

void Room::AllPlayerNum(int playerNum)
{
	m_playerNum = playerNum;
}

void Room::PlusPlayerReadyNum()
{
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	++m_enterClientNum;
}

void Room::PlusPlayerZenithReadyNum()
{
	std::lock_guard<std::mutex> lock(m_PlayerMx);
	++m_enterZenithNum;
}

void Room::PushStartGameButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID()) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}

	g_network.SendGameStart(GetClients());			// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������
	AllPlayerNum(m_clients.size());					// ���ӿ� ������ �÷��̾ ����̾�?

	m_IsGaming = true;
	std::cout << "[INFO] ��[" << m_room_id << "] ������ �����Ͽ����ϴ�!\n";
}

void Room::PushStartZenithButton(int RoomMasterID)
{
	if (RoomMasterID != GetRoomMasterID()) {
		std::cout << "[ERROR] Ŭ���̾�Ʈ [" << RoomMasterID << "] �� ������ �ƴϿ��� ������ �� �����ϴ�.\n";
		// TODO : Ŭ������ �� ���� �ƴϿ��� ���� ���Ѵٴ� �� ������� �ҵ�.(���߿�)
		return;
	}
	g_network.SendStartZenithStage(GetClients());			// ���ӹ� �ȿ� ���� ���� ������� ���ӽ��� ��Ŷ ������
}

void Room::StartGame()
{
	m_RoomState = Stage::CHALLENGE_STAGE;
	m_timer = std::thread(&Room::ChallengeTimerThread, this);
}

void Room::ChallengeTimerThread()
{
	// ���� �������� 8��
	std::this_thread::sleep_for(std::chrono::seconds(CHALLENGE_TIME));
	if (m_stopTimer) return;
	RepairTime();
}

void Room::StartZenithStage()
{
	if (m_timer.joinable()) 
		m_timer.join();

	m_RoomState = Stage::ZENITH_STAGE;
	m_timer = std::thread(&Room::ZenithTimerThread, this);
}

void Room::ZenithTimerThread() {
	// ���� �������� 5��
	std::this_thread::sleep_for(std::chrono::seconds(ZENITH_TIME));
	if (m_stopTimer) return;
	EndGame();
}

void Room::RepairTime()
{
	m_RoomState = Stage::REPAIR_TIME;
	g_network.SendStartRepairTime(GetClients());	// �ٵ� ������ ������ �̵�����
}

void Room::EndGame()
{
	if (m_timer.joinable()) {
		m_timer.join();
	}
	m_IsGaming = false;
	m_RoomState = Stage::LOBBY;

	std::cout << "[INFO][" << m_room_id << "]�� ������ �����Ͽ����ϴ�!\n";
	// TODO
}

void Room::AddGold(int plusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	m_inventory.gold += plusgold;
	g_network.SendUpdateInventory(GetClients());
	// update packet
}

void Room::SpendGold(int minusgold)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	if (m_inventory.gold > 0)
		m_inventory.gold -= minusgold;
	//else
		// �� �� �Ἥ ��ȭ �� ����
	// update packet
}

void Room::ADDJobWeapon(JobWeapon weapon)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobWeapons[weapon];
	// update packet
}

void Room::DecideJobWeapon(JobWeapon weapon)
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobWeapons[weapon];
	// update packet
}

void Room::AddJobDocument(JobDocument job)			
{
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	++m_inventory.JobDocuments[job];
	// update packet
}

void Room::DecideJobDocument(JobDocument job)		// Ŭ�� �ڵ忡�� �ѹ� �������� �ڱ� ���� �������ϱ� �ٲٱ� ������ ����� �ҵ�. (�ٸ� ������ Ŭ���� �ȵǰ� ����)
{													// �׸��� ����1 �������� ������ Ŭ������ �ȵǰ� Ŭ����ü���� ����. (�������� �ؾ��� ���� ���ʿ� ������)
	std::lock_guard<std::mutex> lock(m_inventoryMx);
	--m_inventory.JobDocuments[job];
	// update packet
}