#include "generator.h"
#include <ranges>

template <class T>
tl::generator<T> firstn(T t) {
   T num = T{ 0 };
   while (num < t) {
      co_yield num;
      ++num;
   }
}

template <class T>
tl::generator<T> iota(T t = T{ 0 }) {
   while (true) {
      co_yield t;
      t++;
   }
}


#include <iostream>
struct regular_void{};
namespace tl {
   tl::generator<const regular_void> clock() {
      while (true) {
         std::cout << "tick";
         co_yield {};
         std::cout << "tock";
         co_yield{};
      }
   }
}
#include <vector>
#include <string>
#include <string_view>

tl::generator<std::vector<std::string>> split_by_lines_and_whitespace(std::string_view sv) {
   std::vector<std::string> res;

   auto start = sv.begin();
   auto pos = sv.begin();

   while (pos != sv.end()) {
      if (*pos == ' ') {
         res.push_back(std::string(start, pos));
         start = pos;
      }
      if (*pos == '\n') {
         res.push_back(std::string(start, pos));
         co_yield res;
         res.clear();
         start = pos;
      }
      ++pos;
   }
}
int main() {
   auto input = R"(
one two three
i am a new line
hello
)";

   for (auto line : (split_by_lines_and_whitespace(input))) {
      std::cout << "Line: ";
      for (auto s : line) {
         std::cout << s << ',';
      }
      std::cout << '\n';
   }
}