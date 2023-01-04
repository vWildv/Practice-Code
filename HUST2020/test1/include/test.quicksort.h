#ifndef _TEST_QUICKSORT_CASE1_H
#define _TEST_QUICKSORT_CASE1_H

#include <quicksort.h>
#include <algorithm>
#include <stdlib.h>

void test_swap(int a,int b){
    int aa=a,bb=b;
    swap(&a,&b);
    EXPECT_EQ(aa,b);
    EXPECT_EQ(bb,a);
}

void test_partition(){
    int a[]={ 6, 5, 2, 7, 3, 9, 8, 4, 10, 1 };
    int index=partition(a,0,9);
    for(int i=0;i<index-1;i++){
        EXPECT_LT(a[i],a[index]);
    }
    for(int i=index+1;i<10;i++){
        EXPECT_GT(a[i],a[index]);
    }
}

void test_sort(){
    int a[]={ 6, 5, 2, 7, 3, 9, 8, 4, 10, 1 };
    int b[]={ 6, 5, 2, 7, 3, 9, 8, 4, 10, 1 };
    quicksort(a,0,9);
    std::sort(b,b+10);
    for(int i=0;i<10;i++){
        EXPECT_EQ(a[i],b[i]);
    }
}

TEST(test, quicksort) {
    test_swap(5,7);
    test_partition();
    test_sort();
}
    
#endif
