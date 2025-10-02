import matplotlib.pyplot as plt

# script to generate different benchmark plots using matplotlib, configure which plot to generate by setting the plot variable

base = "/home/cl/shared_partition/Cross_Platform_Work/Uni/GRA_Project/Implementierung/testing_and_benchmarking/benchmark_results/presentation/"

plot =5

plt.figure(figsize=(16/1.4, 9/1.4))

if plot ==1:

    plt.title("Optimierung ohne SIMD - Optimierungsstufe Default vs. O3")
    in_files = {
        #Default
        "bench_run_clemens_v3_no_opt_no_debug/bench_basic_implementation.csv" : {"color": '#2ca02c', 'label':"Basisimplementierung"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_floating_point_no_table_just_maths.csv" : {"color": '#d62728', 'label':"Umformung"},

         # O3
        "bench_run_v3_O3_no_debug/bench_basic_implementation.csv" : {"color": '#2ca02c', 'label':"Basisimplementierung, O3"},
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized_floating_point_no_table_just_maths.csv" : {"color": '#d62728', 'label':"Umformung, O3"},

        #Default
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Umformung, Festkommarechnung mit Rundung"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_no_rounding_no_table.csv" : {"color": '#8c564b', 'label':"Umformung, Festkommarechnung ohne Rundung"},

        # O3
       "bench_run_v3_O3_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Mathe, Festkommarechnung mit Rundung, O3"},
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized_no_rounding_no_table.csv" : {"color": '#8c564b', 'label':"Umformung, Festkommarechnung ohne Rundung, O3"},

        }
elif plot == 2:
    plt.title("Optimierung ohne SIMD - Quadrat-Tabelle vs. arithmetische Berechnung auf Default vs. O3")
    in_files = {
        #Default
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_floating_point.csv" : {"color": '#9467bd', 'label':"Umformung, Quadrat-Tabelle"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_floating_point_no_table_just_maths.csv" : {"color": '#d62728', 'label':"Umformung"},

        # O3
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized_floating_point_no_table_just_maths.csv" : {"color": '#d62728', 'label':"Umformung, O3"},
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized_floating_point.csv" : {"color": '#9467bd', 'label':"Umformung, Quadrat-Tabelle, O3"},

        # Default
         "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Umformung, Festkommarechnung mit Rundung"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized.csv" : {"color": '#e377c2', 'label':"Umformung, Festkommarechnung mit Rundung, Quadrat-Tabelle"},

        # O3
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Umformung, Festkommarechnung mit Rundung, O3"},
        "bench_run_v3_O3_no_debug/bench_non_simd_optimized.csv" : {"color": '#e377c2', 'label':"Umformung, Festkommarechnung mit Rundung, Quadrat-Tabelle, O3"},

        }
elif plot ==3:
    plt.title("SIMD - Optimierungsstufe Default vs. O3")
    in_files = {
        #Default
        "bench_run_clemens_v3_no_opt_no_debug/bench_sse_instruction_optimized_float.csv" : {"color": '#7f7f7f', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Floats)"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_sse_instruction_optimized_integer.csv" : {"color": '#bcbd22', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Integern)"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_avx_instruction.csv" : {"color": '#1f77b4', 'label':"Umformung, AVX Erweiterung (Kontrastbestimmung mit floats)"},

        # O3
        "bench_run_v3_O3_no_debug/bench_sse_instruction_optimized_float.csv" : {"color": '#7f7f7f', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Floats), O3"},
        "bench_run_v3_O3_no_debug/bench_sse_instruction_optimized_integer.csv" : {"color": '#bcbd22', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Integern), O3"},
        "bench_run_v3_O3_no_debug/bench_avx_instruction.csv" : {"color": '#1f77b4', 'label':"Umformung, AVX Erweiterung (Kontrastbestimmung mit floats), O3"},

    }

elif plot == 4:
    plt.title("Übersicht - Optimierung Default")
    in_files = {
        #Default
        "bench_run_clemens_v3_no_opt_no_debug/bench_basic_implementation_with_basic_instructions.csv" : {"color": '#ff7f0e', 'label':"Basisoperationen"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_basic_implementation.csv" : {"color": '#2ca02c', 'label':"Basisimplementierung"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Umformung, Festkommarechnung mit Rundung"},

        "bench_run_clemens_v3_no_opt_no_debug/bench_sse_instruction_optimized_float.csv" : {"color": '#7f7f7f', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Floats)"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_sse_instruction_optimized_integer.csv" : {"color": '#bcbd22', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Integern)"},
        "bench_run_clemens_v3_no_opt_no_debug/bench_avx_instruction.csv" : {"color": '#1f77b4', 'label':"Umformung, AVX Erweiterung (Kontrastbestimmung mit floats)"},
        }

elif plot == 5:
    plt.title("Übersicht - Optimierung O3")
    in_files = {
        # O3
         "bench_run_v3_O3_no_debug/bench_basic_implementation_with_basic_instructions.csv" : {"color": '#ff7f0e', 'label':"Basisoperationen, O3"},
        "bench_run_v3_O3_no_debug/bench_basic_implementation.csv" : {"color": '#2ca02c', 'label':"Basisimplementierung, O3"},
         "bench_run_v3_O3_no_debug/bench_non_simd_optimized_no_table.csv" : {"color": '#17becf', 'label':"Umformung, Festkommarechnung mit Rundung, O3"},
        "bench_run_v3_O3_no_debug/bench_sse_instruction_optimized_float.csv" : {"color": '#7f7f7f', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Floats), O3"},
        "bench_run_v3_O3_no_debug/bench_sse_instruction_optimized_integer.csv" : {"color": '#bcbd22', 'label':"Umformung, SSE Erweiterung (Kontrastbestimmung mit Integern), O3"},
        "bench_run_v3_O3_no_debug/bench_avx_instruction.csv" : {"color": '#1f77b4', 'label':"Umformung, AVX Erweiterung (Kontrastbestimmung mit Floats), O3"},

     }






for in_file in in_files.keys():
    pixel_count = []
    execution_time = []
    iterations = []
    total_time = []
    with open(base+in_file) as file:
      
        for line in file:
            p, e, i, t = line.split(',')
            pixel_count.append(float((p.strip())))
            execution_time.append(float((e.strip())))
            iterations.append(int((i.strip())))
            total_time.append(float((t.strip())))
            
    plt.plot(pixel_count, execution_time, marker='s', linestyle='-', color=in_files[in_file]['color'], label=in_files[in_file]['label'])

        
        




plt.xlabel('Bildgröße in Pixel')
plt.ylabel('Laufzeit in ms')
plt.grid(True)
plt.legend()

plt.show()
