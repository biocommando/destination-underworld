/*
    BAKE is a dead-simple task executor that executes a set of JS functions
    in a sequence. Aimed for a usecase where you're too lazy to setup any
    more proper build tooling.

    Usage:
    Create bake.json. It contains keys that define the task sequences. Each
    task is defined as a string, and is interpreted as being a JS file name
    located at relative path ./bake/. The bake.json can have following special
    keys:
    - default: default task sequence in case none is defined
    - verbose: enable verbose logging

    You can reference other task sequences using syntax "$taskSeqName".

    Each JS file should contain a function with signature (variables: object) => undefined.
    The variables object contains a few commonly used helper variables:
    - fs, the fs core library
    - sh, execSync from child_process
    - log, logging with console.log
    - debug, logging with console.log if verbose = true

    Example:
    
    bake.json:
    {
        "debug": [ "set_debug_compiler", "generate_hello_world", "compile_hello_world" ],
        "default": "debug"
    }
    
    bake/set_debug_compiler.js:
    module.exports = v => v.compiler = 'gcc'

    bake/generate_hello_world.js:
    module.exports = v => v.fs.writeFileSync('hello.c', '#include<stdio.h>\n int main() { puts("Hello world!"); return 0; } ')

    bake/compile_hello_world.js
    module.exports = v => v.sh(`${v.compiler} hello.c`)

    Run the config using command: node bake.js
*/

const fs = require('fs')

const configName = process.argv.pop()

const configFile = JSON.parse(fs.readFileSync('bake.json').toString())

const log = console.log
const debug = configFile.verbose ? log : () => {}

let baseConfig = configFile[configName]

if (!baseConfig) {
    debug(`No config matching ${configName}, fall back to ${configFile.default}`)
    baseConfig = configFile[configFile.default]
}

const scriptVariables = {
    fs, sh: require('child_process').execSync,
    log, debug
}

function executeConfig(config) {
    debug('Executing config', config)

    config.forEach(task => {
        if (task.startsWith('$')) {
            executeConfig(configFile[task.slice(1)])
            return
        }

        log(`Executing file ./bake/${task}.js`)
        const taskFn = require('./bake/' + task)
        taskFn(scriptVariables)
    })
}

executeConfig(baseConfig)

log('Done')