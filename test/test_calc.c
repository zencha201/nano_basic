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

static void test_calc_basic_value()
{
    TEST_BEGIN("calc basic value test");

    {
        NB_I8 code[] = "123";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 123);
        TEST(pos == 3);
    }
    {
        NB_I8 code[] = "-456";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == -456);
        TEST(pos == 4);
    }
    {
        NB_I8 code[] = "0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);
        TEST(pos == 1);
    }

    TEST_END();
}

static void test_calc_variable()
{
    TEST_BEGIN("calc variable test");

    /* 変数Aに100を設定 */
    memory_variable_set(0, 100);  /* A = 100 */
    memory_variable_set(1, 200);  /* B = 200 */
    memory_variable_set(25, 999); /* Z = 999 */
    memory_variable_set(26, 777); /* @ = 777 */

    {
        NB_I8 code[] = "A";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 100);
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "B";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 200);
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "Z";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 999);
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "@";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 777);
        TEST(pos == 1);
    }

    TEST_END();
}

static void test_calc_add_sub()
{
    TEST_BEGIN("calc add/sub test");

    {
        NB_I8 code[] = "1+2";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 3);
    }
    {
        NB_I8 code[] = "10-5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 5);
    }
    {
        NB_I8 code[] = "100+200+300";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 600);
    }
    {
        NB_I8 code[] = "1000-100-50";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 850);
    }
    {
        NB_I8 code[] = "10+20-5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 25);
    }

    TEST_END();
}

static void test_calc_mul_div_mod()
{
    TEST_BEGIN("calc mul/div/mod test");

    {
        NB_I8 code[] = "3*4";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 12);
    }
    {
        NB_I8 code[] = "15/3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 5);
    }
    {
        NB_I8 code[] = "10%3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);
    }
    {
        NB_I8 code[] = "2*3*4";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 24);
    }
    {
        NB_I8 code[] = "100/10/2";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 5);
    }

    TEST_END();
}

static void test_calc_priority()
{
    TEST_BEGIN("calc operator priority test");

    {
        NB_I8 code[] = "1+2*3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 7);  /* (1+2)*3 = 9 ではなく 1+(2*3) = 7 */
    }
    {
        NB_I8 code[] = "10-2*3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 4);  /* 10-(2*3) = 4 */
    }
    {
        NB_I8 code[] = "100/10+5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 15);  /* (100/10)+5 = 15 */
    }
    {
        NB_I8 code[] = "2+3*4-5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 9);  /* 2+(3*4)-5 = 9 */
    }

    TEST_END();
}

static void test_calc_comparison()
{
    TEST_BEGIN("calc comparison test");

    {
        NB_I8 code[] = "5>3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* true */
    }
    {
        NB_I8 code[] = "5<3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);  /* false */
    }
    {
        NB_I8 code[] = "5=5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* true */
    }
    {
        NB_I8 code[] = "5!3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* true */
    }
    {
        NB_I8 code[] = "5!5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);  /* false */
    }
    {
        NB_I8 code[] = "10>5>2";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* (10>5)>2 = 1>2 = 0, or 10>(5>2) = 10>1 = 1 */
    }

    TEST_END();
}

static void test_calc_logical()
{
    TEST_BEGIN("calc logical test");

    {
        NB_I8 code[] = "1&1";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* true && true = true */
    }
    {
        NB_I8 code[] = "1&0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);  /* true && false = false */
    }
    {
        NB_I8 code[] = "0&0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);  /* false && false = false */
    }
    {
        NB_I8 code[] = "1|0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* true || false = true */
    }
    {
        NB_I8 code[] = "0|0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 0);  /* false || false = false */
    }
    {
        NB_I8 code[] = "1|1&0";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        /* (1|1)&0 = 1&0 = 0, or 1|(1&0) = 1|0 = 1 */
        /* 演算子優先順位により 1|(1&0) = 1 */
        LOG("result value: %d\r\n", value);
    }

    TEST_END();
}

static void test_calc_complex()
{
    TEST_BEGIN("calc complex expression test");

    memory_variable_set(0, 10);  /* A = 10 */
    memory_variable_set(1, 5);   /* B = 5 */

    {
        NB_I8 code[] = "A+B";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 15);
    }
    {
        NB_I8 code[] = "A*B-20";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 30);  /* 10*5-20 = 30 */
    }
    {
        NB_I8 code[] = "A>B";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* 10>5 = true */
    }
    {
        NB_I8 code[] = "A>5&B<10";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* (10>5)&(5<10) = 1&1 = 1 */
    }

    TEST_END();
}

static void test_calc_parentheses()
{
    TEST_BEGIN("calc parentheses test");

    {
        /* 基本的な括弧: (1+2)*3 = 9 */
        NB_I8 code[] = "(1+2)*3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 9);  /* (1+2)*3 = 3*3 = 9 */
    }
    {
        /* 括弧なしとの比較: 1+2*3 = 7 */
        NB_I8 code[] = "1+2*3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 7);  /* 1+(2*3) = 1+6 = 7 */
    }
    {
        /* ネストした括弧: ((1+2)*3) = 9 */
        NB_I8 code[] = "((1+2)*3)";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 9);
    }
    {
        /* 複数の括弧: (1+2)*(3+4) = 21 */
        NB_I8 code[] = "(1+2)*(3+4)";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 21);  /* 3*7 = 21 */
    }
    {
        /* 除算と括弧: (10+2)/3 = 4 */
        NB_I8 code[] = "(10+2)/3";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 4);  /* 12/3 = 4 */
    }
    {
        /* 減算と括弧: 10-(2+3) = 5 */
        NB_I8 code[] = "10-(2+3)";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 5);  /* 10-5 = 5 */
    }
    {
        /* 複雑な括弧: (1+2)*(3-4)+5 = 2 */
        NB_I8 code[] = "(1+2)*(3-4)+5";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 2);  /* 3*(-1)+5 = -3+5 = 2 */
    }
    {
        /* 深いネスト: (((1+2)*3)+4)*2 = 26 */
        NB_I8 code[] = "(((1+2)*3)+4)*2";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 26);  /* (((3)*3)+4)*2 = (9+4)*2 = 13*2 = 26 */
    }
    {
        /* 変数と括弧: (A+B)*C */
        memory_variable_set(0, 5);   /* A = 5 */
        memory_variable_set(1, 3);   /* B = 3 */
        memory_variable_set(2, 2);   /* C = 2 */
        
        NB_I8 code[] = "(A+B)*C";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 16);  /* (5+3)*2 = 8*2 = 16 */
    }
    {
        /* 比較演算と括弧: (5+3)>7 = 1 (true) */
        NB_I8 code[] = "(5+3)>7";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* 8>7 = true */
    }
    {
        /* 論理演算と括弧: (1&0)|(1&1) = 1 */
        NB_I8 code[] = "(1&0)|(1&1)";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* 0|1 = 1 */
    }
    {
        /* 連続した括弧: (1)(2) - これは1を返すはず（2つ目の括弧は無視される可能性）*/
        NB_I8 code[] = "(1)(2)";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        LOG("result value for (1)(2): %d\r\n", value);
        /* 実装に依存するが、おそらく2が返される */
    }
    {
        /* 括弧内の負数: (-5+3)*2 = -4 */
        NB_I8 code[] = "(-5+3)*2";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == -4);  /* (-2)*2 = -4 */
    }
    {
        /* 剰余と括弧: (10+3)%4 = 1 */
        NB_I8 code[] = "(10+3)%4";
        NB_SIZE pos = 0;
        NB_VALUE value = 0;
        TEST(IS_SUCCESS(calc(code, sizeof(code), &pos, &value)));
        TEST(value == 1);  /* 13%4 = 1 */
    }

    TEST_END();
}

static void test_calc_get_variable_pos()
{
    TEST_BEGIN("calc_get_variable_pos test");

    {
        NB_I8 code[] = "A";
        NB_SIZE pos = 0;
        NB_SIZE index = 0;
        TEST(IS_SUCCESS(calc_get_variable_pos(code, sizeof(code), &pos, &index)));
        TEST(index == 0);  /* A = index 0 */
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "Z";
        NB_SIZE pos = 0;
        NB_SIZE index = 0;
        TEST(IS_SUCCESS(calc_get_variable_pos(code, sizeof(code), &pos, &index)));
        TEST(index == 25);  /* Z = index 25 */
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "@";
        NB_SIZE pos = 0;
        NB_SIZE index = 0;
        TEST(IS_SUCCESS(calc_get_variable_pos(code, sizeof(code), &pos, &index)));
        TEST(index == 26);  /* @ = index 26 */
        TEST(pos == 1);
    }
    {
        NB_I8 code[] = "A(0)";
        NB_SIZE pos = 0;
        NB_SIZE index = 0;
        memory_variable_set(0, 5);  /* A = 5 */
        TEST(IS_SUCCESS(calc_get_variable_pos(code, sizeof(code), &pos, &index)));
        /* A(0) の場合、A + calc(0) = 0 + 0 = 0 */
        TEST(index == 0);
        TEST(pos == 4);
    }
    {
        NB_I8 code[] = "@(1)";
        NB_SIZE pos = 0;
        NB_SIZE index = 0;
        TEST(IS_SUCCESS(calc_get_variable_pos(code, sizeof(code), &pos, &index)));
        TEST(index == 27);  /* @(1) = @ + 1 = 26 + 1 = 27 */
        TEST(pos == 4);
    }

    TEST_END();
}

int main(int argc, char *argv[])
{
    memory_init(memory, CODE_SIZE, VALUE_SIZE, STACK_SIZE);
    
    test_calc_basic_value();
    test_calc_variable();
    test_calc_add_sub();
    test_calc_mul_div_mod();
    test_calc_priority();
    test_calc_parentheses();
    test_calc_comparison();
    test_calc_logical();
    test_calc_complex();
    test_calc_get_variable_pos();
    
    return 0;
}
