#!/usr/bin/env node
// ============================================================================
// check_napi.js - Vérification de l'installation Node-API
// ============================================================================

const fs = require('fs');
const path = require('path');

console.log('='.repeat(70));
console.log('Node-API Installation Check');
console.log('='.repeat(70));

// ============================================================================
// 1. Vérifier Node.js
// ============================================================================

console.log('\n📦 Node.js Version:');
console.log(`   ${process.version}`);

if (parseInt(process.version.slice(1)) < 14) {
    console.log('   ⚠️  Warning: Node.js 14+ recommended for Node-API');
} else {
    console.log('   ✅ Node.js version OK');
}

// ============================================================================
// 2. Vérifier node-addon-api
// ============================================================================

console.log('\n📦 Checking node-addon-api:');

let napiInstalled = false;
let napiPath = null;

try {
    napiPath = require.resolve('node-addon-api');
    napiInstalled = true;
    console.log('   ✅ node-addon-api is installed');
    console.log(`   📁 Location: ${napiPath}`);
} catch (e) {
    console.log('   ❌ node-addon-api NOT installed');
    console.log('   💡 Install with: npm install node-addon-api');
}

// ============================================================================
// 3. Vérifier le header node_api.h
// ============================================================================

console.log('\n📦 Checking node_api.h header:');

let nodeApiHeader = null;

// Chemins possibles pour node_api.h
const possiblePaths = [
    path.join(process.execPath, '../../include/node/node_api.h'),
    path.join(process.execPath, '../include/node/node_api.h'),
    '/usr/include/node/node_api.h',
    '/usr/local/include/node/node_api.h',
    path.join(process.env.NODE_PATH || '', 'node_api.h')
];

for (const p of possiblePaths) {
    if (fs.existsSync(p)) {
        nodeApiHeader = p;
        break;
    }
}

if (nodeApiHeader) {
    console.log('   ✅ node_api.h found');
    console.log(`   📁 Location: ${nodeApiHeader}`);
} else {
    console.log('   ⚠️  node_api.h not found in standard locations');
    console.log('   💡 This is OK - it\'s included with Node.js');
}

// ============================================================================
// 4. Vérifier node-gyp
// ============================================================================

console.log('\n📦 Checking node-gyp:');

const { execSync } = require('child_process');

try {
    const gypVersion = execSync('node-gyp --version', { encoding: 'utf8' }).trim();
    console.log(`   ✅ node-gyp is installed: ${gypVersion}`);
} catch (e) {
    console.log('   ❌ node-gyp NOT installed');
    console.log('   💡 Install with: npm install -g node-gyp');
}

// ============================================================================
// 5. Vérifier les include paths que node-addon-api utilise
// ============================================================================

if (napiInstalled) {
    console.log('\n📦 Node-addon-api include paths:');
    
    try {
        const napiInclude = require('node-addon-api').include;
        console.log(`   ✅ Include path: ${napiInclude}`);
        
        // Vérifier que le dossier existe
        if (fs.existsSync(napiInclude)) {
            console.log('   ✅ Include directory exists');
            
            // Lister les fichiers
            const files = fs.readdirSync(napiInclude);
            console.log(`   📄 Files found: ${files.length}`);
            
            // Vérifier napi.h
            if (files.includes('napi.h')) {
                console.log('   ✅ napi.h found');
            } else {
                console.log('   ⚠️  napi.h not found');
            }
        } else {
            console.log('   ❌ Include directory does not exist!');
        }
    } catch (e) {
        console.log(`   ❌ Error: ${e.message}`);
    }
}

// ============================================================================
// 6. Tester un simple binding
// ============================================================================

console.log('\n📦 Testing simple binding compilation:');

const testBinding = `
#include <node_api.h>

napi_value Init(napi_env env, napi_value exports) {
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
`;

const testBindingGyp = `
{
  "targets": [{
    "target_name": "test",
    "sources": ["test.cpp"],
    "include_dirs": [
      "<!@(node -p \\"require('node-addon-api').include\\")"
    ]
  }]
}
`;

const testDir = path.join(__dirname, 'napi_test_temp');

try {
    // Créer un dossier de test temporaire
    if (!fs.existsSync(testDir)) {
        fs.mkdirSync(testDir);
    }
    
    // Écrire les fichiers de test
    fs.writeFileSync(path.join(testDir, 'test.cpp'), testBinding);
    fs.writeFileSync(path.join(testDir, 'binding.gyp'), testBindingGyp);
    
    console.log('   📝 Created test files');
    
    // Essayer de configurer
    process.chdir(testDir);
    execSync('node-gyp configure', { stdio: 'pipe' });
    console.log('   ✅ node-gyp configure successful');
    
    // Nettoyer
    process.chdir(__dirname);
    fs.rmSync(testDir, { recursive: true, force: true });
    
    console.log('   ✅ Basic binding test PASSED');
    
} catch (e) {
    console.log('   ❌ Basic binding test FAILED');
    console.log(`   Error: ${e.message}`);
    
    // Nettoyer en cas d'erreur
    try {
        process.chdir(__dirname);
        if (fs.existsSync(testDir)) {
            fs.rmSync(testDir, { recursive: true, force: true });
        }
    } catch (cleanupError) {
        // Ignorer les erreurs de nettoyage
    }
}

// ============================================================================
// 7. Afficher les variables d'environnement pertinentes
// ============================================================================

console.log('\n📦 Environment variables:');

const relevantEnvVars = [
    'NODE_PATH',
    'NODE_GYP_FORCE_PYTHON',
    'npm_config_node_gyp',
    'PYTHON'
];

for (const envVar of relevantEnvVars) {
    if (process.env[envVar]) {
        console.log(`   ${envVar} = ${process.env[envVar]}`);
    }
}

// ============================================================================
// 8. Résumé et recommandations
// ============================================================================

console.log('\n' + '='.repeat(70));
console.log('Summary & Recommendations');
console.log('='.repeat(70));

const issues = [];
const recommendations = [];

if (!napiInstalled) {
    issues.push('node-addon-api not installed');
    recommendations.push('Run: npm install node-addon-api');
}

try {
    execSync('node-gyp --version', { stdio: 'pipe' });
} catch (e) {
    issues.push('node-gyp not installed');
    recommendations.push('Run: npm install -g node-gyp');
}

// Vérifier Python (requis par node-gyp)
try {
    const pythonVersion = execSync('python --version', { encoding: 'utf8', stdio: 'pipe' });
    console.log(`\n🐍 Python: ${pythonVersion.trim()}`);
} catch (e) {
    try {
        const python3Version = execSync('python3 --version', { encoding: 'utf8', stdio: 'pipe' });
        console.log(`\n🐍 Python: ${python3Version.trim()}`);
    } catch (e2) {
        issues.push('Python not found');
        recommendations.push('Install Python 3.x (required by node-gyp)');
    }
}

if (issues.length === 0) {
    console.log('\n✅ All checks passed! You\'re ready to build Node-API addons.');
} else {
    console.log('\n⚠️  Issues found:');
    issues.forEach(issue => console.log(`   - ${issue}`));
    
    console.log('\n💡 Recommendations:');
    recommendations.forEach(rec => console.log(`   - ${rec}`));
}

console.log('\n' + '='.repeat(70));

// ============================================================================
// 9. Commandes utiles
// ============================================================================

console.log('\n📚 Useful commands:');
console.log('   npm install node-addon-api     - Install Node-API wrapper');
console.log('   npm install -g node-gyp        - Install node-gyp globally');
console.log('   node-gyp configure             - Configure build');
console.log('   node-gyp build                 - Build addon');
console.log('   node-gyp rebuild               - Clean + configure + build');
console.log('   node-gyp rebuild --verbose     - Build with verbose output');

console.log('\n');