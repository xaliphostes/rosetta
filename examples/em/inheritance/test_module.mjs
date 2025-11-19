import createModule from './inheritancejs.js';
import { writeFileSync } from 'fs';

async function runTests() {
    console.log('Loading WebAssembly module...');
    let Module = await createModule();
    // enhanceRosettaClasses(Module);
    
    console.log('Module loaded!');

    // Export TypeScript declarations
    const tsDeclarations = Module.generateTypeScript();
    writeFileSync('module.d.ts', tsDeclarations);

    // Create shapes
    const circle = new Module.Circle2D(5.0);
    const rect = new Module.Rectangle2D(4.0, 3.0);
    const square = new Module.Square(2.0);

    // Use inherited methods
    console.log(circle.describe());      // "Circle with area 78.54..."
    console.log(rect.describe());        // "Rectangle with area 12.0"
    console.log(square.describe());      // "Square with area 4.0"

    // Access properties
    console.log(circle.radius);          // 5.0
    console.log(rect.width, rect.height);// 4.0 3.0
    console.log(square.side);            // 2.0

    // Call virtual methods through base class behavior
    console.log(circle.area());          // 78.54...
    console.log(rect.area());            // 12.0
    console.log(square.area());          // 4.0

    // Use free functions with derived classes
    console.log(Module.totalArea(circle, rect));  // 90.54...

    // Check inheritance
    console.log(circle.$instanceof("Shape"));      // true
    console.log(square.$instanceof("Rectangle2D"));// true
    console.log(square.$instanceof("Shape"));      // true (two levels up)

    // Get inheritance info
    const info = Module.getInheritanceInfo("Square");
    console.log(info.bases);      // ["Rectangle2D"]
    console.log(info.ancestors);  // ["Rectangle2D", "Shape"]

    // Animals
    const dog = new Module.Dog("Rex", 5, "German Shepherd");
    const cat = new Module.Cat("Luna", 3, true);

    console.log(dog.introduce());  // "I'm Rex, a 5 year old German Shepherd dog. Woof!"
    console.log(cat.introduce());  // "I'm Luna, a 3 year old indoor cat. Meow!"

    console.log(Module.makeConversation(dog, cat));
    // "Rex says: Woof! Luna replies: Meow!"

    // Check type info
    console.log(Module.Shape.$isAbstract());   // true
    console.log(Module.Circle2D.$isAbstract()); // false

    // Get metadata with inheritance
    const dogMeta = Module.Dog.$meta();
    console.log(dogMeta.bases);    // ["Animal"]
    console.log(dogMeta.isPolymorphic); // true
}

runTests().catch(err => {
    console.error('Error:', err);
    process.exit(1);
});
