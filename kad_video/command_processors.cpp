#include <ht_bucket.h>

template <class Item, typename Key> int HT<Item,Key>::save_to_disk(char *data, int size, int key)
    {
        char *filename= new char[80];
        strcpy(filename,"C:\\projects\\p2p_video\\output\\chunk-stream0-");
        char num[5];
        sprintf(num,"%05i", key);
        strcat(filename,num);
        strcat(filename,".m4s");

        QFile f(filename);
        if (f.open(QIODevice::WriteOnly)) cout << filename <<" file opened succesfully\n";
        else {cout << filename<<" file opening error\n"; return 0;}
        delete filename;

        if (f.write(data,size)>0) {
#ifdef DEBUG_OUTPUT
            cout << "write success\n";
#endif
            f.close();
            return 1;
        }
        else return 0;
        f.close();
        return 0;
}


template <class Item, typename Key> void HT<Item,Key>::process_command(char *d, node_hash_table *t){
QString data(d);
QString left,right;

if(data.contains("PRINT_DATA")) {t->node_data_print();}

if(data.contains("ITEM")) {
    int v;
    int space=data.indexOf(' ');
    if (space!=-1){
    v=(data.mid(space+1)).toInt();
    node_data_item *s_v=t->search_data(v);
    if (s_v) cout << s_v->Key << " " << s_v->size << endl;
    }
}

if(data.contains("PRINT_NODE")) {t->print();}

if(data.contains("SEARCH")) {
    int dst, v;
    int space=data.indexOf(' ');
    if (space!=-1){
    left=data.left(space);
    right=data.mid(space+1);

    data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
    dst=left.toInt();
    v=right.toInt();

    if(!(t->node_search(dst,v))) cout << "SEARCH FAILED";
    }
}
if(data.contains("FIND_DATA")) {
    int dst, v;
    int space=data.indexOf(' ');
    if (space!=-1){
    left=data.left(space);
    right=data.mid(space+1);

    data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
    dst=left.toInt();
    v=right.toInt();

    if(!(t->value_search(dst,v))) cout << "SEARCH FAILED";}
}
if(data.contains("PLAY")) {
QTime t_total;

    int dst;
    int space=data.indexOf(' ');
    if (space!=-1){
    left=data.left(space);
    right=data.mid(space+1);
    dst=right.toInt();



    for (int i=1;i<1105;i++) {   //number of files from manifest
        t_total.start();
        bucket_item *cur_item=t->value_search(dst,i*100);  //get first packet from file 1

            if(cur_item==NULL) {cout <<"SEARCH FAILED"; return;}
             cout << "searched for " << t_total.elapsed() << endl;

        //       store packet in local data repository
             node_msg msg_send;
             node_data_item msg_send_value;
             msg_send.value=&msg_send_value;

               strcpy(msg_send.src.src_ip,"127.0.0.1");
               msg_send.value->Value=cur_item->ip;   //= value , see node_sender DANGER
               msg_send.value->Key=cur_item->ID;   //   = key
               msg_send.value->size=cur_item->Udp_port;   // = size, here size !=0, in ordinary STORE size=0;

               msg_send.command=STORE;
               msg_send.src.src_id=t->id();
               msg_send.src.src_port=t->my_port();
               msg_send.dst=t->id();
               t->process_query(&msg_send);

        //--------------------------------

        int pack_num = (int)cur_item->ip[1];  //save first packet, get value from header
        char *cur_file=(char *)malloc(pack_num*UDP_PSIZE);
        memcpy (cur_file, cur_item->ip +2,UDP_PSIZE);
        size_t cur_file_size=UDP_PSIZE;

        for (int j=1; j< pack_num; j++){
        t_total.start();
        cur_item=t->value_search(dst,i*100+j);
        if(cur_item==NULL) {cout << "SEARCH FAILED";return;}
        //------------------       store packet in local data repository
             node_msg msg_send;
             node_data_item msg_send_value;
             msg_send.value=&msg_send_value;

               strcpy(msg_send.src.src_ip,"127.0.0.1");
               msg_send.value->Value=cur_item->ip;   //= value , see node_sender DANGER
               msg_send.value->Key=cur_item->ID;   //   = key
               msg_send.value->size=cur_item->Udp_port;   // = size, here size !=0, in ordinary STORE size=0;

               msg_send.command=STORE;
               msg_send.src.src_id=t->id();
               msg_send.src.src_port=t->my_port();
               msg_send.dst=t->id();
               t->process_query(&msg_send);

        //--------------------------------
        memcpy (cur_file+cur_file_size, cur_item->ip +2,cur_item->Udp_port-2);
        cur_file_size+=cur_item->Udp_port-2;
        cout << "searched for " << t_total.elapsed() << endl;

#ifdef DEBUG_OUTPUT
        cout << "pack size = " << cur_item->Udp_port-2 << endl;
        cout << "pack num = " << j << endl;
#endif
        }
        HT::save_to_disk (cur_file,cur_file_size,i);
    }
   }
  }
}

template <class Item, typename Key> int HT<Item,Key>::process_message(char *d,node_msg *msg_send)
{
    QString data(d);
    QString left,right;

    int space=data.indexOf(' ');
    if (space!=-1){
    left=data.left(space);
    right=data.mid(space+1);

     if(left.contains("PING")) {
        msg_send->command=PING;

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->dst=left.toInt();

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->src.src_id=left.toInt();

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        strcpy(msg_send->src.src_ip,(left.toStdString()).c_str());

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->src.src_port=left.toInt();
#ifdef DEBUG_OUTPUT
        cout  <<"PING " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                    msg_send->src.src_port << endl;
#endif
        return 1;
     }


    if(left.contains("FIND_NODE"))
    {
        msg_send->command=FIND_NODE;
        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->dst=left.toInt();

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->src.src_id=left.toInt();

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        strcpy(msg_send->src.src_ip,(left.toStdString()).c_str());

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->src.src_port=left.toInt();

        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
        msg_send->value->Key=left.toInt();



        cout  <<"FIND_NODE " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
        msg_send->src.src_port<<" ; "  << msg_send->value->Key<<endl;
        return 1;
    }

    if(left.contains("FIND_VALUE")) {
        msg_send->command=FIND_VALUE;
        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->dst=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->src.src_id=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              strcpy(msg_send->src.src_ip,(left.toStdString()).c_str());

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->src.src_port=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->value->Key=left.toInt();
#ifdef DEBUG_OUTPUT
              cout  <<"FIND_VALUE " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                          msg_send->src.src_port<<" ; "  << msg_send->value->Key<<endl;
              #endif
              return 1;
    }

    if(left.contains("STORE")) {
        msg_send->command=STORE;
        data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->dst=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->src.src_id=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              strcpy(msg_send->src.src_ip,(left.toStdString()).c_str());

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->src.src_port=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              msg_send->value->Key=left.toInt();

              data=right; space=data.indexOf(' '); left=data.left(space); right=data.mid(space+1);
              strcpy(msg_send->value->Value,(left.toStdString()).c_str());

#ifdef DEBUG_OUTPUT
              cout  <<"STORE " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                          msg_send->src.src_port<<" ; "  << msg_send->value->Key<< " ; " << msg_send->value->Value<<endl;
#endif
               return 1;
    }

    }

    //cout << d << " processed \n";
    return 0;
}
