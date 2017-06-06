#ifndef MY_STACK_H
#define MY_STACK_H

template <class Item> class Stack {
private:
    struct node {
        Item item; node *next;
        node (Item x,node *t){
            item=x;next=t;
        }
    };

    int N;
    int MAX_N;
    node *head;

public:
    Stack(int maxN){
        N=0;
        MAX_N=maxN;
        head=NULL;
    }

    int push(Item item){
        node *new_node;

        if (N<MAX_N){
            N++;
            new_node=new node(item,head);
            head=new_node;
            return 1;
        }
        else return 0;  //stack is full
    }

    Item pop()
    {
        if (N>0) {N--;
       node *t=head;
       head=head->next;
       return t->item;
        }
        else return NULL;   //stack is empty
    }

    int empty() {return (N>0)?0:1;}
    int size(){return N;}
};

#endif // MY_STACK_H
