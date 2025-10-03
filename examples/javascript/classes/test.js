// examples/javascript/classes/test.js
// Simple test for pointer handling

const rosetta = require('./build/Release/rosetta')

console.log('=== Pointer Handling Test ===\n')

// Test 1: Basic pointer retrieval
console.log('Test 1: Get pointer from B')
console.log('---------------------------')
const b = new rosetta.B(42)
console.log('Created B(42)')

const a = b.getA()
console.log('Called b.getA():', a)

if (!a || a === null || a === undefined) {
    console.error('❌ FAIL: b.getA() returned', a)
    console.error('   Expected: A object')
    process.exit(1)
}
console.log('✓ PASS: b.getA() returned an object\n')

// Test 2: Access property through pointer
console.log('Test 2: Read property through pointer')
console.log('--------------------------------------')
try {
    const value = a.value
    console.log('a.value =', value)

    if (value !== 42) {
        console.error('❌ FAIL: Expected value=42, got value=' + value)
        process.exit(1)
    }

    console.log('✓ PASS: Can read property through pointer\n')
} catch (e) {
    console.error('❌ FAIL: Could not access a.value')
    console.error('   Error:', e.message)
    process.exit(1)
}

// Test 3: Modify through pointer (CRITICAL TEST)
console.log('Test 3: Modify through pointer')
console.log('-------------------------------')
try {
    console.log('Setting a.value = 100')
    a.value = 100

    console.log('Reading back a.value:', a.value)
    console.log('Reading b.getAValue():', b.getAValue())

    const directValue = b.getAValue()

    if (directValue !== 100) {
        console.error('❌ FAIL: Pointer is not referencing original object')
        console.error('   Expected b.getAValue()=100, got', directValue)
        console.error('   This means the pointer was copied, not referenced')
        process.exit(1)
    }

    console.log('✓ PASS: Pointer correctly references original object\n')
} catch (e) {
    console.error('❌ FAIL: Could not modify through pointer')
    console.error('   Error:', e.message)
    process.exit(1)
}

// Test 4: Multiple pointer accesses
console.log('Test 4: Multiple pointer accesses')
console.log('----------------------------------')
try {
    const a1 = b.getA()
    const a2 = b.getA()

    console.log('Got two pointers: a1 and a2')

    a1.value = 200
    console.log('Set a1.value = 200')

    const v2 = a2.value
    console.log('Read a2.value:', v2)

    if (v2 !== 200) {
        console.error('❌ FAIL: Multiple pointers not referencing same object')
        console.error('   Expected a2.value=200, got', v2)
        process.exit(1)
    }

    console.log('✓ PASS: Multiple pointers reference same object\n')
} catch (e) {
    console.error('❌ FAIL: Multiple pointer test failed')
    console.error('   Error:', e.message)
    process.exit(1)
}

// Test 5: Create standalone A and compare
console.log('Test 5: Standalone object vs pointer')
console.log('-------------------------------------')
try {
    const standalone = new rosetta.A(999)
    console.log('Created standalone A(999)')
    console.log('standalone.value =', standalone.value)

    const pointer = b.getA()
    console.log('pointer.value =', pointer.value)

    if (standalone.value === pointer.value) {
        console.error('❌ FAIL: Standalone and pointer have same value')
        console.error('   They should be different objects')
        process.exit(1)
    }

    console.log('✓ PASS: Standalone and pointer are different objects\n')
} catch (e) {
    console.error('❌ FAIL: Standalone object test failed')
    console.error('   Error:', e.message)
    process.exit(1)
}

console.log('=================================')
console.log('✓ ALL TESTS PASSED!')
console.log('=================================')
console.log('Pointer handling is working correctly.')
console.log('Changes through pointer affect the original C++ object.')