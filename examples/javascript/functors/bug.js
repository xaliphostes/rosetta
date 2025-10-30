const addon = require('./build/Release/functors.node');

console.log('Testing VectorTransform step by step...\n');

try {
    console.log('[1] Creating VectorTransform...');
    const transform = new addon.VectorTransform();
    console.log('[2] Created successfully');
    
    console.log('[3] Setting scale to 2.0...');
    transform.set_scale(2.0);
    console.log('[4] Scale set successfully');
    
    console.log('[5] Setting offset to 3.0...');
    transform.set_offset(3.0);
    console.log('[6] Offset set successfully');
    
    console.log('[7] Creating input array...');
    const input = [1, 2, 3, 4, 5];
    console.log('[8] Input array:', input);
    
    console.log('[9] About to call transform.call(input)...');
    const output = transform.call(input);
    console.log('[10] Call returned!');
    
    console.log('[11] Output:', output);
    
} catch (e) {
    console.log('ERROR:', e.message);
    console.log('Stack:', e.stack);
}