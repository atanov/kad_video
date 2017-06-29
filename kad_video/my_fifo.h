
#ifndef MY_FIFO_H
#define MY_FIFO_H

template <class Item> struct my_fifo{
private:
    struct node {
        Item item;
        node *next;
        node (Item i, node *n){item=i; next=n;}
    };

    node *head;
    node *tail;
    int N;
    Item dum;


public:
    my_fifo(){
        head=NULL;
        tail=NULL;
        N=0;
    }

    int empty(){
        return (N>0)?0:1;
    }

    void put(Item i){
        node *t = tail;
        tail=new node(i, NULL); //
        if(N==0) head=tail;
        else t -> next =tail;
        N++;
       }

    Item get(){
        if (N==0) return dum;

        else
        {
            Item r=head->item;
            node *t=head->next;
            delete(head);
            head=t; N--;
            return r;
        }
    }
};

#endif // MY_FIFO_H
