#ifndef _TEST_QUICKSORT_CASE1_H
#define _TEST_QUICKSORT_CASE1_H

#include <stdio.h>
#include <quicksort.h>
#include <algorithm>

int a[100000]={};
//int b[100000]={};

void input(int *arr){
    freopen("data1.txt","r",stdin);
    for(int i=0;i<100000;i++) scanf("%d",arr+i);
    fclose(stdin);
}

/*
void verify(){
    for(int i=0;i<100000;i++){
        EXPECT_EQ(a[i],b[i]);
        return;
    }
}
*/

TEST(test, quicksort_v1) {
    input(a);
    quick_sort_v1(a,0,99999);
    //input(b);
    //std::sort(b,b+100000);
    //verify();
}

TEST(test, quicksort_v2) {
    input(a);
    quick_sort_v2(a,0,99999);
    //input(b);
    //std::sort(b,b+100000);
    //verify();
}

TEST(test, quicksort_v3) {
    input(a);
    quick_sort_v3(a,0,99999);
    //input(b);
    //std::sort(b,b+100000);
    //verify();
}

TEST(test, quicksort_v4) {
    input(a);
    quick_sort_v4(a,0,99999);
    //input(b);
    //std::sort(b,b+100000);
    //verify();
}

TEST(test,std_sort) {
    input(a);
    std::sort(a,a+100000);
}
    
#endif
