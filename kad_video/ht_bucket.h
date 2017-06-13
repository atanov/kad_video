#ifndef HT_BUCKET_H
#define HT_BUCKET_H

#include <QUdpSocket>
#include <QString>
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
        Item myself;
        //service variables
        Item *K_items;   //node items array
        node_data_item answer_item;
        node_data_item find_node_item;
        QUdpSocket *udp;


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

    HT(int m_ID, char *m_ip, int m_port, QUdpSocket *m_udp):node_data(100),s(100){
        //node_data.node_data(100);  //init node data structure
        K_items=new Item[k_size];
        udp=m_udp;

        M=ID_size;
        heads = new link[M];
        myself.ID=m_ID;
        myself.ip=new char[80];
        strcpy(myself.ip,m_ip);
        myself.Udp_port=m_port;

        node_ID=myself.key();

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
    QByteArray build_query(node_msg *msg)
    {
        QByteArray answer;
        switch (msg->command){
        case PING: answer.append("PING "); break;
        case STORE: answer.append("STORE "); break;
        case FIND_NODE: answer.append("FIND_NODE "); break;
        case FIND_VALUE: answer.append("FIND_VALUE "); break;
        }

        answer.append(QString::number(msg->dst)+' ');
        answer.append(QString::number(msg->src.src_id)+' ');
        answer.append(QByteArray(msg->src.src_ip)+' ');
        answer.append(QString::number(msg->src.src_port)+' ');
        answer.append(QString::number(msg->value->Key)+' ');
        answer.append(QByteArray(msg->value->Value)+' ');

        return answer;
    }

    int process_answer(char *d,node_msg *msg_rec) {
        QString data(d);
        QString left,right;

        int space=data.indexOf(' ');
        if (space!=-1){
        left=data.left(space);
        right=data.mid(space+1);

         if(left.contains("ANSWER")) {
             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             msg_rec->src.src_id=left.toInt();

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             strcpy(msg_rec->src.src_ip,(left.toStdString()).c_str());

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             msg_rec->src.src_port=left.toInt();

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             msg_rec->dst=left.toInt();

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             msg_rec->command=left.toInt();

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             msg_rec->value->Key=left.toInt();

             data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
             strcpy(msg_rec->value->Value,(left.toStdString()).c_str());

         cout <<" ___ "<< d << " received and processed\n";
         return 1;
         }
    }
       return 0;
  }

     int node_sender(int c, node_data_item *v, int rec_id, int *list_size, bucket_item **nodes_list){

        bucket_item *search_rec = search_item(rec_id);
        //node_hash_table *rec;

        if (!search_rec) //rec=(node_hash_table *)search_rec->value();    //get link by id from buckets
          //  else
        {
            cout << "command "<< c <<" error, node " << rec_id <<" has not found\n";
            return 0;
        }

//////////////  replace here for UDP transport
        node_msg msg_send={{id(),myself.ip,myself.Udp_port},search_rec->key(),c,v};
        udp->writeDatagram(build_query(&msg_send) ,QHostAddress(QString(search_rec->value())), search_rec->udp_port());
        ////node_msg msg_rec=rec->process_query(&msg_send);  //send msg to selected node in DHT

        QByteArray buffer;
        QHostAddress sender;
        quint16 senderPort;
        node_msg msg_rec;
        node_data_item msg_rec_value;
        char srcip[80];
        char msgrecvalue[80];
        msg_rec.src.src_ip=srcip;   //char * initialization
        msg_rec_value.Value=msgrecvalue;
        msg_rec.value=&msg_rec_value;

        QString kdata_answer;
        QString left,right;
        int space; int udp_port;
        char cur_ip[80];

        while(1)  //get messages from UDP queue until answer or exit on timeout
        {         //now without timeout;
            if (udp->hasPendingDatagrams()){
                buffer.resize(udp->pendingDatagramSize());
                udp->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);

                if (process_answer(buffer.data(),&msg_rec)) break;
            }
        }

//////////////


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

                //parse string here
                kdata_answer=QString(msg_rec.value->Value);
                space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                id=left.toInt();

                i=0;
                while((id>=0)&&(i<k_size))
                {
                    kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                    strcpy(cur_ip,(left.toStdString()).c_str());

                    kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                    udp_port=left.toInt();

                    cout << "id = " << id <<"; value = " <<  cur_ip <<"\n";  //(((bucket_item *)msg_rec.value->value())[i]).value()

                    K_items[i].ID=id; strcpy(K_items[i].ip,cur_ip);K_items[i].Udp_port=udp_port;
                    i++;

                    kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                    id=left.toInt();

                }

                *list_size=i;
                *nodes_list=K_items;

            return 1;
            break;

        case FIND_VALUE:
            if (!strcmp(msg_rec.value->value(),"NOT_FOUND")){return 0;}
            if (msg_rec.value->key()==v->key()) {
                    cout << "Found id = " << msg_rec.value->key() << "; value = " << msg_rec.value->value() << " \n";

                    //*******  DNAGER !!! *************************
                    K_items[0].ID=msg_rec.value->key();   //return data item as k_bucket item witrh ip=value;
                    K_items[0].ip=msg_rec.value->value();
                    //************************************************
                    *list_size=777;
                    *nodes_list=K_items;
                    return 1;}
            else {cout << "Closest nodes for id = " << msg_send.value->key() << ":\n";
                cout << "Node id= " << rec_id <<".Closest nodes for id = " << msg_send.value->key() << ":\n";

                    //parse string here
                    kdata_answer=QString(msg_rec.value->Value);
                    space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                    id=left.toInt();

                    i=0;
                    while((id>=0)&&(i<k_size))
                    {
                        kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                        strcpy(cur_ip,(left.toStdString()).c_str());

                        kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                        udp_port=left.toInt();

                        cout << "id = " << id <<"; value = " <<  cur_ip <<"\n";  //(((bucket_item *)msg_rec.value->value())[i]).value()

                        K_items[i].ID=id; strcpy(K_items[i].ip,cur_ip);K_items[i].Udp_port=udp_port;
                        i++;

                        kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                        id=left.toInt();
                    }

                    *list_size=i;
                    *nodes_list=K_items;
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


         if (node_sender(PING,&answer_item,cur_id,NULL,NULL)) cout << "PING OK\n";
         else {cout <<"PING FAILED\n";return NULL;}

         //find_node
         int list_size;
         bucket_item *nodes_list;

         find_node_item.Key=id_to_found; find_node_item.Value="FIND_NODE";    //find node with id=id_to_found;
         if(node_sender(FIND_NODE,&find_node_item,cur_id,&list_size,&(nodes_list))) cout << "FIND NODE OK\n";
         else {cout << "FIND NODE FAILED\n"; return NULL;}

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

            if (cur_link->key()==id()) continue;  //loop protect
            insertF(cur_link);
            //find metric
             metr[i].metr=metric((cur_link->key()),id_to_found); metr[i].id=cur_link->key();
             if (metr[i].metr==0) {stop_processing=1;cout << id_to_found << " is found\n";}
            }

         if (stop_processing) {while(!s.empty()) s.pop();return cur_link;}  //flush stack & exit

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


    bucket_item *process_stack_value(int id_to_found){
        while(!(s.empty())){
         int cur_id=s.pop();


         if (node_sender(PING,&answer_item,cur_id,NULL,NULL)) cout << "PING OK\n";
         else {cout <<"PING FAILED\n";return NULL;}

         //find_node
         int list_size;
         bucket_item *nodes_list;

         find_node_item.Key=id_to_found; find_node_item.Value="FIND_VALUE";    //find node with id=id_to_found;
         if(node_sender(FIND_VALUE,&find_node_item,cur_id,&list_size,&(nodes_list))) cout << "FIND VALUE OK\n";
         else {cout << "FIND VALUE FAILED\n"; return NULL;}

         if (list_size==777) {cout << id_to_found << " is found\n"; while(!s.empty()) s.pop();return nodes_list;}   //flush stack & exit

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


            if (cur_link->key()==id()) continue;  //loop protect
            insertF(cur_link);
            //find metric
             metr[i].metr=metric((cur_link->key()),id_to_found); metr[i].id=cur_link->key();
           }

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


    int node_search(int first_query, int id_to_found)
    {
        s.push(first_query);
        if(process_stack(id_to_found)==NULL) return 0;
        else return 1;
    }

    int value_search(int first_query, int id_to_found)
    {
        s.push(first_query);
        if(process_stack_value(id_to_found)==NULL) return 0;
        else return 1;
    }


//-------  Transport receive functions ----------------------

 node_msg process_query(node_msg *msg)
    {   int this_id=(int)id();
        char *value_found;
        char k_items_buf[255];
        node_msg ans={{this_id,(char *)this,0},0,0,NULL};
        //node_data_item
                answer_item.Key=0; strcpy(answer_item.Value,"OK");    //OK by default

        switch(msg->command){
        case PING:
            if(msg->dst==this_id){
                ans.src.src_id=myself.ID;
                //ans.src.src_ip=(char *)this;
                ans.src.src_ip=myself.ip;
                ans.src.src_port=myself.Udp_port;

                ans.dst=msg->src.src_id;
                ans.value=&answer_item;
            }
            update_HT(msg->src.src_id,msg->src.src_ip,msg->src.src_port);
            break;

        case STORE:
            if(msg->dst==this_id){
                //store value
                node_data_item *store_item=new node_data_item(msg->value->key(),msg->value->value());
                node_data.insert(store_item);

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                ans.src.src_ip=myself.ip;
                ans.src.src_port=myself.Udp_port;

                ans.dst=msg->src.src_id;
                ans.value=&answer_item;
            }
            break;

        case FIND_NODE:

            if (msg->dst==this_id){

                if(Find_node(msg->value->key(),K_items)) {
                    strcpy(answer_item.Value,"");
                    for (int i=0; i<k_size;i++){
                    strcat(answer_item.Value,itoa((K_items[i]).ID,k_items_buf,10)); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,(K_items[i]).ip); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,itoa((K_items[i]).Udp_port,k_items_buf,10)); strcat(answer_item.Value,";");
                    }

                answer_item.Key=msg->command;}   // replace here (char *)K_items -> (char *)str="K_items[0],....,K_items[1]"; replace processor
                else strcpy(answer_item.Value,"NOT_FOUND");

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                ans.src.src_ip=myself.ip;
                ans.src.src_port=myself.Udp_port;

                ans.dst=msg->src.src_id;

                ans.value=&answer_item;
               }

            break;

        case FIND_VALUE:

            if (msg->dst==this_id){
                int fvalue=Find_value(msg->value->key(),K_items,&value_found);
                if(fvalue>0) {
                    strcpy(answer_item.Value,"");
                    for (int i=0; i<k_size;i++){
                    strcat(answer_item.Value,itoa((K_items[i]).ID,k_items_buf,10)); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,(K_items[i]).ip); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,itoa((K_items[i]).Udp_port,k_items_buf,10)); strcat(answer_item.Value,";");
                    }//answer_item.Value=(char *)K_items;
                    answer_item.Key=msg->command;}
                else if(fvalue<0) {answer_item.Value=value_found;answer_item.Key=msg->value->key();}
                else answer_item.Value="NOT_FOUND";

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                ans.src.src_ip=myself.ip;
                ans.src.src_port=myself.Udp_port;

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
