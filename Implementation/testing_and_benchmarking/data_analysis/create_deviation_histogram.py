import matplotlib.pyplot as plt

# script to generate histograms of deviation between implementations


base = "../deviation_distribution_results/"
in_files = {
    "presentation/dev_base_vs_integer_sse.csv" : {"color": 'b', 'label':"Kontrastberechnung mit Integern", "alpha":0.7},
    "presentation/dev_base_vs_floating_sse.csv" : {"color": 'r', 'label':"Kontrastberechnung mit Floating Point", "alpha":0.7},
            }

data_percent = {}

# metric for the overall deviation of an implementation: in which interval around zero deviation are e.g. 97% of the pixels
percent = 0.97

plt.figure(figsize=(16/1.4, 9/1.4))

for in_file in in_files.keys():

    count =[]
    deviation = []
    with open(base+in_file) as file:
        
        sum = int(file.readline())
        for line in file:
            d, c = line.split(',')
            count.append(float((c.strip()))/sum)
            deviation.append(float((d.strip())))
        percentage = count[255]
        r = 0
        while (percentage <= percent) :
            r += 1
            percentage +=  count[255-r]+ count[255+r]
            
        
        data_percent[in_file] = r
        
            
    plt.bar(deviation, count,color=in_files[in_file]['color'], label=in_files[in_file]['label'], alpha = in_files[in_file]["alpha"], width=1)


print(data_percent)







plt.title('Pixelverteilung bezüglich Abweichung')
plt.xlabel('Abweichung')
plt.ylabel('Anteil')
plt.legend()

plt.show()
        
