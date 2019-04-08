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

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <check.h>
#include <string.h>
#include "../src/convolve.h"

#define TRUE 1
#define FALSE 0

char * root = "/Users/tylergillson/Dropbox/UofC/W2019/CPSC.501/Assignments/";
char * input_suffix = "A4.Audio/DrySounds/Guitar/GuitarDry.wav";
char * ir_suffix = "A4.Audio/ImpulseResponses/Mono/big_hall.wav";
char * out_suffix = "A4/src/out.wav";
char input_path[200], ir_path[200], out_path[200];
	 
START_TEST(test_initialize) {
	
	initialize(input_path, ir_path, 0);
	
	// Check that WaveData structs were properly initialized:
	ck_assert(X.length != -1);
	ck_assert(H.length != -1);
}
END_TEST

START_TEST(test_convolve) {
	
	initialize(input_path, ir_path, 0);
	convolve(out_path, 0);
	
	// Ensure output data array's length is non-zero:
	ck_assert_msg((P != 0) == TRUE, "P should be non-zero. P: %d", P);	
	
	double min = DBL_MAX, max = DBL_MIN, total = 0.0, avg;
	for (int i = 0; i < P; i++) {
		if (Y[i] < min)
			min = Y[i];
		
		if (Y[i] > max)
			max = Y[i];
		
		total += Y[i];
	}
	avg = total / P;
	
	// Check that convolved data is non-zero and within [-1.0, 1.0]:
	ck_assert_msg((avg != 0) == TRUE, 
		"Output data average should be non-zero. Avg: %.3f", avg);
	
	ck_assert_msg((min >= -1.0) == TRUE,
		"Output data min should be >= -1.0. Min: %.3f", min);
	
	ck_assert_msg((max <= 1.0) == TRUE,
		"Output data max should be <= 1.0. Max: %.3f", max);
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

void setup_paths() {
	strcpy(input_path, root);
	strcpy(ir_path, root);
	strcpy(out_path, root);
	strcat(input_path, input_suffix);
	strcat(ir_path, ir_suffix);
	strcat(out_path, out_suffix);
}

int main(int argc, char **argv) {
	setup_paths();
	
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
