#ifndef HT_BUCKET_H
#define HT_BUCKET_H

#include <iostream>
#include <st_basic.h>
#include <my_stack.h>
#include <ht_items.h>
#include <my_sort.h>

//---------------------------------   main class -----------------------------------

template <class Item, typename Key>
class HT{
typedef uint16_t uint;
typedef HT<Item,Key> node_hash_table;

private:
        ST <node_data_item,Key> node_data;   //node data structure
        Stack <Key> s;    //stack for recursive requestes
        Item *nullItem;
        Key node_ID;
        //service variables
        Item *K_items;   //node items array
        node_data_item answer_item;
        node_data_item find_node_item;

    struct node {
        Item *item; node* next;
        node(Item *x, node* t){
            item = x; next = t;
        }
    };
    typedef node *link;
    uint8_t N[ID_size],M;
    link *heads;
    node *searchR(link t, Key v)
    {
        if (!t) return NULL;
        if (t->item->key() == v) return t;//t->item;
        return searchR(t->next, v);
    }

    Item *searchR_item(link t, Key v)
    {
        if (!t) return NULL;
        if (t->item->key() == v) return t->item;
        return searchR_item(t->next, v);
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

    int hash (Key key, int M){   //inline
        int n=0;
        uint32_t u_key=(uint32_t)key;

        while (u_key){
            u_key=u_key>>1;
            n++;
        }
        return n;
    }

    int metric (Key key1, Key key2){
        return ((uint32_t)key1)^((uint32_t)key2);
    }

    void move_to_top(node *v){
        int k=v->item->key();
        node *h=heads[hash(k,M)];
        node *t=h;
        if (v==t) {cout << "already on the top\n";return;}  //already on top
        else {while ((t->next)!=v) t=t->next;
            t->next=v->next;   //del from current pos
            v->next=h;   //and place it before head
            heads[hash(k,M)]=v;//update head
       }
    }

public:

    HT(int ID):node_data(100),s(100){
        //node_data.node_data(100);  //init node data structure
        K_items=new Item[k_size];

        M=ID_size;
        heads = new link[M];
        node_ID=ID;
        for (uint i=0; i<M;i++) heads[i]=NULL;
        for (uint i=0;i<ID_size;i++) N[i] = 0;

    }

    int count(){ return N; }
    //Key
    int id(){return (int) node_ID;}

    void node_data_print(){
        node_data.print();
    }

    node *search(Key v){
       return searchR(heads[hash(v,M)], v);
    }

    Item *search_item(Key v){
       return searchR_item(heads[hash(v,M)], v);
    }

    void print(){
        for (uint i=0; i<M; i++) {
            cout << "heads[" <<i <<"]:\n";
            printR(heads[i]);
        }
    }
    void insert(Item *x){
        int i = hash(x->key(),M);
        if (N[i]<k_size) {heads[i] = new node(x, heads[i]); N[i]++;}
      }

    void insertF(Item *x){
        cout << "insertF: " << x->key() <<"; ";
        update_HT(x->key(),x->value(),x->udp_port());
    }

    void update_HT(Key k,char *ip,int ip_port){
        node *found = search(k);
        Item *found_item;
        if (found) {cout << "founded " << found->item->key() << ";\n";
            move_to_top(found);
        }
        else {//cout << "not founded " << k << ";\n";
        found_item=new Item(k,ip,ip_port);
        insert(found_item);
        };
    }
//-------  Transport send functions ----------------------

     int node_sender(int c, node_data_item *v, int rec_id, int *list_size, bucket_item **nodes_list){

        bucket_item *search_rec = search_item(rec_id);
        node_hash_table *rec;

        if (search_rec) rec=(node_hash_table *)search_rec->value();    //get link by id from buckets
            else {
            cout << "command "<< c <<" error, node " << rec_id <<" has not found\n";
            return 0;
        }


        node_msg msg_send={{id(),(char *)this},rec->id(),c,v};
        node_msg msg_rec=rec->process_query(&msg_send);  //send msg to selected node in DHT

        int i,id;
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
            cout << "Node id= " << rec_id <<".Closest nodes for id = " << msg_send.value->key() << ":\n";

                //for (i=0, id=0;(id>=0)&&(i<k_size); i++)
                i=0; id=(((bucket_item *)msg_rec.value->value())[i]).key();
                while((id>=0)&&(i<k_size))
                {
                    cout << "id = " << id <<"; value = " << (((bucket_item *)msg_rec.value->value())[i]).value() <<"\n";
                    i++;
                    id=(((bucket_item *)msg_rec.value->value())[i]).key();
                }

                *list_size=i;
                *nodes_list=(bucket_item *)msg_rec.value->value();

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

    bucket_item *process_stack(int id_to_found){

        while(!(s.empty())){
         int cur_id=s.pop();

         if (node_sender(PING,NULL,cur_id,NULL,NULL)) cout << "PING OK\n";
         else cout <<"PING FAILED\n";

         //find_node
         int list_size;
         bucket_item *nodes_list;

         find_node_item.Key=id_to_found; find_node_item.Value="FIND_NODE";    //find node with id=id_to_found;
         if(node_sender(FIND_NODE,&find_node_item,cur_id,&list_size,&(nodes_list))) cout << "FIND NODE OK\n";
         else cout << "FIND NODE FAILED\n";

         struct metr_struct{
         int metr;
         int id;
         };

         metr_struct metr[k_size];
         for (int i=0; i<k_size;i++) metr[i].metr=0xFFFF;
         int stop_processing=0;
         bucket_item *cur_link;

         //insert found nodes into [send] buckets and add to stack
         for (int i=0;i<list_size;i++) {
            cur_link=&(nodes_list[i]);
             insertF(cur_link);
             if (cur_link->key()==id()) continue;  //loop protect
             //find metric
             metr[i].metr=metric((cur_link->key()),id_to_found); metr[i].id=cur_link->key();
             if (metr[i].metr==0) {stop_processing=1;cout << id_to_found << " is found\n";}
            }

         if (stop_processing) return cur_link;

         quicksort(metr,0,k_size-1,&metr_struct::metr);  //sort struct array with key=metr;
         //for (int i=0; i<k_size; i++) cout << "metr [" << i<<"] = " << metr[i].metr << " , " << metr[i].id <<";\n";

         for (int i=0; i<a_size;i++){
         if (metr[i].metr <0xFFFF){
           if (!(s.push(metr[i].id)))   cout << "Stack overflowed !!!!\n";
          }
         }
        }

        return NULL;
     }

    void node_search(int first_query, int id_to_found)
    {
        s.push(first_query);
        process_stack(id_to_found);
    }

//-------  Transport receive functions ----------------------

 node_msg process_query(node_msg *msg)
    {   int this_id=(int)id();
        char *value_found;
        node_msg ans={{this_id,(char *)this},0,0,NULL};
        //node_data_item
                answer_item.Key=0; answer_item.Value="OK";    //OK by default

        switch(msg->command){
        case PING:
            if(msg->dst==this_id){
                ans.src.src_id=this_id;
                ans.src.src_ip=(char *)this;
                ans.dst=msg->src.src_id;
                ans.value=&answer_item;
            }
            update_HT(msg->src.src_id,msg->src.src_ip,127);
            break;

        case STORE:
            if(msg->dst==this_id){
                //store value
                node_data_item *store_item=new node_data_item(msg->value->key(),msg->value->value());
                node_data.insert(store_item);

                //answer ok
                ans.src.src_id=this_id;
                ans.src.src_ip=(char *)this;
                ans.dst=msg->src.src_id;
                ans.value=&answer_item;
            }
            break;

        case FIND_NODE:

            if (msg->dst==this_id){

                if(Find_node(msg->value->key(),K_items)) {answer_item.Value=(char *)K_items; answer_item.Key=msg->command;}
                else answer_item.Value="NOT_FOUND";

                //answer ok
                ans.src.src_id=this_id;
                ans.src.src_ip=(char *)this;
                ans.dst=msg->src.src_id;

                ans.value=&answer_item;
               }

            break;

        case FIND_VALUE:

            if (msg->dst==this_id){
                int fvalue=Find_value(msg->value->key(),K_items,&value_found);
                if(fvalue>0) {answer_item.Value=(char *)K_items;answer_item.Key=msg->command;}
                else if(fvalue<0) {answer_item.Value=value_found;answer_item.Key=msg->value->key();}
                else answer_item.Value="NOT_FOUND";

                //answer ok
                ans.src.src_id=this_id;
                ans.src.src_ip=(char *)this;
                ans.dst=msg->src.src_id;

                ans.value=&answer_item;
               }

            break;

        default:
            break;
        }

        return ans;
    }

//--------------- DHT functions----------------
    int Store(node_data_item *v){   //STORE
        node_data.insert(v);
        return 1;
    }

    int Ping(){
        return 1;
    }

    int Find_node(Key v, Item *t){  //get k_size closest nodes from buckets
        int N_nodes=0;
        int index_list[M];
        for (uint8_t i=0; i<M;index_list[i++]=0);

        index_list[0]=hash(v,M);

        for (int i=1, r=1, j=0, k=0; i<M;r++){
            j=index_list[0]+r;  //right edge
            k=index_list[0]-r;  //left edge

            if (k>=0)index_list[i++]=k;
            if (j<M) index_list[i++]=j;
            }

//        for (int i=0; i<M;i++) {
//            cout << "index_list["<<i<<"] = " << index_list[i] << "\n";
//        }

        for (int i=0; i<M;i++){
        int cur_bucket=index_list[i];
        link cur_node=heads[cur_bucket];

        while (cur_node) {
            t[N_nodes++]=*(cur_node->item);
            //cout << "Node " << N_nodes << ": id = " << cur_node->item->key() << " ;\n";
            cur_node=cur_node->next;
            if (N_nodes==k_size) return N_nodes;
        }
        }

        return N_nodes;
    }

    int Find_value(Key v, Item *t,char **value_found){  //node_data_item
        //first check value
        node_data_item *node_data_search=node_data.search(v);
        if (node_data_search) {*(value_found)=node_data_search->value();
                return -1;
        }

        //if value not found check k_size closest nodes
        int N_nodes=0;
        int index_list[M];
        for (uint8_t i=0; i<M;index_list[i++]=0);

        index_list[0]=hash(v,M);

        for (int i=1, r=1, j=0, k=0; i<M;r++){
            j=index_list[0]+r;  //right edge
            k=index_list[0]-r;  //left edge

            if (k>=0)index_list[i++]=k;
            if (j<M) index_list[i++]=j;
            }

//        for (int i=0; i<M;i++) {
//            cout << "index_list["<<i<<"] = " << index_list[i] << "\n";
//        }

        for (int i=0; i<M;i++){
        int cur_bucket=index_list[i];
        link cur_node=heads[cur_bucket];

        while (cur_node) {
            t[N_nodes++]=*(cur_node->item);
            cout << "Node " << N_nodes << ": id = " << cur_node->item->key() << " ;\n";
            cur_node=cur_node->next;
            if (N_nodes==k_size) return N_nodes;
        }
        }

        return N_nodes;
    }

};

#endif // ST_LIST_H
