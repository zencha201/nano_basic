#include "nano_basic_local.h"

NB_BOOL util_strskipuntil(const NB_I8 *str, NB_SIZE *pos, NB_I8 ch, NB_SIZE size)
{
    NB_SIZE pos_tmp = *pos;

    while(str[pos_tmp] == ch && str[pos_tmp] != '\0') {
        pos_tmp++;
    }

    *pos = pos_tmp;

    return (str[pos_tmp] == '\0') ? NB_FALSE : NB_TRUE;
}

NB_BOOL util_strcmp(const NB_I8 *str1, const NB_I8 *str2, NB_SIZE size)
{
    NB_SIZE pos = 0;
    for(pos = 0; pos < size; pos++) {
        if(str1[pos] != str2[pos]) {
            return NB_FALSE;
        }
    }
    return NB_TRUE;
}

NB_RESULT util_atoi(const NB_I8 *str, NB_SIZE *pos, NB_SIZE size, NB_I16 *value)
{
    NB_SIZE pos_tmp = (pos != NULL) ? *pos : 0;
    NB_I16 value_tmp = 0;
    NB_BOOL minus_flag = NB_FALSE;

    if(str[pos_tmp] == '-') {
        minus_flag = NB_TRUE;
        pos_tmp++;
    }

    while('0' <= str[pos_tmp] && str[pos_tmp] <= '9' && pos_tmp < size) {
        value_tmp = (value_tmp * 10) + (NB_I16)(str[pos_tmp] - '0');
        pos_tmp++;
    }

    if(minus_flag) {
        value_tmp = -1 * value_tmp;
    }

    if(pos != NULL) {
        *pos = pos_tmp;
    }
    if(value != NULL) {
        *value = value_tmp;
    }

    return NB_RESULT_SUCCESS;
}

NB_SIZE util_strlen(const NB_I8 *str)
{
    NB_SIZE i = 0;
    for(i = 0 ; str[i] != '\0' ; i++) {
        ;
    }
    return i;
}

NB_RESULT util_memset(NB_I8 *dest, NB_I8 value, NB_SIZE size)
{
    NB_SIZE i = 0;
    IF_FALSE_EXIT(dest != NULL);

    for (i = 0 ; i < size ; i++) {
        dest[i] = value;
    }

    return NB_RESULT_SUCCESS;
}

NB_RESULT util_memcpy(NB_I8 *dest, NB_I8 *src, NB_SIZE size, NB_SIZE dest_size)
{
    IF_FALSE_EXIT((dest != NULL) && (src != NULL) && (size <= dest_size));

    do {
        *dest = *src;
        dest++;
        src++;
        size--;
    } while(size > 0);

    return NB_RESULT_SUCCESS;
}

NB_RESULT util_memmove(NB_I8 *dest, NB_I8 *src, NB_SIZE size)
{
    IF_FALSE_EXIT((dest != NULL) && (src != NULL));

    if(src < dest) { /* srcが前ならば、逆順コピーを行う */
        dest += (size - 1);
        src += (size - 1);
        do {
            /* LOG("dest:%d <= src:%d\r\n", *dest, *src); */
            *dest = *src;
            dest--;
            src--;
            size--;
        } while(size > 0);
    } else {
        do {
            *dest = *src;
            dest++;
            src++;
            size--;
        } while(size > 0);
    }

    return NB_RESULT_SUCCESS;
}

void util_print_value(NB_VALUE value)
{
    NB_VALUE tmp = 10000; /* NB_VALUEが2byteを想定 */
    NB_VALUE mod =  0;
    NB_BOOL flag = NB_FALSE;

    if(sizeof(value) == 1) {
        tmp = 100;
    } else if(sizeof(value) == 4) {
        tmp = 1000000000;
    }

    if(value == 0) {
        platform_print_ch('0');
        return;
    }

    if(value < 0) {
        platform_print_ch('-');
        value *= -1;
    }

    while(tmp > 0) {
        mod = (NB_VALUE)(value / tmp);
        /* LOG("<v:%d, m:%d, t:%d>", value, mod, tmp); */
        if(mod != 0) {
            flag = NB_TRUE;
        }

        if(flag) {
            platform_print_ch('0' + mod);
        }
        value -= mod * tmp;
        tmp = (NB_VALUE)(tmp / 10);
    }
    
}