# Top K High Utility Itemsets Mining
Understanding and implementing the EFIM algorithm to solve the top-K HUIM problem.

## Problem definition

<details>
  <summary> Basic definitions that make the problem simple to understand </summary>
  
### Transaction database
Let $I$ be a finite set of items (symbols). An itemset $X$ is a finite set of items such that $X \subseteq I$. A transaction $T$ is a set of items where each item $i \in T$ has an associated _utility_, denoted $U(i, T)$. A transaction databse is a set of transactions.

### Utility of an itemset in a transaction
The utility of an itemset $X$ in a transaction $T (X \subset T)$ is defined as $$U(X, T) = \sum_{i \in X} U(i, T)$$

### Utility of an itemset in a transaction database
Naturally, the utility of an itemset $X$ in a database $D$ is defined as $$U_D(X) = \sum_{T\in D \wedge X\subset T} U(X, T)$$
</details>

The problem of top $K$ high utility itemset mining is to find such $K$ itemsets that their utilities in the given transaction database are the highest. Formally, given a transaction database $D$, find such a subset $S$ of $2^I$ (the set of all subsets of $I$) that
- $|S| = K$
- For any itemset $I_S \in S$ and any itemset $I_\bar{S} \in 2^I \setminus S$, it is satisfied that $U_D(I_S) \geq U_D(I_\bar{S})$

This problem is particularly interesting as the utility measure is neither monotonic nor anti-monotonic, i.e., the utility of an itemset may be lower, higher or equal to that of its subsets. As we add an item to an itemset, the number of transactions that contain it reduce (or remain the same), however its utility also increases due to the newly added item's component in each of the transactions.

## About EFIM
Among [several others](https://www.philippe-fournier-viger.com/spmf/index.php?link=algorithms.php#:~:text=High%2DUtility%20Pattern%20Mining), EFIM (Zida et al., 2015) is a high utility itemset mining algorithm. It solves a slightly different problem which is to find all itemsets that have utility higher than a given a minimum utility threshold. It uses several pruning strategies that greatly reduce the search space.

## Modifying EFIM to solve top K HUIM
### Priority queue version
Rather than having a fixed minimum utility threshold, we start with $min \textunderscore util = 1$ (or any trivial start point). As the EFIM algorithm discovers new itemsets, we continually update the minimum utility threshold to the $K^{th}$ highest utility among the currently discovered itemsets. We only need to maintain the top $K$ itemsets at any moment which calls for a priority queue data structure.

### Binary search version
We binary search on the minimum utility threshold. Simple as it sounds, we start with an interesting range $\[0, max \textunderscore util \textunderscore estimate\]$ and continually halve it by counting the number of HUIs with respect to the median utility threshold using a straightforward run of the EFIM algorithm. However, a naive binary search is rather slow because it takes around $10$ iterations of the costly search method to reduce a range of size $1000$, where we can rather just retrieve the few excess itemsets and delete them later by sorting.


The above repo contains C++ implementations of the above ideas and a comparison of their running times on several datasets available at the [SPMF open source library](https://www.philippe-fournier-viger.com/spmf/index.php?link=datasets.php)

I have also created a standalone executable ([huim.exe](https://github.com/koderkushy/Top-K-High-Utility-Itemset-Mining/tree/main/Algorithms/interface)) to run the algorithms on spmf-formatted datasets via the command line conveniently.

## Results

Execution times of both algorithms on the following datasets were recorded and plotted. They were also compared with the [TKU algorithm](https://www.philippe-fournier-viger.com/spmf/TKDE_TKO.pdf) (Tseng et al., 2015) on two datasets (because it was too slow on the others).

The following running times were achieved on a Windows 11 computer, running on an AMD Ryzen 4500U processor with 6 physical cores and 8 GiB of RAM. The implementations are single threaded however some parts can be parallelized by adding relevant standard execution policies defined in the C++ standard library.
<details>
  <summary>Chicago crimes</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174738071-da2db46c-2766-4b87-8af4-39bf8a510234.png">
</details>

<details>
  <summary>Ecommerce retail</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174132673-d32b3797-b02b-49b8-ab82-fa9dfc3da39a.png">
</details>

<details>
  <summary>Foodmart (with TKU)</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174738741-7daf9439-af8a-460d-9efa-76af5d8bf947.png"> <img src="https://user-images.githubusercontent.com/76262561/174739250-199484cd-f148-416c-a382-5ea0d56cb572.png">
</details>

<details>
  <summary>Fruithut</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174738875-de3b0fac-41d4-4188-87b6-f9146a839d43.png">
</details>

<details>
  <summary>Mushroom (with TKU)</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174739075-d5d6685f-b10c-4c03-a34c-a641eccba647.png"> <img src="https://user-images.githubusercontent.com/76262561/174739276-91adb5b4-d2dd-46fc-af65-05a6758075a1.png">
</details>

### The following are randomly generated databases

<details>
  <summary>Random 1</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174132665-d69559cb-b43f-4d20-afb0-674a62f14e65.png">
</details>

<details>
  <summary>Random 2</summary>
  <img src="https://user-images.githubusercontent.com/76262561/174132668-d588536e-a634-4a9c-b411-0fd37bddf1f2.png">
</details>


## Conclusions

- Both versions vastly outperform the [TKU algorithm](https://www.philippe-fournier-viger.com/spmf/TKDE_TKO.pdf) (Tseng et al., 2015).
- The priority queue version has an almost uniform running time hardly wavering even on the logarithmic scale which is quite promising if we're looking to mine a larger number of itemsets.
- The binary search version however does not have a very monotonic performance. Since the maximum utility estimate is usually way beyond the actually maximum utility value, it plays a crucial role in determining how many iterations the algorithm takes before reaching meaningful minimum utility thresholds. Thereafter, the number of iterations it takes to close in on the sweet region varies discretely with the values of $K$. Thus, in some cases (like those of Ecommerce retail, Foodmart, Mushroom, Random 1&2) there are regions where the run time decreases with $K$.
- The binary search version outperforms the priority queue version on relatively smaller values of $K$, which are practically more meaningful.
