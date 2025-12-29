#ifndef __TEST_H__
#define __TEST_H__

#define TEST_BEGIN(TEST_TITLE) \
NB_U32 __count_ok = 0; \
NB_U32 __count_ng = 0; \
NB_I8 *__test_title = TEST_TITLE; \
LOG("Start %s test\r\n", __test_title); \

#define TEST_END() \
LOG("End %s test total:%d, OK:%d, NG:%d\r\n", __test_title, (__count_ok + __count_ng), __count_ok, __count_ng); \

#define TEST(EXP) \
{ \
    NB_BOOL __result =  (EXP); \
    if(__result) { \
        LOG("[TEST] OK > %s:%d: %s\n", __FILE__, __LINE__, TOSTRING(EXP));\
        __count_ok++; \
    } else { \
        LOG("[TEST] NG > %s:%d: %s\n", __FILE__, __LINE__, TOSTRING(EXP));\
        __count_ng++; \
    } \
} \

#endif /* __TEST_H__ */
