// kad_video.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ht_bucket.h>

typedef HT<bucket_item,int> node_hash_table;
using namespace std;
const int NODES_SIZE=100;

int main(int argc, char* argv[])   //place all logic here
{//-------------  new version --------------


    node_hash_table **nodes;
    nodes=(node_hash_table **)malloc(NODES_SIZE*sizeof(node_hash_table *));

    for (int i=0; i<NODES_SIZE; i++){
        nodes[i]=new node_hash_table(i*11);
    }

    bucket_item *new_item;
    for (int i=1; i<NODES_SIZE; i+=10){   //nodes[0]  know some other nodes
        new_item=new bucket_item(nodes[i]->id(),(char *)nodes[i],127);
        nodes[0]->insert(new_item);
    }

    for (int i=1;i<NODES_SIZE;i++){    //everybody knows about nodes[0];
        new_item=new bucket_item(nodes[0]->id(),(char *)nodes[0],127);
        nodes[i]->insert(new_item);
    }


    new_item= new bucket_item(nodes[1]->id(), (char *)nodes[1],127);  //nodes[2] meet nodes[1]
    nodes[2]->insertF(new_item);

    cout  << "  recursive find node:  \n";
    nodes[2]->node_search(nodes[1]->id(),nodes[31]->id());

//    //----------------------
//    cout << "_______nodes[0]: \n";
//    nodes[0]->print();

    //cout <<" _______nodes[1], id=11: \n";
    //nodes[1]->print();


    //*******
    for (int i=0;i<NODES_SIZE;i++){
        delete nodes[i];
    }
    free(nodes);
    return 0;
}
