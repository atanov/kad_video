// kad_video.cpp : Defines the entry point for the console application.
//

#include <windows.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ht_bucket.h>

//struct node_msg {
// struct Src{
//     int src_id;
//     char *src_ip;
//     int src_port;
// };

// Src src;
// int dst;
// int command;
// node_data_item *value;
//};

typedef HT<bucket_item,int> node_hash_table;
using namespace std;
const int NODES_SIZE=100;

struct th_params{
    QUdpSocket *udp;
    int node_id;
    char *node_ip;
    int node_port;
    int port_rec;
};

void process_command(char *d, node_hash_table *t){
QString data(d);
QString left;

if(data.contains("PRINT_DATA")) {t->node_data_print();}
if(data.contains("PRINT_NODE")) {t->print();}
if(data.contains("SEARCH")) {if(!(t->node_search(0,341))) cout << "SEARCH FAILED";}
if(data.contains("FIND_DATA")) {if(!(t->value_search(1,777))) cout << "SEARCH FAILED";}
////    nodes[2]->node_search(nodes[1]->id(),nodes[31]->id());

}

int process_message(char *d,node_msg *msg_send)   //replace with transmit on value
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

        cout  <<"PING " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                    msg_send->src.src_port << endl;
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

              cout  <<"FIND_VALUE " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                          msg_send->src.src_port<<" ; "  << msg_send->value->Key<<endl;
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

              cout  <<"STORE " << msg_send->dst<<" ; " << msg_send->src.src_id <<" ; " << msg_send->src.src_ip <<" ; " <<
                          msg_send->src.src_port<<" ; "  << msg_send->value->Key<< " ; " << msg_send->value->Value<<endl;
               return 1;
    }

    }

    cout << d << " processed \n";
    return 0;
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
    answer.append(QByteArray(msg->value->Value)+' ');

    return answer;
}

DWORD WINAPI UDPListen(CONST LPVOID Param) {

  th_params *param=(th_params *)Param;

  node_msg msg_send;
  node_data_item msg_send_value;
  char srcip[80];
  char msgsendvalue[80];
  msg_send.src.src_ip=srcip;   //char * initialization
  msg_send_value.Value=msgsendvalue;
  msg_send.value=&msg_send_value;


  node_msg msg_rec;
  node_hash_table node(param->node_id, param->node_ip, param->node_port,param->udp);
    cout << "node created. thread message \n";

  QByteArray buffer;
  QHostAddress sender;
  quint16 senderPort;

  while (1)
  {
   if (param->udp->hasPendingDatagrams()){
       //receive data from udp
       buffer.resize(param->udp->pendingDatagramSize());
       param->udp->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);
       qDebug() <<" hello world from" << sender.toString() << ": " << buffer.data();

       //check for COMMAND/MESSAGE
       if(process_message(buffer.data(),&msg_send)){
       msg_rec=node.process_query(&msg_send);
       //answer data to udp
       param->udp->writeDatagram(build_answer(&msg_rec) ,QHostAddress(QString(msg_send.src.src_ip)), msg_send.src.src_port);
       }
       else process_command(buffer.data(),&node);
      }
   }
  //ExitThread(0);
}

int main(int argc, char* argv[])
{//-------------  new version --------------
    int arg_id=0,arg_port=1234,arg_port_rec=1235;

    if (argc>=3) {
    QString arg_idS(argv[1]);
    QString arg_portS(argv[2]);
    QString arg_port_recS(argv[3]);

    arg_id=arg_idS.toInt();
    arg_port=arg_portS.toInt();
    arg_port_rec=arg_port_recS.toInt();
    }

    qDebug() <<"node id: " <<arg_id << endl;
    qDebug() << "port: "<<arg_port << endl;
    qDebug() << "receiver port: "<<arg_port_rec << endl;

    //UDP init
    QUdpSocket *my_udp;
    my_udp= new QUdpSocket();
    my_udp->bind(QHostAddress::LocalHost,arg_port);

    DWORD thread_id;
    HANDLE hThread;
    th_params param;
    param.udp=my_udp;
    param.node_id=arg_id;
    param.node_ip="127.0.0.1";
    param.node_port=arg_port;
    param.port_rec=arg_port_rec;

    hThread=CreateThread(NULL,0,&UDPListen,&param,CREATE_SUSPENDED,&thread_id);
    if (hThread==NULL) {cout << "thread create error\n"; return 0;}

    cout << "main flow message\n";

    if (ResumeThread(hThread)==-1) {
        cout << "Thread resume error \n";
        return 0;
    }


//    //HT init
//    node_hash_table **nodes;
//    nodes=(node_hash_table **)malloc(NODES_SIZE*sizeof(node_hash_table *));

//    for (int i=0; i<NODES_SIZE; i++){
//        nodes[i]=new node_hash_table(i*11);
//    }

//    bucket_item *new_item;
//    for (int i=1; i<NODES_SIZE; i+=10){   //nodes[0]  know some other nodes
//        new_item=new bucket_item(nodes[i]->id(),(char *)nodes[i],127);
//        nodes[0]->insert(new_item);
//    }

//    for (int i=1;i<NODES_SIZE;i++){    //everybody knows about nodes[0];
//        new_item=new bucket_item(nodes[0]->id(),(char *)nodes[0],127);
//        nodes[i]->insert(new_item);
//    }


//    new_item= new bucket_item(nodes[1]->id(), (char *)nodes[1],127);  //nodes[2] meet nodes[1]
//    nodes[2]->insertF(new_item);

//    cout  << "  recursive find node:  \n";
//    nodes[2]->node_search(nodes[1]->id(),nodes[31]->id());

//    //----------------------
//    cout << "_______nodes[0]: \n";
//    nodes[0]->print();

    //cout <<" _______nodes[1], id=11: \n";
    //nodes[1]->print();
        char data_to_udp[80];
        QByteArray Data;

        while (strcmp(data_to_udp,"exit")!=0){
        cin.getline(data_to_udp,80);
        Data.clear();
        Data.append(data_to_udp);
        my_udp->writeDatagram(Data, QHostAddress::LocalHost, arg_port_rec);
        }

        //system("pause");
        TerminateThread(hThread,-1);
    //*******
//    for (int i=0;i<NODES_SIZE;i++){
//        delete nodes[i];
//    }
//    free(nodes);
    return 0;
}

