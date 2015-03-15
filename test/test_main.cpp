#include <gmock/gmock.h>


int main(int argc, char **argv) {
	printf("Running main() from gtest_main.cc\n");
	testing::InitGoogleMock(&argc, argv);
	return RUN_ALL_TESTS();
}
