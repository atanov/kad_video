#ifndef MY_SORT_H
#define MY_SORT_H

template <typename T> int partition (T *a,int l, int r, int T::*key)
{T tmp;

    int i=l-1, j=r; int v=(a[r]).*key;
    for (;;)
    {
        while ((a[++i]).*key < v);
        while (v<(a[--j]).*key) if (j==l) break;
        if (i>=j) break;

        tmp=a[i]; a[i]=a[j]; a[j]=tmp;
     }
    tmp=a[i]; a[i]=a[r]; a[r]=tmp;
    return i;
}

template <typename T> void quicksort(T *a,int l, int r,int T::*key)
{   int i;
    if (r<=l) return;
    i=partition<T>(a,l,r,key);
    quicksort<T>(a,l,i-1,key);
    quicksort<T>(a,i+1,r,key);
}

#endif // MY_SORT_H
