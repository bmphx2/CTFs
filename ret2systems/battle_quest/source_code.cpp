#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <array>

#include "prints.h"


#define HERO_COUNT 10
#define SPELL_COUNT 10

typedef uint64_t Spell;

class Hero;

class Game {
    public:
    Game(): spell_book(), heroes(), total_heroes(0) {}

    auto& get_all_heroes() {
        return heroes;
    }

    Hero* get_hero(const int index) {
        return heroes[index-1];
    }

    const unsigned int get_total_heroes() {
        return total_heroes;
    }

    bool join_party(Hero* new_hero) {
        if (total_heroes >= 10)
            return false;

        heroes[total_heroes++] = new_hero;
        return true;
    }

    bool add_spell(Spell new_spell) {
        if (total_spells >= 10) {
            return false;
        }
        spell_book[total_spells++] = new_spell;
        return true;
    }

    auto& get_all_spells() {
        return spell_book;
    }

    const Spell get_spell(const int index) {
        return spell_book[index];
    }

    const unsigned int get_total_spells() {
        return total_spells;
    }

    void heal_all(uint64_t health);

    Hero* get_enemy() {
        return enemy;
    }

    void set_enemy(Hero* new_enemy) {
        enemy = new_enemy;
    }

    void do_battle(Hero*);

    void player_turn();

    private:
    std::array<Spell, SPELL_COUNT> spell_book;
    std::array<Hero*, HERO_COUNT> heroes;

    unsigned int total_heroes;
    unsigned int total_spells;
    Hero* enemy;
};

class Hero {
    public:
    Hero(const std::string action_name_, const std::string action_description_,
            const uint64_t attack_power_, const uint64_t healing_power_,
        uint64_t health_=100):
        attack_power(attack_power_), healing_power(healing_power_),
        action_name(action_name_), action_description(action_description_)
    {
        name = new char[32];
        health = health_;
    }

    ~Hero() {
        delete name;
    }

    const char* get_name() {
        return name;
    }

    void set_name(std::string new_name) {
        new_name.copy(name, 31);
        new_name[std::min(new_name.size(), 31ul)] = '\0';
    }

    const uint64_t get_health() {
        return health;
    }

    void heal(const uint64_t amount) {
        health = std::min(health+amount, 100ul);
    }

    void damage(const uint64_t amount) {
        if (amount > health)
            health = 0;
        else
            health -= amount;
    }


    void do_hero_turn(Game*, bool);
    void do_enemy_turn(Game*);

    void do_attack(Game*, Hero* target);

    void do_heal(Game*);

    virtual void do_action(Game*) {}

    private:
    char* name;
    uint64_t attack_power;
    uint64_t healing_power;
    uint64_t health;

    const std::string action_name;
    const std::string action_description;

    protected:
    uint64_t flags;
};


bool yes_or_no(const std::string msg) {
    std::cout << msg << " [yes/no]" << std::endl;
    std::string response;
    std::cin >> response;
    if (!std::cin.good())
        exit(-1);

    return tolower(response[0]) == 'y';
}

void wait_for_enter() {
    std::cout << "[ press enter to continue ]" << std::endl;
    std::cin.ignore(256, '\n');
}


class Bard : public Hero {
    public:
    Bard(): Hero("Musical Charm","Your music is so beautiful, the enemy may join your party", 10, 25) {
        set_name("Bard");
    }

    void do_action(Game* game) {
        Hero* enemy = game->get_enemy();
        if (enemy->get_health() <= 10) {
            std::cout << "\n,-------------------------------\n"
                        << "| Charmed by your music, " << enemy->get_name()
                      << "\n| decides to join your party!\n"
                        << "'-------------------------------\n" << std::endl;
            game->set_enemy(NULL);
            game->join_party(enemy);
            return;
        }
        std::cout << "| " << enemy->get_name() << " looks at you fiercely. You mess up a few notes out of fear" << std::endl;
    }
};

class Wizard : public Hero {
    public:
    Wizard(): Hero("Spell Master","Learn or use spells", 1, 35) {
        set_name("Wizard");
    }

    void do_action(Game* game) {
        Spell spell = 0;

        uint64_t attack_amount = 0;
        uint64_t heal_amount = 0;
        uint64_t cost = 0;
        bool group = false;

        bool new_spell;
        while (true) {
            new_spell = yes_or_no("| Learn a new spell?");


            if (new_spell) {
                std::cout << "|\n| Enter Spell to try:" << std::endl;

                std::ios_base::fmtflags f( std::cin.flags() ); // Save flags
                std::cin >> std::hex >> spell;
                if (!std::cin.good())
                    exit(-1);
                std::cin.flags( f );  // Reset flags

                std::cin.ignore(256, '\n');
            } else {
                if (game->get_total_spells() > 0) {
                    std::cout << "| Select page of spell book (1-" << game->get_total_spells() << ")" << std::endl;
                    unsigned int page;
                    std::cin >> page;
                    if (!std::cin.good())
                        exit(-1);
                    std::cin.ignore(256, '\n');

                    spell = game->get_spell(page-1);
                }
            }

            if (spell == 0) {
                if (new_spell)
                    std::cout << "| The wizard tried to conjure a new spell from the air but could not find her voice" << std::endl;
                else
                    std::cout << "| The wizard desperately flipped though her spell book but couldn't find anything useful" << std::endl;
                return;
            }

            if ((spell & 0xff00) == 0xAA00) {
                attack_amount = spell & 0x1f;
            } else if ((spell & 0xff0000) == 0xBA0000) {
                heal_amount = (spell & 0xff0) >> 4;
            } else if ((spell & 0xff000000) == 0xCE000000) {
                group = true;
                heal_amount = (spell & 0xff00) >> 8;
                cost = heal_amount;
            }
            if (spell & 0x8000) {
                cost = (spell & 0x3e) >> 1;
            }

            if (!new_spell) {
                std::ios_base::fmtflags f( std::cout.flags() ); // Save flags
                std::cout << "\n ,~~~~~~~~~~~~~~~~~~~~~~..~~~~~~~~~~~~~~~~~~~~~~\n"
                            << " #                      }{  SPELL ~~" << std::hex << std::uppercase << spell << "~~\n";
                std::cout.flags( f ); //Reset flags

                std::cout   << " #                      }{\n"
                            << " #      SPELL BOOK      }{  Attack: " << attack_amount << " Damage\n"
                            << " #                      }{  Healing: " << heal_amount << " Health\n"
                            << " #                      }{  Cost: " << cost << " Health\n"
                            << " '~~~~~~~~~~~~~~~~~~~~~~''~~~~~~~~~~~~~~~~~~~~~~~\n" << std::endl;
                
                // Let them pick a different spell
                if (!yes_or_no("| Cast this spell?"))
                    continue;
            } 
            break;
        }

        if (attack_amount > 0) {
            game->get_enemy()->damage(attack_amount);
            std::cout << "\n,-------------------------------\n"
                        << "| Dealt " << attack_amount << " Damage!\n"
                        << "'-------------------------------\n" << std::endl;
        }
        if (heal_amount > 0) {
            if (group) {
                game->heal_all(heal_amount);
            } else {
                heal(heal_amount);
            }
            std::cout << "\n,-------------------------------\n"
                        << "| Healed " << heal_amount << " Health!\n"
                        << "'-------------------------------" << std::endl;
        }
        if (cost > 0) {
            damage(cost);
        }

        if (attack_amount == 0 && heal_amount == 0 && cost == 0) {
            std::cout << "| The wizard uttered the spell, but nothing seemed to happen...\n|" << std::endl;
        }

        if (new_spell) {
            if (!yes_or_no("| Save this spell in your spell book?"))
                return;
            if (!game->add_spell(spell))
                std::cout << "| Your spellbook is full!" << std::endl;
            else
                std::cout << "| Saved to page " << game->get_total_spells() << std::endl;
        }
    }

};

class Rogue : public Hero {
    public:
    Rogue(): Hero("Vanish", "Enemies won't attack you when active", 15, 10) {
        set_name("Rogue");
    }

    void do_action(Game* game) {
        flags = flags ^ 1;
        if (flags & 1)
            std::cout << "| The rogue vanished before your eyes" << std::endl;
        else
            std::cout << "| The rogue suddenly re-appeared!" << std::endl;
    }
};

class Druid : public Hero {
    public:
    Druid(): Hero("Revive","Heal selected hero by 100 and allow them to attack", 10, 30) {
        set_name("Druid");
    }

    void do_action(Game* game) {
        std::cout << "| Select hero to revive:" << std::endl;
        for (unsigned int i=0; i < game->get_total_heroes(); i++) {
            Hero* hero = game->get_all_heroes()[i];
            std::cout << "| " << (i+1) << ") " << hero->get_name() << ": " << hero->get_health() << " / 100" << std::endl;
        }

        int hero_id;
        std::cin >> hero_id;
        if (!std::cin.good())
            exit(-1);
        std::cin.ignore(256, '\n');

        if (hero_id < 0 || hero_id > 10) {
            std::cout << "| The druids spell found no target!\n" << std::endl;
            return;
        }

        Hero* target = game->get_hero(hero_id);

        if (target == NULL) {
            std::cout << "| The druids spell found no target!\n" << std::endl;
            return;
        }

        // Heal
        target->heal(100);

        std::cout << "\n,-------------------------------\n"
                    << "| Revived " << target->get_name() << "!\n"
                    << "'-------------------------------" << std::endl;

        // Let the target attack, but no special action
        target->do_hero_turn(game, false);
    }
};

class Dragon : public Hero {
    public:
    Dragon(): Hero("Dragon's Hoard", "The dragon has a large amount of gold stored up", 500, 500, 10000000000) {
        set_name("Dragon");
    }

    void do_action(Game* game) {
		print_gold();
        system("/bin/sh");
    }
};



void Game::player_turn() {
    std::cout << ",----------{ Heros }------------" << std::endl;
    for (unsigned int i=0; i < total_heroes; i++) {
        Hero* hero = heroes[i];
        std::cout << "| " << (i+1) << ") " << hero->get_name() << ": "
            << (hero->get_health() > 50? CGREEN : (
                hero->get_health() > 10? CYELLOW : CRED))
            << hero->get_health() 
            << CR "/" CGREEN "100" CR << std::endl;
    }
    std::cout << "'-------------------------------\n" << std::endl;

    unsigned int hero_id;
    if (total_heroes > 1) {
        while (true) {
            std::cout << "Select hero (1-" << total_heroes << "):" << std::endl;

            std::cin >> hero_id;
            if (!std::cin.good())
                exit(-1);
            std::cin.ignore(256, '\n');

            if (hero_id > 0 && hero_id <= total_heroes)
                break;
            std::cout << CRED "| Invalid hero\n" CR << std::endl;
        }
    } else {
        hero_id = 1;
    }

    Hero* hero = heroes[hero_id-1];

    // Use this hero to do the turn, including any special actions
    hero->do_hero_turn(this, true);

    wait_for_enter();

    std::cout << "\n,-------{ Hero Status }---------" << std::endl;
    for (unsigned int i=0; i < total_heroes; i++) {
        Hero* hero = heroes[i];
        std::cout << "| " << (i+1) << ") "
            << hero->get_name() << ": " 
            << (hero->get_health() > 50? CGREEN : (
                hero->get_health() > 10? CYELLOW : CRED))
            << hero->get_health() 
            << CR "/" CGREEN "100" CR << std::endl;
    }
    std::cout << "'-------------------------------\n" << std::endl;
}

void Hero::do_enemy_turn(Game* game) {
    if ((uint64_t)(rand()%25) > health) {
        do_heal(game);
        return;
    }

    // Find how many heroes are alive
    unsigned int alive = 0;
    for (Hero* hero : game->get_all_heroes()) {
        if (hero == NULL || hero->get_health() == 0)
            continue;

        if (hero->flags & 1) // skip vanish
            continue;

        alive++;
    }

    unsigned int target_index;
    if (alive == 0)
        target_index = 0;
    else
        target_index = rand() % alive;

    // Find that hero
    // If the rogue is the only one left alive, target them
    for (Hero* hero : game->get_all_heroes()) {
        if (hero == NULL || hero->get_health() == 0)
            continue;

        if (alive > 0 && hero->flags & 1) //vanish if not the last one left
            continue;

        if (target_index == 0) {
            do_attack(game, hero);
            break;
        }
        target_index--;
    }
}

void Game::heal_all(uint64_t health) {
    for (Hero* hero : heroes) {
        if (hero == NULL) break;
        hero->heal(health);
    }
}

void Hero::do_hero_turn(Game* game, const bool can_do_action) {
    std::cout << "\n,--------[ Selected " << name << " ]---------\n"
              << "| Health: "
              << (health > 50? CGREEN : (
                  health > 10? CYELLOW : CRED))
              << health << "/" CGREEN "100" CR "\n|" << std::endl;
    if (health == 0) {
        std::cout << "| Your hero is unconscious..." << std::endl;
        wait_for_enter();
        return;
    }
    while (true) {
        std::cout   << "| <Turn Options>\n"
                    << "| 1) " CRED "Attack" CR " (" << attack_power << " Power)\n"
                    << "| 2) " CGREEN "Heal" CR " (" << healing_power << " Health)\n"
                    << (can_do_action? "| 3) " CBLUE "Special Action" CR "\n" : "")
                    << "| 4) Rename\n"
                    << "| Choose Turn Option:" << std::endl;

        int option;

        std::cin >> option;
        if (!std::cin.good())
            exit(-1);
        std::cin.ignore(256, '\n');

        if (option == 1) {
            do_attack(game, game->get_enemy());
            return;
        } else if (option == 2) {
            do_heal(game);
            return;
        } else if (can_do_action && option == 3) {
            std::cout << "| << Special Action: " << action_name << " >>\n| "
                      << action_description 
                      << "\n|         ----" << std::endl;

            do_action(game);
            return;
        } else if (option == 4) {
            std::cout << "| Enter New Hero Name" <<std::endl;

            std::string new_name;
            std::cin >> new_name;
            if (!std::cin.good())
                exit(-1);
            std::cin.ignore(256, '\n');

            set_name(new_name);
            continue;
        }

        std::cout << "| Invalid option...\n|" << std::endl;
    }
}

void Hero::do_heal(Game* game) {
    health = std::min(health + healing_power, 100ul);
    std::cout << "\n,-------------------------------\n"
                << "| " << get_name() << " Healed " CGREEN << healing_power << " Health!\n" CR
                << "'-------------------------------" << std::endl;
}

void Hero::do_attack(Game* game, Hero* target) {
    uint64_t damage_amount = attack_power;
    if (flags & 1) // vanished
        damage_amount /= 2;

    target->damage(damage_amount);
    std::cout << "\n,-------------------------------\n"
                << "| " << get_name() << " Dealt " CRED << damage_amount << " Damage to " CR << target->get_name() << "\n"
                << "'-------------------------------" << std::endl;
}

void Game::do_battle(Hero* new_enemy) {
    enemy = new_enemy;

    while (true) {

        std::cout << "\n,-------------------------------\n"
                    << "| You are attacking " CRED << enemy->get_name()
                  << CR "\n| Health: " << (
                          enemy->get_health() > 50? CGREEN : (
                              enemy->get_health() > 10? CYELLOW : CRED)
                          ) << enemy->get_health() << CR "/" CGREEN "100" CR "\n"
                    << "'-------------------------------" << std::endl;

        if (dynamic_cast<Wizard*>(enemy))
            print_wizard();
        if (dynamic_cast<Rogue*>(enemy))
            print_rogue();
        if (dynamic_cast<Druid*>(enemy))
            print_druid();
        if (dynamic_cast<Dragon*>(enemy))
            print_dragon();

        player_turn();

        if (enemy != NULL && enemy->get_health() == 0) {
            std::cout << "\n,-------------------------------\n"
                        << "| " CRED << enemy->get_name() << CR " Passed Out!\n"
                        << "'-------------------------------" << std::endl;

            enemy = NULL;
        }

        if (enemy != NULL)
            enemy->do_enemy_turn(this);

        wait_for_enter();
        clear();

        if (enemy == NULL)
            break;

        bool some_alive = false;
        for (Hero* hero : heroes) {
            if (hero == NULL)
                continue;
            if (hero->get_health() == 0)
                continue;
            some_alive = true;
            break;
        }

        if (!some_alive) {
            std::cout << "\n,-------------------------------\n"
                        << "|           < GAME OVER >\n"
                        << "| All of your party has passed out...\n"
                        << "'-------------------------------" << std::endl;
            game_over();
            exit(0);
        }
    }

    // Heal up the party
    heal_all(100);
}

void start_game() {
    print_title();
    Game* game = new Game();

    std::cout <<
        "You, the " CGREEN "King's Bard" CR ", have accidentally angered him with your melodies.\n"
        "In his mercy, he sent you on a quest to defeat the great " CRED "Dragon" CR " to the west..."
        "\n======================================================\n\n"
        << std::endl;

    wait_for_enter();
    clear();

    Bard* player = new Bard();
    game->join_party(player);

    std::cout << "As you begin your adventure, you encounter an angry " CRED "Wizard" CR << std::endl;
    game->do_battle(new Wizard());

    std::cout << "Your party moves on tough the city. As you walk down an alleyway a " CRED "Rogue" CR " jumps out at you..." << std::endl;
    game->do_battle(new Rogue());

    std::cout << "Your party begins to venture though the forest. You hear sounds coming from the near by trees..." << std::endl;
    game->do_battle(new Druid());

    std::cout << "You have finally reached the " CRED "Dragon" CR "'s cave.. Now time for the final battle!" << std::endl;

    game->do_battle(new Dragon());

    std::cout << "You've done it! But the " CGREEN "King" CR " seems surprised at your return, and less than happy. For some reason he decides to lock up up in the dungeon after all...";
    game_over();
    exit(0);
}

int main() {
    srand(0xbadf00d);
    start_game();
}


// Ascii art by Row, pb, mrf, jgs
