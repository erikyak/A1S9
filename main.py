import pandas as pd
import matplotlib.pyplot as plt
import os

files = {
    'randomSorted': 'randomSorted.csv',
    'nearlySorted': 'nearlySorted.csv',
    'reverseSorted': 'reverseSorted.csv'
}

data = {}
for name, file in files.items():
    if os.path.exists(file):
        data[name] = pd.read_csv(file)
    else:
        print(f"Файл {file} не найден!")

plt.style.use('seaborn-v0_8')
colors = {
    'stdQuickSort': 'green',
    'stdMergeSort': 'brown',
    'strQuickSort': 'orange',
    'strMergeSort': 'red',
    'radixSort': 'blue',
    'radixQuickSort': 'purple'
}

for dataset in data.keys():
    plt.figure(figsize=(12, 6))
    df = data[dataset]

    for alg in df['Alg'].unique():
        subset = df[df['Alg'] == alg]
        plt.plot(subset['Size'], subset['Time(ms)'],
                 label=alg, color=colors.get(alg, 'gray'))

    plt.title(f'Время выполнения алгоритмов ({dataset})')
    plt.xlabel('Размер массива')
    plt.ylabel('Время (мс)')
    plt.legend()
    plt.grid(True)
    plt.show()

for dataset in data.keys():
    plt.figure(figsize=(12, 6))
    df = data[dataset]

    for alg in df['Alg'].unique():
        subset = df[df['Alg'] == alg]
        plt.plot(subset['Size'], subset['Comparisons'],
                 label=alg, color=colors.get(alg, 'gray'))

    plt.title(f'Количество сравнений ({dataset})')
    plt.xlabel('Размер массива')
    plt.ylabel('Сравнения')
    plt.legend()
    plt.grid(True)
    plt.show()