#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "predictableRandom.h"
#include "scripting.h"

void printCT_Value(int x)
{
	if (x == CT_RANDOM)
		printf("RANDOM              ");
	if (x == CT_PLUS)
		printf("PLUS                ");
	if (x == CT_MINUS)
		printf("MINUS               ");
	if (x == CT_MULTIPLY)
		printf("MULTIPLY            ");
	if (x == CT_DIVIDE)
		printf("DIVIDE              ");
	if (x == CT_MODULO)
		printf("MODULO              ");
	if (x == CT_EQUAL)
		printf("EQUAL               ");
	if (x == CT_NOT_EQUAL)
		printf("NOT_EQUAL           ");
	if (x == CT_LESS)
		printf("LESS                ");
	if (x == CT_LESS_OR_EQUAL)
		printf("LESS_OR_EQUAL       ");
	if (x == CT_GREATER)
		printf("GREATER             ");
	if (x == CT_GREATER_OR_EQUAL)
		printf("GREATER_OR_EQUAL    ");
	if (x == CT_AND)
		printf("AND                 ");
	if (x == CT_OR)
		printf("OR                  ");
	if (x == CT_SET_VALUE)
		printf("SET_VALUE           ");
	if (x == CT_SIN)
		printf("SIN                 ");
	if (x == CT_COS)
		printf("COS                 ");
	if (x == CT_TAN)
		printf("TAN                 ");
	if (x == CT_LOG10)
		printf("LOG10               ");
	if (x == CT_POW)
		printf("POW                 ");
	if (x == CT_ABS)
		printf("ABS                 ");
	if (x == CT_SQRT)
		printf("SQRT                ");
	if (x == CT_INT)
		printf("INT                 ");
	if (x == CT_IF_EQUAL)
		printf("IF_EQUAL            ");
	if (x == CT_IF_NOT_EQUAL)
		printf("IF_NOT_EQUAL        ");
	if (x == CT_IF_GREATER)
		printf("IF_GREATER          ");
	if (x == CT_IF_LESS)
		printf("IF_LESS             ");
	if (x == CT_GOTO)
		printf("GOTO                ");
	if (x == CT_RETURN)
		printf("RETURN              ");
	if (x == CT_CALL)
		printf("CALL                ");
	if (x == CT_PRINT)
		printf("PRINT               ");
	if (x == CT_INDEX)
		printf("INDEX               ");
	if (x == CT_SET_LOOP)
		printf("SET_LOOP            ");
	if (x == CT_LOOP)
		printf("LOOP                ");
	if (x == CT_LABEL)
		printf("LABEL               ");
	if (x == CT_LABEL_POS)
		printf("LABEL_POS           ");
	if (x == CT_DYNAMIC_GOTO)
		printf("DYNAMIC_GOTO        ");
	if (x == CT_RENDER_STRING)
		printf("RENDER_STRING       ");
	if (x == CT_PRINT_STRING)
		printf("PRINT_STRING        ");
}

void printP_TYPE_Value(int x)
{
	if (x == P_TYPE_CONSTANT)
		printf("CONSTANT            ");
	if (x == P_TYPE_VARIABLE)
		printf("VARIABLE            ");
	if (x == P_TYPE_PARAM)
		printf("PARAM               ");
	if (x == P_TYPE_GLOBAL)
		printf("GLOBAL              ");
	if (x == P_TYPE_GOTO)
		printf("GOTO                ");
}
/*
Reference:

commands:
Name				Command				Description
Addition			+ A B C				A = B + C
Subtraction			- A B C				A = B - C					
Multiplication		* A B C				A = B * C
Division			/ A B C				A = B / C
Mod                 % A B C
Set					= A B				A = B
Random number		random A B C		Set random number between B (min) and C (max) to A, e.g. random V0 C0 C1
Mathematical ops.	op A B				A = op(B), available operations: sin, cos, tan, log10, pow, abs, sqrt
Truncate to int		int A B				A = (int) B
If equals 			if-equal A B C		if A = B then goto C (+-0.01 tolerance)
If not equals 		if-not-equal A B C	if A != B then goto C (+-0.01 tolerance)
If greater 			if-greater A B C	if A > B then goto C
If less 			if-less A B C		if A < B then goto C
**** if an if clause is prefixed with <> its result is negated in order to make if-else-endif structures more readable
**** e.g.:
<>if-less V0 C10 >else
#V0 less than 10
>else
<>if-greater V0 C15 >endif
#V0 greater than 15
>endif
****
Call script			call A B			Call script B and set return value to A
Goto label			goto A				Goto label A.
										Label is a one-word command that is in the beginning of the line so
										technically it can be also a command (see the example).
										If A begins with > it means that file will only be searched forwards.
										It makes it possible to reuse labels.
Set index variable	index A				Sets reference to A as the index variable that can be used in loops
Begin loop			set-loop A B		Begins a loop that uses A as the loop variable.
										Loop will set A = B and subtract 1 from it until A <= 0.
										A must be type of V (variable).
Goto loop start 	loop A				Go to start of the loop that uses A as the loop variable.
Return from script	return A			Returns back to the caller. Return value is A.
Print value			print A				Prints the value of A.
Print text			#text				Prints the text. # must be the first character on the line.
System call			!command			Makes a system call. Same applies as with #.

variables: V0...VAR_MAX
parameters: P0...PARAM_MAX
constants: C##.## (ex: C0, C0.8, C-123)
indexing:
V@ or P@ where @ is automatically replaced by the value of the index variable.
About scope:
- P values are visible to all scripts
- V values are private for each script so script parameters must be passed as P values, can be mapped like this:
	= V0 P0
	= V1 P1
	call V2 script-that-does-something-with-P0-and-P1.txt
	= P0 V0
	= P1 V1
	// do something with V2

Looping and indexing example where P0...9 are set to 1*1,2*2...10*10:
index V1		// *** variables are initialized to 0 so no need to set V1 = 0
set-loop V0 C10 // for(V0 = 10; V0 > 0; V0--) {
    + P@ V1 C1  //     P[V1] = V1 + 1
    * P@ P@ P@  //     P[V1] = P[V1] * P[V1]
    + V1 V1 C1  //     V1 = V1 + 1
loop V0         // }
#checking the result... this should be 7*7=49:
print P6
return C0
----
Label example, alternative way of looping:
+ V0 V0 C1
#Looping!
// goes to the first occurrence of "+", execution is continued from that line:
if-less V0 C10 +
return C0
----
More elegant (and portable) of course would be:
:label_1
+ V0 V0 C1
#Looping!
if-less V0 C10 :label_1
return C0
*/
// This implementation is left for reference. New features won't be implemented here.
double runScript_obsolete(char *fname, double *p, int memSize)
{
	double *v = (double *)malloc(memSize * sizeof(double)), constants[4], *index;
	int i, *loops = (int *)malloc(memSize * sizeof(int));
	for (i = 0; i < memSize; i++)
		v[i] = 0;
	FILE *f = fopen(fname, "r");
	char cmd[4][256];
	double *cmdf[4];
	char s[256], *a = cmd[0], *b = cmd[1], *c = cmd[2], *d = cmd[3], *jumpTo = NULL;
	double returnValue = 0;
	while (!feof(f))
	{
		fgets(s, 256, f);
		if (jumpTo != NULL)
		{
			char found[256];
			sscanf(s, "%s", found);
			if (!strcmp(found, jumpTo))
				jumpTo = NULL;
			else
				continue;
		}
		sscanf(s, "%s %s %s %s", a, b, c, d);
		if (s[0] == '#')
		{
			printf("%s", &s[1]);
			continue;
		}
		else if (s[0] == '!')
		{
			system(&s[1]);
			continue;
		}
		for (i = 0; i < 4; i++)
		{
			int j = 0;
			double k = 0;
			if (cmd[i][1] == '@')
				j = (int)*index;
			else
				sscanf(cmd[i], "%*c%d", &j);
			sscanf(cmd[i], "%*c%f", &k);
			if (cmd[i][0] == 'P')
				cmdf[i] = &p[j];
			else if (cmd[i][0] == 'V')
				cmdf[i] = &v[j];
			else if (cmd[i][0] == 'C')
			{
				cmdf[i] = &constants[i];
				constants[i] = k;
			}
		}
		if (!strcmp(a, "random"))
		{
			double r = (double)(pr_get_random() % 10000) / 10000.0;
			double min = *cmdf[2];
			double max = *cmdf[3];
			*cmdf[1] = min + r * (max - min);
		}
		else if (!strcmp(a, "+"))
		{
			*cmdf[1] = *cmdf[2] + *cmdf[3];
		}
		else if (!strcmp(a, "-"))
		{
			*cmdf[1] = *cmdf[2] - *cmdf[3];
		}
		else if (!strcmp(a, "*"))
		{
			*cmdf[1] = *cmdf[2] * *cmdf[3];
		}
		else if (!strcmp(a, "/"))
		{
			*cmdf[1] = *cmdf[2] / *cmdf[3];
		}
		else if (!strcmp(a, "="))
		{
			*cmdf[1] = *cmdf[2];
		}
		else if (!strcmp(a, "sin"))
		{
			*cmdf[1] = sin(*cmdf[2]);
		}
		else if (!strcmp(a, "cos"))
		{
			*cmdf[1] = cos(*cmdf[2]);
		}
		else if (!strcmp(a, "tan"))
		{
			*cmdf[1] = tan(*cmdf[2]);
		}
		else if (!strcmp(a, "log10"))
		{
			*cmdf[1] = log10(*cmdf[2]);
		}
		else if (!strcmp(a, "pow"))
		{
			*cmdf[1] = pow(*cmdf[2], *cmdf[3]);
		}
		else if (!strcmp(a, "abs"))
		{
			*cmdf[1] = fabs(*cmdf[2]);
		}
		else if (!strcmp(a, "sqrt"))
		{
			*cmdf[1] = sqrt(*cmdf[2]);
		}
		else if (!strcmp(a, "int"))
		{
			*cmdf[1] = (int)(*cmdf[2]);
		}
		else if (!strcmp(a, "if-equal") || !strcmp(a, "<>if-not-equal"))
		{
			if (*cmdf[1] > *cmdf[2] - 0.01 && *cmdf[1] < *cmdf[2] + 0.01)
				jumpTo = d;
		}
		else if (!strcmp(a, "if-not-equal") || !strcmp(a, "<>if-equal"))
		{
			if (*cmdf[1] < *cmdf[2] - 0.01 || *cmdf[1] > *cmdf[2] + 0.01)
				jumpTo = d;
		}
		else if (!strcmp(a, "if-greater") || !strcmp(a, "<>if-less"))
		{
			if (*cmdf[1] > *cmdf[2])
				jumpTo = d;
		}
		else if (!strcmp(a, "if-less") || !strcmp(a, "<>if-greater"))
		{
			if (*cmdf[1] < *cmdf[2])
				jumpTo = d;
		}
		else if (!strcmp(a, "goto"))
		{
			jumpTo = b;
		}
		else if (!strcmp(a, "return"))
		{
			returnValue = *cmdf[1];
			break;
		}
		else if (!strcmp(a, "call"))
		{
			*cmdf[1] = runScript_obsolete(c, p, memSize);
		}
		else if (!strcmp(a, "print"))
		{
			printf("%f\n", *cmdf[1]);
		}
		else if (!strcmp(a, "index"))
		{
			index = cmdf[1];
		}
		else if (!strcmp(a, "set-loop"))
		{
			int loop = (int)(cmdf[1] - v);
			loops[loop] = ftell(f);
			*cmdf[1] = *cmdf[2];
		}
		else if (!strcmp(a, "loop"))
		{
			int loop = (int)(cmdf[1] - v);
			*cmdf[1] -= 1;
			if (*cmdf[1] > 0.1)
				fseek(f, loops[loop], SEEK_SET);
		}
		if (jumpTo != NULL && jumpTo[0] != '>')
			fseek(f, 0, SEEK_SET);
	}
	fclose(f);
	free(loops);
	free(v);
	return returnValue;
}

void renderString(MemScript *script, ScriptString *s)
{
	int targetI = 0;
	for (int i = 0; s->orig[i] != 0; i++)
	{
		if (s->orig[i] == '\\')
		{
			i++;
			if (s->orig[i] == 'n')
			{
				s->str[targetI++] = '\n';
			}
			else if (s->orig[i] == 't')
			{
				s->str[targetI++] = '\t';
			}
			else if (s->orig[i] == 'P' || s->orig[i] == 'G')
			{
				int isParam = s->orig[i] == 'P';
				int a = 0;
				for (i = i + 1; s->orig[i] >= '0' && s->orig[i] <= '9' && s->orig[i]; i++)
					a = a * 10 + (s->orig[i] - '0');
				char formatting[8] = "%f";
				int j = 0;
				for (; s->orig[i] != ';' && s->orig[i]; i++)
					formatting[j++] = s->orig[i];
				if (j > 0)
					formatting[j] = 0;
				char valueStr[32];
				sprintf(valueStr, formatting, isParam ? script->parameters[a] : script->globals[a]);
				s->str[targetI] = 0;
				strcat(s->str, valueStr);
				targetI += strlen(valueStr);
			}
			else if (s->orig[i] == '\\')
			{
				s->str[targetI++] = '\\';
			}
		}
		else
		{

			s->str[targetI] = s->orig[i];
			targetI++;
		}
	}
	s->str[targetI] = 0;
}

void initMemCmd(MemCmd *cmd)
{
	for (int i = 0; i < 3; i++)
	{
		cmd->params[i].type = P_TYPE_NOT_USED;
		cmd->params[i].index = P_INDEX_NO_INDEX;
		cmd->params[i].constantValue = 0;
	}
}

void freeMemScript(MemScript *script)
{
	free(script->commands);
	free(script->entryPoints);
	free(script->parameterDefinitions);
	if (script->parameters != NULL)
	{
		free(script->parameters);
	}
	if (script->globals != NULL)
	{
		free(script->globals);
	}
	if (script->strings != NULL)
	{
		free(script->strings);
	}
}

void translateMemScript(char *fname, MemScript *script, int numberOfParamsToAllocate)
{
	const int gotoParamIdx = 2;
	LabeledInt labels[256];
	for (int i = 0; i < 256; i++)
	{
		labels[i].label[0] = 0;
		labels[i].value = -1;
	}
	script->commands = (MemCmd *)malloc(sizeof(MemCmd));
	script->commandCount = 0;
	script->entryPoints = (LabeledInt *)malloc(sizeof(LabeledInt));
	script->entryPointCount = 0;
	script->parameterDefinitions = (LabeledInt *)malloc(sizeof(LabeledInt));
	script->parameterDefinitionsCount = 0;
	script->strings = NULL;
	script->stringCount = 0;
	script->variableCount = 32;
	script->globalsCount = 32;
	script->parameters = (double *)malloc(sizeof(double) * numberOfParamsToAllocate);

	NamedVariable *namedVariables = (NamedVariable *)malloc(sizeof(NamedVariable));
	int namedVariablesCount = 0;

	FILE *f = fopen(fname, "r");
	char cmd[4][256];
	char s[256], *a = cmd[0], *b = cmd[1], *c = cmd[2], *d = cmd[3], *jumpTo = NULL;
	int labelIdx = 0;
	int position = 0;
	while (!feof(f))
	{
		int noCommand = 0;
		fgets(s, 256, f);
		sscanf(s, "%s", a);
		if (a[0] == ':' || a[0] == '>')
		{
			strcpy(labels[labelIdx].label, a);
			labelIdx++;
		}
	}
	int totalLabelCount = labelIdx;
	fseek(f, SEEK_SET, 0);
	labelIdx = 0;
	while (!feof(f))
	{
		jumpTo = NULL;
		MemCmd *memCmd = &(script->commands[position]);
		initMemCmd(memCmd);
		int noCommand = 0, parseError = 1;
		fgets(s, 256, f);
		if (s[0] == '$')
		{
			char *p, *p2;
			for (p = s + 1; *p != ' ' && *p; p++)
				;
			*p = 0;
			int stringIdx = 0;
			sscanf(s + 1, "%d", &stringIdx);
			for (p2 = p + 1; *p2 != '\n' && *p2; p2++)
				;
			*p2 = 0;
			if (stringIdx >= 0 && stringIdx < script->stringCount)
			{
				strcpy(script->strings[stringIdx].orig, p + 1);
				printf("String %s read\n", script->strings[stringIdx].orig);
			}
			continue;
		}
		int readStrs = -1;
		readStrs = sscanf(s, "%s = %s %s %s", b, c, a, d);
		if (readStrs < 2)
		{
			readStrs = sscanf(s, "%s := %s %s %s", b, a, c, d);
		}
		if (readStrs < 2)
		{
			readStrs = sscanf(s, "%s %s %s %s", a, b, c, d);
		}
		else if (readStrs == 2)
		{
			a[0] = '=';
			a[1] = 0;
		}
		if (readStrs <= 0)
			continue;
		if (a[0] == '/' && a[1] == '/')
		{
			continue;
		}
		for (int i = 1; i < 4; i++)
		{
			CmdParam *param = &(memCmd->params[i - 1]);
			int j = 0;
			double k = 0;
			if (sscanf(cmd[i], "%lf", &k))
			{
				*param = (CmdParam){P_TYPE_CONSTANT, P_INDEX_NO_INDEX, k};
				continue;
			}

			if (cmd[i][1] == '@')
			{
				j = P_INDEX_FROM_INDEX_VARIABLE;
			}
			else
			{
				sscanf(cmd[i], "%*c%d", &j);
			}
			sscanf(cmd[i], "%*c%lf", &k);
			if (cmd[i][0] == 'P')
			{
				*param = (CmdParam){P_TYPE_PARAM, j, 0.0};
			}
			else if (cmd[i][0] == 'V')
			{
				*param = (CmdParam){P_TYPE_VARIABLE, j, 0.0};
			}
			else if (cmd[i][0] == 'G')
			{
				*param = (CmdParam){P_TYPE_GLOBAL, j, 0.0};
			}
			else if (cmd[i][0] == '.')
			{
				int found = 0;
				for (int idx = 0; idx < namedVariablesCount; idx++)
				{
					if (!strcmp(namedVariables[idx].name, cmd[i]))
					{
						*param = namedVariables[idx].variable;
						found = 1;
						break;
					}
				}
				if (!found)
				{
					*param = (CmdParam){P_TYPE_CONSTANT, P_INDEX_NO_INDEX, k};
				}
			}
		}
		if (readStrs == 1 && (a[0] == ':' || a[0] == '>'))
		{
			labels[labelIdx].value = position;
			printf("%d: %s@%d (%s)\n", labelIdx, labels[labelIdx].label, position, a);
			labelIdx++;
			parseError = 0;
		}
		else if (a[0] == '.')
		{
			// .entry-point setup
			// .entry-point main-loop
			if (!strcmp(a, ".entry-point"))
			{
				script->entryPointCount++;
				script->entryPoints = (LabeledInt *)realloc(script->entryPoints, script->entryPointCount * sizeof(LabeledInt));
				LabeledInt *entryPoint = &(script->entryPoints[script->entryPointCount - 1]);
				strcpy(entryPoint->label, b);
				entryPoint->value = position;
			}
			// .define-parameter tempo 0
			// .define-parameter sample-rate 1
			else if (!strcmp(a, ".define-parameter") || !strcmp(a, ".param"))
			{
				script->parameterDefinitionsCount++;
				script->parameterDefinitions = (LabeledInt *)realloc(script->parameterDefinitions, script->parameterDefinitionsCount * sizeof(LabeledInt));
				LabeledInt *parameterDefinition = &(script->parameterDefinitions[script->parameterDefinitionsCount - 1]);
				strcpy(parameterDefinition->label, b);
				int paramIdx = 0;
				sscanf(c, "P%d", &paramIdx);
				parameterDefinition->value = paramIdx;
			}
			// .memory-requirement globals 100
			// .memory-requirement variables 123
			else if (!strcmp(a, ".memory-requirement") || !strcmp(a, ".mem"))
			{
				int count = 32;
				sscanf(c, "%d", &count);
				if (!strcmp(b, "globals") || !strcmp(b, "g"))
				{
					script->globalsCount = count;
				}
				else if (!strcmp(b, "variables") || !strcmp(b, "v"))
				{
					script->variableCount = count;
				}
			}
			// .var .a G0
			else if (!strcmp(a, ".variable") || !strcmp(a, ".var"))
			{
				namedVariablesCount++;
				namedVariables = (NamedVariable *)realloc(namedVariables, sizeof(NamedVariable) * namedVariablesCount);
				NamedVariable *var = &(namedVariables[namedVariablesCount - 1]);
				strcpy(var->name, b);
				var->variable = memCmd->params[1];
			}
			else if (!strcmp(a, ".string-count"))
			{
				script->stringCount = 1;
				sscanf(b, "%d", &script->stringCount);
				script->strings = (ScriptString *)malloc(sizeof(ScriptString) * script->stringCount);
				printf("Memory %d reserved for %d strings\n", sizeof(ScriptString) * script->stringCount, script->stringCount);
			}
			parseError = 0;
		}
		// the below lines are counted as commands, the above are just "metadata" for building the commands
		if (!strcmp(a, "random"))
		{
			memCmd->type = CT_RANDOM;
		}
		else if (!strcmp(a, "+"))
		{
			memCmd->type = CT_PLUS;
		}
		else if (!strcmp(a, "-"))
		{
			memCmd->type = CT_MINUS;
		}
		else if (!strcmp(a, "*"))
		{
			memCmd->type = CT_MULTIPLY;
		}
		else if (!strcmp(a, "/"))
		{
			memCmd->type = CT_DIVIDE;
		}
		else if (!strcmp(a, "%"))
		{
			memCmd->type = CT_MODULO;
		}
		else if (!strcmp(a, "=="))
		{
			memCmd->type = CT_EQUAL;
		}
		else if (!strcmp(a, "!="))
		{
			memCmd->type = CT_NOT_EQUAL;
		}
		else if (!strcmp(a, "<"))
		{
			memCmd->type = CT_LESS;
		}
		else if (!strcmp(a, "<="))
		{
			memCmd->type = CT_LESS_OR_EQUAL;
		}
		else if (!strcmp(a, ">"))
		{
			memCmd->type = CT_GREATER;
		}
		else if (!strcmp(a, ">="))
		{
			memCmd->type = CT_GREATER_OR_EQUAL;
		}
		else if (!strcmp(a, "&&"))
		{
			memCmd->type = CT_AND;
		}
		else if (!strcmp(a, "||"))
		{
			memCmd->type = CT_OR;
		}
		else if (!strcmp(a, "="))
		{
			memCmd->type = CT_SET_VALUE;
		}
		else if (!strcmp(a, "sin"))
		{
			memCmd->type = CT_SIN;
		}
		else if (!strcmp(a, "cos"))
		{
			memCmd->type = CT_COS;
		}
		else if (!strcmp(a, "tan"))
		{
			memCmd->type = CT_TAN;
		}
		else if (!strcmp(a, "log10"))
		{
			memCmd->type = CT_LOG10;
		}
		else if (!strcmp(a, "pow"))
		{
			memCmd->type = CT_POW;
		}
		else if (!strcmp(a, "abs"))
		{
			memCmd->type = CT_ABS;
		}
		else if (!strcmp(a, "sqrt"))
		{
			memCmd->type = CT_SQRT;
		}
		else if (!strcmp(a, "int"))
		{
			memCmd->type = CT_INT;
		}
		else if (!strcmp(a, "if-equal") || !strcmp(a, "<>if-not-equal"))
		{
			memCmd->type = CT_IF_EQUAL;
			jumpTo = d;
		}
		else if (!strcmp(a, "if-not-equal") || !strcmp(a, "<>if-equal"))
		{
			memCmd->type = CT_IF_NOT_EQUAL;
			jumpTo = d;
		}
		else if (!strcmp(a, "if-greater") || !strcmp(a, "<>if-less"))
		{
			memCmd->type = CT_IF_GREATER;
			jumpTo = d;
		}
		else if (!strcmp(a, "if-less") || !strcmp(a, "<>if-greater"))
		{
			memCmd->type = CT_IF_LESS;
			jumpTo = d;
		}
		else if (!strcmp(a, "goto"))
		{
			memCmd->type = CT_GOTO;
			jumpTo = b;
		}
		else if (!strcmp(a, "return"))
		{
			memCmd->type = CT_RETURN;
		}
		else if (!strcmp(a, "call"))
		{
			memCmd->type = CT_CALL;
			jumpTo = c;
			printf("Calling %s\n", c);
		}
		else if (!strcmp(a, "print"))
		{
			memCmd->type = CT_PRINT;
		}
		else if (!strcmp(a, "index"))
		{
			memCmd->type = CT_INDEX;
		}
		else if (!strcmp(a, "set-loop"))
		{
			memCmd->type = CT_SET_LOOP;
		}
		else if (!strcmp(a, "loop"))
		{
			memCmd->type = CT_LOOP;
		}
		else if (!strcmp(a, "get-pos"))
		{
			memCmd->type = CT_LABEL_POS;
			jumpTo = c;
		}
		else if (!strcmp(a, "goto-var"))
		{
			memCmd->type = CT_DYNAMIC_GOTO;
		}
		else if (!strcmp(a, "render-string"))
		{
			memCmd->type = CT_RENDER_STRING;
		}
		else if (!strcmp(a, "print-string"))
		{
			memCmd->type = CT_PRINT_STRING;
		}
		else if (!strcmp(a, "fire-event"))
		{
			memCmd->type = CT_FIRE_EVENT;
		}
		else if (!strcmp(a, "init-event"))
		{
			memCmd->type = CT_INIT_EVENT;
		}
		else if (!strcmp(a, "event-data-set"))
		{
			memCmd->type = CT_SET_EVENT_DATA;
		}
		else if (!strcmp(a, "event-data-get"))
		{
			memCmd->type = CT_GET_EVENT_DATA;
		}
		else
		{
			noCommand = 1;
			if (parseError)
				printf("Code parse error: %s\n", s);
		}
		if (jumpTo != NULL)
		{
			for (int i = 0; i < 256; i++)
			{
				if (!strcmp(labels[i].label, jumpTo))
				{
					memCmd->params[gotoParamIdx].type = P_TYPE_GOTO;
					memCmd->params[gotoParamIdx].index = i;
					break;
				}
			}
		}
		if (!noCommand)
		{
			position++;
			script->commandCount = position;
			script->commands = (MemCmd *)realloc(script->commands, sizeof(MemCmd) * (script->commandCount + 1));
		}
	}
	for (int i = 0; i < script->commandCount; i++)
	{
		if (script->commands[i].params[gotoParamIdx].type == P_TYPE_GOTO)
		{
			int labelIndex = script->commands[i].params[gotoParamIdx].index;
			int labelPosition = labels[labelIndex].value;
			if (labels[labelIndex].label[0] != '>')
			{
				script->commands[i].params[gotoParamIdx].index = labelPosition;
			}
			else
			{
				// Look-forward labels, we want to get the first label that is after the jump point
				char *labelName = labels[labelIndex].label;
/*				printf("Matching label %s\n", labelName);
				printf("Expected: %s at %d\n", labels[labelIdx - 1].label, labels[labelIdx - 1].value);*/
				for (int j = 0; j < totalLabelCount; j++)
				{
					labelPosition = labels[j].value;
					if (!strcmp(labelName, labels[j].label) && labelPosition > i)
					{
                        //printf("Found at %d\n", labelPosition);
						script->commands[i].params[gotoParamIdx].index = labelPosition;
						break;
					}
				//	printf("Was not: %s\t", labels[j].label);
				}
			}
		}
	}
	printf("%d vs %d\n", totalLabelCount, labelIdx);
	
	if (script->globalsCount > 0)
	{
		script->globals = (double *)malloc(sizeof(double) * script->globalsCount);
		for (int i = 0; i < script->globalsCount; i++)
		{
			script->globals[i] = 0;
		}
	}
	else
	{
		script->globals = NULL;
	}
	for (int i = 0; i < script->commandCount ; i++)
	{
		printf("%3d:\t", i);
		printCT_Value(script->commands[i].type);
		for (int j = 0; j < 3; j++)
		{
			if (script->commands[i].params[j].type == P_TYPE_NOT_USED)
				continue;
			printf("P%d ", j);
			printP_TYPE_Value(script->commands[i].params[j].type);
			printf("%d %f\t", script->commands[i].params[j].index, script->commands[i].params[j].constantValue);
		}
		printf("\n");
	}
	free(namedVariables);
	fclose(f);
}

double runMemScriptFromPosition(MemScript *script, int command_position, void (*event_handler)(MemScriptEvent*))
{
    MemScriptEvent *event = NULL;
    int eventDataIdx;
	double *variables = NULL, constants[4], *index;
	if (script->variableCount > 0)
	{
		variables = (double *)malloc(script->variableCount * sizeof(double));
		for (int i = 0; i < script->variableCount; i++)
		{
			variables[i] = 0;
		}
	}
	int i, *loops = (int *)malloc(script->variableCount * sizeof(int));
	double *cmdf[3];
	double returnValue = 0;
	int position = command_position >= 0 ? command_position : 0;
	while (position < script->commandCount)
	{
		MemCmd memCmd = script->commands[position];
		for (int i = 0; i < 3; i++)
		{
			int j = 0;
			CmdParam param = memCmd.params[i];
			if (param.index == P_INDEX_FROM_INDEX_VARIABLE)
			{
				j = (int)*index;
			}
			else
			{
				j = param.index;
			}

			if (param.type == P_TYPE_PARAM)
			{
				cmdf[i] = &script->parameters[j];
			}
			else if (param.type == P_TYPE_VARIABLE)
			{
				cmdf[i] = &variables[j];
			}
			else if (param.type == P_TYPE_GLOBAL)
			{
				cmdf[i] = &script->globals[j];
			}
			else if (param.type == P_TYPE_CONSTANT)
			{
				cmdf[i] = &constants[i];
				constants[i] = param.constantValue;
			}
		}
		// Note that 'a' corresponds to string 'b' in the translator and so on
		double *a = cmdf[0], *b = cmdf[1], *c = cmdf[2];
		int gotoPosition = memCmd.params[2].index - 1;
		int cmdType = memCmd.type;
		if (cmdType == CT_RANDOM)
		{
			const double r = (double)(pr_get_random() % 30000) / 30000.0;
			const double min = *b;
			const double max = *c;
			*a = min + r * (max - min);
		}
		else if (cmdType == CT_PLUS)
		{
			*a = *b + *c;
		}
		else if (cmdType == CT_MINUS)
		{
			*a = *b - *c;
		}
		else if (cmdType == CT_MULTIPLY)
		{
			*a = *b * *c;
		}
		else if (cmdType == CT_DIVIDE)
		{
			*a = *b / *c;
		}
		else if (cmdType == CT_MODULO)
		{
			const int n1 = *b;
			const int n2 = *c;
			*a = n1 % n2;
		}
		else if (cmdType == CT_EQUAL)
		{
			*a = *b > *c - 0.01 && *b < *c + 0.01 ? 1 : 0;
		}
		else if (cmdType == CT_NOT_EQUAL)
		{
			*a = *b < *c - 0.01 || *b > *c + 0.01 ? 1 : 0;
		}
		else if (cmdType == CT_LESS)
		{
			*a = *b < *c ? 1 : 0;
		}
		else if (cmdType == CT_LESS_OR_EQUAL)
		{
			*a = *b < *c + 0.01 ? 1 : 0;
		}
		else if (cmdType == CT_GREATER)
		{
			*a = *b > *c ? 1 : 0;
		}
		else if (cmdType == CT_GREATER_OR_EQUAL)
		{
			*a = *b > *c - 0.01 ? 1 : 0;
		}
		else if (cmdType == CT_AND)
		{
			*a = *b > 0.5 && *c > 0.5 ? 1 : 0;
		}
		else if (cmdType == CT_OR)
		{
			*a = *b > 0.5 || *c > 0.5 ? 1 : 0;
		}
		else if (cmdType == CT_SET_VALUE)
		{
			*a = *b;
		}
		else if (cmdType == CT_SIN)
		{
			*a = sin(*b);
		}
		else if (cmdType == CT_COS)
		{
			*a = cos(*b);
		}
		else if (cmdType == CT_TAN)
		{
			*a = tan(*b);
		}
		else if (cmdType == CT_LOG10)
		{
			*a = log10(*b);
		}
		else if (cmdType == CT_POW)
		{
			*a = pow(*b, *c);
		}
		else if (cmdType == CT_ABS)
		{
			*a = fabs(*b);
		}
		else if (cmdType == CT_SQRT)
		{
			*a = sqrt(*b);
		}
		else if (cmdType == CT_INT)
		{
			*a = (int)(*b);
		}
		else if (cmdType == CT_IF_EQUAL)
		{
			if (*a > *b - 0.01 && *a < *b + 0.01)
				position = gotoPosition;
		}
		else if (cmdType == CT_IF_NOT_EQUAL)
		{
			if (*a < *b - 0.01 || *a > *b + 0.01)
				position = gotoPosition;
		}
		else if (cmdType == CT_IF_GREATER)
		{
			if (*a > *b)
				position = gotoPosition;
		}
		else if (cmdType == CT_IF_LESS)
		{
			if (*a < *b)
				position = gotoPosition;
		}
		else if (cmdType == CT_GOTO)
		{
			position = gotoPosition;
		}
		else if (cmdType == CT_RETURN)
		{
			returnValue = *a;
			break;
		}
		else if (cmdType == CT_CALL)
		{
			// Goto position is normally offset by -1 because position is incremented at the end of the loop.
			// This doesn't happen when calling a new script
			*a = runMemScriptFromPosition(script, gotoPosition + 1, event_handler);
		}
		else if (cmdType == CT_PRINT)
		{
			printf("%f\n", *a);
		}
		else if (cmdType == CT_INDEX)
		{
			index = a;
		}
		else if (cmdType == CT_SET_LOOP)
		{
			int loop = (int)(a - variables);
			loops[loop] = position;
			*a = *b;
		}
		else if (cmdType == CT_LOOP)
		{
			int loop = (int)(a - variables);
			*a -= 1;
			if (*a > 0.1)
				position = loops[loop];
		}
		else if (cmdType == CT_LABEL_POS)
		{
			*a = gotoPosition;
		}
		else if (cmdType == CT_DYNAMIC_GOTO)
		{
			if (*a >= 0)
			{
				position = *a;
			}
		}
		else if (cmdType == CT_RENDER_STRING)
		{
			int idx = *a;
			if (idx < script->stringCount)
				renderString(script, &script->strings[idx]);
		}
		else if (cmdType == CT_PRINT_STRING)
		{
			int idx = *a;
			if (idx < script->stringCount)
				printf(script->strings[idx].str);
		}
		else if (cmdType == CT_FIRE_EVENT && event_handler != NULL && event != NULL)
		{
			(*event_handler)(event);
			eventDataIdx = 0;
		}
		else if (cmdType == CT_INIT_EVENT)
		{
             if (event != NULL)
             {
		        free(event->data);
		        if (event->returnData != NULL)
		        {
		            free(event->returnData);
                }
		        free(event);
             }
             int event_id = *a;
             int size = *b;
             event = (MemScriptEvent*)malloc(sizeof(MemScriptEvent));
             event->data = (double*)malloc(sizeof(double) * size);
             event->size = size;
             event->returnData = NULL;
             event->returnDataSize = 0;
             eventDataIdx = 0;
        }
		else if (cmdType == CT_SET_EVENT_DATA && event != NULL && eventDataIdx < event->size - 1)
		{
             event->data[eventDataIdx] = *a;
             eventDataIdx++;
        }
		else if (cmdType == CT_GET_EVENT_DATA && event != NULL && eventDataIdx < event->returnDataSize - 1)
		{
             *a = event->returnData[eventDataIdx];
             eventDataIdx++;
        }
		position++;
	}
/*	for (int i = 0; i < script->globalsCount; i++) printf("G%d = %.0f\t", i, script->globals[i]);
	printf("\n");*/
	
     if (event != NULL)
     {
        free(event->data);
        if (event->returnData != NULL)
        {
            free(event->returnData);
        }
        free(event);
     }
	free(loops);
	if (variables != NULL)
	{
		free(variables);
	}
	return returnValue;
}

double runMemScript(MemScript *script, char *entryPointName, void (*event_handler)(MemScriptEvent*))
{
	//printf("Running from %s\n", entryPointName);
	int entryPoint = -1;
	for (int i = 0; i < script->entryPointCount && entryPointName != NULL; i++)
	{
		if (!strcmp(script->entryPoints[i].label, entryPointName))
		{
			entryPoint = script->entryPoints[i].value;
			break;
		}
	}
	//printf("Running from pos %d\n", entryPoint);
	if (entryPoint == -1)
	{
		// exit if entrypoint is not found
		return 0;
	}
	return runMemScriptFromPosition(script, entryPoint, event_handler);
}

double *getParameterByName(MemScript *script, char *name)
{
	//printf("get parameter: %s\n", name);
	for (int j = 0; j < script->parameterDefinitionsCount; j++)
	{
		//printf("Check param <%s> == <%s>\n", script->parameterDefinitions[j].label, name);
		if (!strcmp(name, script->parameterDefinitions[j].label))
		{
			//printf("Return param %s @ %d with value %f\n", script->parameterDefinitions[j].label, script->parameterDefinitions[j].value, script->parameters[script->parameterDefinitions[j].value]);
			return &(script->parameters[script->parameterDefinitions[j].value]);
		}
	}
	return NULL;
}

void setMemScriptParameter(MemScript *script, char *parameter, double value)
{
	double *param = getParameterByName(script, parameter);
	if (param != NULL)
	{
		*param = value;
	}
}

double getMemScriptParameter(MemScript *script, char *parameter)
{
	double *param = getParameterByName(script, parameter);
	if (param != NULL)
	{
		return *param;
	}
	return 0;
}

void memoizeMemScript(MemScript *script, MemScriptIdxCache *cache, char **entryPointNames, int entryPointNameCount, char **paramNames, int parameterNameCount)
{
	cache->entryPointMap = (int *)malloc(sizeof(int) * entryPointNameCount);
	cache->paramMap = (int *)malloc(sizeof(int) * parameterNameCount);
	for (int i = 0; i < entryPointNameCount; i++)
	{
		cache->entryPointMap[i] = 0;
		for (int j = 0; j < script->entryPointCount; j++)
		{
			if (!strcmp(entryPointNames[i], script->entryPoints[j].label))
			{
				cache->entryPointMap[i] = script->entryPoints[j].value;
				break;
			}
		}
	}

	for (int i = 0; i < parameterNameCount; i++)
	{
		cache->paramMap[i] = -1;
		for (int j = 0; j < script->parameterDefinitionsCount; j++)
		{
			if (!strcmp(paramNames[i], script->parameterDefinitions[j].label))
			{
				cache->entryPointMap[i] = script->parameterDefinitions[j].value;
				break;
			}
		}
	}
}

void freeMemScriptIdxCache(MemScriptIdxCache *cache)
{
	free(cache->entryPointMap);
	free(cache->paramMap);
}

/*int main(int argc, char **argv)
{
	srand((int)time(NULL));
	if (argc < 2)
		return 0;
	MemScript script;
	printf("translating script %s\n", argv[1]);
	translateMemScript(argv[1], &script);

	printf("translation done\n");
	script.parameters = (double*)malloc(sizeof(double) * 1000);
	double *p = script.parameters;
	int i;
	char *entry = NULL;
	int runtimes = 1;
	for (i = 0; i < argc - 2; i++)
	{
		if (argv[i + 2][0] == '!')
		{
			entry = argv[i + 2];
			continue;
		}
		char s[100];
		double d;
		sscanf(argv[i + 2], "%lf:%s", &d, s);
		if (!strcmp(s, "runtimes"))
		{
			runtimes = (int)d;
			continue;
		}
		for (int j = 0; j < script.parameterDefinitionsCount; j++)
		{
			printf("Check param <%s> == <%s>\n", script.parameterDefinitions[j].label, s);
			if (!strcmp(s, script.parameterDefinitions[j].label))
			{
				printf("Set param %s @ %d to %f\n", script.parameterDefinitions[j].label, j, d);
				p[script.parameterDefinitions[j].value] = d;
				break;
			}
		}
	}
	int entryPoint = 0;
	for (int i = 0; i < script.entryPointCount && entry != NULL; i++)
	{
		if (!strcmp(script.entryPoints[i].label, entry))
		{
			entryPoint = script.entryPoints[i].value;
			break;
		}
	}
	while (runtimes--)
	{
		printf("running script with %d commands\n", script.commandCount);
		double res = runMemScript(&script, entryPoint);
		printf("Script output: %f\n", res);
		for (int i = 0; i < script.globalsCount; i++)
		{
			printf("Global %d: %f\n", i, script.globals[i]);
		}
	}
	freeMemScript(&script);

	return 0;
}*/
