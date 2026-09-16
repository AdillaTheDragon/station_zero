#ifndef STATION_ZERO_GAME_H_
#define STATION_ZERO_GAME_H_

#include <string>
#include <vector>

struct Item {
  std::string id;
  std::string name;
  std::string command_name;
  std::string type;
  int value = 0;
  std::string description;
};

struct Enemy {
  std::string id;
  std::string name;
  int hp = 1;
  int damage = 1;
  std::string description;
  bool boss = false;
};

struct Room {
  std::string id;
  std::string name;
  std::string description;
  std::string north;
  std::string south;
  std::string east;
  std::string west;
  std::vector<std::string> items;
  std::string enemy_id;
  int enemy_hp = 0;
  bool enemy_defeated = false;
};

struct UiText {
  std::string key;
  std::string text;
};

class Game {
 public:
  Game(std::string asset_folder);
  bool Load();
  void Run();

 private:
  std::string asset_folder_;
  std::vector<Item> items_;
  std::vector<Enemy> enemies_;
  std::vector<Room> rooms_;
  std::vector<UiText> ui_texts_;

  int hp_ = 40;
  int max_hp_ = 40;
  std::string room_id_ = "cell";
  std::vector<std::string> inventory_;
  std::string weapon_id_ = "pipe";

  bool power_on_ = false;
  bool lockdown_off_ = false;
  bool warden_defeated_ = false;
  bool running_ = true;
  bool won_ = false;

  bool LoadItems();
  bool LoadEnemies();
  bool LoadRooms();
  bool LoadUi();

  void ShowIntro();
  void ShowHelp();
  void ShowRoom();
  void ShowInventory();
  void ShowMap();
  void ExecuteCommand(std::string command);

  void Move(std::string direction);
  void Take(std::string item_name);
  void Use(std::string item_name);
  void Attack();

  int FindRoom(std::string id);
  int FindItem(std::string id);
  int FindEnemy(std::string id);
  int FindInventoryItemByCommand(std::string command_name);
  int FindRoomItemByCommand(std::string command_name);

  bool HasItem(std::string item_id);
  bool EnemyAlive();
  int WeaponDamage();
  void RemoveItem(std::string item_id);
  std::string GetUi(std::string key);
  std::string NormalizeCommand(std::string command);
  bool HasSpaces(std::string command);
  void CheckVictory();
};

#endif
