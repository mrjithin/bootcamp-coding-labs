#include "spellcheck.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

namespace rv=std::ranges::views;

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
  auto view = source | rv::filter([&dictionary](const auto& token){
    return !dictionary.contains(token.content);
  }) | rv::transform([&dictionary](const auto& token){
    auto close_words=dictionary | rv::filter([&token](const auto& word){
      return levenshtein(token.content,word)==1;
    });

    auto suggestions = std::set(close_words.begin(),close_words.end());

    return Misspelling(token,suggestions);
  }) | rv::filter([](const auto& misspelling){
    return !misspelling.suggestions.empty();
  });

  return std::set<Misspelling>(view.begin(),view.end());
};

/* Helper methods */

#include "utils.cpp"