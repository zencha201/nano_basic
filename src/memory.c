#include "nano_basic_local.h"

/* #define __DEBUG__ */
#ifdef __DEBUG__
#define SHOW_MEMORY_MAP() SHOW_BUF( _code, _memory_size);
#define SHOW_VALUE() SHOW_BUF( _code, _memory_size);
#define SHOW_STACK() SHOW_BUF( _code, _memory_size);
#else /* __DEBUG__ */
#define SHOW_MEMORY_MAP()
#define SHOW_VALUE()
#define SHOW_STACK()
#endif /* __DEBUG__ */

static NB_I8 *_code = NULL;
static NB_SIZE _code_size = 0;
static NB_SIZE _code_free_size = 0;
#define _code_use_size ((NB_SIZE)(_code_size - _code_free_size))
static NB_VALUE *_value = NULL;
static NB_BOOL _value_size = 0;
static NB_LINE_NUM *_stack = NULL;
static NB_SIZE _stack_size = 0;
static NB_SIZE _stack_top = 0;
#define _memory_size ((NB_SIZE)(_code_size + _value_size + _stack_size))

/*
初期化処理
*/
void memory_init(NB_I8 *memory, NB_SIZE code_size, NB_SIZE value_size, NB_SIZE stack_size)
{
    _code = memory;
    _code_size = code_size;
    _value = (NB_VALUE *)(_code + _code_size);
    _value_size = value_size;
    LOG("_code_size + _value_size =%d\r\n,", _code_size + _value_size);
    _stack = (NB_LINE_NUM *)(_code + _code_size + _value_size);
    _stack_size = stack_size;

    LOG("_code = %p\r\n", _code);
    LOG("_code_size = %d\r\n", _code_size);
    LOG("_value = %p\r\n", _value);
    LOG("_value_size = %d\r\n", _value_size);
    LOG("_stack = %p\r\n", _stack);
    LOG("_stack_size = %d\r\n", _stack_size);

    memory_code_clear();
    memory_value_clear();
    memory_stack_clear();
}

void memory_code_clear()
{
    util_memset(_code, 0x00, _code_size);
    _code_free_size = _code_size;
    SHOW_MEMORY_MAP();
}

/**
 * コード領域の指定した行の位置を取得する。
 * NB_RESULT_SUCCESS : 見つかった
 * NB_RESULT_FALSE : 見つからない (num の直後のposを返す)
 * NB_RESULT_ERROR_MEMORY : バッファ最後まで見つからない
 */
static NB_RESULT memory_code_search(NB_LINE_NUM num, NB_SIZE *pos)
{
    const NB_CODE *code = NULL;
    *pos = 0;

    /* LOG("memory_code_search: pos:%d, _code_use_size:%d,\r\n", *pos, _code_use_size); */
    while(*pos < _code_use_size) {
        code = (NB_CODE *)&_code[*pos];
        /* LOG("memory_code_search: num:%d, code->num:%d,\r\n", num, code->num); */
        if(code->num == num) { /* 行番号が見つかる */
            return NB_RESULT_SUCCESS;
        } else if(code->num > num) { /* 検索中の行番号よりも大きい(見つからない) */
            return NB_RESULT_FALSE;
        }
        *pos += NB_CODE_SIZE(code->size);
    }
    return NB_RESULT_ERROR_MEMORY;
}

/*
 * コード領域の指定した行を削除する。
*/
NB_RESULT memory_code_delete(NB_LINE_NUM num)
{
    NB_SIZE pos = 0;
    const NB_CODE *code = NULL;
    NB_SIZE delete_size = 0;

    /* 指定された行が存在するか確認する */
    IF_ERROR_EXIT(memory_code_search(num, &pos));

    code = (NB_CODE *)&_code[pos];
    delete_size = NB_CODE_SIZE(code->size);
    IF_ERROR_EXIT(util_memmove(&_code[pos], &_code[pos + delete_size], _code_size - pos - delete_size));

    /* LOG("memory_code_delete: before: _code_free_size:%d, delete_size:%d\r\n", _code_free_size, delete_size); */
    _code_free_size += delete_size;
    /* LOG("memory_code_delete: after : _code_free_size:%d\r\n", _code_free_size); */
    return NB_RESULT_SUCCESS;
}

/*
 * コード領域の指定した行の内容のポインタを取得する。
*/
NB_RESULT memory_code_get(NB_LINE_NUM num, NB_I8 **buf, NB_SIZE *size)
{
    NB_SIZE pos = 0;
    NB_CODE *code = NULL;

    IF_ERROR_EXIT(memory_code_search(num, &pos));
    code = (NB_CODE *)&_code[pos];
    *buf = code->buf;
    *size = code->size;

    return NB_RESULT_SUCCESS;
}

/*
 * コード領域の指定した行の内容をバッファにセットする。
*/
NB_RESULT memory_code_set(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size)
{
    NB_RESULT result = NB_RESULT_SUCCESS;
    NB_SIZE pos = 0;
    NB_CODE *code = NULL;

    /* 指定された行が存在するか確認する */
    result = memory_code_search(num, &pos);

    /* サイズチェック */
    if(result == NB_RESULT_SUCCESS) {
        code = (NB_CODE *)&_code[pos];
        IF_FALSE_EXIT(NB_CODE_SIZE(size) <= (_code_free_size + code->size));
    } else {
        IF_FALSE_EXIT(NB_CODE_SIZE(size) <= _code_free_size);
    }

    /* コード領域に領域確保する */
    if(result != NB_RESULT_SUCCESS && result != NB_RESULT_FALSE) { /* 指定した行が見つからない場合は、コード領域の末端に追加する。 */
        /* LOG("memory_code_set: not found line number.\r\n"); */
        pos = _code_use_size;
    } else {
        if(result == NB_RESULT_SUCCESS) { /* 指定した行が見つかった場合は、行を削除した後コード領域のスペースを確保する。 */
            /* LOG("memory_code_set: found line number.\r\n"); */
            IF_ERROR_EXIT(memory_code_delete(num));
        } else {
            /* LOG("memory_code_set: found next line number.\r\n"); */
        }
        IF_ERROR_EXIT(util_memmove(&_code[pos] + NB_CODE_SIZE(size), &_code[pos], _code_size - pos - NB_CODE_SIZE(size)));
    }

    /* 指定された行がない場合は、指定された行を挿入する */
    /* バッファをデータを書き込む */
    code = (NB_CODE *)&_code[pos];
    code->num = num;
    code->size = size;
    IF_ERROR_EXIT(util_memcpy(code->buf, buf, code->size, code->size));
    _code_free_size -= NB_CODE_SIZE(code->size);
    LOG("memory_code_set: num:%d, size:%d, pos:%d, use:%d, free:%d, total:%d\r\n", code->num, code->size, pos, _code_use_size, _code_free_size, _code_size);

    SHOW_MEMORY_MAP();
    return NB_RESULT_SUCCESS;
}

/*
 * 指定した行の次の行の行番号を取得する。
*/
NB_RESULT memory_code_next(NB_LINE_NUM *num)
{
    NB_I8 *buf = NULL;
    NB_SIZE size = 0;
    NB_CODE *code = NULL;

    IF_ERROR_EXIT(memory_code_get(*num, &buf, &size));
    code = (NB_CODE *)(buf + size);

    /* 使用中サイズを超えた場合は、次の行は存在しないとみなす */
    if((NB_I8 *)code >= _code + _code_use_size) {
        return NB_RESULT_FALSE;
    }
    /* LOG("BUF:0x%04x, size:%d, NUM:0x%04x\n", *buf, size, code->num); */
    *num = code->num;

    return NB_RESULT_SUCCESS;
}

/*
 * 行番号を振りなおす。
*/
NB_RESULT memory_code_set_line_number(NB_LINE_NUM num, NB_LINE_NUM step)
{
    NB_SIZE pos = 0;
    NB_CODE *code = NULL;

    LOG("memory_code_set_line_number: %d, %d\r\n", num, step);

    while(pos < _code_use_size) {
        code = (NB_CODE *)&_code[pos];
        if(code->num != 0) { /* 0行目はREPL用なのでスキップ */
            code->num = num;
            num += step;
        }
        pos = pos + NB_CODE_SIZE(code->size);
    }

    SHOW_MEMORY_MAP();
    return NB_RESULT_SUCCESS;
}

void memory_value_clear()
{
    util_memset((NB_I8 *)_value, 0x00, _value_size);
    SHOW_VALUE();
}

NB_RESULT memory_variable_get(NB_SIZE pos, NB_VALUE *value)
{
    IF_FALSE_EXIT((pos >= 0) && (pos * sizeof(NB_VALUE) < _value_size));
    *value = _value[pos];
    /* LOG("memory_variable_get: pos:%d, value:%d\r\n", pos, *value); */

    return NB_RESULT_SUCCESS;
}

NB_RESULT memory_variable_set(NB_SIZE pos, NB_VALUE value)
{
    IF_FALSE_EXIT((pos >= 0) && (pos * sizeof(NB_VALUE) < _value_size));
    _value[pos] = value;
    /* LOG("memory_variable_set: pos:%d, value:%d\r\n", pos, value); */

    SHOW_VALUE();
    return NB_RESULT_SUCCESS;
}

void memory_stack_clear(void)
{
    util_memset((NB_I8 *)_stack, 0x00, _stack_size);
    _stack_top = -1;
    SHOW_STACK();
}

NB_RESULT memory_stack_push(NB_LINE_NUM value)
{
    /* LOG("_stack_top:%d(%d), size=%d\r\n", _stack_top, _stack_top * sizeof(NB_LINE_NUM), _stack_size); */
    IF_FALSE_EXIT((_stack_top == -1) || ((_stack_top + 1) * sizeof(NB_LINE_NUM) < _stack_size));
    _stack_top++;
    _stack[_stack_top] = value;
    SHOW_STACK();
    return NB_RESULT_SUCCESS;
}

NB_RESULT memory_stack_pop(NB_LINE_NUM *value)
{
    IF_FALSE_EXIT((_stack_top >= 0));
    
    *value = _stack[_stack_top];
    _stack[_stack_top] = 0x00;
    _stack_top--;
    SHOW_STACK();
    return NB_RESULT_SUCCESS;
}
