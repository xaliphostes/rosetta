const rosetta = require('./build/Release/rosetta');

console.log('=== Automatic JavaScript Bindings Test ===\n');

// Create objects using constructors
// -------------------------------------------------------
console.log('1. Creating objects...');
const person = new rosetta.Person("Coucou", 12, 1.74);
const vehicle = new rosetta.Vehicle("BMW", "Serie 3", 2021);

console.log('Person created:', person.getClassName());
console.log('Vehicle created:', vehicle.getClassName());
console.log(`Person: ${person.name}, age ${person.age}, height ${person.height}m`);
console.log(`Vehicle: ${vehicle.brand} ${vehicle.model} (${vehicle.year}), ${vehicle.mileage} miles`);
console.log();

// Test direct property access (natural JavaScript style)
// -------------------------------------------------------
console.log('2. Direct property access...');
person.name = "Alice";
person.age = 30;
person.height = 1.65;
person.isActive = true;

vehicle.brand = "Honda";
vehicle.model = "Civic";
vehicle.year = 2022;
vehicle.mileage = 5000.5;

console.log(`Person: ${person.name}, age ${person.age}, height ${person.height}m`);
console.log(`Vehicle: ${vehicle.brand} ${vehicle.model} (${vehicle.year}), ${vehicle.mileage} miles`);
console.log();

// Test method calls
// -------------------------------------------------------
console.log('3. Method calls...');
person.introduce();
person.celebrateBirthday();
console.log('Description:', person.getDescription());

vehicle.start();
vehicle.drive(100.5);
console.log('Info:', vehicle.getInfo());
vehicle.stop();
console.log();

// Test introspection utilities
// -------------------------------------------------------
console.log('4. Introspection utilities...');
console.log('Person class name:', person.getClassName());
console.log('Person members:', person.getMemberNames());
console.log('Person methods:', person.getMethodNames());

console.log('Has "name" member:', person.hasMember("name"));
console.log('Has "weight" member:', person.hasMember("weight"));
console.log('Has "introduce" method:', person.hasMethod("introduce"));
console.log();

// Test dynamic access
// -------------------------------------------------------
console.log('5. Dynamic member access...');
console.log('Age via introspection:', person.getMemberValue("age"));
person.setMemberValue("height", 1.80);
console.log('Height after dynamic set:', person.getMemberValue("height"));
console.log();

// Test dynamic method calls
// -------------------------------------------------------
console.log('6. Dynamic method calls...');
person.callMethod("introduce", []);
const description = person.callMethod("getDescription", []);
console.log('Description from dynamic call:', description);
console.log();

// Test JSON export
// -------------------------------------------------------
console.log('7. JSON export...');
console.log('Person as JSON:');
console.log(person.toJSON());
console.log();

// Test property getters/setters (alternative syntax)
// -------------------------------------------------------
console.log('8. Getter/setter methods...');
console.log('Person name via getter:', person.getName());
person.setName("Bob");
console.log('Person name after setter:', person.getName());
console.log();

// Test property enumeration
// -------------------------------------------------------
console.log('9. Property enumeration...');
console.log('Person properties:');
for (const prop in person) {
    console.log(`  ${prop}: ${typeof person[prop]}`);
}
console.log();

// Test advanced scenarios
// -------------------------------------------------------
console.log('10. Advanced scenarios...');

// // Chain method calls
// person.setName("Charlie").setAge(35).setHeight(1.75);
// console.log('After chaining (if supported):', person.getDescription());

// Batch property update
const personData = {
    name: "Diana",
    age: 28,
    height: 1.68,
    isActive: false
};

Object.keys(personData).forEach(key => {
    if (person.hasMember(key)) {
        person.setMemberValue(key, personData[key]);
    }
});

console.log('After batch update:', person.getDescription());
console.log();

// Performance test
// -------------------------------------------------------
console.log('11. Performance test...');
const startTime = Date.now();
for (let i = 0; i < 1000; i++) {
    person.setAge(i % 100);
    person.getAge();
}
const endTime = Date.now();
console.log(`1000 property operations took: ${endTime - startTime}ms`);
console.log();

console.log('=== All tests completed successfully! ===');