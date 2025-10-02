# Conversion of Color Images to Grayscale Images and Brightness/Contrast Adjustment

### Basic Implementation - V1
A naive implementation without parallelization or optimizations, serving as a reference implementation to assess the accuracy and correctness of the other implementations.

### Basic Operations - V2
Includes the calculation of the square root of the variance using basic arithmetic operations.  
For this, we used the [Heron method](https://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Heron's_method).

## Optimization
### No SIMD - V3
In this variant, we replaced the calculation of pixel values with fixed-point numbers and integer arithmetic instead of floating-point arithmetic (full precision).  
With the following transformation, the sum for calculating sigma can be computed entirely with integers:
$$
\sigma^2 = \frac{1}{|D|} * \sum_{q \in Q} ( q - \mu )^2 = \frac{1}{|D|} * \sum_{q \in Q}  q^2 -2q\mu + \mu^2  \\
= \frac{1}{|D|} * (\sum_{q \in Q}  q^2 - \sum_{q \in Q} 2q\mu + \sum_{q \in Q} \mu^2)
= \frac{1}{|D|} * ((\sum_{q \in Q}  q^2 )- (2\mu*\sum_{q \in Q} q) + (|D| \mu^2))
$$

Additionally, we used a lookup table for squaring.

### SSE Floating-Point – Main Implementation
Our SSE-SIMD version also uses the above transformation. We use integer arithmetic (faster and more parallelizable), with an acceptable loss of precision.  
For contrast adjustment, however, we use floating-point arithmetic, as otherwise large inaccuracies occur.

### AVX Floating-Point - V5
Same procedure as the main implementation, but using 256-bit vectors.

### SSE Integer - V4
In V4, contrast adjustment is also carried out without floating-point numbers.  
This is only possible with a loss of accuracy, which may be tolerable in image processing. Through algebraic transformation, the latter part of the formula can be precomputed and then applied as an integer operation with the pixel values:
$$
 \frac{k}{\sigma} * q + (1-\frac{k}{\sigma}) * \mu = \frac{k}{\sigma} * q + \mu -\frac{k}{\sigma} * \mu =  \frac{k}{\sigma} * q -\frac{k}{\sigma} * \mu + \mu = \frac{k}{\sigma} * (  q - \mu + \frac{\sigma}{k} * \mu ) 
$$

## Benchmark Environment
- Architecture: x86_64  
- Processor: Intel(R) Core(TM) i7-10870H CPU @ 2.20GHz  
- Memory: 16,186,248 bytes  
- Kernel: Linux 6.1.0-22-amd64 #1 SMP PREEMPT_DYNAMIC  
- Operating System: Debian 6.1.94-1 (2024-06-21)  
- Compiler: gcc (Debian 12.2.0-14) 12.2.0  
- Compiler options: With O3 or default, see Makefile targets "benchmarking" and "benchmarking_O3"

## Results Based on Benchmarks
- SIMD yields significant performance gains, as long as one accepts some loss in precision.  
- Without SIMD, the main potential for improvement lies in avoiding floating-point arithmetic.  
- A lookup table for arithmetic base operations yields, at best, a small improvement.  

---
