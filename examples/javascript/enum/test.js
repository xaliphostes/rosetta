const rosetta = require('./build/Release/rosetta');

// Access enum values
console.log('Status enum:', rosetta.Status);
// Output: { Active: 0, Inactive: 1, Pending: 2 }

console.log('Priority enum:', rosetta.Priority);
// Output: { Low: 0, Medium: 1, High: 2, Critical: 3 }

// Create task
const task = new rosetta.Task();

// Set status using enum
task.setStatus(rosetta.Status.Active);
console.log('Status:', task.getStatus()); // 0
console.log('Status name:', task.getStatusName()); // "Active"

// Set priority
task.setPriority(rosetta.Priority.High);
console.log('Priority:', task.getPriority()); // 2

// Direct property access also works
task.status = rosetta.Status.Inactive;
console.log('Status:', task.status); // 1

// Can also use numeric values directly
task.status = 2; // Pending
console.log('Status name:', task.getStatusName()); // "Pending"

// Enums are frozen (immutable)
try {
    rosetta.Status.NewValue = 99; // Throws error
} catch (e) {
    console.log('Cannot modify enum');
}