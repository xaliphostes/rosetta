/*
 * Test suite for rosetta::core::Any class
 * Tests constructors with pointers, references, const variants, and copy semantics
 */

#include "TEST.h"
#include <rosetta/core/any.h>

#include <iostream>
#include <memory>
#include <string>

using namespace rosetta::core;

// ============================================================================
// Helper class to track copies and moves
// ============================================================================
struct CopyTracker {
    static int copy_count;
    static int move_count;
    static int instance_count;

    int         id;
    int         value;
    std::string name;

    static void reset_counters() {
        copy_count     = 0;
        move_count     = 0;
        instance_count = 0;
    }

    CopyTracker(int v = 0, const std::string &n = "default")
        : id(++instance_count), value(v), name(n) {
        std::cout << "  [CopyTracker] Constructed id=" << id << " value=" << value << std::endl;
    }

    CopyTracker(const CopyTracker &other)
        : id(++instance_count), value(other.value), name(other.name) {
        ++copy_count;
        std::cout << "  [CopyTracker] COPY from id=" << other.id << " to id=" << id << std::endl;
    }

    CopyTracker(CopyTracker &&other) noexcept
        : id(++instance_count), value(other.value), name(std::move(other.name)) {
        ++move_count;
        std::cout << "  [CopyTracker] MOVE from id=" << other.id << " to id=" << id << std::endl;
    }

    CopyTracker &operator=(const CopyTracker &other) {
        if (this != &other) {
            value = other.value;
            name  = other.name;
            ++copy_count;
            std::cout << "  [CopyTracker] COPY ASSIGN from id=" << other.id << " to id=" << id
                      << std::endl;
        }
        return *this;
    }

    CopyTracker &operator=(CopyTracker &&other) noexcept {
        if (this != &other) {
            value = other.value;
            name  = std::move(other.name);
            ++move_count;
            std::cout << "  [CopyTracker] MOVE ASSIGN from id=" << other.id << " to id=" << id
                      << std::endl;
        }
        return *this;
    }

    ~CopyTracker() { std::cout << "  [CopyTracker] Destroyed id=" << id << std::endl; }

    bool operator==(const CopyTracker &other) const {
        return value == other.value && name == other.name;
    }
};

int CopyTracker::copy_count     = 0;
int CopyTracker::move_count     = 0;
int CopyTracker::instance_count = 0;

// ============================================================================
// TEST: Basic value construction (baseline)
// ============================================================================
TEST(Any, ConstructFromValue) {
    std::cout << "\n--- Test: Construct from value ---\n";
    CopyTracker::reset_counters();

    {
        CopyTracker original(42, "original");
        std::cout << "Creating Any from lvalue...\n";
        Any a(original);

        EXPECT_TRUE(a.has_value());
        EXPECT_EQ(a.as<CopyTracker>().value, 42);
        EXPECT_STREQ(a.as<CopyTracker>().name, "original");

        // Should have made at least one copy (lvalue -> Any storage)
        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;
        EXPECT_GE(CopyTracker::copy_count, 1);
    }

    std::cout << "Final - Copies: " << CopyTracker::copy_count
              << ", Moves: " << CopyTracker::move_count << std::endl;
}

// ============================================================================
// TEST: Move construction (should avoid copies)
// ============================================================================
TEST(Any, ConstructFromRvalue) {
    std::cout << "\n--- Test: Construct from rvalue (move) ---\n";
    CopyTracker::reset_counters();

    {
        std::cout << "Creating Any from rvalue...\n";
        Any a(CopyTracker(100, "moved"));

        EXPECT_TRUE(a.has_value());
        EXPECT_EQ(a.as<CopyTracker>().value, 100);

        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;

        // Ideally should have moves, not copies
        // Note: actual behavior depends on implementation
    }

    std::cout << "Final - Copies: " << CopyTracker::copy_count
              << ", Moves: " << CopyTracker::move_count << std::endl;
}

// ============================================================================
// TEST: Raw pointer storage
// ============================================================================
TEST(Any, ConstructFromPointer) {
    std::cout << "\n--- Test: Construct from raw pointer ---\n";
    CopyTracker::reset_counters();

    {
        CopyTracker  original(200, "pointed");
        CopyTracker *ptr = &original;

        std::cout << "Creating Any from raw pointer...\n";
        Any a(ptr); // Stores the POINTER, not the object

        EXPECT_TRUE(a.has_value());

        // The Any stores a CopyTracker*, not a CopyTracker
        CopyTracker *retrieved = a.as<CopyTracker *>();
        EXPECT_EQ(retrieved, ptr);
        EXPECT_EQ(retrieved->value, 200);

        // NO copies should be made - we're storing the pointer itself
        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, 0);

        // Modify through retrieved pointer
        retrieved->value = 999;
        EXPECT_EQ(original.value, 999); // Original should be modified
    }

    std::cout << "Final - Copies: " << CopyTracker::copy_count
              << ", Moves: " << CopyTracker::move_count << std::endl;
}

// ============================================================================
// TEST: Const pointer storage
// ============================================================================
TEST(Any, ConstructFromConstPointer) {
    std::cout << "\n--- Test: Construct from const pointer ---\n";
    CopyTracker::reset_counters();

    {
        CopyTracker        original(300, "const_pointed");
        const CopyTracker *cptr = &original;

        std::cout << "Creating Any from const pointer...\n";
        Any a(cptr);

        EXPECT_TRUE(a.has_value());

        // Retrieve as const pointer
        const CopyTracker *retrieved = a.as<const CopyTracker *>();
        EXPECT_EQ(retrieved, cptr);
        EXPECT_EQ(retrieved->value, 300);

        // NO copies - just storing the pointer
        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, 0);
    }
}

// ============================================================================
// TEST: Reference wrapper (to store reference-like semantics)
// ============================================================================
TEST(Any, ConstructFromReferenceWrapper) {
    std::cout << "\n--- Test: Construct from std::reference_wrapper ---\n";
    CopyTracker::reset_counters();

    {
        CopyTracker original(400, "referenced");

        std::cout << "Creating Any from reference_wrapper...\n";
        Any a(std::ref(original));

        EXPECT_TRUE(a.has_value());

        // Access via reference_wrapper - the Any stores the wrapper
        // The const as<T>() has special handling for reference_wrapper
        const CopyTracker &ref = a.as<CopyTracker>();
        EXPECT_EQ(ref.value, 400);

        // NO copies of CopyTracker - reference_wrapper is lightweight
        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, 0);

        // Verify it's truly a reference - modify original
        original.value          = 888;
        const CopyTracker &ref2 = a.as<CopyTracker>();
        EXPECT_EQ(ref2.value, 888); // Should see the change
    }
}

// ============================================================================
// TEST: Const reference wrapper
// ============================================================================
TEST(Any, ConstructFromConstReferenceWrapper) {
    std::cout << "\n--- Test: Construct from std::cref (const reference_wrapper) ---\n";
    CopyTracker::reset_counters();

    {
        CopyTracker original(500, "const_referenced");

        std::cout << "Creating Any from const reference_wrapper...\n";
        Any a(std::cref(original));

        EXPECT_TRUE(a.has_value());

        // Access via const reference_wrapper
        auto &wrapper = a.as<std::reference_wrapper<const CopyTracker>>();
        EXPECT_EQ(wrapper.get().value, 500);

        // NO copies
        std::cout << "Copies: " << CopyTracker::copy_count << ", Moves: " << CopyTracker::move_count
                  << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, 0);

        // Verify reference semantics
        original.value = 777;
        EXPECT_EQ(wrapper.get().value, 777);
    }
}

// ============================================================================
// TEST: Any copy creates deep copy
// ============================================================================
TEST(Any, CopyingAnyMakesCopy) {
    std::cout << "\n--- Test: Copying Any makes deep copy ---\n";
    CopyTracker::reset_counters();

    {
        Any a1(CopyTracker(600, "deep_copy_test"));
        int copies_after_a1 = CopyTracker::copy_count;
        int moves_after_a1  = CopyTracker::move_count;

        std::cout << "After creating a1 - Copies: " << copies_after_a1
                  << ", Moves: " << moves_after_a1 << std::endl;

        std::cout << "Copying a1 to a2...\n";
        Any a2(a1); // Copy constructor

        std::cout << "After copying - Copies: " << CopyTracker::copy_count
                  << ", Moves: " << CopyTracker::move_count << std::endl;

        // Should have one more copy (deep copy of stored value)
        EXPECT_GT(CopyTracker::copy_count, copies_after_a1);

        // Verify independence
        a1.as<CopyTracker>().value = 111;
        EXPECT_EQ(a2.as<CopyTracker>().value, 600); // a2 should be unchanged
    }
}

// ============================================================================
// TEST: Any move avoids copy
// ============================================================================
TEST(Any, MovingAnyAvoidsCopy) {
    std::cout << "\n--- Test: Moving Any avoids copy ---\n";
    CopyTracker::reset_counters();

    {
        Any a1(CopyTracker(700, "move_test"));
        int copies_after_a1 = CopyTracker::copy_count;

        std::cout << "After creating a1 - Copies: " << copies_after_a1 << std::endl;

        std::cout << "Moving a1 to a2...\n";
        Any a2(std::move(a1)); // Move constructor

        std::cout << "After moving - Copies: " << CopyTracker::copy_count << std::endl;

        // Moving Any should NOT create additional copies of CopyTracker
        EXPECT_EQ(CopyTracker::copy_count, copies_after_a1);

        // a1 should be empty after move
        EXPECT_FALSE(a1.has_value());
        EXPECT_TRUE(a2.has_value());
        EXPECT_EQ(a2.as<CopyTracker>().value, 700);
    }
}

// ============================================================================
// TEST: Pointer to const vs const pointer
// ============================================================================
TEST(Any, PointerToConstVsConstPointer) {
    std::cout << "\n--- Test: Pointer to const vs const pointer ---\n";

    int        value        = 42;
    const int *ptr_to_const = &value; // Pointer to const int
    int *const const_ptr    = &value; // Const pointer to int

    // Store pointer-to-const
    Any a1(ptr_to_const);
    EXPECT_TRUE(a1.has_value());
    const int *retrieved1 = a1.as<const int *>();
    EXPECT_EQ(*retrieved1, 42);

    // Store const-pointer (the pointer itself is const, but points to mutable int)
    Any a2(const_ptr);
    EXPECT_TRUE(a2.has_value());
    // Note: The Any stores a copy of the pointer value, const-ness of the pointer
    // itself doesn't transfer in the same way
    int *retrieved2 = a2.as<int *>();
    EXPECT_EQ(*retrieved2, 42);

    // Modify through const_ptr stored in Any
    *retrieved2 = 100;
    EXPECT_EQ(value, 100);
}

// ============================================================================
// TEST: Storing shared_ptr (reference counting)
// ============================================================================
TEST(Any, SharedPtrStorage) {
    std::cout << "\n--- Test: shared_ptr storage ---\n";
    CopyTracker::reset_counters();

    std::weak_ptr<CopyTracker> weak;

    {
        auto ptr = std::make_shared<CopyTracker>(800, "shared");
        weak     = ptr;

        EXPECT_EQ(ptr.use_count(), 1);

        std::cout << "Creating Any from shared_ptr...\n";
        Any a(ptr);

        // shared_ptr should be copied, increasing ref count
        EXPECT_EQ(ptr.use_count(), 2);

        auto retrieved = a.as<std::shared_ptr<CopyTracker>>();
        EXPECT_EQ(retrieved.use_count(), 3); // ptr, Any's copy, retrieved
        EXPECT_EQ(retrieved->value, 800);

        // NO copies of CopyTracker itself
        std::cout << "CopyTracker copies: " << CopyTracker::copy_count << std::endl;
    }

    // After scope, weak should be expired
    EXPECT_TRUE(weak.expired());
}

// ============================================================================
// TEST: unique_ptr storage (move only)
// ============================================================================
TEST(Any, UniquePtrStorage) {
    std::cout << "\n--- Test: unique_ptr storage (move only) ---\n";
    CopyTracker::reset_counters();

    {
        auto ptr = std::make_unique<CopyTracker>(900, "unique");

        std::cout << "Moving unique_ptr into Any...\n";
        Any a(std::move(ptr));

        EXPECT_TRUE(a.has_value());
        CHECK(ptr == nullptr); // Original should be null after move

        // Check that it's marked as non-copyable
        EXPECT_FALSE(a.is_copyable());

        auto &retrieved = a.as<std::unique_ptr<CopyTracker>>();
        CHECK(retrieved != nullptr);
        EXPECT_EQ(retrieved->value, 900);

        // NO copies of CopyTracker
        std::cout << "CopyTracker copies: " << CopyTracker::copy_count << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, 0);
    }
}

// ============================================================================
// TEST: Copying Any with non-copyable type throws
// ============================================================================
TEST(Any, CopyingNonCopyableThrows) {
    std::cout << "\n--- Test: Copying Any with non-copyable type throws ---\n";
    CopyTracker::reset_counters();

    {
        auto ptr = std::make_unique<CopyTracker>(950, "non_copyable");
        Any  a(std::move(ptr));

        EXPECT_TRUE(a.has_value());
        EXPECT_FALSE(a.is_copyable());

        // Attempting to copy should throw std::logic_error
        std::cout << "Attempting to copy Any containing unique_ptr...\n";
        EXPECT_THROW(Any a2(a), std::logic_error);

        // Move should still work
        std::cout << "Moving Any containing unique_ptr...\n";
        Any a3(std::move(a));
        EXPECT_TRUE(a3.has_value());
        EXPECT_FALSE(a.has_value()); // Original moved-from

        EXPECT_EQ(a3.as<std::unique_ptr<CopyTracker>>()->value, 950);
    }
}

// ============================================================================
// TEST: is_copyable() method
// ============================================================================
TEST(Any, IsCopyableMethod) {
    std::cout << "\n--- Test: is_copyable() method ---\n";

    // Empty Any is copyable
    Any empty;
    EXPECT_TRUE(empty.is_copyable());

    // Copyable type
    Any a_int(42);
    EXPECT_TRUE(a_int.is_copyable());

    Any a_string(std::string("hello"));
    EXPECT_TRUE(a_string.is_copyable());

    Any a_shared(std::make_shared<int>(10));
    EXPECT_TRUE(a_shared.is_copyable());

    // Non-copyable type
    Any a_unique(std::make_unique<int>(20));
    EXPECT_FALSE(a_unique.is_copyable());
}

// ============================================================================
// TEST: Type mismatch throws
// ============================================================================
TEST(Any, TypeMismatchThrows) {
    std::cout << "\n--- Test: Type mismatch throws ---\n";

    int  value = 42;
    int *ptr   = &value;

    Any a(ptr);

    // Correct type works
    EXPECT_NO_THROW(a.as<int *>());

    // Wrong type throws
    EXPECT_THROW(a.as<double *>(), std::bad_cast);
    EXPECT_THROW(a.as<int>(), std::bad_cast);
    EXPECT_THROW(a.as<const int *>(), std::bad_cast); // Different type!
}

// ============================================================================
// TEST: Empty Any throws on access
// ============================================================================
TEST(Any, EmptyAnyThrows) {
    std::cout << "\n--- Test: Empty Any throws on access ---\n";

    Any empty;
    EXPECT_FALSE(empty.has_value());

    EXPECT_THROW(empty.as<int>(), std::bad_cast);
    EXPECT_THROW(empty.as<int *>(), std::bad_cast);
}

// ============================================================================
// TEST: Reset clears value
// ============================================================================
TEST(Any, ResetClearsValue) {
    std::cout << "\n--- Test: Reset clears value ---\n";
    CopyTracker::reset_counters();

    {
        Any a(CopyTracker(1000, "to_reset"));
        EXPECT_TRUE(a.has_value());

        int instances_before = CopyTracker::instance_count;
        std::cout << "Instances before reset: " << instances_before << std::endl;

        a.reset();

        EXPECT_FALSE(a.has_value());
        EXPECT_STREQ(a.type_name(), "empty");
    }
}

// ============================================================================
// TEST: Storing C-string converts to std::string
// ============================================================================
TEST(Any, CStringConvertsToString) {
    std::cout << "\n--- Test: C-string converts to std::string ---\n";

    const char *cstr = "hello world";
    Any         a(cstr);

    EXPECT_TRUE(a.has_value());

    // Should be stored as std::string, not const char*
    EXPECT_NO_THROW(a.as<std::string>());
    EXPECT_STREQ(a.as<std::string>(), "hello world");

    // This should throw because it's stored as string, not char*
    EXPECT_THROW(a.as<const char *>(), std::bad_cast);
}

// ============================================================================
// TEST: Assignment operator makes copy
// ============================================================================
TEST(Any, AssignmentMakesCopy) {
    std::cout << "\n--- Test: Assignment operator makes copy ---\n";
    CopyTracker::reset_counters();

    {
        Any a1(CopyTracker(1100, "assign_source"));
        Any a2;

        int copies_before = CopyTracker::copy_count;
        std::cout << "Copies before assignment: " << copies_before << std::endl;

        a2 = a1;

        std::cout << "Copies after assignment: " << CopyTracker::copy_count << std::endl;
        EXPECT_GT(CopyTracker::copy_count, copies_before);

        // Verify independence
        a1.as<CopyTracker>().value = 222;
        EXPECT_EQ(a2.as<CopyTracker>().value, 1100);
    }
}

// ============================================================================
// TEST: Move assignment avoids copy
// ============================================================================
TEST(Any, MoveAssignmentAvoidsCopy) {
    std::cout << "\n--- Test: Move assignment avoids copy ---\n";
    CopyTracker::reset_counters();

    {
        Any a1(CopyTracker(1200, "move_assign_source"));
        Any a2;

        int copies_before = CopyTracker::copy_count;
        std::cout << "Copies before move assignment: " << copies_before << std::endl;

        a2 = std::move(a1);

        std::cout << "Copies after move assignment: " << CopyTracker::copy_count << std::endl;
        EXPECT_EQ(CopyTracker::copy_count, copies_before);

        EXPECT_FALSE(a1.has_value());
        EXPECT_TRUE(a2.has_value());
        EXPECT_EQ(a2.as<CopyTracker>().value, 1200);
    }
}

// ============================================================================
// TEST: Type info consistency
// ============================================================================
TEST(Any, TypeInfoConsistency) {
    std::cout << "\n--- Test: Type info consistency ---\n";

    int        i   = 42;
    int       *pi  = &i;
    const int *cpi = &i;

    Any a_int(i);
    Any a_ptr(pi);
    Any a_cptr(cpi);

    // Each should have different type_index
    EXPECT_TRUE(a_int.get_type_index() != a_ptr.get_type_index());
    EXPECT_TRUE(a_ptr.get_type_index() != a_cptr.get_type_index());

    // Type indices should match expected types
    EXPECT_TRUE(a_int.get_type_index() == std::type_index(typeid(int)));
    EXPECT_TRUE(a_ptr.get_type_index() == std::type_index(typeid(int *)));
    EXPECT_TRUE(a_cptr.get_type_index() == std::type_index(typeid(const int *)));
}

// ============================================================================
// TEST: Pointer stability check
// ============================================================================
TEST(Any, PointerStabilityCheck) {
    std::cout << "\n--- Test: Pointer stability after Any operations ---\n";

    int  original = 42;
    int *ptr      = &original;

    Any a(ptr);

    // Get the stored pointer multiple times
    int *p1 = a.as<int *>();
    int *p2 = a.as<int *>();

    // Should be the same pointer value
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(p1, ptr);

    // Modifying through any of them affects the original
    *p1 = 100;
    EXPECT_EQ(original, 100);
    EXPECT_EQ(*p2, 100);
}

// ============================================================================
// Summary of copy behavior
// ============================================================================
TEST(Any, SummaryOfCopyBehavior) {
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "SUMMARY: Any class copy behavior\n";
    std::cout << "============================================================\n";
    std::cout << "1. Any(T value)          -> COPIES the value into storage\n";
    std::cout << "2. Any(T&& rvalue)       -> MOVES the value (if movable)\n";
    std::cout << "3. Any(T* pointer)       -> Stores the POINTER (no copy of T)\n";
    std::cout << "4. Any(const T* cptr)    -> Stores the POINTER (no copy of T)\n";
    std::cout << "5. Any(std::ref(x))      -> Stores reference_wrapper (no copy)\n";
    std::cout << "6. Any copy ctor         -> DEEP COPY of stored value\n";
    std::cout << "7. Any move ctor         -> Transfers ownership (no copy)\n";
    std::cout << "8. Any(shared_ptr<T>)    -> Copies shared_ptr (ref count++)\n";
    std::cout << "9. Any(unique_ptr<T>)    -> Moves unique_ptr (no copy)\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "NOTE: Non-copyable types (like unique_ptr) can be stored!\n";
    std::cout << "      Copying an Any with non-copyable type throws at runtime.\n";
    std::cout << "      Use is_copyable() to check before copying.\n";
    std::cout << "============================================================\n";

    EXPECT_TRUE(true); // Always passes - this is documentation
}

RUN_TESTS()