const inputFile = process.argv.pop()

const fs = require('fs')

let cfile, hfile, statename, commandsname, commandname
let debug_print, fieldsetters
let command_idx = 0

function write_files() {
    if (cfile) {
        cfile += '}\n' + debug_print + '\n' + fieldsetters + '\n'
        fs.writeFileSync(`dispatch_${commandsname}.h`, hfile)
        fs.writeFileSync(`dispatch_${commandsname}.c`, cfile)
        cfile = undefined
        hfile = undefined
    }
}

function init(name) {
    write_files()
    commandsname = name
    cfile = `
#include "dispatch_${name}.h"

void dispatch__${name}(command_file_DispatchDto *dto)
{
`

    hfile = `#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <command_file.h>
#include "$$STATE_H$$"

void dispatch__${name}(command_file_DispatchDto *dto);
`

    statename = 'State'
    commandname = 'Command'
    debug_print = ''
    fieldsetters = ''
}
const json = JSON.parse(fs.readFileSync(inputFile).toString())

Object.keys(json)
    .forEach(commandSetName => {
        const def = json[commandSetName]
        init(commandSetName)
        const config = def[':config']
        statename = config.state
        const prefix = config.prefix || `${commandSetName}_`
        const defaultHandler = !!config.default
        const defaultRequired = !!config.defaultrequired
        hfile = hfile.replace('$$STATE_H$$', 'state_' + statename + '.h')
        cfile += `    ${statename} *state = (${statename} *)dto->state;\n{REQUIRED_FLAGS_INIT_PLACEHOLDER}\n`
        let requiredCount = 0
        Object.keys(def).filter(x => x !== ':config')
            .forEach(command => {
                command_idx = 0
                commandname = command

                debug_print += `\nvoid debug_${prefix}${commandname}_DispatchDto([[maybe_unused]] const struct ${prefix}${commandname}_DispatchDto *dto)
{
    printf("DEBUG :: ${commandname}:\\n");
`
                hfile += `
struct ${prefix}${commandname}_DispatchDto
{
    ${statename} *state;
    char *skip_label;
`
                const commandStr = config.list ? '' : commandname
                cfile += `\n    if (!strcmp(dto->command, "${commandStr}"))
    {
        struct ${prefix}${commandname}_DispatchDto cd;
        cd.state = state;
        cd.skip_label = dto->skip_label;
`
                const params = Object.keys(def[command]).filter(x => !x.startsWith('validate:') && x !== ':required')
                params.slice(0, 32).forEach(param => {
                    cfile += `        if (!dto->parameters[${command_idx}])
        {
            printf("${commandname}.${param} at index ${command_idx} missing\\n");
            return;
        }
`
                    let format = 's'

                    if (!['int', 'float', 'str'].includes(def[command][param]))
                        throw `invalid parameter type: ${commandSetName}.${command}.${param} ${def[command][param]}`

                    if (def[command][param] === 'str') {
                        hfile += `    const char *${param};\n`
                        cfile += `        cd.${param} = dto->parameters[${command_idx}];\n`
                    } else {
                        const ctype = def[command][param] === 'float' ? 'double' : 'int'
                        format = def[command][param] === 'float' ? 'lf' : 'd'
                        hfile += `    ${ctype} ${param};\n`
                        cfile += `        if (sscanf(dto->parameters[${command_idx}], "%${format}", &cd.${param}) != 1)
        {
            printf("Invalid ${commandname}.${param} at index ${command_idx}\\n");
            return;
        }
`
                    }
                    const validator = def[command]['validate:' + param]
                    if (validator) {
                        cfile += generate_validator(validator, param, def[command][param], command)
                    }
                    debug_print += `    printf("${param} = %${format}\\n", dto->${param});\n`
                    command_idx++;
                })
                debug_print += '}'

                let required = defaultRequired
                if (def[command][':required'] !== undefined) {
                    required = def[command][':required']
                }
                if (required) {
                    cfile += `        SET_command_file_RequiredFlags_FLAG(${requiredCount}, state->required_flags.flags);\n`
                    requiredCount++
                    if (requiredCount > 800)
                        console.log('WARNING: max number of required fields reached!')
                }
                cfile += `        dispatch__handle_${prefix}${commandname}(&cd);
        return;
    }
`
                hfile += `};

void dispatch__handle_${prefix}${commandname}(struct ${prefix}${commandname}_DispatchDto *);

${config.debug ? '' : '//'}void debug_${prefix}${commandname}_DispatchDto(const struct ${prefix}${commandname}_DispatchDto *);
`
                if (config.fieldsetter) {
                    fieldsetters += `void dispatch__handle_${prefix}${commandname}(struct ${prefix}${commandname}_DispatchDto *dto)
{
    dto->state->${commandname} = dto->field;
}
`
                }
                if (!config.debug)
                    debug_print = ''
            })
        if (!defaultHandler) {
            cfile += `    printf("Command '%s' not recognized\\n", dto->command);\n`
        } else {
            hfile += `
struct ${commandSetName}_default_DispatchDto
{
    command_file_DispatchDto *parent;
    ${statename} *state;
    char *skip_label;
    const char *command;
};

void dispatch__handle_${commandSetName}_default(struct ${commandSetName}_default_DispatchDto *);`
            cfile += `
    struct ${commandSetName}_default_DispatchDto default_dto;
    default_dto.parent = dto;
    default_dto.state = state;
    default_dto.skip_label = dto->skip_label;
    default_dto.command = dto->command;
    dispatch__handle_${commandSetName}_default(&default_dto);
`
        }
        if (requiredCount > 0) {
            cfile = cfile.replace('{REQUIRED_FLAGS_INIT_PLACEHOLDER}', `    state->required_flags.count = ${requiredCount};`)
        } else {
            cfile = cfile.replace('{REQUIRED_FLAGS_INIT_PLACEHOLDER}\n', '')
        }
    })

function generate_validator(validate, param, type, command) {
    let validator = ''

    const rangeValidator = (param, paramName) => {
        validator += '        if (!('
        const expressions = []
        if (validate.min !== undefined)
            expressions.push(`${param} >= ${validate.min}`)
        if (validate.max !== undefined)
            expressions.push(`${param} <= ${validate.max}`)
        validator += expressions.join(' && ')
        const format = {
            str: 'd',
            int: 'd',
            float: 'lf'
        }[type]
        validator += `)) {
            printf("Validation ${validate.type} ${validate.min}..${validate.max} failed for parameter ${paramName}=%${format}\\n", ${param});
            return;
        }
`
    }

    if (validate.type === 'range' && (type === 'int' || type === 'float')) {
        rangeValidator('cd.' + param, param)
    } else if (validate.type === 'length' && type === 'str') {
        validator += `        int length__${param} = strlen(cd.${param});\n`
        rangeValidator('length__' + param, param)
    } else if (validate.type === 'choice' && (type === 'int' || type === 'str')) {
        const compare = (aVar, bConst) => type === 'int' ? `${aVar} == ${bConst}` : `!strcmp(${aVar}, "${bConst}")`
        validator += '        if (!(\n            '
        validator += validate.options.map(option => compare('cd.' + param, option)).join('\n         || ')
        const format = {
            str: 's',
            int: 'd',
            float: 'lf'
        }[type]
        validator += `)) {
            printf("Validation ${validate.type} ${validate.options} failed for parameter ${param}=%${format}\\n", cd.${param});
            return;
        }
`
    } else {
        throw `invalid validator for ${command}.${param}: ${JSON.stringify(validate)}`
    }
    return validator
}

write_files()