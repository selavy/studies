#include "monsters_generated.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "trie_generated.h"

using namespace MyGame::Sample;

#if 1
void read_trie(const char* filename)
{
    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);
    infile.seekg(0, std::ios::end);
    int length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::vector<char> data(length);
    infile.read(data.data(), length);
    infile.close();

    auto trie = GetDoubleArray(data.data());
    flatbuffers::Verifier v((const uint8_t*)data.data(), data.size());
    assert(trie->Verify(v));

    auto es = trie->entries();
    auto es_len = es->size();
    std::cout << "# of entries: " << es_len << "\n";
    for (std::size_t i = 0; i < es_len; ++i) {
        const auto& e = es->Get(i);
        std::cout << "\t" << e->base() << " " << e->check() << " " << e->term() << "\n";
    }
}
#endif

void trie_tests()
{
    std::vector<int>  bases  = {  0, 1, 2, 3, 4, 5 };
    std::vector<int>  checks = { -1, 0, 0, 1, 0, 6 };
    std::vector<bool> terms  = { true, true, false, false, false, false };
    std::vector<DoubleEntry> entries;
    for (std::size_t i = 0; i < bases.size(); ++i) {
        entries.emplace_back(bases[i], checks[i], terms[i]);
    }

    flatbuffers::FlatBufferBuilder builder(1024);
    // auto es = builder.CreateVectorOfStructs(entries.data(), entries.size());
    // DoubleArrayBuilder double_array_builder(builder);
    // double_array_builder.add_entries(es);
    // auto double_array = double_array_builder.Finish();
    auto double_array = CreateDoubleArrayDirect(builder, &entries);
    builder.Finish(double_array);

    uint8_t* buf  = builder.GetBufferPointer();
    int      size = builder.GetSize();

    const char* filename = "dictionary.bin";
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file for writing!\n");
        exit(1);
    }
    if (fwrite(buf, size, 1, fp) != 1) {
        fprintf(stderr, "error: unable to write to file: %d\n", errno);
        exit(1);
    }
    fclose(fp);

    read_trie(filename);
}

void read_monster(const char* filename)
{
    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);
    infile.seekg(0, std::ios::end);
    int length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::vector<char> data(length);
    infile.read(data.data(), length);
    infile.close();

    auto monster = GetMonster(data.data());

    std::cout << "HP:   " << monster->hp() << "\n"
              << "Mana: " << monster->mana() << "\n"
              << "Name: " << monster->name()->c_str() << "\n";
}


int main(int argc, char** argv)
{
    flatbuffers::FlatBufferBuilder builder(1024);

    auto weapon_one_name = builder.CreateString("Sword");
    short weapon_one_damage = 3;

    auto weapon_two_name = builder.CreateString("Axe");
    short weapon_two_damage = 5;

    auto sword = CreateWeapon(builder, weapon_one_name, weapon_one_damage);
    auto axe   = CreateWeapon(builder, weapon_two_name, weapon_two_damage);

    auto name  = builder.CreateString("Orc");
    unsigned char treasure[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    auto inventory = builder.CreateVector(treasure, 10);

    std::vector<flatbuffers::Offset<Weapon>> weapons_vector;
    weapons_vector.push_back(sword);
    weapons_vector.push_back(axe);
    auto weapons = builder.CreateVector(weapons_vector);

    Vec3 points[] = { Vec3(1.0f, 2.0f, 3.0f), Vec3(4.0f, 5.0f, 6.0f) };
    auto path = builder.CreateVectorOfStructs(points, 2);

    auto position = Vec3(1.0f, 2.0f, 3.0f);

    int hp = 300;
    int mana = 150;

    // auto orc = CreateMonster(builder, &position, mana, hp, name, inventory, Color_Red, weapons, Equipment_Weapon, axe.Union(), path);

    MonsterBuilder monster_builder(builder);
    monster_builder.add_pos(&position);
    monster_builder.add_hp(hp);
    monster_builder.add_name(name);
    monster_builder.add_inventory(inventory);
    monster_builder.add_color(Color_Red);
    monster_builder.add_weapons(weapons);
    monster_builder.add_equipped_type(Equipment_Weapon);
    monster_builder.add_equipped(axe.Union());
    auto orc = monster_builder.Finish();

    builder.Finish(orc);

    uint8_t* buf = builder.GetBufferPointer();
    int      size = builder.GetSize();

    const char* filename = "monster.bin";
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file for writing!\n");
        exit(1);
    }
    if (fwrite(buf, size, 1, fp) != 1) {
        fprintf(stderr, "error: unable to write to file: %d\n", errno);
        exit(1);
    }
    fclose(fp);

    read_monster(filename);

    trie_tests();

    return 0;
}
