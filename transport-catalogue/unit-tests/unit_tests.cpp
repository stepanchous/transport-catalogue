#include "unit_tests.h"

#include <iostream>

#include "test_framework.h"

using namespace trc;

int main() {
#if defined(INPUT_READER) || defined(ALL)
    test::InputReader TEST_INPUT_READER;
    RUN_TEST(TEST_INPUT_READER);
    std::cout << std::endl;
#endif
#if defined(TRANSPORT_CATALOGUE) || defined(ALL)
    test::TransportCatalogue TEST_TRANSPORT_CATALOGUE;
    RUN_TEST(TEST_TRANSPORT_CATALOGUE);
#endif
}
