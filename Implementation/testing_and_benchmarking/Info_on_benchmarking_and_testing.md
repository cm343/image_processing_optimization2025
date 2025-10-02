# Benchmarks

- The "benchmarking_main.c" is used to execute benchmarks for all implementations. It generates random input images, which are written to the folder "generated_benchmark_inputs" and then loaded from there for the benchmarking of the different Implementations.
- The execution of the "benchmark_main.c", writes the benchmark data to "benchmark_results/development".
- The execution is possible by the Makefile-Target "benchmark" or "benchmark_O3"
- The results which are used in the presentation slides, are accessible in "benchmark_results/presentation", as well as the generated diagrams used in the presentation.

# Testing

- The "testing_main.c" is used to test implementations. It generates random input images of different types, which are written to the folder "generated_test_inputs" and then loaded from there for the testing of the different Implementations. 
  In addition to that it tests with some images from "coco_dataset_val_2017_ppms", which contains some images of the "val 2017"-part of the [coco-dataset](https://cocodataset.org/). 
- The execution is possible by the Makefile-Target "testing"
- The results are printed to command line

# Deviation Computing
- For better determination of the precision of our different SIMD Versions, we decided to compute distributions of the deviation between the base implementations
- This is performed on the images in "coco_dataset_val_2017_ppms" which are from the "val 2017"-part of the [coco-dataset](https://cocodataset.org/) and listed in "dataset_images_paths.txt". Each image is tested with 100 random samples for the other parameters (a,b,c,brightness,contrast)
- The distribution data as well as the corresponding diagrams used in the presentation slides, are accessible in "deviation_distribution_results/presentation"
- The execution of "deviation_distribution_main.c" writes it's results to "deviation_distribution_results/development"

# Data Analysis
- The directory "data_analysis" contains python scripts, which were used to visualize the benchmark and deviation results, as well as converting images from the [coco-dataset](https://cocodataset.org/) to .ppm format.