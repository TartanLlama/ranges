#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <iostream>
#include "tl/chunk_by_key.hpp"

struct dog {
  std::string name;
  int age;
};

TEST_CASE("chunk_by_key") {
   std::vector<dog> dogs{
      {"fido", 12},
      {"bob", 12},
      {"katherine", 9},
      {"max", 12},
      {"sy", 12},
   };

   int group_id = 0;
   int i = 0;
   for (auto&& [key, group] : tl::chunk_by_key_view(dogs, [](auto&& d) { return d.age; })) {
      for (auto&& d : group) {
         switch (i) {
         case 0: REQUIRE(d.name == "fido"); REQUIRE(group_id == 0); REQUIRE(key == 12); break;
         case 1: REQUIRE(d.name == "bob"); REQUIRE(group_id == 0); REQUIRE(key == 12); break;
         case 2: REQUIRE(d.name == "katherine"); REQUIRE(group_id == 1); REQUIRE(key == 9); break;
         case 3: REQUIRE(d.name == "max"); REQUIRE(group_id == 2); REQUIRE(key == 12); break;
         case 4: REQUIRE(d.name == "sy"); REQUIRE(group_id == 2); REQUIRE(key == 12); break;
         default: REQUIRE(false);
         }
         ++i;
      }
      group_id++;
   }
}

TEST_CASE("chunk_by_key pipe") {
  std::vector<dog> dogs{
     {"fido", 12},
     {"bob", 12},
     {"katherine", 9},
     {"max", 12},
     {"sy", 12},
  };

  int group_id = 0;
  int i = 0;
  for (auto&& [key, group] : dogs | tl::views::chunk_by_key([](auto&& d) { return d.age; })) {
    for (auto&& d : group) {
      switch (i) {
      case 0: REQUIRE(d.name == "fido"); REQUIRE(group_id == 0); REQUIRE(key == 12); break;
      case 1: REQUIRE(d.name == "bob"); REQUIRE(group_id == 0); REQUIRE(key == 12); break;
      case 2: REQUIRE(d.name == "katherine"); REQUIRE(group_id == 1); REQUIRE(key == 9); break;
      case 3: REQUIRE(d.name == "max"); REQUIRE(group_id == 2); REQUIRE(key == 12); break;
      case 4: REQUIRE(d.name == "sy"); REQUIRE(group_id == 2); REQUIRE(key == 12); break;
      default: REQUIRE(false);
      }
      ++i;
    }
    group_id++;
  }
}