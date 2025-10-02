
# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# ---Assignment Configuration---

ASSIGNMENT_MAIN := main.c # Name of the file containing the Rahmenprogramm

ASSIGNMENT_TARGET_NAME := imageprocessing

ASSIGNMENT_IMPLEMENTATION_SOURCES := process_ppm.c common.c benchmark.c basic_implementation.c basic_implementation_with_basic_instructions.c avx_instruction.c sse_instruction_optimized_integer.c sse_instruction_optimized_float.c non_simd_optimized.c # list of sources for the assignement build (implementations etc.)

ASSIGNMENT_CFLAGS := -Wall -lm -O3 -msse2 -msse3 -march=native # Compiler Flags for the Assignment Build


# ---Assignement Configuration End---


# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------


# Determine if clang or gcc is available

# executes the shell command "command -v gcc", which returns the path to the gcc command (compiler) or nothing if the command gcc is not found. Assigns this return to the variable cc
CC := $(shell command -v gcc)

# strips removes leading or trailing whitespaces, ifeq comparison of the result of strip with the empty string
ifeq ($(strip $(CC)),)
    $(error gcc is not available. Exiting.) # error if gcc is not found
endif




# Combine Sources
ASSIGNMENT_SOURCES := $(ASSIGNMENT_MAIN) $(ASSIGNMENT_IMPLEMENTATION_SOURCES)




# ---------------------------------------
# TARGETS
# ---------------------------------------


# ----- Assignment -----

# Build target for the final assignment. Has to be the first target specified, to be the default build target
$(ASSIGNMENT_TARGET_NAME): $(ASSIGNMENT_SOURCES)
	$(CC) -o $@ $^ $(ASSIGNMENT_CFLAGS)


ASSIGNMENT_RUNNING_FLAGS = -V0 -B5 --coeffs 1,2,3 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/stop_1.ppm

#sanatize: $(ASSIGNMENT_SOURCES)
#	clang -o $@ $^ $(ASSIGNMENT_CFLAGS)

ADDITIONAL_TARGET_DEPENDENCIES := process_ppm.c
ADDITIONAL_TARGETS_IMPLEMENTATIONS := basic_implementation.c basic_implementation_with_basic_instructions.c sse_instruction_optimized_float.c sse_instruction_optimized_integer.c non_simd_optimized.c avx_instruction.c


# ----- Only implementations -----

# benchmark target with default optimization (testing_and_benchmarking/benchmarking_main.c)
benchmark: testing_and_benchmarking/benchmarking_main.c testing_and_benchmarking/testing.c testing_and_benchmarking/benchmark.c testing_and_benchmarking/io_benchmark.c  $(ADDITIONAL_TARGET_DEPENDENCIES) $(ADDITIONAL_TARGETS_IMPLEMENTATIONS)
	$(CC) -o $@ $^ -msse2 -msse3 -march=native -lm -Wextra -Wall

#benchmark with option -O3 (testing_and_benchmarking/benchmarking_main.c)
benchmark_O3: testing_and_benchmarking/benchmarking_main.c testing_and_benchmarking/testing.c testing_and_benchmarking/benchmark.c testing_and_benchmarking/io_benchmark.c  $(ADDITIONAL_TARGET_DEPENDENCIES) $(ADDITIONAL_TARGETS_IMPLEMENTATIONS)
	$(CC) -o $@ $^ -msse2 -msse3 -march=native -lm -O3 -Wextra -Wall

# target to run automated implementation tests (testing_and_benchmarking/testing_main.c)
testing:  testing_and_benchmarking/testing_main.c testing_and_benchmarking/testing.c $(ADDITIONAL_TARGET_DEPENDENCIES) $(ADDITIONAL_TARGETS_IMPLEMENTATIONS)
	$(CC) -o $@ $^   -msse2 -msse3 -march=native -lm -g -Wextra -Wall

# target to compute deviation ( testing_and_benchmarking/deviation_distribution_main.c)
compute_deviation: testing_and_benchmarking/deviation_distribution_main.c testing_and_benchmarking/testing.c $(ADDITIONAL_TARGET_DEPENDENCIES) $(ADDITIONAL_TARGETS_IMPLEMENTATIONS)
	$(CC) -o $@ $^   -msse2 -msse3 -march=native -g -lm -Wextra -Wall



# ----- Testing with main.c -----

TESTS = test_basic \
		test_double_1_1 \
		test_double_2_2 \
		test_double_10_10 \
        test_comments \
        test_decimal_dimensions \
        test_large_max_val \
        test_negative_values \
        test_empty_file \
        test_unusual_filenames \
        test_file_overwrite \
        test_decimal_height \
        test_letters_in_dimensions \
        test_max_val_null \
        test_missing_width \
        test_negative_and_third_dimension \
        test_width_height_issues


test_basic:
	@echo "\n\n################### Running test: Basic Test ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 1,2,3 -o outputfile.jpg --brightness 10 --contrast 10 ./test_imgs/test_1_1.ppm

test_double_1_1:
	@echo "\n\n################### Running test: Double Image 1x1 Pixel ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 1,2,3 -o outputfile.jpg --brightness 10 --contrast 10 ./test_imgs/double_1_1.ppm

test_double_2_2:
	@echo "\n\n################### Running test: Double Image 2x2 Pixel ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 1,2,3 -o outputfile.jpg --brightness 10 --contrast 10 ./test_imgs/double_2_2.ppm

test_double_10_10:
	@echo "\n\n################### Running test: Double Image 10x10 Pixel ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 1,2,3 -o outputfile.jpg --brightness 10 --contrast 10 ./test_imgs/double_10_10.ppm

test_comments:
	@echo "\n\n################### Running test: Images with Comments ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_comments2.ppm

test_decimal_dimensions:
	@echo "\n\n################### Running test: Decimal Dimensions ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_decimal_width.ppm

test_large_max_val:
	@echo "\n\n################### Running test: Large Maximum Value ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_giant_max_val.ppm

test_negative_values:
	@echo "\n\n################### Running test: Negative Values ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_negative_height.ppm

test_empty_file:
	@echo "\n\n################### Running test: Empty File ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_empty.ppm

test_unusual_filenames:
	@echo "\n\n################### Running test: Unusual Filenames ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_null_width.ppm

test_file_overwrite:
	@echo "\n\n################### Running test: File Overwrite ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_working.ppm

test_decimal_height:
	@echo "\n\n################### Running test: Decimal Height ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_decimal_height.ppm

test_letters_in_dimensions:
	@echo "\n\n################### Running test: Letters in Dimensions ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_letters_in_width.ppm
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_letters_in_width2.ppm

test_max_val_null:
	@echo "\n\n################### Running test: Max Val Null ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_max_val_null.ppm

test_missing_width:
	@echo "\n\n################### Running test: Missing Width ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_null_width.ppm

test_negative_and_third_dimension:
	@echo "\n\n################### Running test: Negative and Third Dimension ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_negative_width.ppm
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_third_val_in_header.ppm

test_width_height_issues:
	@echo "\n\n################### Running test: Width and Height Issues ###################"
	-valgrind ./$(ASSIGNMENT_TARGET_NAME) -V4 -B5 --coeffs 3.4,3.3,2.0 -o outputfile.pgm --brightness 10 --contrast 10 ./test_imgs/test_100_100_width_height_null.ppm


# Make command for running
run:
	./$(ASSIGNMENT_TARGET_NAME) $(ASSIGNMENT_RUNNING_FLAGS)

tests: $(TESTS)


clean_tests:
	rm ./outputfile.pgm $(ASSIGNMENT_TARGET_NAME)

# clean up the assignment
clean:
	rm $(ASSIGNMENT_TARGET_NAME)

.PHONY: $(ASSIGNMENT_TARGET_NAME) clean