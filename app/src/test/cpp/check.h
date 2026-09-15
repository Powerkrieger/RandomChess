// Minimal test helpers: CHECK() records failures instead of aborting, so one run reports all of
// them; the test's main() returns the failure count as the exit code.
#ifndef RANDOMCHESS_TEST_CHECK_H
#define RANDOMCHESS_TEST_CHECK_H

#include <cstdio>

inline int g_failures = 0;

// Variadic so that commas inside brace initialisers (Square{1, 2}) don't split the argument.
#define CHECK(...)                                                                \
    do {                                                                          \
        if (!(__VA_ARGS__)) {                                                     \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #__VA_ARGS__);    \
            ++g_failures;                                                         \
        }                                                                         \
    } while (0)

#define TEST(name)                                                                \
    static void name();                                                           \
    struct name##_registrar {                                                     \
        name##_registrar() { std::printf("--- %s\n", #name); name(); }            \
    } name##_instance;                                                            \
    static void name()

inline int finish() {
    if (g_failures == 0) {
        std::printf("all checks passed\n");
    } else {
        std::printf("%d check(s) failed\n", g_failures);
    }
    return g_failures;
}

#endif //RANDOMCHESS_TEST_CHECK_H
