// =============================================================================
// RAII & Smart Pointers — Stage 5: The Leak Hunt
// leak_hunt.cpp
//
// This file contains THREE bugs. Your job:
//
//   Step 1 — Read the code and predict what will go wrong for each bug
//             BEFORE compiling. Write your predictions as comments.
//
//   Step 2 — Compile and run. Compare the cerr log against the
//             EXPECTED OUTPUT block at the bottom of this file.
//             Notice which "destroyed" lines are missing or wrong.
//
//   Step 3 — Compile with AddressSanitizer (see below). Each bug produces
//             a distinct ASan report — match each report to a bug.
//
//   Step 4 — Fix all three bugs using only RAII / smart-pointer tools.
//             No raw new/delete in your fix.
//
// Compile (buggy):
//   g++ -std=c++20 -Wall -Wextra -fsanitize=address,undefined -g leak_hunt.cpp -o leak_hunt
//
// The three bug types:
//   Bug A — raw new with no delete on an exception path
//   Bug B — shared_ptr reference cycle (same as Stage 4)
//   Bug C — unique_ptr passed by value when the caller doesn't need ownership
// =============================================================================

#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>
#include <atomic>

static int next_id() {
    static std::atomic<int> counter{0};
    return ++counter;
}

// ─────────────────────────────────────────────────────────────────────────────
// Resource: a simple named heap-allocated value that logs its lifetime.
// ─────────────────────────────────────────────────────────────────────────────
struct Resource {
    int         id_;
    std::string label_;
    int         value_;

    Resource(std::string label, int value)
        : id_(next_id()), label_(std::move(label)), value_(value)
    {
        std::cerr << "[Resource #" << id_ << " \"" << label_ << "\" val=" << value_ << "] born\n";
    }

    ~Resource() {
        std::cerr << "[Resource #" << id_ << " \"" << label_ << "\"] destroyed\n";
    }

    void describe() const {
        std::cerr << "  Resource #" << id_ << " \"" << label_ << "\" = " << value_ << "\n";
    }
};

// =============================================================================
// BUG A — raw new leaks on exception path
//
// process_data allocates a Resource with raw new, does some work,
// then deletes it. But if the work throws, the delete is skipped.
//
// Your prediction: 
// When x>50, it throws an exception and the Resource delete line is skipped, causing a memory leak. 
// ASan symptom:   
// Direct leak of 48 byte(s) in 1 object(s) allocated from:
// #0 0x7591610fe548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
// #1 0x6006c5b4e927 in process_data(int) /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:84
// #2 0x6006c5b4f82e in main /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:171
// #3 0x75916002a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
// #4 0x75916002a28a in __libc_start_main_impl ../csu/libc-start.c:360
// #5 0x6006c5b4e5c4 in _start (/mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt+0x1d5c4) (BuildId: ba1dcc56e0b70ab403fd5afc3eb1cd3aa5f99992)
// Fix: 
// Make the Resource a unique_ptr and remove the delete line.   
// =============================================================================
static void risky_work(int x) {
    if (x > 50)
        throw std::runtime_error("value too large");
    std::cerr << "  risky_work: processed " << x << "\n";
}

void process_data(int x) {
    auto r = std::make_unique<Resource>("process_data", x);

    risky_work(x);

    r->describe();
}

// =============================================================================
// BUG B — ???
//
// Your prediction: 
// Since Publisher and Subscriber cyclically reference each other with shared_ptr,
// they will never be destroyed, causing a memory leak.
// ASan symptom:    
// Indirect leak of 40 byte(s) in 1 object(s) allocated from:
//     #0 0x726c4dafe548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
//     #1 0x582b5db982e0 in std::__new_allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long, void const*) /usr/include/c++/13/bits/new_allocator.h:151
//     #2 0x582b5db9665b in std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long) /usr/include/c++/13/bits/allocator.h:198
//     #3 0x582b5db9665b in std::allocator_traits<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >::allocate(std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&, unsigned long) /usr/include/c++/13/bits/alloc_traits.h:482
//     #4 0x582b5db9665b in std::__allocated_ptr<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > > std::__allocate_guarded<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >(std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&) /usr/include/c++/13/bits/allocated_ptr.h:98
//     #5 0x582b5db95256 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::__shared_count<Subscriber, std::allocator<void>, int>(Subscriber*&, std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:969
//     #6 0x582b5db9451d in std::__shared_ptr<Subscriber, (__gnu_cxx::_Lock_policy)2>::__shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:1712
//     #7 0x582b5db93119 in std::shared_ptr<Subscriber>::shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr.h:464
//     #8 0x582b5db900df in std::shared_ptr<Subscriber> std::make_shared<Subscriber, int>(int&&) /usr/include/c++/13/bits/shared_ptr.h:1010
//     #9 0x582b5db89c5a in run_pubsub() /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:130
//     #10 0x582b5db8a7bd in main /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:183
//     #11 0x726c4ca2a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
//     #12 0x726c4ca2a28a in __libc_start_main_impl ../csu/libc-start.c:360
//     #13 0x582b5db895c4 in _start (/mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt+0x1d5c4) (BuildId: 2aee0cc9a296bd30dc84370437b849445c0843e3)

// Indirect leak of 40 byte(s) in 1 object(s) allocated from:
//     #0 0x726c4dafe548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
//     #1 0x582b5db980ce in std::__new_allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long, void const*) /usr/include/c++/13/bits/new_allocator.h:151
//     #2 0x582b5db959db in std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long) /usr/include/c++/13/bits/allocator.h:198
//     #3 0x582b5db959db in std::allocator_traits<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >::allocate(std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&, unsigned long) /usr/include/c++/13/bits/alloc_traits.h:482
//     #4 0x582b5db959db in std::__allocated_ptr<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > > std::__allocate_guarded<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >(std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&) /usr/include/c++/13/bits/allocated_ptr.h:98
//     #5 0x582b5db94db2 in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::__shared_count<Publisher, std::allocator<void>, int>(Publisher*&, std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:969
//     #6 0x582b5db941d1 in std::__shared_ptr<Publisher, (__gnu_cxx::_Lock_policy)2>::__shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:1712
//     #7 0x582b5db92f67 in std::shared_ptr<Publisher>::shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr.h:464
//     #8 0x582b5db8ff96 in std::shared_ptr<Publisher> std::make_shared<Publisher, int>(int&&) /usr/include/c++/13/bits/shared_ptr.h:1010
//     #9 0x582b5db89b95 in run_pubsub() /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:129
//     #10 0x582b5db8a7bd in main /mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt.cpp:183
//     #11 0x726c4ca2a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
//     #12 0x726c4ca2a28a in __libc_start_main_impl ../csu/libc-start.c:360
//     #13 0x582b5db895c4 in _start (/mnt/d/bootcamp/bootcamp-coding-labs/assignment2.1/lab/leak_hunt+0x1d5c4) (BuildId: 2aee0cc9a296bd30dc84370437b849445c0843e3)

// SUMMARY: AddressSanitizer: 80 byte(s) leaked in 2 allocation(s).
// Fix:             
// Change the internal shared_ptr of members to a weak_ptr to break the cycle.
// =============================================================================
struct Subscriber;
struct Publisher {
    int id_;
    std::weak_ptr<Subscriber> subscriber_; 

    explicit Publisher(int id) : id_(id) {
        std::cerr << "[Publisher  #" << id_ << "] born\n";
    }
    ~Publisher() {
        std::cerr << "[Publisher  #" << id_ << "] destroyed\n";
    }
};

struct Subscriber {
    int id_;
    std::weak_ptr<Publisher> publisher_;

    explicit Subscriber(int id) : id_(id) {
        std::cerr << "[Subscriber #" << id_ << "] born\n";
    }
    ~Subscriber() {
        std::cerr << "[Subscriber #" << id_ << "] destroyed\n";
    }
};

void run_pubsub() {
    auto pub = std::make_shared<Publisher>(1);
    auto sub = std::make_shared<Subscriber>(2);

    pub->subscriber_ = sub;
    sub->publisher_  = pub;

    std::cerr << "  pub.use_count=" << pub.use_count()
              << "  sub.use_count=" << sub.use_count() << "\n";
}

// =============================================================================
// BUG C — unique_ptr passed by value unnecessarily
//
// Your prediction: 
// After moving the unique_ptr, the original unique_ptr becomes nullptr and 
// dereferencing nullptr results in a crash.
// Fix:             
// Pass a raw pointer to print_resource and remove std::move. 
// Note: ASan won't catch this one — it's not a memory error, it's a logic
// error. The log will show the resource dying at the wrong time.
// =============================================================================
void print_resource(const Resource* r) { 
    std::cerr << "  print_resource: ";
    r->describe();
}

void use_resource() {
    auto r = std::make_unique<Resource>("use_resource", 77);

    print_resource(r.get()); 

    // What's the value of r now?
    std::cerr << "  back in use_resource, value=" << r->value_ << "\n"; 
}

// =============================================================================
// main — runs all three buggy scenarios
// =============================================================================
int main() {
    std::cerr << "=== Stage 5: Leak Hunt ===\n\n";

    // ── Bug A ──────────────────────────────────────────────────────────────
    std::cerr << "-- Bug A: process_data(10) --\n";
    try { process_data(10); } catch (...) {}
    std::cerr << "  (no exception — did the resource get destroyed?)\n\n";

    std::cerr << "-- Bug A: process_data(99) --\n";
    try {
        process_data(99);
    } catch (const std::exception& e) {
        std::cerr << "  caught: " << e.what() << "\n";
    }
    std::cerr << "  (exception path — was the resource destroyed before the throw?)\n\n";

    // ── Bug B ──────────────────────────────────────────────────────────────
    std::cerr << "-- Bug B: pub/sub cycle --\n";
    run_pubsub();
    std::cerr << "  (did Publisher and Subscriber both get destroyed?)\n\n";

    // ── Bug C ──────────────────────────────────────────────────────────────
    std::cerr << "-- Bug C: unique_ptr by value --\n";
    // Comment this out once you understand the crash — fix it first.
    use_resource();
    std::cerr << "  (was the resource still alive after print_resource returned?)\n\n";

    std::cerr << "=== end of main ===\n";
    return 0;
}

// =============================================================================
// EXPECTED OUTPUT after all three bugs are fixed
// (IDs may differ; the important thing is the pairing of born/destroyed)
//
// === Stage 5: Leak Hunt ===
//
// -- Bug A: process_data(10) --
// [Resource #1 "process_data" val=10] born
//   risky_work: processed 10
//   Resource #1 "process_data" = 10
// [Resource #1 "process_data"] destroyed        <-- must appear BEFORE "no exception"
//   (no exception — did the resource get destroyed?)
//
// -- Bug A: process_data(99) --
// [Resource #2 "process_data" val=99] born
// [Resource #2 "process_data"] destroyed        <-- must appear BEFORE "caught"
//   caught: value too large
//   (exception path — was the resource destroyed before the throw?)
//
// -- Bug B: pub/sub cycle --
// [Publisher  #1] born
// [Subscriber #2] born
//   pub.use_count=1  sub.use_count=1             <-- use_count stays 1 with weak_ptr fix
// [Subscriber #2] destroyed                     <-- both must appear here
// [Publisher  #1] destroyed
//   (did Publisher and Subscriber both get destroyed?)
//
// -- Bug C: unique_ptr by value --
// [Resource #3 "use_resource" val=77] born
//   print_resource:   Resource #3 "use_resource" = 77
//   back in use_resource, value=77               <-- resource still alive
// [Resource #3 "use_resource"] destroyed         <-- destroyed when use_resource() returns
//   (was the resource still alive after print_resource returned?)
//
// === end of main ===
// =============================================================================

// =============================================================================
// FIX GUIDE (read only after attempting fixes yourself)
//
// Bug A fix:
//   Replace:  Resource* r = new Resource("process_data", x);
//   With:     auto r = std::make_unique<Resource>("process_data", x);
//   Delete the manual `delete r;` line.
//   unique_ptr's destructor runs even when an exception unwinds the stack.
//
// Bug B fix:
//   In Publisher, change:
//       std::shared_ptr<Subscriber> subscriber_;
//   to:
//       std::weak_ptr<Subscriber> subscriber_;
//   Then to use it: if (auto s = subscriber_.lock()) { ... }
//
// Bug C fix:
//   Change the signature of print_resource to take by const reference:
//       void print_resource(const Resource& r)
//   or to take a raw (non-owning) pointer:
//       void print_resource(const Resource* r)
//   Remove the std::move at the call site.
//   Rule of thumb: if a function doesn't need to *own* the resource,
//   it should not take a smart pointer by value.
// =============================================================================
