#include <quicksort.h>

inline int partition_v3(int *a,int l,int r){
    int p=*(a+l);
    int i=l,j=r;
    while(i<j){
        while(*(a+j)>=p && i<j) j--;
        while(*(a+i)<=p && i<j) i++;
        std::swap(*(a+i),*(a+j));
    }
    std::swap(*(a+l),*(a+i));
    return i;
}

void quick_sort_v3(int *arr, int l, int r) {
    while (l < r) {
        int x = partition_v3(arr, l, r);
        quick_sort_v3(arr, l, x);
        l = x + 1;
    }
    return ;
}
