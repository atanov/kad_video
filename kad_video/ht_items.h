#ifndef HT_ITEMS_H
#define HT_ITEMS_H

#include <string.h>

#define PING 0
#define STORE 1
#define FIND_NODE 2
#define FIND_VALUE 3
using namespace std;



//const uint8_t K=3;

const uint8_t ID_size=32;
const uint8_t k_size=4;
const uint8_t a_size=3;

struct node_data_item{

public:
    int Key;
    char *Value;
    int size;
    char *backup;

    //node_data_item(){Key=0; Value= new char[80]; strcpy(Value,"OK");}  //default
    node_data_item(){size=0;Key=0; Value= new char[1024]; backup=Value; strcpy(Value,"OK"); }  //default

    node_data_item(int k, char *v)
    {
        Key = k;
        Value = v;
        size=0;
    }

    /*node_data_item(int k, char *v, int s){
        Key=k;
        Value=v;
        size=s;
    }*/

    node_data_item(int k, char *v, int s){
        Key=k;
        Value=new char[s];
        memcpy(Value,v,s);
        size=s;
    }

    int key(){
        return Key;
    }

    char *value(){
        return Value;
    }

};

struct bucket_item{

public:
     int ID;
     char *ip;
     int Udp_port;

    bucket_item(int id_init, char *ip_init, int udp_port_init)
    {
        ID = id_init;
        ip = new char[80]; strcpy(ip,ip_init);
        Udp_port=udp_port_init;
    }

    bucket_item()  //default
    {
        ID = -1;
        ip = new char[80]; strcpy(ip,"0.0.0.0");
        Udp_port=0;
    }


    int key() {return ID;}
    char *value() {return ip;}
    int udp_port() {return Udp_port;}
};


struct node_msg {
 struct Src{
     int src_id;
     char *src_ip;
     int src_port;

     Src(){src_ip=new char[20];}
 };

 Src src;
 int dst;
 int command;
 node_data_item *value;

 node_msg (){
     value = new node_data_item;
 }
};

class asked_list{
private:
    struct node{
        int value;
        node *next;
        node (int v,node *n){
            value=v; next=n;
        }
    };

    node *head;

public:
    asked_list(){
        head=NULL;
    }

    void insert (int v){
        head=new node(v,head);
    }

    int search(int v){
    node *cur=head;
    while (cur){
        if (cur->value==v) return 1;
        cur=cur->next;
    }
    return 0;
    }

    void reset(){
        node *cur=head;
        node *t;
        while (cur) {t=cur; cur=cur->next; delete(t);}
        head=NULL;
    }
};

#endif // HT_ITEMS_H
