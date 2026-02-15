#include "nano_basic_local.h"

static NB_LINE_NUM _num = 0; /* プログラムカウンタ */
static NB_SIZE _pos = 0; /* 実行中の位置 */

#ifdef __ENABLE_COMMAND_HELP__
CommandFuncList _command[] = {
    {"HELP", command_help},
#else /* __ENABLE_COMMAND_HELP__ */
static CommandFuncList _command[] = {
#endif /* __ENABLE_COMMAND_HELP__ */
    {"REM", command_rem},
    {"LET", command_let},
    {"PRINT", command_print},
    {"INPUT", command_input},
    {"IF", command_if},
    {"THEN", command_then},
    {"GOTO", command_goto},
    {"GOSUB", command_gosub},
    {"RETURN", command_return},
    {"END", command_end},
    {"RUN", command_run},
    {"NEW", command_new},
    {"LOAD", command_load},
    {"SAVE", command_save},
    {"RENUM", command_renum},
    {"LIST", command_list},
    {"DEL", command_del},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
    {"", NULL},
};

#define COMMAND_SIZE (sizeof(_command) / sizeof(CommandFuncList))

void nano_basic_init(NB_I8 *memory, NB_SIZE code_size, NB_SIZE value_size, NB_SIZE stack_size)
{
    command_init();
    memory_init(memory, code_size, value_size, stack_size);
    _num = 0;
    _pos = 0;
}

NB_BOOL nano_basic_add_command(const NB_I8 *name, CommandFunc func)
{
    NB_SIZE i = 0;
    for(i = 0 ; i < COMMAND_SIZE ; i++) {
        if(_command[i].func == NULL) {
            _command[i].name = name;
            _command[i].func = func;
            LOG("nano_basic_add_command: %s(%p)\r\n", _command[i].name, _command[i].func);
            return NB_TRUE;
        }
    }
    return NB_FALSE;
}

NB_BOOL nano_basic_set(const NB_I8 *code, NB_I8 *work_buf, NB_SIZE work_buf_size)
{
    NB_LINE_NUM num = 0;
    NB_SIZE pos = 0;
    NB_SIZE size = 0;

    while(size + 1 < work_buf_size && code[size] != '\0') {
        work_buf[size] = code[size];
        size++;
    }
    work_buf[size] = '\0';
    size++;

    return IS_SUCCESS(nano_basic_set_code(work_buf, size, &num, &pos));
}

NB_RESULT nano_basic_set_code(NB_I8 *buf, NB_SIZE size, NB_LINE_NUM *num, NB_SIZE *pos)
{
    NB_SIZE i = 0;
    NB_SIZE j = 0;
    NB_BOOL string_flag = NB_FALSE;
    NB_BOOL sep_flag = NB_FALSE;
    
    *num = 0;
    *pos = 0;

    /* バッファの先頭部分が数値か判定 */
    /* 数値の場合は、行として扱う。なければ0行目として扱う */
    util_atoi(buf, pos, size, num);
    LOG("nano_basic_set_code: num:%d, pos:%d\r\n", *num, *pos);

    util_strskipuntil(buf, pos, NB_CODE_STR_SEPALATE_PARAMETER, size);

    /* コード成形 */
    for(i = *pos; i < size; i++) {
        if(buf[i] == NB_CODE_STRING_SIGN) {
            string_flag = !string_flag;
        }

        /* 文字列以外ならば、小文字は大文字に変換する */
        if(!string_flag && 'a' <=buf[i] && buf[i] <= 'z') {
            buf[j] = buf[i] - 'a' + 'A';
            j++;
            sep_flag = NB_FALSE;

        /* パラメータ間の空白は1文字のみ受け付ける */
        } else if(buf[i] == NB_CODE_STR_SEPALATE_PARAMETER && !sep_flag) {
            buf[j] = buf[i];
            j++;
            sep_flag = NB_TRUE;

        } else if(buf[i] == NB_CODE_STR_SEPALATE_PARAMETER && sep_flag) {
            /* 何もしない */

        } else if(buf[i] == NB_CODE_STR_TERMINATE) {
            if(sep_flag) { /* 最後が空白の場合は戻す */
                j--;
            }
            buf[j] = buf[i];
            j++;
            sep_flag = NB_FALSE;

        } else {
            LOG("[%c,%d]", buf[i], sep_flag);
            
            buf[j] = buf[i];
            j++;
            sep_flag = NB_FALSE;
        }
    }
    
    IF_ERROR_EXIT(memory_code_set(*num, buf, j));
    return NB_RESULT_SUCCESS;
}

NB_RESULT nano_basic_proc_command(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_RESULT result = NB_RESULT_SUCCESS;
    NB_SIZE len = 0;
    NB_I8 i = 0;

    LOG("nano_basic_proc_command: num:%d, %p\r\n", *num, num);
    
    for(i = 0; i < COMMAND_SIZE; i++) {
        len = util_strlen(_command[i].name);
        if(_command[i].func != NULL && util_strcmp(&code[*pos], _command[i].name, len)) {
            *pos = *pos + len;
            util_strskipuntil(code, pos, NB_CODE_STR_SEPALATE_PARAMETER, size);
            LOG("nano_basic_proc_command:IN  name:%s, size:%d, *pos:%d, *state:%d\r\n", _command[i].name, size, *pos, *state);
            result = _command[i].func(num, code, size, pos, state);
            LOG("nano_basic_proc_command:OUT name:%s, size:%d, *pos:%d, *state:%d, result:%d\r\n", _command[i].name, size, *pos, *state, result);
            return result;
        }
    }
    return NB_RESULT_ERROR_RUNTIME;
}

static void nano_basic_print_error(NB_RESULT result)
{
    NB_I8 *msg = "";

    switch (result) {
        case NB_RESULT_ERROR_MEMORY: 
            msg = "error MEMORY";
            break;
        case NB_RESULT_ERROR_SYNTAX: 
            msg = "error SYNTAX";
            break;
        case NB_RESULT_ERROR_RUNTIME:
            msg = "error RUNTIME";
            break;
        default:
            msg = "error";
    }
    util_print_value(_num);
    platform_print_ch(' ');
    while(*msg != '\0') {
        platform_print_ch(*msg);
        msg++;
    }
    platform_print_ch('\r');
    platform_print_ch('\n');

}

NB_STATE nano_basic_proc(NB_STATE state, NB_I8 *buf, NB_SIZE size)
{
    NB_I8 *code = NULL;
    NB_LINE_NUM num = 0;
    NB_SIZE pos = 0;
    NB_SIZE code_size = 0;
    NB_RESULT result = NB_RESULT_SUCCESS;

    /* コマンド入力状態の場合は、コマンド解析を行う */
    if(state == NB_STATE_REPL) {

        /* バッファをメモリに格納 */
        if(IS_SUCCESS(nano_basic_set_code(buf, size, &num, &pos))) {
            LOG("success memory_code_set. num:%d, pos:%d, size:%d, size-pos:%d\r\n", num, pos, size, size-pos);
        } else {
            LOG("failed memory_code_set. num:%d, pos:%d, size:%d, size-pos:%d\r\n", num, pos, size, size-pos);
        }

        /* もし行番号が0以外ならば、処理を終了 */
        if(num != 0) {
            return NB_STATE_REPL;
        }
        _num = num;
    }

    num = _num; /* コマンド実行前の行番号を保持 */
    if(memory_code_get(_num, &code, &code_size) == NB_RESULT_SUCCESS) {
        result = nano_basic_proc_command(&_num, code, code_size, &_pos, &state);
        if(result == NB_RESULT_SUCCESS) {
            /* LOG("nano_basic_proc: nano_basic_proc_command success next num:%d\r\n", _num); */
        } else if(result == NB_RESULT_FALSE) {
            /* LOG("nano_basic_proc: nano_basic_proc_command false next num:%d\r\n", _num); */
            state = NB_STATE_REPL;
        }
        else {
            /* LOG("nano_basic_proc: nano_basic_proc_command(%d) failed\r\n", _num); */
            nano_basic_print_error(result);
            state = NB_STATE_REPL;
        }
    } else {
        /* LOG("nano_basic_proc: memory_code_get(%d) failed\r\n", _num); */
        state = NB_STATE_REPL;
    }

    /* RUNモード以外、またはランモードで次に実行する行番号が異なる場合、_posをリセットする */
    if(state != NB_STATE_RUN_MODE || _num != num) {
        _pos = 0;
    }

    /* 次に実行するものがあれば、NB_STATE_LOOPをリターン、なければNB_STATE_INPUT_CODE */
    return state;
}


void nano_basic_set_input_value(NB_VALUE value)
{
    command_set_input_value(value);
}