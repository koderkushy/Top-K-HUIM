# Top K High Utility Itemsets Mining
Understanding and implementing the EFIM algorithm to solve the top-K HUIM problem.

## Problem definition

<details>
  <summary> Basic definitions </summary>
  
### Transaction database
Let $I$ be a finite set of items (symbols). An itemset $X$ is a finite set of items such that $X \subseteq I$. A transaction $T$ is a set of items where each item $i \in T$ has an associated _utility_, denoted $U(i, T)$. A transaction databse is a set of transactions.

### Utility of an itemset in a transaction
The utility of an itemset $X$ in a transaction $T (X \subset T)$ is defined as $$U(X, T) = \sum_{i \in X} U(i, T)$$

### Utility of an itemset in a transaction database
Naturally, the utility of an itemset $X$ in a database $D$ is defined as $$U_D(X) = \sum_{T\in D \wedge X\subset T} U(X, T)$$
</details>

The problem of top $K$ high utility itemsets mining is to find top $K$ itemsets by utility in the given transaction database.

This problem is particularly interesting as the utility measure is neither monotonic nor anti-monotonic, i.e., the utility of an itemset may be lower, higher or equal to that of its subsets.

## About EFIM
Among [several others](https://www.philippe-fournier-viger.com/spmf/index.php?link=algorithms.php#:~:text=High%2DUtility%20Pattern%20Mining), EFIM (Zida et al., 2015) is a high utility itemset mining algorithm. It finds all itemsets that have utility higher than a given a minimum utility threshold using pruning strategies that greatly reduce the search space.

## Modifying EFIM to solve top K HUIM
### Priority queue version
Rather than having a fixed minimum utility threshold, we start with $min \textunderscore util = 1$. As the EFIM algorithm discovers new itemsets, we update the minimum utility threshold to the $K^{th}$ highest utility among the currently discovered itemsets. We only need to maintain the top $K$ itemsets which calls for a priority queue data structure.

### Binary search version
We binary search on the minimum utility threshold. However, a naive binary search is rather slow because it takes around $10$ iterations of the costly search method to reduce a range of size $1000$. Rather we must prematurely retrieve the itemsets and delete the excess by sorting.


The above repo contains C++ implementations of the above ideas and a comparison of their running times on several datasets available at the [SPMF open source library](https://www.philippe-fournier-viger.com/spmf/index.php?link=datasets.php)

I have also created ([huim.exe](https://github.com/koderkushy/Top-K-High-Utility-Itemset-Mining/tree/main/Algorithms/interface)) to run the algorithms on spmf-formatted datasets via the command line conveniently.

## Results

Execution times of both algorithms on the following datasets were recorded and plotted. They were also compared with the [TKU algorithm](https://www.philippe-fournier-viger.com/spmf/TKDE_TKO.pdf) (Tseng et al., 2015) on two datasets.

The following running times were achieved on a standard Windows 11 laptop.
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

### Randomly generated databases

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
- The priority queue version has an almost uniform running time hardly wavering.
- The binary search version however does not have a very monotonic performance. Since the maximum utility estimate is usually way beyond the actual maximum utility value, several iterations are required for the search to converge to a meaningful range.
- The binary search version outperforms the priority queue version on relatively smaller values of $K$.
