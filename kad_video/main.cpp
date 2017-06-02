// kad_video.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ht_bucket.h>

#define PING 0
#define STORE 1
#define FIND_NODE 2
#define FIND_VALUE 3
typedef HT<bucket_item,int> node_hash_table;

using namespace std;


//struct node_msg {
// struct Src{
//     int src_id;
//     char *src_ip;
// };

// Src src;
// int dst;
// int command;
// node_data_item *value;
//};
//-==-----------------==--------------
//struct node_data_item{
//private: int Key;
//    char *Value;

//public:
//    node_data_item(int k, char *v)
//    {
//        Key = k;
//        Value = v;
//    }
//    int key(){
//        return Key;
//    }

//    char *value(){
//        return Value;
//    }
//};
//====--====--====--====--====--====--====--====--====--====--====--====--====--====--====--====

int node_sender(node_hash_table *send, int c, node_data_item *v, int rec_id){

    bucket_item *search_rec = send->search_item(rec_id);
    node_hash_table *rec;

    if (search_rec) rec=(node_hash_table *)search_rec->value();    //get link by id from buckets
        else {
        cout << "command "<< c <<" error, node " << rec_id <<" has not found\n";
        return 0;
    }


    node_msg msg_send={{send->id(),(char *)send},rec->id(),c,v};
    node_msg msg_rec=rec->process_query(&msg_send);  //send msg to selected node in DHT

    //analyze answer
    switch (c)
    {
    case PING:
        if (!strcmp(msg_rec.value->value(),"OK")){return 1;}
        break;

    case STORE:
    if (!strcmp(msg_rec.value->value(),"OK")){return 1;}
        break;

    case FIND_NODE:
    if (!strcmp(msg_rec.value->value(),"NOT_FOUND")){return 0;}

        return 1;
        break;

    case FIND_VALUE:
        if (!strcmp(msg_rec.value->value(),"NOT_FOUND")){return 0;}
        if (msg_rec.value->key()==v->key()) {
                cout << "Found id = " << msg_rec.value->key() << "; value = " << msg_rec.value->value() << " \n";
                return 1;}
        else {cout << "Closest nodes for id = " << msg_send.value->key() << ":\n";
        for (int i=0,id=0;(id>=0)&&(i<k_size);i++){
            id=(((bucket_item *)msg_rec.value->value())[i]).key();
            cout << "id = " << id <<"; value = " << (((bucket_item *)msg_rec.value->value())[i]).value() <<"\n";
        }
        }

        return 1;
        break;

    default:
        break;
    }

    return 0;
}

int main(int argc, char* argv[])   //place all logic here
{

    node_hash_table node1(1),node2(4);
    bucket_item *new_item,*new_item2;

    node_data_item *data_item;

    for (int i=0;i<100;i++){
    new_item =new bucket_item(i*11,"127.0.0.1",i);
    node1.insert(new_item);
    }

    data_item=new node_data_item(1234,"Aleksandr Pushkin_1");
    node1.Store(data_item);

    data_item=new node_data_item(5678,"Lev Tolstoy_1");
    node1.Store(data_item);

    //node1.print();
    //node1.node_data_print();

    new_item2=new bucket_item(1,(char *)(&node1),8080);   //node2(4) knows only node1, node1(1) knows nothing about node2(4)
    node2.insert(new_item2);
//------------------------------------------------

    if (node_sender(&node2,PING,NULL,node1.id())) cout << "PING OK\n";   //node1(1) meet node2(4)
    else cout <<"PING FAILED\n";

    //node1.node_data_print();

    data_item=new node_data_item(888,"Feodor Dostoebsky");
    if(node_sender(&node2,STORE,data_item,node1.id())) cout << "STORE OK\n";
    else cout << "STORE FAILED\n";

    data_item=new node_data_item(889,"Feodor Dostoebsky_1");
    if(node_sender(&node2,STORE,data_item,node1.id())) cout << "STORE OK\n";
    else cout << "STORE FAILED\n";

    //node1.node_data_print();
    node1.print();
     //find_node
    node_data_item *find_node_item=new node_data_item(4,"FIND_NODE");    //find node with id=4;
    if(node_sender(&node2,FIND_NODE,find_node_item,node1.id())) cout << "FIND NODE OK\n";
    else cout << "FIND NODE FAILED\n";
    
    //find_value
    node_data_item *find_value_item=new node_data_item(890,"FIND_VALUE");
    if(node_sender(&node2,FIND_VALUE,find_value_item,node1.id())) cout << "FIND VALUE OK\n";
    else cout << "FIND VALUE FAILED\n";

//    if (node_sender(&node1,PING,NULL,node2.id())) cout << "PING OK\n";  //node2(4) already knows node1(1)
//    else cout <<"PING FAILED\n";

//    new_item =new bucket_item(5,"192.168.0.111",111);
//    node1.insert(new_item);

//    new_item =new bucket_item(6,"192.168.0.111",111);
//    node1.insert(new_item);

//    node1.print();
//  //  node2.print();

//    if (node_sender(&node2,PING,NULL,node1.id())) cout << "PING OK\n";  //node1(1) updates node2(4) record
//    else cout <<"PING FAILED\n";
//    node1.print();
//    cout << "zhopa\n";
    return 0;
}
