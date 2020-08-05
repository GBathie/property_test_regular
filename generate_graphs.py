import matplotlib.pyplot as plt
import tikzplotlib as tpl

def gen_time_graph(fname):
	x = []
	exact = []
	approx1 = []
	approx2 = []
	approx3 = []
	with open(fname, 'r') as f:
		for l in f.readlines()[1:]:
			a,b,c,d,e = list(map(int, l.split(' ')))
			x.append(a)
			exact.append((b/1000)/50)
			approx1.append((c/1000)/50)
			approx2.append((d/1000)/50)
			approx3.append((e/1000)/50)

	plt.figure()
	plt.title("Execution time of the algorithms (avg over 50 runs)")
	plt.plot(x, exact, label='exact')
	plt.plot(x, approx1, label='approx, $\\varepsilon = 0.5, \\delta = 0.3$')
	plt.plot(x, approx2, label='approx, $\\varepsilon = 0.3, \\delta = 0.3$')
	plt.plot(x, approx3, label='approx, $\\varepsilon = 0.1, \\delta = 0.1$')
	plt.xlabel('Size of the input')
	plt.ylabel('Time (ms)')
	# plt.xscale('log')
	plt.legend(loc='upper left')
	plt.savefig(fname[:-4] + ".png")
	tpl.save(fname[:-4] + ".tex")


if __name__ == '__main__':
	gen_time_graph('time01.txt')
	gen_time_graph('time_random.txt')