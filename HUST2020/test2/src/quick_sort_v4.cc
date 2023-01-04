#include <quicksort.h>

inline int three_choose(int *arr, int l, int r) {
    int a = arr[l], b = arr[r], c = arr[(l + r) >> 1];
    if((a>=b&&a<=c)||(a<=b&&a>=c)) return l;
    if((b>=a&&b<=c)||(b<=a&&b>=c)) return r;
    if((c>=a&&c<=b)||(c<=a&&c>=b)) return ((l+r)>>1);
}

inline int partition_v4(int *a,int l,int r){
    int p=three_choose(a,l,r);
    int i=l,j=r;
    while(i<j){
        while(*(a+j)>=*(a+p) && i<j) j--;
        while(*(a+i)<=*(a+p) && i<j) i++;
        std::swap(*(a+i),*(a+j));
    }
    std::swap(*(a+p),*(a+i));
    return i;
}

void quick_sort_v4(int *arr, int l, int r) {
    while (l < r) {
        int x = partition_v4(arr, l, r);
        quick_sort_v4(arr, l, x);
        l = x + 1;
    }
    return ;
}