#include "game.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<std::string> Split(std::string text, char separator) {
  std::vector<std::string> result;
  std::stringstream stream(text);
  std::string part;

  while (std::getline(stream, part, separator)) {
    result.push_back(part);
  }

  return result;
}

std::string ReplaceNewLines(std::string text) {
  std::string result;

  for (int i = 0; i < (int)text.size(); i++) {
    if (text[i] == '\\' && i + 1 < (int)text.size() && text[i + 1] == 'n') {
      result += '\n';
      i++;
    } else {
      result += text[i];
    }
  }

  return result;
}

Game::Game(std::string asset_folder) {
  asset_folder_ = asset_folder;
}

bool Game::Load() {
  if (!LoadItems()) return false;
  if (!LoadEnemies()) return false;
  if (!LoadRooms()) return false;
  if (!LoadUi()) return false;

  inventory_.push_back("pipe");

  for (int i = 0; i < (int)rooms_.size(); i++) {
    if (rooms_[i].enemy_id != "") {
      int enemy_index = FindEnemy(rooms_[i].enemy_id);
      if (enemy_index != -1) {
        rooms_[i].enemy_hp = enemies_[enemy_index].hp;
      }
    }
  }

  if (FindRoom("cell") == -1) {
    std::cout << "Ошибка: стартовая комната не найдена.\n";
    return false;
  }

  return true;
}

bool Game::LoadItems() {
  std::ifstream file(asset_folder_ + "/items.csv");
  if (!file.is_open()) {
    std::cout << "Не удалось открыть items.csv\n";
    return false;
  }

  std::string line;
  std::getline(file, line);

  while (std::getline(file, line)) {
    if (line == "") continue;

    std::vector<std::string> cells = Split(line, ';');
    if (cells.size() < 6) continue;

    Item item;
    item.id = cells[0];
    item.name = cells[1];
    item.command_name = cells[2];
    item.type = cells[3];
    item.value = std::atoi(cells[4].c_str());
    item.description = cells[5];
    items_.push_back(item);
  }

  return true;
}

bool Game::LoadEnemies() {
  std::ifstream file(asset_folder_ + "/enemies.csv");
  if (!file.is_open()) {
    std::cout << "Не удалось открыть enemies.csv\n";
    return false;
  }

  std::string line;
  std::getline(file, line);

  while (std::getline(file, line)) {
    if (line == "") continue;

    std::vector<std::string> cells = Split(line, ';');
    if (cells.size() < 6) continue;

    Enemy enemy;
    enemy.id = cells[0];
    enemy.name = cells[1];
    enemy.hp = std::atoi(cells[2].c_str());
    enemy.damage = std::atoi(cells[3].c_str());
    enemy.description = cells[4];
    enemy.boss = cells[5] == "1";
    enemies_.push_back(enemy);
  }

  return true;
}

bool Game::LoadRooms() {
  std::ifstream file(asset_folder_ + "/rooms.csv");
  if (!file.is_open()) {
    std::cout << "Не удалось открыть rooms.csv\n";
    return false;
  }

  std::string line;
  std::getline(file, line);

  while (std::getline(file, line)) {
    if (line == "") continue;

    std::vector<std::string> cells = Split(line, ';');
    while (cells.size() < 9) cells.push_back("");

    Room room;
    room.id = cells[0];
    room.name = cells[1];
    room.description = cells[2];
    room.north = cells[3];
    room.south = cells[4];
    room.east = cells[5];
    room.west = cells[6];

    if (cells[7] != "") {
      room.items = Split(cells[7], '|');
    }

    room.enemy_id = cells[8];
    rooms_.push_back(room);
  }

  return true;
}

bool Game::LoadUi() {
  std::ifstream file(asset_folder_ + "/ui.csv");
  if (!file.is_open()) {
    std::cout << "Не удалось открыть ui.csv\n";
    return false;
  }

  std::string line;
  std::getline(file, line);

  while (std::getline(file, line)) {
    if (line == "") continue;

    int separator = (int)line.find(';');
    if (separator == -1) continue;

    UiText text;
    text.key = line.substr(0, separator);
    text.text = ReplaceNewLines(line.substr(separator + 1));
    ui_texts_.push_back(text);
  }

  return true;
}

void Game::Run() {
  ShowIntro();
  ShowRoom();

  while (running_) {
    std::cout << "\n> ";

    std::string command;
    if (!std::getline(std::cin, command)) {
      break;
    }

    ExecuteCommand(command);
  }
}

void Game::ShowIntro() {
  std::cout << GetUi("intro") << "\n";
}

void Game::ShowHelp() {
  std::cout << "\n" << GetUi("help") << "\n";
}

void Game::ShowRoom() {
  int room_index = FindRoom(room_id_);
  if (room_index == -1) return;

  Room& room = rooms_[room_index];

  std::cout << "\n----------------------------------------\n";
  std::cout << room.name << "\n";
  std::cout << "----------------------------------------\n";
  std::cout << room.description << "\n";

  if (EnemyAlive()) {
    int enemy_index = FindEnemy(room.enemy_id);
    if (enemy_index != -1) {
      std::cout << "\nВраг: " << enemies_[enemy_index].name;
      std::cout << " (здоровье: " << room.enemy_hp << "/" << enemies_[enemy_index].hp << ")\n";
      std::cout << enemies_[enemy_index].description << "\n";
    }
  }

  if (room.items.size() > 0) {
    std::cout << "\nПредметы:\n";
    for (int i = 0; i < (int)room.items.size(); i++) {
      int item_index = FindItem(room.items[i]);
      if (item_index != -1) {
        std::cout << "  - " << items_[item_index].name;
        std::cout << " [" << items_[item_index].command_name << "]\n";
      }
    }
  }

  std::cout << "\nВыходы:";
  if (room.north != "") std::cout << " north";
  if (room.south != "") std::cout << " south";
  if (room.east != "") std::cout << " east";
  if (room.west != "") std::cout << " west";
  std::cout << "\n";
}

void Game::ShowInventory() {
  std::cout << "\nИнвентарь:\n";

  if (inventory_.size() == 0) {
    std::cout << "  пусто\n";
  }

  for (int i = 0; i < (int)inventory_.size(); i++) {
    int item_index = FindItem(inventory_[i]);
    if (item_index == -1) continue;

    std::cout << "  - " << items_[item_index].name;
    if (inventory_[i] == weapon_id_) {
      std::cout << " (в руках)";
    }
    std::cout << "\n";
  }

  std::cout << "Здоровье: " << hp_ << "/" << max_hp_ << "\n";
}

void Game::ShowMap() {
  std::string cell = "[ ] Камера";
  std::string tunnel = "[ ] Тоннель";
  std::string storage = "[ ] Склад";
  std::string junction = "[ ] Коридор";
  std::string workshop = "[ ] Мастерская";
  std::string reactor = "[ ] Реактор";
  std::string checkpoint = "[ ] Пост";
  std::string laboratory = "[ ] Лаборатория";
  std::string security = "[ ] Комната охраны";
  std::string server = "[ ] Серверная";
  std::string command_center = "[ ] Командный центр";
  std::string elevator = "[ ] Лифт";
  std::string core_access = "[ ] К ядру";
  std::string core = "[ ] Ядро";
  std::string surface = "[ ] Поверхность";

  if (room_id_ == "cell") cell = "[*] Камера";
  if (room_id_ == "service_tunnel") tunnel = "[*] Тоннель";
  if (room_id_ == "storage") storage = "[*] Склад";
  if (room_id_ == "junction") junction = "[*] Коридор";
  if (room_id_ == "workshop") workshop = "[*] Мастерская";
  if (room_id_ == "reactor") reactor = "[*] Реактор";
  if (room_id_ == "checkpoint") checkpoint = "[*] Пост";
  if (room_id_ == "laboratory") laboratory = "[*] Лаборатория";
  if (room_id_ == "security_office") security = "[*] Комната охраны";
  if (room_id_ == "server_room") server = "[*] Серверная";
  if (room_id_ == "command_center") command_center = "[*] Командный центр";
  if (room_id_ == "elevator_lobby") elevator = "[*] Лифт";
  if (room_id_ == "core_access") core_access = "[*] К ядру";
  if (room_id_ == "core") core = "[*] Ядро";
  if (room_id_ == "surface") surface = "[*] Поверхность";

  std::cout << "\nКАРТА СТАНЦИИ\n";
  std::cout << "[*] - вы здесь\n";
  std::cout << "Север сверху, юг снизу, запад слева, восток справа.\n\n";

  std::cout << "                                                " << surface << "\n";
  std::cout << "                                                |\n";
  std::cout << "                                                " << core << "\n";
  std::cout << "                                                |\n";
  std::cout << "                                                " << core_access << "\n";
  std::cout << "                                                |\n";
  std::cout << "                                                " << elevator << "\n";
  std::cout << "                                                |\n";
  std::cout << "                                                " << command_center << "\n";
  std::cout << "                                                |\n";
  std::cout << reactor << "             " << checkpoint
            << " -------------- " << security << " ------------ " << server << "\n";
  std::cout << "|                       |                       |\n";
  std::cout << workshop << " -------- " << junction
            << " ----------- " << laboratory << "\n";
  std::cout << "                        |\n";
  std::cout << "                        " << storage << "\n";
  std::cout << "                        |\n";
  std::cout << "                        " << tunnel << "\n";
  std::cout << "                        |\n";
  std::cout << "                        " << cell << "\n";
}
void Game::ExecuteCommand(std::string command) {
  command = NormalizeCommand(command);

  if (command == "") {
    std::cout << "Вы ничего не ввели. Напишите help.\n";
    return;
  }

  if (HasSpaces(command)) {
    std::cout << "В команде не должно быть пробелов. Например: north или take_medkit.\n";
    return;
  }

  // Короткие версии команд.
  if (command == "h") command = "help";
  if (command == "l") command = "look";
  if (command == "n") command = "north";
  if (command == "s") command = "south";
  if (command == "e") command = "east";
  if (command == "w") command = "west";
  if (command == "i") command = "inventory";
  if (command == "m") command = "map";
  if (command == "a") command = "attack";
  if (command == "q") command = "quit";

  if (command.find("t_") == 0) {
    command = "take_" + command.substr(2);
  }

  if (command.find("u_") == 0) {
    command = "use_" + command.substr(2);
  }

  if (command == "help") {
    ShowHelp();
    return;
  }

  if (command == "look") {
    ShowRoom();
    return;
  }

  if (command == "inventory") {
    ShowInventory();
    return;
  }

  if (command == "map") {
    ShowMap();
    return;
  }

  if (command == "north" || command == "south" ||
      command == "east" || command == "west") {
    Move(command);
    return;
  }

  if (command == "attack") {
    Attack();
    return;
  }

  if (command == "quit") {
    running_ = false;
    std::cout << "Игра завершена.\n";
    return;
  }

  if (command == "take_" || command == "use_") {
    std::cout << "После нижнего подчёркивания нужно написать предмет.\n";
    return;
  }

  if (command.find("take_") == 0) {
    Take(command.substr(5));
    return;
  }

  if (command.find("use_") == 0) {
    Use(command.substr(4));
    return;
  }

  std::cout << "Неизвестная команда. Напишите help.\n";
}

void Game::Move(std::string direction) {
  if (EnemyAlive()) {
    std::cout << "Враг не даёт уйти. Сначала победите его.\n";
    return;
  }

  int room_index = FindRoom(room_id_);
  if (room_index == -1) return;

  Room& room = rooms_[room_index];
  std::string next_room = "";

  if (direction == "north") next_room = room.north;
  if (direction == "south") next_room = room.south;
  if (direction == "east") next_room = room.east;
  if (direction == "west") next_room = room.west;

  if (next_room == "") {
    std::cout << "В эту сторону прохода нет.\n";
    return;
  }

  if (room_id_ == "junction" && direction == "north" && !power_on_) {
    std::cout << GetUi("lock_power") << "\n";
    return;
  }

  if (room_id_ == "security_office" && direction == "east" && !HasItem("keycard")) {
    std::cout << GetUi("lock_keycard") << "\n";
    return;
  }

  if (room_id_ == "security_office" && direction == "north" && !lockdown_off_) {
    std::cout << GetUi("lock_lockdown") << "\n";
    return;
  }

  if (room_id_ == "elevator_lobby" && direction == "north") {
    if (!lockdown_off_ || !HasItem("elevator_key")) {
      std::cout << GetUi("lock_elevator") << "\n";
      return;
    }
  }

  if (room_id_ == "core" && direction == "north" && !warden_defeated_) {
    std::cout << GetUi("lock_warden") << "\n";
    return;
  }

  room_id_ = next_room;
  ShowRoom();
  CheckVictory();
}

void Game::Take(std::string item_name) {
  if (EnemyAlive()) {
    std::cout << "Сначала нужно победить врага.\n";
    return;
  }

  int room_item_position = FindRoomItemByCommand(item_name);
  if (room_item_position == -1) {
    std::cout << "Такого предмета здесь нет.\n";
    return;
  }

  int room_index = FindRoom(room_id_);
  Room& room = rooms_[room_index];
  std::string item_id = room.items[room_item_position];
  int item_index = FindItem(item_id);

  if (item_index == -1) return;

  inventory_.push_back(item_id);
  room.items.erase(room.items.begin() + room_item_position);

  std::cout << "Вы взяли: " << items_[item_index].name << ".\n";

  if (items_[item_index].type == "weapon") {
    if (items_[item_index].value > WeaponDamage()) {
      weapon_id_ = item_id;
      std::cout << "Новое оружие автоматически взято в руки.\n";
    }
  }
}

void Game::Use(std::string item_name) {
  if (EnemyAlive()) {
    std::cout << "Во время боя используйте attack.\n";
    return;
  }

  int inventory_position = FindInventoryItemByCommand(item_name);
  if (inventory_position == -1) {
    std::cout << "У вас нет такого предмета.\n";
    return;
  }

  std::string item_id = inventory_[inventory_position];
  int item_index = FindItem(item_id);
  if (item_index == -1) return;

  if (items_[item_index].type == "heal") {
    if (hp_ == max_hp_) {
      std::cout << "Здоровье уже полное.\n";
      return;
    }

    hp_ += items_[item_index].value;
    if (hp_ > max_hp_) hp_ = max_hp_;
    RemoveItem(item_id);
    std::cout << "Вы использовали аптечку. Здоровье: " << hp_ << "/" << max_hp_ << "\n";
    return;
  }

  if (item_id == "fuse") {
    if (room_id_ != "reactor") {
      std::cout << "Предохранитель нужно использовать у реактора.\n";
      return;
    }

    if (power_on_) {
      std::cout << "Реактор уже работает.\n";
      return;
    }

    power_on_ = true;
    RemoveItem(item_id);
    std::cout << GetUi("reactor_on") << "\n";
    return;
  }

  if (item_id == "admin_token") {
    if (room_id_ != "security_office") {
      std::cout << "Токен нужно использовать в комнате охраны.\n";
      return;
    }

    if (!power_on_) {
      std::cout << "Сначала нужно запустить реактор.\n";
      return;
    }

    if (lockdown_off_) {
      std::cout << "Блокировка уже отключена.\n";
      return;
    }

    lockdown_off_ = true;
    std::cout << GetUi("lockdown_off") << "\n";
    return;
  }

  std::cout << "Этот предмет нельзя использовать вручную.\n";
}

void Game::Attack() {
  if (!EnemyAlive()) {
    std::cout << "Здесь некого атаковать.\n";
    return;
  }

  int room_index = FindRoom(room_id_);
  Room& room = rooms_[room_index];
  int enemy_index = FindEnemy(room.enemy_id);
  if (enemy_index == -1) return;

  int damage = WeaponDamage();
  room.enemy_hp -= damage;
  std::cout << "Вы наносите " << damage << " урона.\n";

  if (room.enemy_hp <= 0) {
    room.enemy_hp = 0;
    room.enemy_defeated = true;
    std::cout << "Враг побеждён: " << enemies_[enemy_index].name << ".\n";

    if (enemies_[enemy_index].boss) {
      warden_defeated_ = true;
      std::cout << "Путь к выходу открыт.\n";
    }
    return;
  }

  hp_ -= enemies_[enemy_index].damage;
  std::cout << enemies_[enemy_index].name << " наносит вам ";
  std::cout << enemies_[enemy_index].damage << " урона.\n";

  if (hp_ <= 0) {
    hp_ = 0;
    running_ = false;
    std::cout << "\n*** ПОРАЖЕНИЕ ***\n";
    std::cout << "Вы погибли. Чтобы попробовать снова, запустите игру ещё раз.\n";
    return;
  }

  std::cout << "Здоровье: " << hp_ << "/" << max_hp_ << "\n";
}

int Game::FindRoom(std::string id) {
  for (int i = 0; i < (int)rooms_.size(); i++) {
    if (rooms_[i].id == id) return i;
  }
  return -1;
}

int Game::FindItem(std::string id) {
  for (int i = 0; i < (int)items_.size(); i++) {
    if (items_[i].id == id) return i;
  }
  return -1;
}

int Game::FindEnemy(std::string id) {
  for (int i = 0; i < (int)enemies_.size(); i++) {
    if (enemies_[i].id == id) return i;
  }
  return -1;
}

int Game::FindInventoryItemByCommand(std::string command_name) {
  for (int i = 0; i < (int)inventory_.size(); i++) {
    int item_index = FindItem(inventory_[i]);
    if (item_index != -1) {
      if (items_[item_index].command_name == command_name) return i;
    }
  }
  return -1;
}

int Game::FindRoomItemByCommand(std::string command_name) {
  int room_index = FindRoom(room_id_);
  if (room_index == -1) return -1;

  for (int i = 0; i < (int)rooms_[room_index].items.size(); i++) {
    int item_index = FindItem(rooms_[room_index].items[i]);
    if (item_index != -1) {
      if (items_[item_index].command_name == command_name) return i;
    }
  }
  return -1;
}

bool Game::HasItem(std::string item_id) {
  for (int i = 0; i < (int)inventory_.size(); i++) {
    if (inventory_[i] == item_id) return true;
  }
  return false;
}

bool Game::EnemyAlive() {
  int room_index = FindRoom(room_id_);
  if (room_index == -1) return false;

  if (rooms_[room_index].enemy_id == "") return false;
  if (rooms_[room_index].enemy_defeated) return false;
  return true;
}

int Game::WeaponDamage() {
  int item_index = FindItem(weapon_id_);
  if (item_index == -1) return 2;
  return items_[item_index].value;
}

void Game::RemoveItem(std::string item_id) {
  for (int i = 0; i < (int)inventory_.size(); i++) {
    if (inventory_[i] == item_id) {
      inventory_.erase(inventory_.begin() + i);
      return;
    }
  }
}

std::string Game::GetUi(std::string key) {
  for (int i = 0; i < (int)ui_texts_.size(); i++) {
    if (ui_texts_[i].key == key) return ui_texts_[i].text;
  }
  return "";
}

std::string Game::NormalizeCommand(std::string command) {
  while (command.size() > 0 &&
         (command[0] == ' ' || command[0] == '\t' || command[0] == '\r')) {
    command.erase(command.begin());
  }

  while (command.size() > 0 &&
         (command[command.size() - 1] == ' ' ||
          command[command.size() - 1] == '\t' ||
          command[command.size() - 1] == '\r')) {
    command.pop_back();
  }

  for (int i = 0; i < (int)command.size(); i++) {
    if (command[i] >= 'A' && command[i] <= 'Z') {
      command[i] = (char)(command[i] + 32);
    }
  }

  return command;
}

bool Game::HasSpaces(std::string command) {
  for (int i = 0; i < (int)command.size(); i++) {
    if (command[i] == ' ' || command[i] == '\t') return true;
  }
  return false;
}

void Game::CheckVictory() {
  if (room_id_ == "surface" && warden_defeated_) {
    won_ = true;
    running_ = false;
    std::cout << "\n*** ПОБЕДА ***\n";
    std::cout << GetUi("victory") << "\n";
  }
}
