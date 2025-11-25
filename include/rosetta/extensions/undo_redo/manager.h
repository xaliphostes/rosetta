// ============================================================================
// Complete Undo/Redo Using Rosetta Reflection
// ============================================================================

#pragma once
#include "../../rosetta.h"
#include <iomanip>
#include <iostream>
#include <memory>

namespace rosetta::core {

    class PropertyChangeCommand;
    class ObjectMemento;

    /**
     * @brief Generic undo/redo manager for tracking and reverting property changes
     *
     * @details
     * UndoRedoManager provides a type-safe, reflection-based system for implementing
     * undo/redo functionality. It works with any class registered with the Rosetta
     * reflection system, automatically capturing and restoring field values through
     * their registered getters and setters.
     *
     * The manager uses the Command pattern to encapsulate each property change as
     * a reversible operation. Changes are stored in a stack-based history that supports
     * both undo (reverting to previous states) and redo (reapplying undone changes).
     *
     * **Key Features:**
     * - Generic: Works with any type registered via ClassMetadata
     * - Type-safe: Values are stored in type-erased Any containers with runtime checking
     * - Automatic: Uses reflection to access fields via getters/setters
     * - Branch-aware: Creating new changes after undo clears the redo stack
     * - Descriptive: Optional human-readable descriptions for each change
     *
     * **Architecture:**
     * ```
     * Change Applied
     *     ↓
     * [Undo Stack] ← Active history
     *     ↓ undo()
     * [Redo Stack] ← Undone operations
     *     ↓ redo()
     * [Undo Stack] ← Reapplied
     * ```
     *
     * **Thread Safety:**
     * This class is NOT thread-safe. External synchronization is required if
     * accessed from multiple threads.
     *
     * **Memory Considerations:**
     * - Each command stores two copies of the field value (old and new)
     * - For large objects, consider using snapshot-based HistoryManager instead
     * - Limit history size in production to prevent unbounded memory growth
     *
     * @code{.cpp}
     * // Register class
     * Registry::instance().add<Transform>("Transform")
     *     .property("x", &Transform::getX, &Transform::setX)
     *     .property("y", &Transform::getY, &Transform::setY);
     *
     * // Use undo/redo manager
     * Transform transform;
     * UndoRedoManager manager;
     *
     * // Apply tracked changes
     * manager.applyChange(&transform, "x", 100.0, "Move right");
     * manager.applyChange(&transform, "y", 50.0, "Move down");
     *
     * // Undo changes
     * manager.undo();  // y back to 0
     * manager.undo();  // x back to 0
     *
     * // Redo changes
     * manager.redo();  // x back to 100
     * manager.redo();  // y back to 50
     * @endcode
     *
     * @see PropertyChangeCommand
     * @see HistoryManager For snapshot-based undo/redo
     * @see ClassMetadata For registering properties
     * @see Any For type-erased value storage
     *
     * @note All field access must go through registered properties (getters/setters).
     *       Direct field modification will not be tracked.
     *
     * @warning Modifying objects outside of applyChange() will break undo/redo consistency.
     *          Always use the manager to apply changes you want to track.
     *
     * @since Rosetta 1.0
     */
    class UndoRedoManager {
    public:
        /**
         * @brief Apply a property change and record it for undo/redo
         *
         * @details
         * Captures the current field value, applies the new value, and creates a
         * reversible command that is pushed onto the undo stack. If there are any
         * operations in the redo stack, they are cleared (new timeline branch).
         *
         * The change is applied through the object's registered metadata, invoking
         * the setter method associated with the field. The old and new values are
         * stored in type-erased Any containers.
         *
         * @tparam T Type of the object being modified (must be registered)
         * @tparam ValueType Type of the new value (must match field type)
         *
         * @param object Pointer to the object to modify
         * @param field_name Name of the field/property to change
         * @param new_value New value to apply
         * @param description Optional human-readable description of the change
         *
         * @throws std::runtime_error If the class is not registered
         * @throws std::out_of_range If the field name is not found
         * @throws std::bad_cast If ValueType doesn't match the field type
         *
         * @pre object must not be nullptr
         * @pre field_name must exist in T's registered metadata
         * @pre ValueType must be compatible with the field's type
         *
         * @post Object's field is updated to new_value
         * @post Command is pushed to undo stack
         * @post Redo stack is cleared
         *
         * @code{.cpp}
         * Shape shape;
         * UndoRedoManager mgr;
         *
         * mgr.applyChange(&shape, "color", std::string("red"), "Set to red");
         * mgr.applyChange(&shape, "rotation", 45.0, "Rotate 45 degrees");
         * @endcode
         */
        template <typename T, typename ValueType>
        void applyChange(T *object, const std::string &field_name, const ValueType &new_value,
                         const std::string &description = "");

        void undo();
        void redo();

        bool canUndo() const;
        bool canRedo() const;

        void clear();

        size_t undoCount() const;
        size_t redoCount() const;

        void printHistory() const;

    private:
        std::vector<PropertyChangeCommand> undo_stack_;
        std::vector<PropertyChangeCommand> redo_stack_;
        std::vector<std::string>           descriptions_;
        std::vector<std::string>           redo_descriptions_;
    };

    // ============================================================================
    // State Snapshot System (Memento Pattern)
    // ============================================================================

    /**
     * @brief Snapshot-based history manager implementing the Memento pattern
     *
     * @details
     * HistoryManager provides a coarse-grained undo/redo system that captures complete
     * object state snapshots rather than individual field changes. Each snapshot (memento)
     * stores the entire state of an object at a specific point in time, allowing
     * navigation through distinct configurations.
     *
     * Unlike UndoRedoManager which tracks individual property changes, HistoryManager
     * is ideal for scenarios where:
     * - Multiple fields change together as a logical unit
     * - You want named save points (e.g., "Draft 1", "Final Version")
     * - State transitions are discrete and well-defined
     * - Memory overhead of full snapshots is acceptable
     *
     * **Architecture:**
     * ```
     * [Snapshot 0] ← Initial state
     *      ↓
     * [Snapshot 1] ← First save point
     *      ↓
     * [Snapshot 2] ← Second save point  ← current_index_
     *      ↓
     * [Snapshot 3] ← (discarded if new save at index 2)
     * ```
     *
     * **Memory Characteristics:**
     * - Each snapshot stores a complete copy of all registered fields
     * - Memory usage: O(n * m) where n = snapshots, m = fields per object
     * - Suitable for moderate history sizes (tens to hundreds of snapshots)
     * - Consider implementing snapshot limits or compression for large objects
     *
     * **Comparison with UndoRedoManager:**
     *
     * | Feature              | HistoryManager           | UndoRedoManager          |
     * |---------------------|--------------------------|--------------------------|
     * | Granularity         | Whole object snapshots   | Individual field changes |
     * | Memory per change   | O(total fields)          | O(1) per field          |
     * | Best for            | Configuration presets    | Interactive editing      |
     * | Named checkpoints   | Yes                      | Optional descriptions    |
     * | Batch operations    | Natural                  | Multiple commands        |
     *
     * **Thread Safety:**
     * This class is NOT thread-safe. External synchronization required for
     * concurrent access.
     *
     * @code{.cpp}
     * // Register class
     * Registry::instance().add<Document>("Document")
     *     .property("title", &Document::getTitle, &Document::setTitle)
     *     .property("content", &Document::getContent, &Document::setContent)
     *     .property("author", &Document::getAuthor, &Document::setAuthor);
     *
     * Document doc;
     * HistoryManager history;
     *
     * // Save initial state
     * history.saveState(&doc, "Initial draft");
     *
     * // Make changes and save configuration
     * doc.setTitle("My Article");
     * doc.setContent("Lorem ipsum...");
     * doc.setAuthor("John Doe");
     * history.saveState(&doc, "First complete draft");
     *
     * // More changes
     * doc.setTitle("My Updated Article");
     * doc.setContent("Updated content...");
     * history.saveState(&doc, "Revision 1");
     *
     * // Navigate history
     * history.undo(&doc);  // Back to "First complete draft"
     * history.undo(&doc);  // Back to "Initial draft"
     * history.redo(&doc);  // Forward to "First complete draft"
     * @endcode
     *
     * @see ObjectMemento For the snapshot storage mechanism
     * @see UndoRedoManager For fine-grained field-level undo/redo
     * @see captureState() For manual state capture
     * @see restoreState() For manual state restoration
     *
     * @note Snapshots are created explicitly via saveState(). Modifications made
     *       between snapshots are not tracked automatically.
     *
     * @warning Creating a new snapshot after undo operations will discard all
     *          forward history (standard undo/redo behavior).
     *
     * @since Rosetta 1.0
     */
    class HistoryManager {
    public:
        template <typename T> void saveState(T *object, const std::string &label = "");
        template <typename T> void undo(T *object);
        template <typename T> void redo(T *object);
        bool                       canUndo() const;
        bool                       canRedo() const;
        void                       printHistory() const;

    private:
        std::vector<ObjectMemento> history_;
        int                        current_index_ = -1;
    };

} // namespace rosetta::core

#include "inline/manager.hxx"