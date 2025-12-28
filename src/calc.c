#include "nano_basic_local.h"

static NB_RESULT calc_pri_4th(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value);


NB_RESULT calc_get_variable_pos(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_SIZE *index)
{
    NB_VALUE offset = 0;
    IF_FALSE_EXIT(('A' <= code[*pos] && code[*pos] <= 'Z') || (code[*pos] == '@'));
    if(code[*pos] == '@') {
        *index = 'Z' - 'A' + 1;
    } else {
        *index = code[*pos] - 'A';
    }
    *pos = *pos + 1;

    if(code[*pos] == '(') {
        *pos = *pos + 1;
        calc(code, size, pos, &offset);
        *index = *index + offset;
        *pos = *pos + 1; /* ')' をスキップ */
    }

    return NB_RESULT_SUCCESS;
}

static NB_RESULT get_variable(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_SIZE index = 0;
    
    IF_ERROR_EXIT(calc_get_variable_pos(code, size, pos, &index));
    IF_ERROR_EXIT(memory_variable_get(index, value));

    return NB_RESULT_SUCCESS;
}

static NB_RESULT get_value(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_BOOL minus_flag = NB_FALSE;

    *value =  0;

    if(code[*pos] == '-') {
        minus_flag = NB_TRUE;
        *pos = *pos + 1;   
    }

    while('0' <= code[*pos] && code[*pos] <= '9') {
        *value = *value * 10 + (code[*pos] - '0');
        *pos = *pos + 1;
    }

    if(minus_flag == NB_TRUE) {
        *value = *value * -1;
    }

    return NB_RESULT_SUCCESS;
}

static NB_RESULT get_value_or_variable(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    if((code[*pos] == '-') || ('0' <= code[*pos] && code[*pos] <= '9')) {
        IF_ERROR_EXIT(get_value(code, size, pos, value));
    } else if(('A' <= code[*pos] && code[*pos] <= 'Z') || (code[*pos] == '@')) {
        IF_ERROR_EXIT(get_variable(code, size, pos, value));
    } else {
        /* 変数、数値でない場合は、0を返す ("(" の時が該当) */
        *value = 0;
    }
    return NB_RESULT_SUCCESS;
}


static NB_RESULT calc_pri_zero(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_VALUE value1 = 0;
    NB_VALUE value2 = 0;
    IF_ERROR_EXIT(get_value_or_variable(code, size, pos, &value1));

    while(1) {
        if(code[*pos] == '(') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_4th(code, size, pos, &value2));

            /* 閉じカッコの場合は、posを進める (最後に閉じていない場合があるので、チェックする) */
            if(code[*pos] == ')') {
                *pos = *pos + 1;
            }
            value1 = value2;
        } else {
            break;
        }    
    }

    *value = value1;
    return NB_RESULT_SUCCESS;
}


static NB_RESULT calc_pri_1st(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_VALUE value1 = 0;
    NB_VALUE value2 = 0;
    IF_ERROR_EXIT(calc_pri_zero(code, size, pos, &value1));

    while(1) {
        if(code[*pos] == '*') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_zero(code, size, pos, &value2));
            value1 = value1 * value2;
        } else if(code[*pos] == '/') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_zero(code, size, pos, &value2));
            value1 = value1 / value2;
        } else if(code[*pos] == '%') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_zero(code, size, pos, &value2));
            value1 = value1 % value2;
        } else {
            break;
        }    
    }

    *value = value1;
    return NB_RESULT_SUCCESS;
}

static NB_RESULT calc_pri_2d(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_VALUE value1 = 0;
    NB_VALUE value2 = 0;
    IF_ERROR_EXIT(calc_pri_1st(code, size, pos, &value1));

    while(1) {
        if(code[*pos] == '+') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_1st(code, size, pos, &value2));
            value1 = value1 + value2;
        } else if(code[*pos] == '-') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_1st(code, size, pos, &value2));
            value1 = value1 - value2;
        } else {
            break;
        }    
    }
    *value = value1;
    return NB_RESULT_SUCCESS;
}

static NB_RESULT calc_pri_3rd(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_VALUE value1 = 0;
    NB_VALUE value2 = 0;
    IF_ERROR_EXIT(calc_pri_2d(code, size, pos, &value1));

    while(1) {
        if(code[*pos] == '=') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_2d(code, size, pos, &value2));
            value1 = value1 == value2;
        } else if(code[*pos] == '!') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_2d(code, size, pos, &value2));
            value1 = value1 != value2;
        } else if(code[*pos] == '<') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_2d(code, size, pos, &value2));
            value1 = value1 < value2;
        } else if(code[*pos] == '>') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_2d(code, size, pos, &value2));
            value1 = value1 > value2;
        } else {
            break;
        }    
    }
    *value = value1;
    return NB_RESULT_SUCCESS;
}

static NB_RESULT calc_pri_4th(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_VALUE value1 = 0;
    NB_VALUE value2 = 0;
    IF_ERROR_EXIT(calc_pri_3rd(code, size, pos, &value1));

    while(1) {
        if(code[*pos] == '&') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_3rd(code, size, pos, &value2));
            value1 = value1 && value2;
        } else if(code[*pos] == '|') {
            *pos = *pos + 1;
            IF_ERROR_EXIT(calc_pri_3rd(code, size, pos, &value2));
            value1 = value1 || value2;
        } else {
            break;
        }
    }
    *value = value1;
    return NB_RESULT_SUCCESS;
}

NB_RESULT calc(const NB_I8 *code, NB_SIZE size, NB_SIZE *pos, NB_VALUE *value)
{
    NB_RESULT result = 0;
    LOG("calc: pos:%d, exp:[%s", *pos, &code[*pos]);
    result = calc_pri_4th(code, size, pos, value);
    LOG("] = %d (%d)\r\n", *value, result);
    return result;
}