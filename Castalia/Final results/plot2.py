import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import random
import pandas as pd
from scipy import stats

if False:

	labels = ['Hole 1', 'Hole 2', 'Hole 3']
	mlp = []
	ich = []
	stable = []
	gpsr = []
	rainbow = []

	mlp_err = []
	ich_err = []
	stable_err = []
	gpsr_err = []
	rainbow_err = []

	d = {'mlp': [78.03445221,85.52957498,90.02021243,87.02512711,88.75568622], 'ich': [78.02095722,76.76146507,78.5260843,78.01597644,82.04177636], 'stable': [66.78320116,67.03692602,67.25447619,69.00839829,73.50304019], 'gpsr': [38.51278665,38.28747951,38.00758619,38.01548572,38.2947899], 'rainbow': [65.29559689,72.78681532,74.02583634,78.00824132,63.53540265]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [94.26558638,92.53293258,96.00327385,93.00327385,93.00327385], 'ich': [79.78978282,79.29687412,79.7797044,82.01149442,80.51948811], 'stable': [66.00237273,67.00233785,71.02630415,66.02400171,71.02630415], 'gpsr': [32.7738338,32.77931644,32.77184382,32.78207316,32.77399158], 'rainbow': [52.26717676,55.50758762,52.5012143,54.54784049,54.54784049]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [98.28979524,98.03469056,112.0121538,115.3008537,101.2700464], 'ich': [85.5146498,85.77706643,85.76280786,86.05058483,86.24810083], 'stable': [61.79580579,61.788002,61.79049459,61.51825383,59.29043719], 'gpsr': [38.5327485,38.53667749,38.52732667,38.54537566,38.54537566], 'rainbow': [83.51722587,86.02234728,80.52270892,82.75781638,83.49818797]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);




	x = np.arange(len(labels))  # the label locations
	width = 0.7/5  # the width of the bars

	fig, axs = plt.subplots(1, 3, gridspec_kw={'wspace': 0.5})
	rects1 = axs[0].bar(x - width*2, mlp, width, yerr = mlp_err, label='k-MLP', color='w', hatch='///', edgecolor='xkcd:azure', ecolor='xkcd:azure', capsize=2)
	rects2 = axs[0].bar(x - width, ich, width, yerr = ich_err, label='ADAV', color='w', hatch='xxx', edgecolor='xkcd:orange', ecolor='xkcd:orange', capsize=2)
	rects3 = axs[0].bar(x, stable, width, yerr = stable_err, label='STABLE', color='w', hatch='***', edgecolor='xkcd:gray', ecolor='xkcd:gray', capsize=2)
	rects4 = axs[0].bar(x + width, gpsr, width, yerr = gpsr_err, label='GPSR', color='w', hatch='\\\\\\', edgecolor='xkcd:gold', ecolor='xkcd:gold', capsize=2)
	rects5 = axs[0].bar(x + width*2, rainbow, width, yerr = rainbow_err, label='ALBA-R', color='w', hatch='xxxxxxx', edgecolor='xkcd:blue', ecolor='xkcd:blue', capsize=2)

	# # Add some text for labels, title and custom x-axis tick labels, etc.
	axs[0].set_ylabel('Network lifetime (seconds)', fontsize='x-large')
	# ax.set_title('Scores by group and gender')
	axs[0].set_xticks(x)
	axs[0].set_xticklabels(labels, fontsize='x-large')
	axs[0].tick_params(labelsize='x-large')
	axs[0].set_title('(a) Network lifetime', fontsize='x-large', y=-0.4)

	
	labels = ['Hole 1', 'Hole 2', 'Hole 3']
	mlp = []
	ich = []
	stable = []
	gpsr = []
	rainbow = []

	mlp_err = []
	ich_err = []
	stable_err = []
	gpsr_err = []
	rainbow_err = []

	labels = ['Hole 1', 'Hole 2', 'Hole 3']
	mlp = []
	ich = []
	stable = []
	gpsr = []
	rainbow = []

	mlp_err = []
	ich_err = []
	stable_err = []
	gpsr_err = []
	rainbow_err = []

	d = {'mlp': [0.134808131,0.129876181,0.12149231,0.135037143,0.132420732], 'ich': [0.118754929,0.111829582,0.112386207,0.122976858,0.119426414], 'stable': [0.110039736,0.116426775,0.103876178,0.117195003,0.111440369], 'gpsr': [0.016659985,0.01836531,0.016791144,0.019495446,0.019734975], 'rainbow': [0.09941707,0.109295724,0.085648521,0.094356768,0.089524757]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [0.130236884,0.137670538,0.135705432,0.133595428], 'ich': [0.105817824,0.112754831,0.105671003,0.106177629], 'stable': [0.111002581,0.112765308,0.113306167,0.110368645], 'gpsr': [0.036325702,0.036141899,0.036397489,0.035659997], 'rainbow': [0.139157639,0.131660016,0.125269935,0.115970916]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [0.190813339,0.209414394,0.20310617,0.211863756], 'ich': [0.132894168,0.127345702,0.128267172,0.146056983], 'stable': [0.153812104,0.155933826,0.149485608,0.158209345], 'gpsr': [0.040652479,0.038079392,0.038051333,0.040151313], 'rainbow': [0.385633613,0.364419218,0.459613858,0.45806644]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	rects1 = axs[1].bar(x - width*2, mlp, width, yerr = mlp_err, label='k-MLP', color='w', hatch='///', edgecolor='xkcd:azure', ecolor='xkcd:azure', capsize=2)
	rects2 = axs[1].bar(x - width, ich, width, yerr = ich_err, label='ADAV', color='w', hatch='xxx', edgecolor='xkcd:orange', ecolor='xkcd:orange', capsize=2)
	rects3 = axs[1].bar(x, stable, width, yerr = stable_err, label='STABLE', color='w', hatch='***', edgecolor='xkcd:gray', ecolor='xkcd:gray', capsize=2)
	rects4 = axs[1].bar(x + width, gpsr, width, yerr = gpsr_err, label='GPSR', color='w', hatch='\\\\\\', edgecolor='xkcd:gold', ecolor='xkcd:gold', capsize=2)
	rects5 = axs[1].bar(x + width*2, rainbow, width, yerr = rainbow_err, label='ALBA-R', color='w', hatch='xxxxxxx', edgecolor='xkcd:blue', ecolor='xkcd:blue', capsize=2)

	# # Add some text for labels, title and custom x-axis tick labels, etc.
	axs[1].set_ylabel('Balancing index', fontsize='x-large')
	# ax.set_title('Scores by group and gender')
	axs[1].set_xticks(x)
	axs[1].set_xticklabels(labels, fontsize='x-large')
	axs[1].tick_params(labelsize='x-large')
	axs[1].set_title('(b) Balancing index', fontsize='x-large', y=-0.4)




	labels = ['Hole 1', 'Hole 2', 'Hole 3']
	mlp = []
	ich = []
	stable = []
	gpsr = []
	rainbow = []

	mlp_err = []
	ich_err = []
	stable_err = []
	gpsr_err = []
	rainbow_err = []

	d = {'mlp': [1.260718333,1.211402825,1.200352778,1.223506111,1.21035], 'ich': [1.267421111,1.231433333,1.207031667,1.237499444,1.228227778], 'stable': [1.218431667,1.219482222,1.181678333,1.242157778,1.186976667], 'gpsr': [2.098661111,2.341039444,2.152300556,2.427115556,2.597770556], 'rainbow': [2.095249861,1.871,1.827063333,1.751965,1.838577222]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [1.207713889,1.209949861,1.213253889,1.212708889], 'ich': [1.222856111,1.280482778,1.257597446,1.259163889], 'stable': [1.192952778,1.191328151,1.19593,1.18861111], 'gpsr': [4.756460556,4.536757357,4.716636667,4.628001111], 'rainbow': [2.189554444,2.223205441,2.119077222,2.105562778]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	d = {'mlp': [1.233213095,1.205827318,1.19921,1.207728333], 'ich': [1.217491071,1.216501388,1.210852222,1.217188333], 'stable': [1.231531548,1.168313715,1.162326111,1.173451667], 'gpsr': [4.984975,4.595393115,4.651400556,4.808855], 'rainbow': [4.346508333,4.202776235,4.174837778,4.427270556]}
	df = pd.DataFrame(data=d).T
	mean = df.mean(axis = 1)
	mlp.append(mean['mlp']);
	ich.append(mean['ich']);
	stable.append(mean['stable']);
	gpsr.append(mean['gpsr']);
	rainbow.append(mean['rainbow']);
	std = df.std(axis = 1)
	n= df.shape[1]
	yerr = std / np.sqrt(n) * stats.t.ppf(1-0.05/2, n - 1)
	mlp_err.append(yerr[0]);
	ich_err.append(yerr[1]);
	stable_err.append(yerr[2]);
	gpsr_err.append(yerr[3]);
	rainbow_err.append(yerr[4]);

	rects1 = axs[2].bar(x - width*2, mlp, width, yerr = mlp_err, label='k-MLP', color='w', hatch='///', edgecolor='xkcd:azure', ecolor='xkcd:azure', capsize=2)
	rects2 = axs[2].bar(x - width, ich, width, yerr = ich_err, label='ADAV', color='w', hatch='xxx', edgecolor='xkcd:orange', ecolor='xkcd:orange', capsize=2)
	rects3 = axs[2].bar(x, stable, width, yerr = stable_err, label='STABLE', color='w', hatch='***', edgecolor='xkcd:gray', ecolor='xkcd:gray', capsize=2)
	rects4 = axs[2].bar(x + width, gpsr, width, yerr = gpsr_err, label='GPSR', color='w', hatch='\\\\\\', edgecolor='xkcd:gold', ecolor='xkcd:gold', capsize=2)
	rects5 = axs[2].bar(x + width*2, rainbow, width, yerr = rainbow_err, label='ALBA-R', color='w', hatch='xxxxxxx', edgecolor='xkcd:blue', ecolor='xkcd:blue', capsize=2)

	# # Add some text for labels, title and custom x-axis tick labels, etc.
	axs[2].set_ylabel('Average stretch', fontsize='x-large')
	# ax.set_title('Scores by group and gender')
	axs[2].set_xticks(x)
	axs[2].set_xticklabels(labels, fontsize='x-large')
	axs[2].tick_params(labelsize='x-large')
	axs[2].set_title('(c) Average stretch', fontsize='x-large', y=-0.4)
	
	handles, labels = axs[0].get_legend_handles_labels()
	
	fig.tight_layout()
	order = [0,1,2,3,4]
	fig.legend([handles[idx] for idx in order],[labels[idx] for idx in order], ncol=5, loc='lower center', shadow=True, fontsize='x-large')
	plt.show()





if False:
	isBound = 1
	xlabel = 'Number of communication pairs'
	ylabel1 = 'Network lifetime (seconds)'
	ylabel2 = 'Balncing index'
	ylabel3 = 'Average stretch'
	
	label = ["k-MLP", "ADAV", "STABLE", "GPSR", "ALBA-R"]
	linestyles = ['o-.', 'x--', 'v:', 's--', 'd--']
	linewidth = 1.5
	markersize = 7

	fig, axs = plt.subplots(1, 3, gridspec_kw={'wspace': 0.5})

	axs[0].grid(linestyle=':', linewidth=1.5)
	axs[0].set_xticks(range(0,31,5))
	axs[1].grid(linestyle=':', linewidth=1.5)
	axs[1].set_xticks(range(0,31,5))
	axs[2].grid(linestyle=':', linewidth=1.5)
	axs[2].set_xticks(range(0,31,5))
 
	axs[0].set_xlabel(xlabel, fontsize='x-large')
	axs[0].set_ylabel(ylabel1, fontsize='x-large')
	axs[1].set_xlabel(xlabel, fontsize='x-large')
	axs[1].set_ylabel(ylabel2, fontsize='x-large')
	axs[2].set_xlabel(xlabel, fontsize='x-large')
	axs[2].set_ylabel(ylabel3, fontsize='x-large')

	axs[0].tick_params(labelsize='x-large')
	axs[1].tick_params(labelsize='x-large')
	axs[2].tick_params(labelsize='x-large')
	axs[0].set_title('(a) Network lifetime', fontsize='x-large', y=-0.5)
	axs[1].set_title('(b) Balancing index', fontsize='x-large', y=-0.5)
	axs[2].set_title('(c) Average stretch', fontsize='x-large', y=-0.5)


	X = [10,15,20,25,30]
	Y1 = [119.798873550946,101.755558783474,88.015760007356,77.527063446012,69.752329480556]
	Y2 = [0.16219112384697112,0.14087846537074228,0.1236078351239271,0.11716120031490705,0.11374884084501495]
	Y3 = [1.136455,1.13302,1.13524,1.13501,1.13367]
	axs[0].plot(X, Y1,  linestyles[0], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[1].plot(X, Y2,  linestyles[0], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[2].plot(X, Y3,  linestyles[0], label=label[0], linewidth=linewidth, markersize=markersize)
	Y1 = [73.018378697555,57.525439275294,49.265309972738,44.009620533354,40.511544500874]
	Y2 = [0.14413404477709985,0.11617447575315441,0.0996951716250116,0.0908479654561691,0.08405810088567829]
	Y3 = [1.12841,1.14584,1.12887,1.1394,1.13587]
	axs[0].plot(X, Y1,  linestyles[1], label=label[1], linewidth=linewidth, markersize=markersize)
	axs[1].plot(X, Y2,  linestyles[1], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[2].plot(X, Y3,  linestyles[1], label=label[0], linewidth=linewidth, markersize=markersize)
	Y1 = [65.2864190586,52.252036087126,45.016315224332,40.291344650193,37.257626740976]
	Y2 = [0.07649881208606478,0.06377302567477436,0.05552986256334151,0.05083055729587526,0.04810483894291723]
	Y3 = [1.09482,1.08877,1.09078,1.08949,1.08904]
	axs[0].plot(X, Y1,  linestyles[2], label=label[2], linewidth=linewidth, markersize=markersize)
	axs[1].plot(X, Y2,  linestyles[2], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[2].plot(X, Y3,  linestyles[2], label=label[0], linewidth=linewidth, markersize=markersize)
	Y1 = [46.513524445819,38.261627607982,34.009237312185,31.266765804281,29.502124965637]
	Y2 = [0.06446618865347192,0.06491302526310662,0.06499672746206488,0.06489728369824561,0.06482597820844381]
	Y3 = [4.68209,4.48669,4.65324,4.55121,4.57454]
	axs[0].plot(X, Y1,  linestyles[3], label=label[3], linewidth=linewidth, markersize=markersize)
	axs[1].plot(X, Y2,  linestyles[3], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[2].plot(X, Y3,  linestyles[3], label=label[0], linewidth=linewidth, markersize=markersize)
	Y1 = [66.014653132744,53.03234405079,45.777619133579,41.028998210514,37.77950515729]
	Y2 = [0.3419763437344604,0.34215855340988327,0.32300699557574153,0.32443937087642466,0.3197256121155062]
	Y3 = [2.69346,3.07618,3.21639,3.50642,2.78643]
	axs[0].plot(X, Y1,  linestyles[4], label=label[4], linewidth=linewidth, markersize=markersize)
	axs[1].plot(X, Y2,  linestyles[4], label=label[0], linewidth=linewidth, markersize=markersize)
	axs[2].plot(X, Y3,  linestyles[4], label=label[0], linewidth=linewidth, markersize=markersize)

	handles, labels = axs[0].get_legend_handles_labels()
	
	fig.tight_layout()
	order = [0,1,2,3,4]
	fig.legend([handles[idx] for idx in order],[labels[idx] for idx in order], ncol=5, loc='lower center', shadow=True, fontsize='x-large')
	plt.show()



if True:
	fig, ax = plt.subplots(1)

	xlabel = 'The death sequence of nodes'
	ylabel = 'Time until the death of a node (seconds)'
	
	dashList = [(1, 0), (3, 1, 1, 1), (1, 2), (4, 2, 1, 2, 1, 2), (3, 2)]
	linewidth = 2
	label = ["k-MLP", "ADAV", "STABLE", "GPSR", "ALBA-R"]

	# plt.grid(linestyle=':', linewidth=1.5)
	# plt.xticks(range(0,81,20))
	# plt.yticks(range(0,81,20))
	# plt.xlim(xmin=0, xmax=80)
	# plt.ylim(ymin=0, ymax=80)
	ax.tick_params(labelsize='xx-large')
	ax.set_xlabel(xlabel, fontsize='xx-large')
	ax.set_ylabel(ylabel, fontsize='xx-large')
	# plt.title('(b) $N = 104976$', fontsize='xxx-large')
	# plt.gca().set_aspect('equal', adjustable='box')

	N = 50
	X = range(1,N+1)
	Y = []
	
	Y1 = [69.752329480556,69.752329480556,70.273114839694,70.273114839694,70.534208575877,70.534208575877,71.288094969446,71.288094969446,71.506142267415,71.506142267415,71.518979741153,71.518979741153,73.011297738529,73.288024523726,73.511391167351,73.534092288245,73.534092288245,74.011334699257,74.011334699257,75.534014763157,75.534014763157,77.011394880361,77.011394880361,77.030558864411,77.273154846514,77.522141828397,77.522141828397,78.254336481732,78.261491782351,78.523652383484,78.537855344497,78.537855344497,78.749328206757,78.749328206757,79.006159921335,79.021198864451,79.021198864451,79.037821992281,79.263037508902,79.768505630351,79.768505630351,79.769656861902,80.002381561995,80.005754864517,80.006690864513,80.007158864511,80.014646864479,80.020262864455,80.021666864449,80.037786769421]
	Y2 = [40.511544500874,44.772969100564,44.772969100564,46.05259445483 ,46.509831505711,46.54260302362 ,47.048382454848,48.7697204773  ,48.774402922991,48.774402922991,50.782311415229,51.026379149857,51.027473276209,51.027473276209,52.521181994481,54.049318454844,54.055870454816,54.295185455128,54.295185455128,54.527289052043,54.527289052043,55.049318454844,57.024410232557,57.507550703603,59.050722454838,59.260454198291,59.260454198291,60.053998454824,60.054466454822,61.050722454838,62.049786454842,62.057742454808,63.048850454846,64.05493445482 ,65.764720263289,65.764720263289,66.026683744069,66.052126454832,66.062422454788,67.010770668273,67.010770668273,67.059146454802,67.061018454794,67.5426921178,67.773100551544,68.007464527477,68.007464527477,68.012496324834,68.523104837989,68.523104837989,68.523104837989]
	Y3 = [37.252164344286,37.252164344286,37.762617491864,38.010004113049,38.010004113049,39.048380112884,40.796624564367,40.835468564201,41.039160460961,41.254921179879,41.254921179879,41.539142849531,42.006072828663,42.006072828663,42.044916828497,42.505312393669,42.763340895734,43.804056895559,44.842432895394,45.883148895219,46.021071975554,46.021071975554,46.27952935679 ,47.255182727048,47.255182727048,48.271291518027,48.271291518027,48.292622726887,48.500359960099,48.785051661209,50.823895661042,50.824831661038,50.82670366103 ,50.832319661006,51.863675660871,51.863675660871,52.902051660706,54.942299660533,55.514464735461,55.514464735461,55.560796735263,56.013230762158,57.284722179585,57.331522179385,58.019755558829,58.061875558649,58.506111667287,58.506111667287,59.000104032656,59.041756032478,59.041756032478]
	Y4 = [29.502124965637,29.502124965637,29.520279277538,29.524843332992,29.524843332992,29.547357297477,29.547357297477,29.751067346899,29.751253040633,29.751253040633,29.751253040633,29.751253040633,29.751253040633,29.751253040633,29.752126235916,29.754951016721,29.755660446178,29.755660446178,29.756043993927,29.756043993927,29.759699187043,29.762483583605,29.762576218744,29.762576218744,29.762576218744,29.766738171473,29.766738171473,29.766738171473,29.771141881102,29.771141881102,29.771141881102,29.772883371664,29.77948238129 ,29.77956486996 ,30.022884800479,30.022884800479,30.025621179596,30.036955106703,30.251233399471,32.020819123377,32.020819123377,32.021249726205,32.021249726205,34.272909090334,34.272909090334,34.272909090334,34.272909090334,34.272909090334,34.284223295625,34.284223295625,34.284223295625]
	Y5 = [37.77950515729 ,37.77950515729 ,38.002168155123,41.041731357935,45.010262838145,45.010262838145,45.020666815852,45.049358615337,49.041939134057,49.788852260936,49.788852260936,51.009491221292,51.020678398525,51.510503082877,51.510503082877,52.260530803423,54.031052175152,55.005216421326,55.048428634565,56.01248240181 ,56.043829312699,57.041540066804,59.014673070621,59.288517643766,60.027740138443,60.518728996068,60.518728996068,61.019146564078,62.022731670143,63.04456016923 ,63.045689898997,65.03856388753 ,65.773089121024,65.773089121024,66.521822053557,66.752314237208,66.754855019925,66.754855019925,67.021836588777,67.021836588777,67.030101147887,67.034645049937,67.044437737937,67.262769686494,67.262769686494,68.010422031825,68.028104462625,68.049472745065,68.049835393577,69.044943565037,69.044943565037]

	# Y1 = [37.254136677146,37.254136677146,37.533796024246,37.533796024246,37.760146373391,39.018858864461,40.29561428761 ,40.542578812174,40.542578812174,40.773899962478,41.011028413776,41.020262864455,41.044976858046,41.044976858046,41.537160799328,41.760841782333,41.760841782333,42.019794864457,44.017922864465,44.023538864441,45.030992336156,45.267988975417,45.267988975417,45.795051638132,46.023070864443,46.795431378156,47.009030864503,47.035207992881,47.035207992881,48.014646864479,50.004350864523,50.009498864501,50.022134864447,51.013242864485,51.019326864459,51.021198864451,51.021666864449,54.004818864521,54.015292445421,54.022602864445,54.037540839128,54.037540839128,55.293466179141,55.503669125981,55.503669125981,55.780988553661,56.043135867217,56.043135867217,56.293465654989,56.293465654989,56.293465654989,56.293465654989]
	# Y2 = [69.753658789736,69.753658789736,70.284307759484,70.284307759484,70.542201477705,70.542201477705,71.294741950214,71.294741950214,71.508647854043,71.508647854043,71.522529827963,71.522529827963,73.010048955081,73.294685670382,73.510490439128,73.542008155737,73.542008155737,73.77767765345 ,75.255139643685,77.008562864505,77.284036721472,77.530980900706,77.530980900706,78.255173041761,78.260511050442,78.538079699377,78.544537935823,78.544537935823,78.753112456532,78.753112456532,79.008934276463,79.015044992191,79.015044992191,79.021198864451,79.021198864451,79.033366864399,79.043574748346,79.265040986009,79.760517559278,79.760517559278,79.769625457025,79.769625457025,79.777495038626,80.003508071399,80.005754864517,80.006690864513,80.007158864511,80.014646864479,80.020262864455,80.021666864449,80.021666864449]

	Y.append(Y1[:50])
	Y.append(Y2[:50])
	Y.append(Y3[:50])
	Y.append(Y4[:50])
	Y.append(Y5[:50])



	for i in range(5):
		ax.plot(X, Y[i], label=label[i], linewidth=linewidth, dashes=dashList[i])
	
	# fig.tight_layout()
	order = [0,1,2,3,4]
	# plt.xlim(xmin=0)
	fig.legend(ncol=5, loc='lower center', shadow=True, fontsize='x-large');
	# plt.legend([handles[idx] for idx in order],[labels[idx] for idx in order], ncol=5, loc='lower center', shadow=True, fontsize='x-large')
	plt.show()