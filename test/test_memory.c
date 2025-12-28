#include "nano_basic_local.h"
#include "test.h"

#define CODE_SIZE (128)
#define VALUE_SIZE (48)
#define STACK_SIZE (16)
static NB_I8 memory[CODE_SIZE + VALUE_SIZE + STACK_SIZE];

/* プラットフォーム関数のモック */
void platform_print_ch(NB_I8 ch)
{
}

NB_VALUE platform_import(NB_VALUE num)
{
    return num * 2;
}

NB_BOOL platform_fopen(const NB_I8 *name, NB_BOOL write_mode)
{
    return NB_FALSE;
}

void platform_fclose()
{
}

NB_BOOL platform_fread(NB_I8 **buf, NB_SIZE *size)
{
    return NB_FALSE;
}

NB_BOOL platform_fwrite(NB_LINE_NUM num, NB_I8 *buf, NB_SIZE size)
{
    return NB_FALSE;
}

static void test_code()
{
    TEST_BEGIN("memory code test");

    {
        NB_I8 buf[] = "1234";
        TEST(IS_SUCCESS(memory_code_set(0x08, buf, sizeof(buf) - 1)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_I8 buf[] = "abcd";
        TEST(IS_SUCCESS(memory_code_set(0x80, buf, sizeof(buf) - 1)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_I8 buf[] = "5678";
        TEST(IS_SUCCESS(memory_code_set(0x0f, buf, sizeof(buf) - 1)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_I8 buf[] = {0xf1, 0xf2, 0xf3, 0xf4, 0x00};
        TEST(IS_SUCCESS(memory_code_set(0x0f, buf, sizeof(buf) - 1)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_I8 *buf;
        NB_SIZE size;
        TEST(IS_SUCCESS(memory_code_get(0x0f, &buf, &size)));
        SHOW_BUF(buf, size);
    }
    {
        NB_I8 *buf;
        NB_SIZE size;
        TEST(memory_code_get(0x22, &buf, &size) != NB_RESULT_SUCCESS);
    }
    {
        TEST(IS_SUCCESS(memory_code_delete(0x0f)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_I8 *buf;
        NB_SIZE size;
        TEST(memory_code_get(0x0f, &buf, &size) != NB_RESULT_SUCCESS);
    }

    TEST_END();
}

static void test_value()
{
    TEST_BEGIN("memory value test");
    {
        NB_SIZE pos = 'A' - 'A';
        NB_VALUE value = 0x0fff;
        TEST(IS_SUCCESS(memory_variable_set(pos, value)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_SIZE pos = 'B' - 'A';
        NB_VALUE value = 0x0aaa;
        TEST(IS_SUCCESS(memory_variable_set(pos, value)));
        SHOW_BUF( memory, sizeof(memory));
    }
    {
        NB_SIZE pos = 'A' - 'A';
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(memory_variable_get(pos, &value)));
        TEST(value == 0x0fff);
    }
    {
        NB_SIZE pos = 'B' - 'A';
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(memory_variable_get(pos, &value)));
        TEST(value == 0x0aaa);
    }
    {
        NB_VALUE value = 0;
        TEST(IS_ERROR(memory_variable_get(-1, &value)));
        TEST(value == 0);
    }
    {
        NB_SIZE pos = (VALUE_SIZE / sizeof(NB_VALUE)) - 1;
        NB_VALUE value = 0x0bbb;
        TEST(IS_SUCCESS(memory_variable_set(pos, value)));
        SHOW_BUF( memory, sizeof(memory));
        TEST(IS_SUCCESS(memory_variable_get(pos, &value)));
        TEST(value == 0x0bbb);
    }
    {
        NB_SIZE pos = (VALUE_SIZE / sizeof(NB_VALUE));
        NB_VALUE value = 0x0bbb;
        TEST(IS_ERROR(memory_variable_set(pos, value)));
        value = 0;
        TEST(IS_ERROR(memory_variable_get(pos, &value)));
        TEST(value == 0);
    }
    TEST_END();
}

static void test_stack()
{
    const NB_SIZE STACK_MAX = STACK_SIZE / sizeof(NB_LINE_NUM);
    TEST_BEGIN("memory stack test");
    {
        NB_LINE_NUM num = 0x00cc;
        TEST(IS_SUCCESS(memory_stack_push(num)));
        SHOW_BUF( memory, sizeof(memory));
        TEST(IS_SUCCESS(memory_stack_pop(&num)));
        SHOW_BUF( memory, sizeof(memory));
        TEST(num == 0x00cc);
    }
    {
        NB_LINE_NUM num = 0x00;
        TEST(IS_ERROR(memory_stack_pop(&num)));
        SHOW_BUF( memory, sizeof(memory));
        TEST(num == 0x00);
    }
    {
        NB_LINE_NUM num = 0;
        for(num = 0 ; num < STACK_MAX ; num++) {
            TEST(IS_SUCCESS(memory_stack_push(num + 1)));
        }
        SHOW_BUF( memory, sizeof(memory));
        TEST(IS_ERROR(memory_stack_push(num + 1)));
    }
    {
        NB_LINE_NUM i = 0;
        NB_LINE_NUM num = 0;
        for(i = 0 ; i < STACK_MAX ; i++) {
            TEST(IS_SUCCESS(memory_stack_pop(&num)));
            TEST(num == (STACK_MAX - i));
        }
        SHOW_BUF( memory, sizeof(memory));
        TEST(IS_ERROR(memory_stack_pop(&num)));
    }
    TEST_END();
}

static void test_code_next()
{
    TEST_BEGIN("memory code next test");

    {
        NB_I8 buf1[] = "PRINT \"A\"";
        NB_I8 buf2[] = "PRINT \"B\"";
        NB_I8 buf3[] = "PRINT \"C\"";
        NB_LINE_NUM num = 0;

        memory_code_clear();
        TEST(IS_SUCCESS(memory_code_set(10, buf1, sizeof(buf1) - 1)));
        TEST(IS_SUCCESS(memory_code_set(20, buf2, sizeof(buf2) - 1)));
        TEST(IS_SUCCESS(memory_code_set(30, buf3, sizeof(buf3) - 1)));

        num = 10;
        TEST(IS_SUCCESS(memory_code_next(&num)));
        TEST(num == 20);

        TEST(IS_SUCCESS(memory_code_next(&num)));
        TEST(num == 30);

        TEST(IS_ERROR(memory_code_next(&num)));
    }

    TEST_END();
}

static void test_code_set_line_number()
{
    TEST_BEGIN("memory code set line number test");

    {
        NB_I8 buf1[] = "PRINT \"A\"";
        NB_I8 buf2[] = "PRINT \"B\"";
        NB_I8 buf3[] = "PRINT \"C\"";
        NB_I8 *buf = NULL;
        NB_SIZE size = 0;

        memory_code_clear();
        TEST(IS_SUCCESS(memory_code_set(10, buf1, sizeof(buf1) - 1)));
        TEST(IS_SUCCESS(memory_code_set(20, buf2, sizeof(buf2) - 1)));
        TEST(IS_SUCCESS(memory_code_set(30, buf3, sizeof(buf3) - 1)));

        /* 行番号を100から20ずつ振り直す */
        TEST(IS_SUCCESS(memory_code_set_line_number(100, 20)));

        /* 新しい行番号で取得できるか確認 */
        TEST(IS_SUCCESS(memory_code_get(100, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(120, &buf, &size)));
        TEST(IS_SUCCESS(memory_code_get(140, &buf, &size)));

        /* 古い行番号では取得できない */
        TEST(IS_ERROR(memory_code_get(10, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(20, &buf, &size)));
        TEST(IS_ERROR(memory_code_get(30, &buf, &size)));
    }

    TEST_END();
}

static void test_clear_functions()
{
    TEST_BEGIN("memory clear functions test");

    {
        NB_I8 buf[] = "TEST";
        NB_I8 *code_buf = NULL;
        NB_SIZE size = 0;
        NB_VALUE value = 0;
        NB_LINE_NUM num = 0;

        /* データを設定 */
        memory_code_set(10, buf, sizeof(buf) - 1);
        memory_variable_set(0, 123);
        memory_stack_push(100);

        /* コードクリア */
        memory_code_clear();
        TEST(IS_ERROR(memory_code_get(10, &code_buf, &size)));

        /* 変数クリア */
        memory_variable_set(0, 456);
        memory_value_clear();
        TEST(IS_SUCCESS(memory_variable_get(0, &value)));
        TEST(value == 0);

        /* スタッククリア */
        memory_stack_push(200);
        memory_stack_clear();
        TEST(IS_ERROR(memory_stack_pop(&num)));
    }

    TEST_END();
}

int main(int argc, char *argv[])
{
    memory_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
    test_code();
    test_value();
    test_stack();
    test_code_next();
    test_code_set_line_number();
    test_clear_functions();
    return 0;
}
