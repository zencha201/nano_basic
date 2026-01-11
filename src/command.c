#include "nano_basic_local.h"

/* INPUTコマンド関連 */
#define INIT_VALUE_INDEX (-1)
static NB_SIZE _input_variable_index = INIT_VALUE_INDEX;
static NB_STATE _input_prev_state = NB_STATE_REPL;
static NB_VALUE _input_value = 0;

void command_init()
{
    _input_variable_index = INIT_VALUE_INDEX;
}

extern CommandFuncList _command[];

NB_RESULT command_help(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    CommandFuncList *p = _command;
    NB_I8 *ch  = 0;
    while(p->func != NULL) {
        ch = p->name;
        while(*ch != NULL) {
            platform_print_ch(*ch);
            ch++;
        }
        platform_print_ch('\r');
        platform_print_ch('\n');
        p++;
    }
    return memory_code_next(num);
}

NB_RESULT command_rem(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    return memory_code_next(num);
}

NB_RESULT command_let(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_SIZE index = 0;
    NB_VALUE value = 0;

    calc_get_variable_pos(code, size, pos, &index);
    IF_FALSE_EXIT(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER);
    *pos = *pos + 1;
    IF_ERROR_EXIT(calc(code, size, pos, &value));
    IF_ERROR_EXIT(memory_variable_set(index, value));

    return memory_code_next(num);
}

NB_RESULT command_print(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE value = 0;

    while(*pos < size && code[*pos] != NB_CODE_STR_TERMINATE) {
        if(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER) { /* 区切り文字 */
            *pos = *pos + 1;
        } else if(code[*pos] == NB_CODE_STRING_SIGN) { /* 文字列 */
            *pos = *pos + 1;
            while(*pos < size && code[*pos] != NB_CODE_STRING_SIGN) {
                platform_print_ch(code[*pos]);
                *pos = *pos + 1;
            }
            platform_print_ch(' ');
            *pos = *pos + 1; /* 最後の文字列記号分をスキップする (バッファサイズをオーバーしてもwhileから抜ける
            ) */
        } else { /* 数式 */
            IF_ERROR_EXIT(calc(code, size, pos, &value));
            util_print_value(value);
            platform_print_ch(' ');
            *pos = *pos + 1;
        }
    }
    platform_print_ch('\r');
    platform_print_ch('\n');

    return memory_code_next(num);
}


void command_set_input_value(NB_VALUE value)
{
    _input_value = value;
}

NB_RESULT command_input(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    if(_input_variable_index == INIT_VALUE_INDEX) { /* 第1段階：変数のインデックス特定と状態をインプットモードに変更 */
        /* LOG("command_input: 1st ph _input_variable_index = %d\r\n", _input_variable_index); */
        IF_ERROR_EXIT(calc_get_variable_pos(code, size, pos, &_input_variable_index));
        _input_prev_state = *state;
        *state = NB_STATE_INPUT_NUMBER;
        return NB_RESULT_SUCCESS;
    } else { /* 第1段階：入力された値を変数にセットして、状態を戻す */
        /* LOG("command_input: 2nd ph _input_variable_index = %d\r\n", _input_variable_index); */
        IF_ERROR_EXIT(memory_variable_set(_input_variable_index, _input_value));
        *state = _input_prev_state;
        _input_variable_index = INIT_VALUE_INDEX;
        return memory_code_next(num);
    }
}

NB_RESULT command_if(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE value = 0;

    /* 式を計算 */
    IF_ERROR_EXIT(calc(code, size, pos, &value));

    if(value > 0) {
        LOG("command_if: true value:%d, pos:%d\r\n", value, *pos);
        IF_FALSE_EXIT(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER && code[*pos + 1] != NB_CODE_STR_TERMINATE);
        *pos = *pos + 1;
        return NB_RESULT_SUCCESS;
    }
    LOG("command_if: false value:%d, pos:%d\r\n", value, *pos);

    return memory_code_next(num);
}

NB_RESULT command_then(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    /* 次のコマンドの先頭を指しているはずなので、何もしない */
    return NB_RESULT_SUCCESS;
}

NB_RESULT command_goto(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE value = 0;

    /* 式を計算 */
    IF_ERROR_EXIT(calc(code, size, pos, &value));
    IF_FALSE_EXIT(value > 0);
    *num = value;
    return NB_RESULT_SUCCESS;
}

NB_RESULT command_gosub(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_VALUE value = 0;

    /* 式を計算 */
    IF_ERROR_EXIT(calc(code, size, pos, &value));
    IF_FALSE_EXIT(value > 0);
    IF_ERROR_EXIT(memory_stack_push(*num));
    *num = value;

    return NB_RESULT_SUCCESS;
}

NB_RESULT command_return(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    IF_ERROR_EXIT(memory_stack_pop(num));
    return memory_code_next(num);
}

NB_RESULT command_end(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    *state = NB_STATE_REPL;
    return memory_code_next(num);
}

NB_RESULT command_run(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    *state = NB_STATE_RUN_MODE;
    return memory_code_next(num);
}

NB_RESULT command_new(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    memory_code_clear();
    return memory_code_next(num);
}

NB_RESULT command_load(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    const NB_I8 *name = "FILE.BAS";
    NB_I8 *buf = NULL;
    NB_SIZE size_tmp = 0;
    NB_LINE_NUM num_tmp = 0;
    NB_SIZE pos_tmp = 0;
    NB_RESULT ret = NB_RESULT_SUCCESS;

    if(code[*pos] != '\0') {
        name = &code[*pos];
    }

    while(code[*pos] != '\0') {
        *pos = *pos + 1;
    }

    IF_FALSE_EXIT(platform_fopen(name, NB_FALSE));

    memory_code_clear();
    while(platform_fread(&buf, &size_tmp) == NB_TRUE) {

        /* 末尾にNULL文字をセットする */
        buf[size_tmp] = '\0';
        size_tmp++;

        if((ret = nano_basic_set_code(buf, size_tmp, &num_tmp, &pos_tmp)) != NB_RESULT_SUCCESS) {
            platform_fclose();
            return ret;
        }
    }
    platform_fclose();

    return memory_code_next(num);
}

NB_RESULT command_save(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    const NB_I8 *name = "FILE.BAS";
    NB_LINE_NUM num_tmp = 0;
    NB_I8 *buf = NULL;
    NB_SIZE size_tmp = 0;

    if(code[*pos] != '\0') {
        name = &code[*pos];
    }

    while(code[*pos] != '\0') {
        *pos = *pos + 1;
    }

    IF_ERROR_EXIT(memory_code_next(&num_tmp));
    IF_FALSE_EXIT(platform_fopen(name, NB_TRUE));

    do {
        IF_ERROR_EXIT(memory_code_get(num_tmp, &buf, &size_tmp)) {
            size_tmp = size_tmp - 2;
            platform_fwrite(num_tmp, buf, size_tmp);
        }
    } while(IS_SUCCESS(memory_code_next(&num_tmp)));

    platform_fclose();

    return memory_code_next(num);
}

NB_RESULT command_renum(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_LINE_NUM start_num = 10;
    NB_LINE_NUM step = 10;
    
    IF_ERROR_EXIT(calc(code, size, pos, &start_num));
    LOG("%d[%c]\r\n", code[*pos], code[*pos]);
    if(code[*pos] == NB_CODE_STR_SEPALATE_PARAMETER) {
        *pos = *pos + 1;
        IF_ERROR_EXIT(calc(code, size, pos, &step));
    }

    IF_ERROR_EXIT(memory_code_set_line_number(start_num, step));
    return memory_code_next(num);
}

NB_RESULT command_list(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_LINE_NUM num_tmp = 0;
    NB_LINE_NUM num_end = -1;
    NB_I8 *buf = NULL;
    NB_SIZE i = 0;
    NB_SIZE size_tmp = 0;
    NB_VALUE value = 0;

    if(code[*pos] != NB_CODE_STR_TERMINATE) { /* 第1パラメータを取得 */
        IF_ERROR_EXIT(calc(code, size, pos, &value));
        num_tmp = value;
        if(code[*pos] != NB_CODE_STR_TERMINATE) { /* 第2パラメータを取得 */
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc(code, size, pos, &value));
            num_end = value;
        } else {
            num_end = num_tmp;
        }
    } else {
        num_tmp = 0;
        IF_ERROR_EXIT(memory_code_next(&num_tmp));
    }

    LOG("command_list: start:%d, end:%d\r\n", num_tmp, num_end);
    do {
        if(IS_SUCCESS(memory_code_get(num_tmp, &buf, &size_tmp))) {
            util_print_value(num_tmp);
            platform_print_ch(' ');
            for(i = 0; i < size_tmp - 1 /* NULL終端を含まない */; i++) {
                platform_print_ch(buf[i]);
            }
            platform_print_ch('\r');
            platform_print_ch('\n');
        }
    } while(IS_SUCCESS(memory_code_next(&num_tmp)) && (num_end == -1 || num_tmp <= num_end));

    return memory_code_next(num);
}

NB_RESULT command_del(NB_LINE_NUM *num, const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_STATE *state)
{
    NB_LINE_NUM num_tmp = 0;
    NB_LINE_NUM num_next = 0;
    NB_LINE_NUM num_end = -1;
    NB_I8 *buf = NULL;
    NB_SIZE i = 0;
    NB_SIZE size_tmp = 0;
    NB_VALUE value = 0;
    NB_RESULT ret = NB_RESULT_SUCCESS;

    if(code[*pos] != NB_CODE_STR_TERMINATE) { /* 第1パラメータを取得 */
        IF_ERROR_EXIT(calc(code, size, pos, &value));
        num_tmp = value;
        if(code[*pos] != NB_CODE_STR_TERMINATE) { /* 第2パラメータを取得 */
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc(code, size, pos, &value));
            num_end = value;
        } else {
            num_end = num_tmp;
        }
    } else {
        num_tmp = 0;
        IF_ERROR_EXIT(memory_code_next(&num_tmp));
    }

    LOG("command_del: start:%d, end:%d\r\n", num_tmp, num_end);
    do {
        if(IS_SUCCESS(memory_code_get(num_tmp, &buf, &size_tmp))) {
            num_next = num_tmp;
            ret = memory_code_next(&num_next);
            (void)memory_code_delete(num_tmp);
            num_tmp = num_next;
        }
    } while(IS_SUCCESS(ret) && (num_end == -1 || num_tmp <= num_end));

    return memory_code_next(num);
}

