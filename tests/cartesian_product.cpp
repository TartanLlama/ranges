#include <catch2/catch.hpp>
#include "tl/cartesian_product.hpp"
#include <iostream>
#include <vector>
#include <list>

TEST_CASE("cartesian") {
   std::vector a{ 0,1,2 };
   std::vector b{ 0,1,2 };
   std::vector c{ 0,1,2 };

   std::vector<std::vector<int>> res{
      {0,0,0},
      {0,0,1},
      {0,0,2},
      {0,1,0},
      {0,1,1},
      {0,1,2},
      {0,2,0},
      {0,2,1},
      {0,2,2},
      {1,0,0},
      {1,0,1},
      {1,0,2},
      {1,1,0},
      {1,1,1},
      {1,1,2},
      {1,2,0},
      {1,2,1},
      {1,2,2},
      {2,0,0},
      {2,0,1},
      {2,0,2},
      {2,1,0},
      {2,1,1},
      {2,1,2},
      {2,2,0},
      {2,2,1},
      {2,2,2},
   };

   auto i = 0;
   for (auto&& [ia, ib, ic] : tl::cartesian_product_view(a, b, c)) {
      REQUIRE(std::tie(ia, ib, ic) == std::tie(res[i][0], res[i][1], res[i][2]));
      ++i;
   }
}

TEST_CASE("bidirectional") {
   std::list a{ 0,1,2 };
   std::list b{ 0,1,2 };
   std::list c{ 0,1,2 };

   std::list<std::vector<int>> res{
      {2,2,2},
      {2,2,1},
      {2,2,0},
      {2,1,2},
      {2,1,1},
      {2,1,0},
      {2,0,2},
      {2,0,1},
      {2,0,0},
      {1,2,2},
      {1,2,1},
      {1,2,0},
      {1,1,2},
      {1,1,1},
      {1,1,0},
      {1,0,2},
      {1,0,1},
      {1,0,0},
      {0,2,2},
      {0,2,1},
      {0,2,0},
      {0,1,2},
      {0,1,1},
      {0,1,0},
      {0,0,2},
      {0,0,1},
      {0,0,0}
   };

   auto res_it = res.begin();
   auto v = tl::cartesian_product_view(a, b, c);
   for (auto it = --std::ranges::next(v.begin(), v.end()); it != v.begin(); --it) {
      REQUIRE(*it == std::tie((*res_it)[0], (*res_it)[1], (*res_it)[2]));
      ++res_it;
   }
}


TEST_CASE("random access") {
   std::vector a{ 0,1,2 };
   std::vector b{ 0,1,2 };
   std::vector c{ 0,1,2 };

   auto v = tl::cartesian_product_view(a, b, c);
   auto it = v.begin();
   it += 4;
   REQUIRE(*it == std::make_tuple(0, 1, 1));
   it += -2;
   REQUIRE(*it == std::make_tuple(0, 0, 2));
   it -= 2;
   REQUIRE(*it == std::make_tuple(0, 0, 0));
   it += 20;
   REQUIRE(*it == std::make_tuple(2, 0, 2));
   it += 7;
   REQUIRE(it == v.end());
}