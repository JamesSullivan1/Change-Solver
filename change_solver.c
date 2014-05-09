#include <stdio.h>
#include <stdlib.h>

#define PENNY 1
#define NICKEL 5
#define DIME 10
#define QUARTER 25

#define COIN_TYPES 4
#define MAX_COINS 20 // Reasonable upper-bound

const int coin_types[] = { PENNY, NICKEL, DIME, QUARTER };

/*
 * Comparator for unique-only values
 */
int icmp(const void *a, const void *b)
{
#define _I(x) *(const int*)x
    return _I(a) < _I(b) ? -1 : _I(a) > _I(b);
#undef _I
}

/*
 * Comparator with equality
 */
int icmpeq(const void *a, const void *b)
{
    const int *arg1 = a;
    const int *arg2 = b;
    if(*arg1 < *arg2) return -1;
    else if(*arg1 == *arg2) return 0;
    return 1;
}

/*
 * Reduces the input array in-place by sorting and removing duplicates,
 *  and returns the new array size.
 */
int sort_and_reduce(int *array, int length)
{
    if(length <= 0) return 0;
    int i,j = 0;
    qsort(array, length, sizeof(int), icmp);
    
    for(i = 0; i < length; i++) {
        if(array[i] != array[j]) array[++j] = array[i];
    }

    return j+1;
}

/*
 * Modifies the *out array to contain all sums possible given the set of coins.
 *  Returns 1 on success and 0 on failure.
 */
int find_sums(int *coins, int n, int *out, int size)
{
    if(n > 30 || n < 0) return 0;
    if(size != (1 << n) - 1) return 0; // Boundary checks

    int i = 0;
    for(i = 1; i < size; i++) {
        int sum = 0;
        int j = 0;
        for(j = 0; j < n; j++) {
            int pos = 1 << j; // Bit mask trick to lexicographically iterate
            if((i & pos) == pos) sum += coins[j];
        }
        out[i] = sum;
    }
    
    return 1;
}

/*
 * Returns : 1 if every change value from 1 - 99 can be made
 *           0 if not
 */
int valid(int *coins, int n, int *sums, int size)
{
    if(n < 0 || n > 30) goto fail;
    if(size != (1<<n) - 1) goto fail;

    int rc = find_sums(coins, n, sums, size);
    if(rc == 0) goto fail;
    
    int size_new = sort_and_reduce(sums, size);

    if(size_new < 100) goto fail;

    int i;
    for(i = 1; i < 101; i++) {
        if(sums[i] != i) goto fail;
    }
    
    return 1;
fail:
    return 0;
}

/*
 * Modifies the input *coins array to contain its lexicographic same-length successor
 *  if it exists. Returns: 1 on success, 0 on failure.
 *  
 *  The lexicographic order is designed in a way that the order of a sequence is
 *   not relevant, only the actual values, and any nondecreasing version of an equivalent
 *   sequence is simply sorted.
 *
 *   Ie, { PENNY, NICKEL, DIME } and { DIME, PENNY, NICKEL } are equivalent.
 */
int find_successor(int *coins, int n)
{
    int i;
    int in_order = 1;
    for(i = 0; i < n-1; i++) {
        if(coins[i+1] < coins[i]) {
            in_order = 0;
        }
    }

    if(in_order == 0) {
        qsort(coins, n, sizeof(int), icmpeq);
    }

    i = n - 1;
    int done = 0;
    while(done == 0 && i >= 0){
        switch(coins[i]) {
            case PENNY:
                coins[i] = NICKEL;
                done = 1;
                break;
            case NICKEL:
                coins[i] = DIME;
                done = 1;
                break;
            case DIME:
                coins[i] = QUARTER;
                done = 1;
                break;
            case QUARTER:
                if( i == 0 ) {
                    i -= 1; 
                    break;
                }
                int next_coin;
                int j;
                switch(coins[i-1]){
                    case PENNY:
                        next_coin = NICKEL;
                        done = 1;
                        break;
                    case NICKEL:
                        next_coin = DIME;
                        done = 1;
                        break;
                    case DIME:
                        next_coin = QUARTER;
                        done = 1;
                        break;
                    case QUARTER:
                        i -= 1;
                        break;
                    default: return 0; // Shouldn't happen
                }
                if(done == 1) {
                    for(j = i-1; j < n; j++) {
                        coins[j] = next_coin;
                    }
                }
                break;
            default:
                return 0; // Should not happen
        }
    }

    if(done == 0) return 0; // No successor
    return 1;
}

/*
 * If an n-coin valid combination exists, puts it into the buffer and 
 *  returns 1. Else returns 0.
 */
int find_combination(int *buf, int n)
{
    // Base combination is all pennies
    int i = 0;
    for(i = 0; i < n; i++) {
        buf[i] = coin_types[0];
    }

    int size = (1 << n) - 1; // Max number of subsets of the coins
    int *sums = (int *)(calloc(size, sizeof(int)));
    int *pos = sums;

    // Iterate through all coin combinations
    int has_successor = 1;
    while(!valid(buf, n, sums, size) && has_successor == 1) {
        int rc = find_successor(buf, n);
        if(rc == 0) has_successor = 0;
        // Cleanup
        for(i = 0; i < size; i++) {
            sums[i] = 0;
        }
    }

    // Deallocated
    if(sums) {
        sums = pos;
        free(sums);
    }

    if(has_successor == 0) { 
        // Cleanup
        for(i = 0; i < n; i++) {
            buf[i] = 0;
        }
        return 0;
    }
    return 1;
}


int main(int argc, char *argv[])
{
    int found = 0;
    int n = 1;
    int *coins;
    while(found == 0 && n < MAX_COINS) {
        printf("n = %d : ", n);
        coins = (int *)(malloc(n*sizeof(int)));
        int rc = find_combination(coins, n);
        if(rc == 1) found = 1;
        else {
            if(coins) free(coins);
            n++;
            printf("failed.\n");
        }
    }

    if(found == 1) {
        printf("Found a minimal %d-coin solution:\n", n);
        int i = 0;
        for(i = 0; i < n; i++){
            printf("%d\t", coins[i]);
        }
        printf("\n");
        goto done;
    }

    goto fail; 

done:
    if(coins) free(coins);
    return 0;
fail:
    printf("Unable to find a solution...\n"); 
    if(coins) free(coins);
    return 1;
}
