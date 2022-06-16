import matplotlib.pyplot as plt
import numpy as np
import sys

datasets = ['chicago', 'ecommerce', 'foodmart', 'fruithut', 'mushroom', 'random_1', 'random_2']

for dataset in datasets:
	with open('dataset_times\\' + dataset + '_times.txt', 'r') as sys.stdin:
		input()

		k = [[], [], []]
		for i in range(3):
			k[i] = list(map(int, input().split()))

		def to_seconds(x):
			return int(x)/1000;

		input()

		pq = [[], [], []]
		for i in range(3):
			pq[i] = list(map(to_seconds, input().split()))

		input()

		bs = [[], [], []]
		for i in range(3):
			bs[i] = list(map(to_seconds, input().split()))

		names = ['small_k', 'medium_k', 'large_k']

		def plot_stuff (k, pq, bs, save_loc, logarithmic):
			plt.figure()
			plt.title(dataset)
			plt.xlabel("Number of top HUIs mined")
			plt.ylabel("Run time (in seconds)")
			if logarithmic:
				plt.xscale('log')
			plt.plot(k, pq, 'o-', label='PQ', color='red')
			plt.plot(k, bs, 'o-', label='BS', color='blue')
			plt.grid()
			plt.legend()
			plt.savefig(save_loc)

		for i in range(3):
			plot_stuff(k[i], pq[i], bs[i], names[i] + '\\' + dataset + '_' + names[i], False)
			
		all_k = k[0]+k[1]+k[2]
		all_pq = pq[0]+pq[1]+pq[2]
		all_bs = bs[0]+bs[1]+bs[2]

		plot_stuff(all_k, all_pq, all_bs, dataset + '_all', True)