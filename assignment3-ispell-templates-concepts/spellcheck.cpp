#include "spellcheck.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

template <typename Iterator, typename UnaryPred>
std::vector<Iterator> find_all(Iterator begin, Iterator end, UnaryPred pred);

Corpus tokenize(std::string& source) {
  /* TODO: Implement this method */
  auto space_its=find_all(source.begin(),source.end(),::isspace);
  
  Corpus tokens;
  std::transform(space_its.begin(),space_its.end()-1,space_its.begin()+1,std::inserter(tokens,tokens.end()),[&source](const auto& it1, const auto& it2){
    return Token(source,it1,it2);
  });

  std::erase_if(tokens,[](const auto& token){
    return token.content.empty();
  });

  return tokens;
}

std::set<Misspelling> spellcheck(const Corpus& source, const Dictionary& dictionary) {
  /* TODO: Implement this method */
  return std::set<Misspelling>();
};

/* Helper methods */

#include "utils.cpp"