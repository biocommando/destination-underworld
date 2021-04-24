#ifndef H_MEM_SCRIPT
#define H_MEM_SCRIPT

// Create a new unique ID using this and substituting x with the already defined IDs
// a=[],`x`.replace(/0x[0-9a-f]+/g, sub => a.push(sub));do{b =  '0x'+Math.round(Math.random()*1000000).toString(16)}while(a.includes(b));console.log(b)
#define CT_RANDOM 0
#define CT_PLUS 1
#define CT_MINUS 2
#define CT_MULTIPLY 3
#define CT_DIVIDE 4
#define CT_MODULO 5
#define CT_GREATER 32
#define CT_GREATER_OR_EQUAL 33
#define CT_LESS 34
#define CT_LESS_OR_EQUAL 35
#define CT_NOT_EQUAL 36
#define CT_EQUAL 37
#define CT_AND 38
#define CT_OR 39
#define CT_SET_VALUE 6
#define CT_SIN 7
#define CT_COS 8
#define CT_TAN 9
#define CT_LOG10 10
#define CT_POW 11
#define CT_ABS 12
#define CT_SQRT 13
#define CT_INT 14
#define CT_IF_EQUAL 15
#define CT_IF_NOT_EQUAL 16
#define CT_IF_GREATER 17
#define CT_IF_LESS 18
#define CT_GOTO 19
#define CT_RETURN 20
#define CT_CALL 21
#define CT_PRINT 22
#define CT_INDEX 23
#define CT_SET_LOOP 24
#define CT_LOOP 25
#define CT_LABEL 26
#define CT_LABEL_POS 27
#define CT_DYNAMIC_GOTO 28
#define CT_RENDER_STRING 29
#define CT_PRINT_STRING 30
#define CT_FIRE_EVENT 31
#define CT_INIT_EVENT 40
#define CT_SET_EVENT_DATA 41
#define CT_GET_EVENT_DATA 42

// print: s='void printValue(int x){\n',`x`.replace(/CT_([^ ]+)/g, (a,b)=>s+='    if(x == CT_'+b+') printf("' + b + ' '.repeat(20 - b.length) +'");\n'),s+'}'

#define P_TYPE_NOT_USED 100
#define P_TYPE_CONSTANT 200
#define P_TYPE_VARIABLE 300
#define P_TYPE_PARAM 400
#define P_TYPE_GLOBAL 500
#define P_TYPE_GOTO 600

#define P_INDEX_FROM_INDEX_VARIABLE -1
#define P_INDEX_NO_INDEX 0

typedef struct
{
	int type;
	int index;
	double constantValue;
} CmdParam;

typedef struct
{
	int type;
	CmdParam params[3];
} MemCmd;

typedef struct
{
	char label[64];
	int value;
} LabeledInt;

typedef struct {
	char str[256];
	char orig[256];
} ScriptString;

typedef struct
{
	MemCmd *commands;
	int commandCount;
	LabeledInt *entryPoints;
	int entryPointCount;
	int variableCount;
	int globalsCount;
	LabeledInt *parameterDefinitions;
	int parameterDefinitionsCount;
	double *parameters;
	double *globals;
	ScriptString *strings;
	int stringCount;
} MemScript;

typedef struct
{
	CmdParam variable;
	char name[64];
} NamedVariable;

typedef struct
{
	int *entryPointMap;
	int *paramMap;
} MemScriptIdxCache;

typedef struct 
{
    int id;
    int size;
    double *data;
    
    int returnDataSize;
    double *returnData;
} MemScriptEvent;

void translateMemScript(char *fname, MemScript *script, int numberOfParamsToAllocate);
void freeMemScript(MemScript *script);
double runMemScript(MemScript *script, char *entryPointName, void (*event_handler)(MemScriptEvent*));
void setMemScriptParameter(MemScript *script, char *parameter, double value);
double getMemScriptParameter(MemScript *script, char *parameter);
#endif
