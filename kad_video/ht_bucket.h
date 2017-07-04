#ifndef HT_BUCKET_H
#define HT_BUCKET_H

#include <QUdpSocket>
#include <QString>
#include <QFile>
#include <iostream>
#include <st_basic.h>
#include <my_stack.h>
#include <my_fifo.h>
#include <ht_items.h>
#include <my_sort.h>
#include <QTime>
#include <mutex>
#include <unistd.h>

#define DEBUG_OUTPUT
#define TIME_OUT 50   //in ms
#define SLEEP_TIME 500  //in us
#define CHECK_FOR_UPDATES
const size_t UDP_PSIZE=50000;
typedef my_fifo<QByteArray> udp_fifo;
extern mutex fifo_mutex;

using namespace std;
//QTime t;
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
        QTime *send_time;
        int node_connected_id;

        static int save_to_disk(char *data, int size, int key);
        static void process_command(char *d, node_hash_table *t);
        static int process_message(char *d,node_msg *msg_send);

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
        /*if (!t) return NULL;
        if (t->item->key() == v) return t->item;
        return searchR_item(t->next, v);*/
        link cur=t;
        while(cur!=NULL) {
            if (cur->item->key() == v) return cur->item;
            cur=cur->next;
        }
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

        if (v==t) {
                    #ifdef DEBUG_OUTPUT
            cout << "already on the top\n";return;  //already on top
#endif
    }
        else {while ((t->next)!=v) t=t->next;
            t->next=v->next;   //del from current pos
            v->next=h;   //and place it before head
            heads[hash(k,M)]=v;//update head
       }
    }

public:

    HT(int m_ID, const char *m_ip, int m_port, QUdpSocket *m_udp, udp_fifo *m_fifo):node_data(100),s(100){
        //node_data.node_data(100);  //init node data structure
        K_items=new Item[k_size];
        udp=m_udp;
        fifo=m_fifo;

        M=ID_size;
        heads = new link[M];
        myself.ID=m_ID;
        //myself.ip=new char[80];
        strcpy(myself.ip,m_ip);
        myself.Udp_port=m_port;
        send_time= new QTime;

        node_ID=myself.key();
        node_connected_id=-1;

        for (uint i=0; i<M;i++) heads[i]=NULL;
        for (uint i=0;i<ID_size;i++) N[i] = 0;

    }

    int last_dst;
    udp_fifo *fifo;
    int count(){ return N; }
    //Key
    int id(){return (int) node_ID;}
    int my_port(){return myself.Udp_port;}
    char *my_ip() {return myself.ip;}

    void node_data_print(){
        node_data.print();
    }
    node_data_item *search_data(int v)
    {
        return node_data.search(v);
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

    int insertF(Item *x){
        #ifdef DEBUG_OUTPUT
        cout << "insertF: " << x->key() <<"; ";
#endif
        return update_HT(x->key(),x->value(),x->udp_port());
    }

    int update_HT(Key k,char *ip,int ip_port){
        node *found = search(k);
        Item *found_item;
        if (found) {
            #ifdef DEBUG_OUTPUT
            cout << "founded " << found->item->key() << ";\n";
#endif

            move_to_top(found); return 1;
        }
        else {//cout << "not founded " << k << ";\n";
        found_item=new Item(k,ip,ip_port);
        insert(found_item);
        return 0;
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

    int process_answer(char *d,int size,node_msg *msg_rec) {

        QByteArray data(d,size);
        QByteArray left,right;

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

             ////!!!!!!!!!  check size first !!!!!!!
             //cout << "size = " << right.size() << endl;

             data=right; //if(right.size()>1024)
             {msg_rec->value->Value=new char[right.size()];}  //create new buffer every new message
             msg_rec->value->size=right.size();
             if (right.size()==50001)
                 cout << "right.size() = "<<right.size() << endl;

            // t.restart();
             memcpy(msg_rec->value->Value,right.toStdString().c_str(), msg_rec->value->size);
            // qDebug() << "time for process answer exchange " << t.elapsed() <<endl;


             //else strcpy(msg_rec->value->Value,(right.toStdString()).c_str());

#ifdef DEBUG_OUTPUT
 cout <<" ___ " << msg_rec->src.src_id << " received and processed\n";
 //cout <<" ___ " << d << " received and processed\n";
#endif

          return 1;
         }
    }
       return 0;
  }

     int node_sender(int c, node_data_item *v, int rec_id, int *list_size, bucket_item **nodes_list){
        //t.restart();
        bucket_item *search_rec = search_item(rec_id);

        if (!search_rec)     //get link by id from buckets
          {
            cout << "command "<< c <<" error, node " << rec_id <<" has not found\n";
            return 0;
        }

//////////////  MESSAGE SEND ////////////////////////////////////////
        node_msg msg_send;
        msg_send.src.src_id=id();
        strcpy(msg_send.src.src_ip,myself.ip);
        msg_send.src.src_port=myself.Udp_port;
        msg_send.dst=search_rec->key();
        msg_send.command=c;
        msg_send.value=v;
        udp->writeDatagram(build_query(&msg_send) ,QHostAddress(QString(search_rec->value())), search_rec->udp_port());
        ////node_msg msg_rec=rec->process_query(&msg_send);  //send msg to selected node in DHT


/////////////////////////////////////////  MESSAGE RECIEVE ////////////////////////////////
        /*QByteArray buffer;
        node_msg msg_rec;

        node_data_item msg_rec_value;
        msg_rec.value=&msg_rec_value;

        while(1)  //get messages from UDP queue until answer or exit on timeout
        {         //now without timeout;
            buffer=check_for_answer();
            if (buffer==NULL) return 0;    //break on time-out
            // t.restart();
            if (process_answer(buffer.data(),buffer.size(),&msg_rec)) break;
        }
         //qDebug() << "time for UDP exchange " << t.elapsed() <<endl;*/
        node_msg msg_rec;

        node_data_item msg_rec_value;
        msg_rec.value=&msg_rec_value;

        msg_rec=udp_listen(1,msg_send.command);   // 1 = from_stack;
   //////////////
           if (msg_rec.command==-1) return 0; //timout, try another time;
           if (msg_rec.command!=msg_send.command) return 0; //garbage answer, return;

        QString kdata_answer;
        QString left,right;
        int space; int udp_port;
        char cur_ip[80];
        int i,id;
      //char *c_answer=msg_rec.value->value();

        //construct c_str from short answers
        char c_answer[10];
        memset(c_answer,0,10);
        if ((msg_rec.value->size)<10) memcpy (c_answer,msg_rec.value->value(),msg_rec.value->size);
        //analyze answer
        switch (c)
        {
        case PING:
            if (!strcmp(c_answer,"OK")){return 1;}
            break;

        case STORE:
        if (!strcmp(c_answer,"OK")){return 1;}
            break;

        case FIND_NODE:
        if (!strcmp(c_answer,"NOT_FOUND")){return 0;}

        #ifdef DEBUG_OUTPUT
        cout << "Node id= " << rec_id << endl;//".Closest nodes for id = " << msg_send.value->key() << ":\n";
        #endif

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
#ifdef DEBUG_OUTPUT
                    cout << "id = " << id <<"; value = " <<  cur_ip <<"\n";  //(((bucket_item *)msg_rec.value->value())[i]).value()
#endif
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
            //t.restart();
            //!!!!!!!!!  first check if value is returned
            if (msg_rec.value->key()==v->key()) {

            last_dst=msg_rec.src.src_id;
#ifdef DEBUG_OUTPUT
                cout << "Found id = " << msg_rec.value->key() << "; value = " <<"\n";// msg_rec.value->value() << " \n";
#endif

                    //*******  DANGER !!! *************************
                    K_items[0].ID=msg_rec.value->key();   //return data item as k_bucket item with ip=value;
                    K_items[0].ip=msg_rec.value->value();
                    K_items[0].Udp_port=msg_rec.value->size;

                    //************************************************
                    *list_size=777;
                    *nodes_list=K_items;
                    //qDebug () << "time for FIND_VALUE processing " << t.elapsed() << endl;
                    return 1;}
            else if (!strcmp(c_answer,"NOT_FOUND")){return 0;}
            else {
                #ifdef DEBUG_OUTPUT
                 cout << "Node id= " << rec_id << endl; //".Closest nodes for id = " << msg_send.value->key() << ":\n";
#endif
                    //parse string here
                    kdata_answer=QString(msg_rec.value->Value);
                    space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                    id=left.toInt();

                    i=0;
                    while((id>=0)&&(i<k_size))
                    {
                        kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                        strcpy(cur_ip,(left.toStdString()).c_str());
                        if ((cur_ip[0]=='O')&(cur_ip[1]=='K'))
                            {cout << "garbage collected\n"; return 0;}   // garbage answer

                        kdata_answer=right; space=kdata_answer.indexOf(';'); left=kdata_answer.left(space); right=kdata_answer.mid(space+1);
                        udp_port=left.toInt();
#ifdef DEBUG_OUTPUT
                        cout << "id = " << id <<"; value = " <<  cur_ip <<"\n";  //(((bucket_item *)msg_rec.value->value())[i]).value()
#endif
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
        asked_list asked;
        asked.insert(id());

        while(!(s.empty())){
         int cur_id=s.pop();


         if (node_sender(PING,&answer_item,cur_id,NULL,NULL)) {
          #ifdef DEBUG_OUTPUT
             cout << "PING OK\n";
#endif
         }
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

              //if (cur_link->key()==id()) continue;  //loop protect
            if (asked.search(cur_link->key())) continue;  //loop protect
              //if(insertF(cur_link)) continue;  //loop protect

            insertF(cur_link);
            asked.insert(cur_link->key());
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
            asked_list asked;
            asked.insert(id());
            while(!(s.empty())){
             int cur_id=s.pop();

             //t.restart();
             int k=0;
             for (k=0; k<3;k++) {
             if (node_sender(PING,&answer_item,cur_id,NULL,NULL)) {
    #ifdef DEBUG_OUTPUT
                 cout << "PING to:"<< cur_id <<" OK\n";
                 break;
             #endif
             }
             }

             //else {cout <<"PING FAILED\n";return NULL;}
             if (k==3) {cout <<"PING FAILED\n";continue;}
            // qDebug("Time elapsed: %d ms", t.elapsed());

             //find_node
             //t.restart();
             int list_size;
             bucket_item *nodes_list;

             find_node_item.Key=id_to_found; find_node_item.Value="FIND_VALUE";    //find node with id=id_to_found;
         for (k=0; k<3; k++){
             if(node_sender(FIND_VALUE,&find_node_item,cur_id,&list_size,&(nodes_list)))
             {
                 #ifdef DEBUG_OUTPUT
                 cout << "FIND VALUE OK\n";
    #endif
             break;
             }
         }
             //else {cout << "FIND VALUE FAILED\n"; return NULL;}
            if (k==3) {cout << "FIND VALUE FAILED\n"; continue;}

             if (list_size==777) {//qDebug("Time elapsed: %d ms", t.elapsed());
                 cout << id_to_found << " is found\n"; while(!s.empty()) s.pop();return nodes_list;}   //flush stack & exit

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

                //if (cur_link->key()==id()) continue;  //loop protect
              if (asked.search(cur_link->key())) continue;  //loop protect
                //if(insertF(cur_link)) continue;  //loop protect

              insertF(cur_link);
              asked.insert(cur_link->key());
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

    Item *value_search(int first_query, int id_to_found)
    {
        s.push(first_query);
        Item *find_value=process_stack_value(id_to_found);
        return find_value;
    }

    //-------  Transport receive functions ----------------------

 node_msg process_query(node_msg *msg)
    {   int this_id=(int)id();
        char *value_found;
        char k_items_buf[255];
        node_msg ans;//={{this_id,(char *)this,0},0,0,NULL};
        //node_data_item
                answer_item.Key=0;
                answer_item.Value=answer_item.backup;    //switch back pointer
                                                         //from value in datastore to default answer value
                strcpy(answer_item.Value,"OK");    //OK by default
                answer_item.size=0;
                ans.command=msg->command;

        switch(msg->command){
        case PING:
            if(msg->dst==this_id){
                ans.src.src_id=myself.ID;
                //ans.src.src_ip=(char *)this;
                strcpy(ans.src.src_ip,myself.ip);
                ans.src.src_port=myself.Udp_port;

                ans.dst=msg->src.src_id;
                ans.value=&answer_item;
            }
            update_HT(msg->src.src_id,msg->src.src_ip,msg->src.src_port);
            break;

        case STORE:
            if(msg->dst==this_id){
                //store value
                node_data_item *store_item=new node_data_item(msg->value->key(),msg->value->value(),
                                                              msg->value->size);
                node_data.insert(store_item);

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                strcpy(ans.src.src_ip,myself.ip);
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
                    sprintf(k_items_buf,"%d",(K_items[i]).ID);
                    //strcat(answer_item.Value,itoa((K_items[i]).ID,k_items_buf,10)); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,k_items_buf); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,(K_items[i]).ip); strcat(answer_item.Value,";");
                    sprintf(k_items_buf,"%d",(K_items[i]).Udp_port);
                    //strcat(answer_item.Value,itoa((K_items[i]).Udp_port,k_items_buf,10)); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,k_items_buf); strcat(answer_item.Value,";");
                    }

                answer_item.Key=msg->command;}   // replace here (char *)K_items -> (char *)str="K_items[0],....,K_items[1]"; replace processor
                else strcpy(answer_item.Value,"NOT_FOUND");

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                strcpy(ans.src.src_ip,myself.ip);
                ans.src.src_port=myself.Udp_port;

                ans.dst=msg->src.src_id;

                ans.value=&answer_item;
               }

            break;

        case FIND_VALUE:

            if (msg->dst==this_id){
                int value_size;
                int fvalue=Find_value(msg->value->key(),K_items,&value_found,&value_size,msg->src.src_id);
                //QByteArray debug_data(value_found,value_size);
                if(fvalue>0) {
                    strcpy(answer_item.Value,"");
                    for (int i=0; i<k_size;i++){
                    sprintf(k_items_buf,"%d",(K_items[i]).ID);
                    //strcat(answer_item.Value,itoa((K_items[i]).ID,k_items_buf,10)); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,k_items_buf); strcat(answer_item.Value,";");
                    strcat(answer_item.Value,(K_items[i]).ip); strcat(answer_item.Value,";");
                    sprintf(k_items_buf,"%d",(K_items[i]).Udp_port);
                    strcat(answer_item.Value,k_items_buf); strcat(answer_item.Value,";");
                    }//answer_item.Value=(char *)K_items;
                    answer_item.Key=msg->command;}
                else if(fvalue<0) {answer_item.Value=value_found;answer_item.Key=msg->value->key(); answer_item.size=value_size;}
                else answer_item.Value="NOT_FOUND";

                //answer ok
                ans.src.src_id=this_id;
                //ans.src.src_ip=(char *)this;
                strcpy(ans.src.src_ip,myself.ip);
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

    int Find_value(Key v, Item *t,char **value_found, int *vsize, int src_id){  //node_data_item
        //first check value
        if((node_connected_id==src_id)|(node_connected_id==-1)){
           node_data_item *node_data_search=node_data.search(v);
       // QByteArray debug_data(node_data_search->Value,node_data_search->size);
        if (node_data_search) {*(value_found)=node_data_search->value();
                               *(vsize)=node_data_search->size;
                                node_connected_id=src_id;    //only if have found
                                return -1;
                              }
        }
        else{
#ifdef DEBUG_OUTPUT
            cout << "already connected" << endl;
#endif
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
            #ifdef DEBUG_OUTPUT
            cout << "Node " << N_nodes << ": id = " << cur_node->item->key() << " ;\n";
#endif
            cur_node=cur_node->next;
            if (N_nodes==k_size) return N_nodes;
        }
        }

        return N_nodes;
    }

    node_msg udp_listen (int from_where, int send_command=0){

        QByteArray buffer;

        node_msg msg_rec;
        msg_rec.command=-2;  // if shit happened;
        node_msg msg_send;

    if (from_where == 1){  //from stack
        send_time->start();
        while (1)
        {
            if (send_time->elapsed()>TIME_OUT) {
                cout << "node_sender timeout, next node \n"; msg_rec.command=-1;return msg_rec;}
            if (fifo->empty()) {continue;usleep(1000);}
            fifo_mutex.lock();
            buffer=fifo->get();
            fifo_mutex.unlock();

            if (process_answer(buffer.data(),buffer.size(),&msg_rec))
                            {if (msg_rec.command==send_command)return msg_rec; else continue;}  //if answer

            if(process_message(buffer.data(),&msg_send)) {  //if another DHT command
             msg_rec=process_query(&msg_send);
             udp->writeDatagram(build_answer(&msg_rec) ,QHostAddress(QString(msg_send.src.src_ip)), msg_send.src.src_port);
        #ifdef DEBUG_OUTPUT
             cout << "got from stack DHT query " << msg_rec.command << " ; from   id = " << msg_send.src.src_id << " to id = " << msg_rec.src.src_id << endl;
        #endif
             }

       }
      }

    else if (from_where==2) {   //from main loop
        //if (fifo->empty()) {msg_rec.command=-1; return msg_rec;}
        while(fifo->empty()) {usleep(1000);}  //this_thread::sleep_for(chrono::microseconds(SLEEP_TIME));
        fifo_mutex.lock();
        buffer=fifo->get();
        fifo_mutex.unlock();
         if(process_message(buffer.data(),&msg_send)) {  //DHT command
         msg_rec=process_query(&msg_send);
         udp->writeDatagram(build_answer(&msg_rec) ,QHostAddress(QString(msg_send.src.src_ip)), msg_send.src.src_port);
    #ifdef DEBUG_OUTPUT
          cout << "got DHT query " << msg_send.command << " ; src = " << msg_send.src.src_id << endl;
    #endif
         }

         else {     //console command
    #ifdef DEBUG_OUTPUT
          cout << "got command \n" << buffer.data() << endl;
    #endif

            process_command(buffer.data(),this);  //or process command
        msg_rec.command=-1;
        }
        return msg_rec;
       }
    msg_rec.command=-1;
    return msg_rec;
    }

QByteArray build_answer(node_msg *msg)
{
    QByteArray answer;
    answer.append("ANSWER ");
    answer.append(QString::number(msg->src.src_id)+' ');
    answer.append(QByteArray(msg->src.src_ip)+' ');
    answer.append(QString::number(msg->src.src_port)+' ');
    answer.append(QString::number(msg->dst)+' ');
    answer.append(QString::number(msg->command)+' ');
    answer.append(QString::number(msg->value->Key)+' ');
    if (msg->value->size!=0) answer.append(QByteArray(msg->value->Value,msg->value->size));
    else answer.append(QByteArray(msg->value->Value));

    return answer;
}

};

#endif // ST_LIST_H
