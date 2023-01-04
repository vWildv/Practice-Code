#include <stdio.h>
#include <quicksort.h>

#ifdef DEBUG
    #include <gtest/gtest.h>
    #include <testcase.h>
#endif

using namespace std;

int main(int argc, char *argv[]) {
    #ifdef DEBUG
        ::testing::InitGoogleTest(&argc, argv);
    #endif

    int a[]={ 6, 5, 2, 7, 3, 9, 8, 4, 10, 1 };
    quicksort(a,0,9);
    for(int i=0;i<10;i++) printf("%d ",a[i]);
    printf("\n");

    #ifdef DEBUG
        return RUN_ALL_TESTS();
    #else
        return 0;
    #endif
}
