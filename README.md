# Top K High Utility Itemset Mining
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

<details>
  
  <summary>Few ideas for the future</summary>
  
  The sweet range, $[K, cap(K)]$ could be made dynamic so that as time progresses, the algorithm gets desperate to quit with even a huge number of itemsets. The exact definition of such a time-varying $cap$ function that yields good results requires the knowledge of several properties of the database, such as the distribution of itemsets with respect to utility.
</details>

The above repo contains C++ implementations of the above ideas and a comparison of their running times on several datasets available at the [SPMF open source library](https://www.philippe-fournier-viger.com/spmf/index.php?link=datasets.php)

I have also created a standalone executable ([huim.exe](https://github.com/koderkushy/Top-K-High-Utility-Itemset-Mining/tree/main/Algorithms/interface)) to run the algorithms on spmf-formatted datasets via the command line conveniently.

## Results

The following running times were achieved on a Windows 11 computer, running on an AMD Ryzen 4500U processor with 6 physical cores and 8 GiB of RAM. The implementations are single threaded however some parts can be parallelized by adding relevant standard execution policies defined in the algorithm library.

![fruithut_all](https://user-images.githubusercontent.com/76262561/174132660-8a936492-7b55-443e-b018-198b40fbc854.png)

![mushroom_all](https://user-images.githubusercontent.com/76262561/174132663-7d6edd72-467c-4ae2-a3ea-09472af24c83.png)

![chicago_all](https://user-images.githubusercontent.com/76262561/174132670-f393c7fe-5ed8-4338-af5f-c5e00b72b8dd.png)

![ecommerce_all](https://user-images.githubusercontent.com/76262561/174132673-d32b3797-b02b-49b8-ab82-fa9dfc3da39a.png)

![foodmart_all](https://user-images.githubusercontent.com/76262561/174132675-eeeaf8e9-c18a-4db2-8c65-8098d065923b.png)

### The following are randomly generated databases

![random_1_all](https://user-images.githubusercontent.com/76262561/174132665-d69559cb-b43f-4d20-afb0-674a62f14e65.png)

![random_2_all](https://user-images.githubusercontent.com/76262561/174132668-d588536e-a634-4a9c-b411-0fd37bddf1f2.png)

## Conclusions
The priority queue version has an almost uniform running time (initially outdone by the binary search version in some cases) hardly wavering even on the logarithmic axis which is quite promising.

The binary search version however has several complications. The execution time depends on how fast the algorithm narrows down to a sweet threshold value.
