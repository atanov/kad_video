/*  hash table basic class */

#ifndef ST_BASIC_H
#define ST_BASIC_H

#endif // ST_BASIC_H
#include <iostream>
using namespace std;

template <class Item, typename Key>
class ST{
private:
    Item *nullItem;

    struct node {
        Item *item; node* next;
        node(Item *x, node* t){
            item = x; next = t;
        }
    };
    typedef node *link;
    int N,M;
    link *heads;
    Item *searchR(link t, Key v)
    {
        if (!t) return NULL;
        if (t->item->key() == v) return t->item;
        return searchR(t->next, v);
    }

    Item *printR(link t)
    {
        if (t == NULL) return NULL;
        cout << t->item->key() << " ; " << t->item->value() << "\n";
        return printR(t->next);
    }

    //inline int hash (Key key, int M){
    //    return ((unsigned)key)/100;
    //}

    inline int hash (Key key, int M){
        return ((uint32_t)key)%M;
    }
public:
    ST(int maxN){
        N = 0; M=maxN/5;
        heads = new link[M];
        for (int i=0; i<M;i++) heads[i]=NULL;
    }

    int count(){ return N; }

    Item *search(Key v){
       return searchR(heads[hash(v,M)], v);
    }

    void print(){
        for (int i=0; i<M; i++) {
            cout << "ST_heads[" <<i <<"]:\n";
            printR(heads[i]);
        }
    }
    void insert(Item *x){
        int i = hash(x->key(),M);
        heads[i] = new node(x, heads[i]);
        N++;
    }
};
