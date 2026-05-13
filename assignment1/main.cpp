/*
 * Assignment 1: Marriage Pact
 * Adapted by Tinkercademy from Stanford CS106L
 * (originally by Haven Whitney, with modifications by Fabio Ibanez
 * & Jacob Roberts-Baca).
 *
 * Complete each STUDENT TODO below. Read the README carefully — the
 * requirements there (ranges, projections, sample, reserve, no raw
 * for-loops in find_matches, iterator-safe erase in run_mixer) are
 * part of the assignment, not optional polish.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * Reads `filename` line by line and returns the applicants.
 *
 * Requirements:
 *   - Take `filename` as `const std::string&`.
 *   - Call `reserve()` before populating, with a sensible capacity.
 *     Justify your choice in short_answer.txt.
 */
std::vector<std::string> get_applicants(const std::string& filename) {
    std::ifstream studentsFile(filename);
    std::vector<std::string> students;
    students.reserve(1000);
    std::string name;
    if (studentsFile.is_open()) {
        while (getline(studentsFile, name)) {
            if (name.back() == '\r') name.pop_back();
            students.push_back(name);
        }
    } else {
        std::cerr << "Error while opening file\n";
    }
    return students;
}

/**
 * Returns the initials of `name`, uppercased.
 *   e.g. initials("Marceline McMillan") == "MM"
 *
 * Requirements:
 *   - Parameter must be `std::string_view` (no allocation).
 */
std::string initials(std::string_view name) {
    std::string initial;
    if (name.size() == 0) {
        return "Empty String";
    }
    initial += name[0];
    int pos = name.find(' ');
    if (pos == std::string_view::npos || pos + 1 >= name.size()) {
        return "No Last Name";
    }
    initial += name[pos + 1];
    for (auto& c : initial) {
        if ('a' <= c && c <= 'z') {
            c -= 'a' - 'A';
        }
    }
    return initial;
}

/**
 * Returns every applicant in `students` who shares initials with `name`.
 *
 * Requirements:
 *   - No raw `for` loops. Use std::ranges::copy_if (or views::filter
 *     piped into a vector). Use a projection where it makes the call
 *     clearer.
 *   - Take `students` as `const std::vector<std::string>&`.
 */
std::vector<std::string> find_matches(
    std::string_view name, const std::vector<std::string>& students) {
    std::string targetInitials = initials(name);
    auto filteredView = students | std::views::filter([&](auto& studentName) {
                            return initials(studentName) == targetInitials;
                        });
    return std::vector<std::string>(filteredView.begin(), filteredView.end());
}

/**
 * Returns one randomly-chosen match, or "NO MATCHES FOUND." if empty.
 *
 * Requirements:
 *   - Use std::sample with a seeded std::mt19937.
 *   - Do NOT use pop_back() or rand() % size.
 */
std::string get_match(const std::vector<std::string>& matches) {
    if (matches.empty()) {
        return "NO MATCHES FOUND.";
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::string match;
    std::sample(matches.begin(), matches.end(), &match, 1, gen);
    return match;
}

/**
 * Runs a multi-round mixer. In each round, scan the remaining
 * applicants left-to-right; for each applicant, look for another
 * applicant with the same initials still in the pool. If found,
 * pair them, remove both from `applicants`, and record the pair.
 * Continue rounds until a full pass yields no new pairs.
 *
 * `applicants` is mutated: paired names are removed. Whatever is
 * left over at the end is unpaired.
 *
 * Requirements:
 *   - The naive "iterate and erase as you go" approach WILL invalidate
 *     your iterator. You must handle this — see the README for the
 *     three acceptable strategies — and document your choice in
 *     short_answer.txt.
 */
std::vector<std::pair<std::string, std::string>> run_mixer(
    std::vector<std::string>& applicants) {
    std::vector<std::pair<std::string, std::string>> pairs;
    auto leftIt = applicants.begin();
    while (leftIt != applicants.end()) {
        auto matches = find_matches(*leftIt, applicants);
        if (matches.size() == 1) {
            leftIt++;
            continue;
        }
        matches.erase(matches.begin());
        std::string rightPair = get_match(matches);
        auto rightIt = find(leftIt + 1, applicants.end(), rightPair);
        pairs.emplace_back(*leftIt, *rightIt);
        applicants.erase(rightIt);
        leftIt = applicants.erase(leftIt);
    }
    return pairs;
}

/* #### Please don't remove this line! #### */
#include "tests/utils.hpp"
