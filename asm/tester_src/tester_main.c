// This program will check your assignments (a further check for forbidden instructions happens after this program).
// Feel free to modify this file to help you in debugging your assembly code.
// Note that if you modify this file, there may be mismatches between the grade that you see from your local tests, and the grade that you will receive.

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

extern long long int assignment_1_0(int, int);
extern long long int assignment_1_1(int, int);
extern long long int assignment_1_2(int);
extern long long int assignment_1_3();
extern long long int assignment_1_4(int);
extern long long int assignment_1_5(int);
extern long long int assignment_1_6(int, int);
extern long long int assignment_1_7(int, int, int);

extern long long int assignment_2_0(int, int, int);
extern long long int assignment_2_1(int);

extern long long int assignment_3_0(int*);
extern long long int assignment_3_1(int*, int);
extern long long int assignment_3_2(int*);
extern long long int assignment_3_3(int*, int);

extern long long int assignment_4_0(void (*fun_ptr)(void));
extern long long int assignment_4_1(int (*fun_ptr)(void*, int), int);

extern long long int assignment_5_0(int16_t*, int, int, int16_t*, int, int16_t*);

/**
 * Helper function for assignment 2.1
 */
static int fibonacci(int n) {
    int a0 = 0;
    int a1 = 1;

    if (n == 0)
        return 0;

    while (--n) {
        int t = a1 + a0;
        a0 = a1;
        a1 = t;
    }
    return a1;
}

/**
 * Helper function for assignment 4.0
 */
static int is_function_accessed_4_0;
void function_4_0() {
    is_function_accessed_4_0 = 1;
    // Unset some caller-saved registers
    asm volatile ("addi t0, x0, 0");
    asm volatile ("addi t1, x0, 0");
    asm volatile ("addi t2, x0, 0");
    asm volatile ("addi t3, x0, 0");
    asm volatile ("addi t4, x0, 0");
    asm volatile ("addi t5, x0, 0");
    asm volatile ("addi t6, x0, 0");
    asm volatile ("fmv.d.x ft0, x0");
    asm volatile ("fmv.d.x ft1, x0");
    asm volatile ("fmv.d.x ft2, x0");
    asm volatile ("fmv.d.x ft3, x0");
    asm volatile ("fmv.d.x ft4, x0");
    asm volatile ("fmv.d.x ft5, x0");
    asm volatile ("fmv.d.x ft6, x0");
    asm volatile ("fmv.d.x ft7, x0");
    asm volatile ("fmv.d.x ft8, x0");
    asm volatile ("fmv.d.x ft9, x0");
    asm volatile ("fmv.d.x ft10, x0");
    asm volatile ("fmv.d.x ft11, x0");
}

/**
 * Helper function for assignment 4.1
 */
static int expected_recursive_call_args[40];
static int index_in_recursive_table;
static int assignment_4_1_success;
int recursive_call_4_1(int recursive_arg) {
    if (expected_recursive_call_args[index_in_recursive_table] < 0) {
        printf("Error: in assignment 4_1. Too many recursive calls. Aborting the tests.\n");
        assignment_4_1_success = 0;
        exit(0);
    }
    if (recursive_arg != expected_recursive_call_args[index_in_recursive_table]) {
        printf("Error: recursive call of assignment 4_1 provided wrong a1 (intermediate result) at step %d (first step is 0). Got 0x%x, expected 0x%x.\n", index_in_recursive_table, recursive_arg, expected_recursive_call_args[index_in_recursive_table]);
        assignment_4_1_success = 0;
    }
    index_in_recursive_table++;
    return assignment_4_1((int (*)(void*, int))recursive_call_4_1, recursive_arg);
}
static void fill_expected_recursive_call_args(int initial_n) {
    for (int i = 0; i < initial_n+1; i++) {
        expected_recursive_call_args[i] = initial_n-i;
    }
    for (int i = initial_n+1; i < 40; i++)
        expected_recursive_call_args[i] = -1;
}

/**
 * Helper functions for assignment 5.0
 */
#define MAX_MATR_SIDE_5_0 15
#define MAX_MATR_TOTAL_SIZE_5_0 (MAX_MATR_SIDE_5_0*MAX_MATR_SIDE_5_0)
static int M0_width;
static int M0_height;
static int M1_width;
static int M1_height;
static int16_t M0[MAX_MATR_TOTAL_SIZE_5_0];
static int16_t M1[MAX_MATR_TOTAL_SIZE_5_0];
static int16_t M2[MAX_MATR_TOTAL_SIZE_5_0];
static int16_t M2_ref[MAX_MATR_TOTAL_SIZE_5_0];

void populate_matrices_5_0() {
    for (int i = 0; i < MAX_MATR_SIDE_5_0; i++)
        for (int j = 0; j < MAX_MATR_SIDE_5_0; j++) {
            M0[i*MAX_MATR_SIDE_5_0+j] = rand();
            M1[i*MAX_MATR_SIDE_5_0+j] = rand();
        }
}
void ref_mult_matrices_5_0() {
    // Zero-initialize the reference and return matrices.
    for (int i = 0; i < MAX_MATR_SIDE_5_0; i++)
        for (int j = 0; j < MAX_MATR_SIDE_5_0; j++) {
            M2[i*MAX_MATR_SIDE_5_0+j] = 0;
            M2_ref[i*MAX_MATR_SIDE_5_0+j] = 0;
        }
    // Compute the reference multiplication.
    for (int i = 0; i < M0_height; ++i)
        for (int j = 0; j < M1_width; ++j) {
            for (int k = 0; k < M1_height; ++k) {
                M2_ref[i*M1_width+j] += M0[i*M0_width+k] * M1[k*M1_width+j];
            }
        }
}

int compare_matrices_5_0() {
    for (int i = 0; i < M0_height; ++i)
        for (int j = 0; j < M1_width; ++j) {
            if (M2_ref[i*M1_width+j] != M2[i*M1_width+j]) {
                // printf("Got mismatch at x=%d,y=%d, expected %d, got %d.\n", j, i, M2_ref[i*M1_width+j], M2[i*M1_width+j]);
                return 0;
            }
        }
    return 1;
}

int dummy_solution_5_0() {
    for (int i = 0; i < M0_height; ++i)
        for (int j = 0; j < M1_width; ++j) {
            for (int k = 0; k < M0_width; ++k) {
                M2[i*M1_width+j] += M0[i*M0_width+k] * M1[k*M1_width+j];
            }
        }
}

/**
 * Check whether registers have indeed been saved.
 */
typedef struct {
    long long int sp; 
    long long int s[12];
} saved_regs_t;

static inline saved_regs_t save_regs() {
    saved_regs_t ret;
    asm volatile ("mv %0, sp" : "=r" (ret.sp));
    asm volatile ("mv %0, s0" : "=r" (ret.s[0]));
    asm volatile ("mv %0, s1" : "=r" (ret.s[1]));
    asm volatile ("mv %0, s2" : "=r" (ret.s[2]));
    asm volatile ("mv %0, s3" : "=r" (ret.s[3]));
    asm volatile ("mv %0, s4" : "=r" (ret.s[4]));
    asm volatile ("mv %0, s5" : "=r" (ret.s[5]));
    asm volatile ("mv %0, s6" : "=r" (ret.s[6]));
    asm volatile ("mv %0, s7" : "=r" (ret.s[7]));
    asm volatile ("mv %0, s8" : "=r" (ret.s[8]));
    asm volatile ("mv %0, s9" : "=r" (ret.s[9]));
    asm volatile ("mv %0, s10" : "=r" (ret.s[10]));
    asm volatile ("mv %0, s11" : "=r" (ret.s[11]));
    return ret;
}

static inline int check_saved_regs(saved_regs_t saved) {
    long long int tmp;
    int ret = 0;
    asm volatile ("mv %0, sp" : "=r" (tmp));
    if (tmp != saved.sp) ret |= 1 << 0;
    asm volatile ("mv %0, s0" : "=r" (tmp));
    if (tmp != saved.s[0]) ret |= 1 << 1;
    asm volatile ("mv %0, s1" : "=r" (tmp));
    if (tmp != saved.s[1]) ret |= 1 << 2;
    asm volatile ("mv %0, s2" : "=r" (tmp));
    if (tmp != saved.s[2]) ret |= 1 << 3;
    asm volatile ("mv %0, s3" : "=r" (tmp));
    if (tmp != saved.s[3]) ret |= 1 << 4;
    asm volatile ("mv %0, s4" : "=r" (tmp));
    if (tmp != saved.s[4]) ret |= 1 << 5;
    asm volatile ("mv %0, s5" : "=r" (tmp));
    if (tmp != saved.s[5]) ret |= 1 << 6;
    asm volatile ("mv %0, s6" : "=r" (tmp));
    if (tmp != saved.s[6]) ret |= 1 << 7;
    asm volatile ("mv %0, s7" : "=r" (tmp));
    if (tmp != saved.s[7]) ret |= 1 << 8;
    asm volatile ("mv %0, s8" : "=r" (tmp));
    if (tmp != saved.s[8]) ret |= 1 << 9;
    asm volatile ("mv %0, s9" : "=r" (tmp));
    if (tmp != saved.s[9]) ret |= 1 << 10;
    asm volatile ("mv %0, s10" : "=r" (tmp));
    if (tmp != saved.s[10]) ret |= 1 << 11;
    asm volatile ("mv %0, s11" : "=r" (tmp));
    if (tmp != saved.s[11]) ret |= 1 << 12;
    return ret;
}

/**
 * Test assignments
 * @return 1 on success, 0 on failure.
 */

static inline int test_assignment_1_0(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        long long int a = rand();
        long long int b = rand();
        long long int got = assignment_1_0(a, b);
        long long int expected = a+b;
        if(got != expected) {
            printf("Diff: 0x%x.\n", got-expected);
            printf("Eq: 0x%x.\n", got==expected);
            printf("assignment_1_0 failed for a=0x%x, b=0x%x. Got 0x%x, expected 0x%x.\n", a, b, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_1(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int must_be_equal = rand() & 1;
        int a = rand();
        int b;
        if (must_be_equal)
            b = a;
        else
            b = rand();
        int got = assignment_1_1(a, b);
        int expected = a==b;
        if(got != expected) {
            printf("assignment_1_1 failed for a=0x%x, b=0x%x. Got 0x%x, expected 0x%x.\n", a, b, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_2(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int a = rand();
        int got = assignment_1_2(a);
        int expected = a<0;
        if(got != expected) {
            printf("assignment_1_2 failed for a=0x%x. Got 0x%x, expected 0x%x.\n", a, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_3() {
    int is_success_1_3 = 0xbadcab1eL == assignment_1_3();
    if (!is_success_1_3)
        printf("assignment_1_3 failed Got 0x%llx, expected 0x%llx.\n", assignment_1_3(), 0xbadcab1eL);
    return is_success_1_3;
}

static inline int test_assignment_1_4(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int a = rand();
        int got = assignment_1_4(a);
        int expected = ~a;
        if(got != expected) {
            printf("assignment_1_4 failed for a=0x%x. Got 0x%x, expected 0x%x.\n", a, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_5(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int a = 1 + (rand() % 31);
        int got = assignment_1_5(a);
        int expected = (1 << 31) >> (a-1);
        if(got != expected) {
            printf("assignment_1_5 failed for a=0x%x. Got 0x%x, expected 0x%x.\n", a, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_6(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int must_be_equal = rand() & 1;
        int a = rand();
        int b = 1 + (rand() % 31);
        int got = assignment_1_6(a, b);
        int expected = a & ~((1 << 31) >> (b-1));
        if(got != expected) {
            printf("assignment_1_6 failed for a=0x%x, b=0x%x. Got 0x%x, expected 0x%x.\n", a, b, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_1_7(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int must_be_equal = rand() & 1;
        int a = rand() & 1;
        int b = rand();
        int c = rand();
        int got = assignment_1_7(a, b, c);
        int expected = a ? c : b;
        if(got != expected) {
            printf("assignment_1_7 failed for a=0x%x, b=0x%x, c=0x%x. Got 0x%x, expected 0x%x.\n", a, b, c, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_2_0(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int must_be_equal = rand() & 1;
        int a = rand() & 1;
        int b = rand();
        int c = rand();
        int got = assignment_2_0(a, b, c);
        int expected = a ? c : b;
        if(got != expected) {
            printf("assignment_2_0 failed for a=0x%x, b=0x%x, c=0x%x. Got 0x%x, expected 0x%x.\n", a, b, c, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_2_1(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        int a = rand() % 48;
        int got = assignment_2_1(a);
        int expected = fibonacci(a);
        if(got != expected) {
            printf("assignment_2_1 failed for a=0x%x. Got 0x%lx, expected 0x%lx.\n", a, got, expected);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_3_0(int num_reps) {
    int is_success = 1;
    int arr[1000];
    for (int i = 0; i < num_reps; i++) {
        int id_in_arr = rand() % 1000;
        int *a = (arr + id_in_arr);
        int before = *a;
        assignment_3_0(a);
        if(*a != before+1) {
            printf("assignment_3_0 failed at addr a=0x%p. Got 0x%x, expected 0x%x.\n", a, *a, before+1);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_3_1(int num_reps) {
    int is_success = 1;
    int arr[1000];
    for (int i = 0; i < num_reps; i++) {
        int id_in_arr = rand() % 1000;
        int *a = (arr + id_in_arr);
        int before = *a;
        assignment_3_1(arr, id_in_arr);
        if(*a != before+1) {
            printf("assignment_3_1 failed at addr a=0x%p and offset b=0x%x. Got 0x%x, expected 0x%x.\n", a, id_in_arr, *a, before+1);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_3_2(int num_reps) {
    int is_success = 1;
    int arr[1000];
    for (int i = 0; i < num_reps; i++) {
        int zero_index = rand() % 1000;
        for (int j = 0; j < 1000; j++)
            if (j == zero_index)
                arr[j] = 0;
            else
                while (!arr[j])
                    arr[j] = rand() % 1024;
        int got = assignment_3_2(arr);
        if(got != zero_index) {
            printf("assignment_3_2 failed at addr a=0x%p. Got 0x%x, expected 0x%x. Table was:\n", arr, got, zero_index);
            printf("[");
            for (int j = 0; j < 1000; j++)
                printf(" %x ", arr[j]);
            printf("]\n");
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_3_3(int num_reps) {
    int is_success = 1;
    int32_t my_int;
    int32_t my_int_location;

    for (int i = 0; i < num_reps; i++) {
        int is_big_endian = rand() & 1;
        my_int = rand();
        if (is_big_endian) {
            *(uint8_t*)(&my_int_location)       = (my_int & 0xFF000000) >> 24;
            *(((uint8_t*)(&my_int_location))+1) = (my_int & 0x00FF0000) >> 16;
            *(((uint8_t*)(&my_int_location))+2) = (my_int & 0x0000FF00) >> 8;
            *(((uint8_t*)(&my_int_location))+3) = (my_int & 0x000000FF);
        } else {
            *(uint8_t*)(&my_int_location)       = (my_int & 0x000000FF);
            *(((uint8_t*)(&my_int_location))+1) = (my_int & 0x0000FF00) >> 8;
            *(((uint8_t*)(&my_int_location))+2) = (my_int & 0x00FF0000) >> 16;
            *(((uint8_t*)(&my_int_location))+3) = (my_int & 0xFF000000) >> 24;
        }

        int32_t got = assignment_3_3(&my_int_location, is_big_endian);
        if(got != my_int) {
            printf("assignment_3_3 failed for a1=%d. Got 0x%x, expected 0x%x\n", is_big_endian, got, my_int);
            is_success = 0;
            break;
        }
    }
    return is_success;
}

static inline int test_assignment_4_0(int num_reps) {
    int is_success = 1;
    is_function_accessed_4_0 = 0;
    for (int i = 0; i < num_reps; i++) {
        saved_regs_t saved = save_regs();
        int saved_regs_check_dummy = check_saved_regs(saved);
        assignment_4_0(function_4_0);
        int saved_regs_check = check_saved_regs(saved);
        if (saved_regs_check ^ saved_regs_check_dummy) {
            printf("assignment_4_0 failed: tampered with a callee-saved register without restoring it.\n");
            is_success = 0;
        }
        if(!is_function_accessed_4_0) {
            printf("assignment_4_0 failed to access function at addr a=0x%p.\n", function_4_0);
            is_success = 0;
        }
    }
    return is_success;
}

static inline int test_assignment_4_1(int num_reps) {
    assignment_4_1_success = 1;
    for (int i = 0; i < num_reps; i++) {
        index_in_recursive_table = 0;
        int a = i % 30;
        // printf("Repetition id: %d with val %d.\n", i, a);
        fill_expected_recursive_call_args(a);
        recursive_call_4_1(a);
        // Check that the function has been called the right number of times.
        if (index_in_recursive_table != a+1) {
            printf("Wrong number of recursive calls.\n");
            assignment_4_1_success = 0;
            return assignment_4_1_success;
        }
    }
    return assignment_4_1_success;
}

static inline int test_assignment_5_0(int num_reps) {
    int is_success = 1;
    for (int i = 0; i < num_reps; i++) {
        // Make a trivial multiplication for the test case.
        if (i == 0) {
            M0_height = 1;
            M0_width = 1;
            M1_width = 1;
        }
        else {
            M0_height = 1+(rand() % (MAX_MATR_SIDE_5_0-1));
            M0_width  = 1+(rand() % (MAX_MATR_SIDE_5_0-1));
            M1_width  = 1+(rand() % (MAX_MATR_SIDE_5_0-1));
        }
        M1_height = M0_width; // Necessary for the matrices to be multipliable.

        // You may uncomment this to help debugging.
        // printf("Testing assignment 5_0 with sizes:\n");
        // printf("M0_height : %d\n", M0_height);
        // printf("M0_width  : %d\n", M0_width);
        // printf("M1_width  : %d\n", M1_width);

        populate_matrices_5_0();
        ref_mult_matrices_5_0();
        assignment_5_0(M0, M0_width, M0_height, M1, M1_width, M2);
        if(!compare_matrices_5_0(M2, M2_ref)) {
            printf("assignment_5_0 failed: wrong matrix multiplication result.\n");
            is_success = 0;
            return is_success;
        }
    }
    return is_success;
}


int main(int argc, char **argv) {
    srand(0);
    
    int num_reps = 100;
    int num_reps_matrmul = 50;

    // The "== 1" is an additional check that the callee-saved registers have not been tampered with.
    int success_1_0 = test_assignment_1_0(num_reps) == 1;
    printf("\nsuccess_1_0: %d\n", success_1_0);
    int success_1_1 = test_assignment_1_1(num_reps) == 1;
    printf("success_1_1: %d\n", success_1_1);
    int success_1_2 = test_assignment_1_2(num_reps) == 1;
    printf("success_1_2: %d\n", success_1_2);
    int success_1_3 = test_assignment_1_3() == 1;
    printf("success_1_3: %d\n", success_1_3);
    int success_1_4 = test_assignment_1_4(num_reps) == 1;
    printf("success_1_4: %d\n", success_1_4);
    int success_1_5 = test_assignment_1_5(num_reps) == 1;
    printf("success_1_5: %d\n", success_1_5);
    int success_1_6 = test_assignment_1_6(num_reps) == 1;
    printf("success_1_6: %d\n", success_1_6);
    int success_1_7 = test_assignment_1_7(num_reps) == 1;
    printf("success_1_7: %d\n", success_1_7);

    int success_2_0 = test_assignment_2_0(num_reps) == 1;
    printf("\nsuccess_2_0: %d\n", success_2_0);
    int success_2_1 = test_assignment_2_1(num_reps) == 1;
    printf("success_2_1: %d\n", success_2_1);

    int success_3_0 = test_assignment_3_0(num_reps) == 1;
    printf("\nsuccess_3_0: %d\n", success_3_0);
    int success_3_1 = test_assignment_3_1(num_reps) == 1;
    printf("success_3_1: %d\n", success_3_1);
    int success_3_2 = test_assignment_3_2(num_reps) == 1;
    printf("success_3_2: %d\n", success_3_2);
    int success_3_3 = test_assignment_3_3(num_reps) == 1;
    printf("success_3_3: %d\n", success_3_3);

    int success_4_0 = test_assignment_4_0(num_reps) == 1;
    printf("\nsuccess_4_0: %d\n", success_4_0);
    int success_4_1 = test_assignment_4_1(num_reps) == 1;
    printf("success_4_1: %d\n", success_4_1);

    int success_5_0 = test_assignment_5_0(num_reps_matrmul) == 1;
    printf("\nsuccess_5_0: %d\n", success_5_0);

    printf("Final execution result: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d.\n", success_1_0, success_1_1, success_1_2, success_1_3, success_1_4, success_1_5, success_1_6, success_1_7, success_2_0, success_2_1, success_3_0, success_3_1, success_3_2, success_3_3, success_4_0, success_4_1, success_5_0);

    return 0;
}
