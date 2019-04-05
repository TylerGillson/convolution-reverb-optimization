/**
 * Sources:
 *     - https://libcheck.github.io/check/doc/check_html/check_3.html
 *     - https://www.systutorials.com/docs/linux/man/1-checkmk/
 * 
 * Compile with:
 *     gcc test.c -lcheck -lsndfile -o test
 * 
 * Run with:
 *     ./test
 * 
 */ 

#include <stdlib.h>
#include <check.h>
#include "../src/convolve.h"
 
START_TEST(test_initialize) {
	
	initialize("/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/A4.Audio/DrySounds/Drum Kit/DrumsDry.wav",
			   "/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/A4.Audio/ImpulseResponses/Mono/big_hall.wav", 0);
	
	// Check that WaveData structs were properly initialized:
	ck_assert(X.length != -1);
	ck_assert(H.length != -1);
}
END_TEST

START_TEST(test_convolve) {
	
	initialize("/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/A4.Audio/DrySounds/Drum Kit/DrumsDry.wav",
			   "/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/A4.Audio/ImpulseResponses/Mono/big_hall.wav", 0);
	convolve("/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/A4/src/out.wav", 0);

	// Check that Y[] was initialized with the proper size:
	ck_assert_int_eq(X.length + H.length - 1, P);
	
	double min = 1.0, max = -1.0, total = 0.0, avg;
	for (int i = 0; i < P; i++) {
		if (Y[i] < min)
			min = Y[i];
		
		if (Y[i] > max)
			max = Y[i];
		
		total += Y[i];
	}
	avg = total / P;
	
	// Check that the convolved data is non-zero and within [-1.0, 1.0]:
	ck_assert(avg != 0);
	ck_assert(min >= -1.0);
	ck_assert(max <= 1.0);
}
END_TEST
	
Suite * convolution_suite(void) {
	Suite *s;
	TCase *tc_core;

	s = suite_create("Convolve");

	/* Core test case */
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, test_initialize);
	tcase_add_test(tc_core, test_convolve);
	tcase_set_timeout(tc_core, 0);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char **argv) {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = convolution_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
