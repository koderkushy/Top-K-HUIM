from ctypes import *
import argparse as agp
import textwrap
import os

parser = agp.ArgumentParser(
	formatter_class=agp.RawDescriptionHelpFormatter,
	description=textwrap.dedent('''\
		A command line tool to execute Top-K high utility itemset mining implementations. The algorithms are inspired from the EFIM algorithm (Fournier-Viger et al, 2015) which mines high utility itemsets from a transactional database given a minimum utility threshold.
		EFIM Paper - http://philippe-fournier-viger.com/EFIM_JOURNAL_VERSION%20KAIS%202016.pdf
		The SPMF open source data mining library - https://www.philippe-fournier-viger.com/spmf/
		'''))

parser.add_argument(
	'algorithm',
	choices=['pq', 'bs'],
	help='The algorithm to be called, priority queue version or binary search version of EFIM',
	)

parser.add_argument(
	'input_path',
	type=str,
	help='Path to the input file (written in spmf format)',
	)

parser.add_argument(
	'K',
	type=int,
	help='The number of top high utility itemsets to be found',
	)

parser.add_argument(
	'-S', '--stats',
	action='store_true',
	help='Toggles printing of various statistics related to the algorithm'
	)

parser.add_argument(
	'-o',
	metavar='<file>',
	type=str,
	help='The path to the output file (will be written in spmf format)'
	)

args = parser.parse_args()

def string (s):
	if s is None:
		return (c_char * 0)();
	string = c_char * len(s)
	word = string()
	for i in range(len(s)):
		word[i] = ord(s[i])
	return word


lib_path = os.getcwd() + "\\interface.so"
print(lib_path)
caller = cdll.LoadLibrary(lib_path);

ipath = string(args.input_path)
opath = string(args.o)
K = args.K
show = int(args.stats)

if args.algorithm == 'pq':
	caller.run(1, ipath, K, opath, show);
else:
	caller.run(0, ipath, K, opath, show);


