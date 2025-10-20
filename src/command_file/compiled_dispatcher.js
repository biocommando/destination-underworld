
/*

struct COMMAND_DispatchDto
{
    const char *s_param;
    int i_param;

    STATE *state;
    char *skip_label;
};

void dispatch__handle_COMMAND(struct COMMAND_DispatchDto *);
*/

/*
#include "dispatch_NAME.h"

void dispatch__NAME(command_file_DispatchDto *dto)
{
    STATE *state = dto->state;

    if (!strcmp(dto->command, "COMMAND"))
    {
        struct COMMAND_DispatchDto cd;
        if (!dto->parameters[0])
        {
            printf("COMMAND.s_param at index 0 missing\n");
            return;
        }
        cd.s_param = dto->parameters[0];
        if (!dto->parameters[1])
        {
            printf("COMMAND.i_param at index 1 missing\n");
            return;
        }
        if (sscanf(dto->parameters[1], "%d", &cd.i_param) != 1)
        {
            printf("Invalid COMMAND.i_param at index 1\n");
        }
        cd.i_param = dto->parameters[1];
        cd.state = state;
        cd.skip_label = dto->skip_label;
        dispatch__handle_COMMAND(&cd);
        return;
      }

}

*/

/*
commands NAME

state STATE

command COMMAND
    str_param s_param
    int_param i_param
end_command

*/

const inputFile = process.argv.pop()

const fs = require('fs')

let cfile, hfile, statename, commandsname, commandname
let debug_print
let command_idx = 0

function write_files() {
    if (cfile) {
        cfile += '}\n' + debug_print + '\n'
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
}
const json = JSON.parse(fs.readFileSync(inputFile).toString())

Object.keys(json)
    .forEach(commandSetName => {
        const def = json[commandSetName]
        init(commandSetName)
        statename = def.state
        hfile = hfile.replace('$$STATE_H$$', 'state_' + statename + '.h')
        cfile += `    ${statename} *state = (${statename} *)dto->state;\n`

        Object.keys(def).filter(x => x !== 'state')
            .forEach(command => {
                command_idx = 0
                commandname = command

                debug_print += `void debug_${commandname}_DispatchDto(const struct ${commandname}_DispatchDto *dto)
{
    printf("DEBUG :: ${commandname}:\\n");
`
                hfile += `
struct ${commandname}_DispatchDto
{
    ${statename} *state;
    char *skip_label;
`
                cfile += `\n    if (!strcmp(dto->command, "${commandname}"))
    {
        struct ${commandname}_DispatchDto cd;
        cd.state = state;
        cd.skip_label = dto->skip_label;
`
                const params = Object.keys(def[command]).filter(x => !x.startsWith('validate:'))
                params.slice(0, 32).forEach(param => {
                    cfile += `        if (!dto->parameters[${command_idx}])
        {
            printf("${commandname}.${param} at index ${command_idx} missing\\n");
            return;
        }
`
                    if (def[command][param] === 'str') {
                        hfile += `    const char *${param};\n`
                        cfile += `        cd.${param} = dto->parameters[${command_idx}];\n`
                    } else {
                        hfile += `    int ${param};\n`
                        cfile += `        if (sscanf(dto->parameters[${command_idx}], "%d", &cd.${param}) != 1)
        {
            printf("Invalid ${commandname}.${param} at index ${command_idx}\\n");
            return;
        }
`
                    }
                    const validator = def[command]['validate:' + param]
                    if (validator) {
                        cfile += generate_validator(validator, param, def[command][param])
                    }
                    debug_print += `    printf("${param} = %${def[command][param] === 'str' ? 's' : 'd'}\\n", dto->${param});\n`
                    command_idx++;
                })
                debug_print += '}'
                cfile += `        dispatch__handle_${commandname}(&cd);
        return;
    }
`
                hfile += `};

void dispatch__handle_${commandname}(struct ${commandname}_DispatchDto *);

void debug_${commandname}_DispatchDto(const struct ${commandname}_DispatchDto *);
`
            })

    })

function generate_validator(validate, param, type) {
    let validator =  ''

    const rangeValidator = (param, paramName) => {
        validator += '        if (!('
        validator += validate.min !== undefined ? `${param} >= ${validate.min}` : 1
        validator += ' && '
        validator += validate.max !== undefined ? `${param} <= ${validate.max}` : 1
        validator += `)) {
            printf("Validation ${validate.type} ${validate.min}..${validate.max} failed for parameter ${paramName}=%d\\n", ${param});
            return;
        }
`
    }

    if (validate.type === 'range' && type === 'int') {
        rangeValidator('cd.' + param, param)
    } else if (validate.type === 'length' && type === 'str') {
        validator += `        int length__${param} = strlen(cd.${param});\n`
        rangeValidator('length__' + param, param)
    } else if (validate.type === 'choice') {
        const compare = (aVar, bConst) => type === 'int' ? `${aVar} == ${bConst}` : `!strcmp(${aVar}, "${bConst}")`
        validator += '        if (!(\n            '
        validator += validate.options.map(option => compare('cd.' + param, option)).join('\n         || ')
        validator += `)) {
            printf("Validation ${validate.type} ${validate.options} failed for parameter ${param}=%${type === 'str' ? 's' : 'd'}\\n", cd.${param});
            return;
        }
`
    } else {
        throw `invalid validator for ${param}: ${JSON.stringify(validate)}`
    }
    return validator
}

write_files()