#ifndef HT_ITEMS_H
#define HT_ITEMS_H
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

    node_data_item(){Key=0; Value="OK";}  //default

    node_data_item(int k, char *v)
    {
        Key = k;
        Value = v;
    }
    int key(){
        return Key;
    }

    char *value(){
        return Value;
    }

};

struct bucket_item{

private: int ID;
    char *ip;
    int Udp_port;

public:
    bucket_item(int id_init, char *ip_init, int udp_port_init)
    {
        ID = id_init;
        ip = ip_init;
        Udp_port=udp_port_init;
    }

    bucket_item()  //default
    {
        ID = -1;
        ip = NULL;
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
 };

 Src src;
 int dst;
 int command;
 node_data_item *value;
};

#endif // HT_ITEMS_H
