void swap(int *a,int *b){
    if(*a==*b) return;
    *a=(*a)^(*b);
    *b=(*a)^(*b);
    *a=(*a)^(*b);
}

int partition(int *a,int l,int r){
    int p=*(a+l);
    int i=l,j=r;
    while(i<j){
        while(*(a+j)>=p && i<j) j--;
        while(*(a+i)<=p && i<j) i++;
        swap(a+i,a+j);
    }
    swap(a+l,a+i);
    return i;
}

void quicksort(int *a,int l,int r){
    if(l<r){
        int index=partition(a,l,r);
        quicksort(a,l,index-1);
        quicksort(a,index+1,r);
    }
}
    



