#include <catch2/catch.hpp>
#include <string>
#include <vector>
#include <iostream>
#include "tl/chunk_by.hpp"

struct dog {
   std::string name;
   int age;
};

TEST_CASE("group_by") {
   std::vector<dog> dogs{
      {"fido", 12},
      {"bob", 12},
      {"katherine", 9},
      {"max", 12},
      {"sy", 12},
   };

   int group_id = 0;
   int i = 0;
   for (auto&& group : tl::chunk_by_view(dogs, [](auto&& left, auto&& right) { return left.age == right.age; })) {
      for (auto&& d : group) {
         switch (i) {
         case 0: REQUIRE(d.name == "fido"); REQUIRE(group_id == 0); break;
         case 1: REQUIRE(d.name == "bob"); REQUIRE(group_id == 0); break;
         case 2: REQUIRE(d.name == "katherine"); REQUIRE(group_id == 1); break;
         case 3: REQUIRE(d.name == "max"); REQUIRE(group_id == 2); break;
         case 4: REQUIRE(d.name == "sy"); REQUIRE(group_id == 2); break;
         default: REQUIRE(false);
         }
         ++i;
      }
      group_id++;
   }
}

TEST_CASE("group_by pipe") {
   std::vector<dog> dogs{
      {"fido", 12},
      {"bob", 12},
      {"katherine", 9},
      {"max", 12},
      {"sy", 12},
   };

   int group_id = 0;
   int i = 0;
   tl::views::chunk_by([](auto&& left, auto&& right) { return left.age == right.age; })(dogs);
   for (auto&& group : dogs | tl::views::chunk_by([](auto&& left, auto&& right) { return left.age == right.age; })) {
      for (auto&& d : group) {
         switch (i) {
         case 0: REQUIRE(d.name == "fido"); REQUIRE(group_id == 0); break;
         case 1: REQUIRE(d.name == "bob"); REQUIRE(group_id == 0); break;
         case 2: REQUIRE(d.name == "katherine"); REQUIRE(group_id == 1); break;
         case 3: REQUIRE(d.name == "max"); REQUIRE(group_id == 2); break;
         case 4: REQUIRE(d.name == "sy"); REQUIRE(group_id == 2); break;
         default: REQUIRE(false);
         }
         ++i;
      }
      group_id++;
   }
}