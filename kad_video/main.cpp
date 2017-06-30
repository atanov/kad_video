// kad_video.cpp : Defines the entry point for the console application.
//

//TODO:
// 1) divide STORE & STORE_local;
// 2) add async answer waiting in node_process;
// 3) add destructors;
// 4) find bug with last byte
// 5) make normal UDP listen


#include <windows.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ht_bucket.h>
#include <my_fifo.h>
#include <command_processors.cpp>


using namespace std;
mutex fifo_mutex;

const int MAX_FILES=1105;
const int THIS_ID=1;

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
const int NODES_SIZE=100;


struct th_params{
    QUdpSocket *udp;
    int node_id;
    char *node_ip;
    int node_port;
    int port_rec;
    udp_fifo *tst_var;
    HANDLE side_thread;

    th_params(){
        node_ip=new char[80];
    }
};

struct file_list{
    int id;
    char *value;
    int size;
};


int load_files(char *name_base, file_list **list)
{    int N=0;
     char *full_name=new char[255];
     QFile *f= new QFile();

     *list = new file_list[MAX_FILES];
     for (int i=0; i<MAX_FILES;i++)  {(*list)[i].id=-1; (*list)[i].value=NULL;}

     for (int i=0; i<MAX_FILES;i++){   //<MAX_FILES
      strcpy(full_name,name_base);
      char num[5];
      sprintf(num,"%05i", i+1);
      strcat(full_name,num);
      strcat (full_name,".m4s");

      f->setFileName(full_name);
      if (f->open(QIODevice::ReadOnly)) cout << full_name <<" file opened succesfully\n";
      else {cout << full_name<<" file opening error\n"; return N;}
      int fsize=f->size();
      char *data_buf = (char *)malloc(fsize);
      if (f->read(data_buf,fsize)<1) cout << "read error\n";

      int iter_num=(fsize/UDP_PSIZE) +1;
      typedef char* block;

      block *cur_packet=new block[iter_num];     //init memory for file blocks
      for (int j=0;j<iter_num;j++){
          cur_packet[j]=new char[UDP_PSIZE+2];
      }

      for (int j=0;j<iter_num-1;j++) {
        (cur_packet[j])[0]=j;
        (cur_packet[j])[1]=iter_num;
        memcpy (cur_packet[j]+2,data_buf+j*UDP_PSIZE,UDP_PSIZE);   //data
        (*list)[N].value=cur_packet[j];
        (*list)[N].id=(i+1)*100+j;    //replace on hash
        (*list)[N].size=UDP_PSIZE+2;    //is used only on transmit
        N++;
       }
      (cur_packet[iter_num-1])[0]=iter_num-1;
      (cur_packet[iter_num-1])[1]=iter_num;
      memcpy (cur_packet[iter_num-1]+2,data_buf+(iter_num-1)*UDP_PSIZE,fsize-(iter_num-1)*UDP_PSIZE);

      (*list)[N].value=cur_packet[iter_num-1];
      (*list)[N].id=(iter_num-1)+(i+1)*100;    //replace on hash
      (*list)[N].size=fsize-(iter_num-1)*UDP_PSIZE + 2;   //+ header_size
      N++;
      f->close();
      }

      delete full_name;
      return N;
}

DWORD WINAPI Udp_Listen(CONST LPVOID Param) {
    th_params *param=(th_params *)Param;
    udp_fifo *fifo=param->tst_var;
    QByteArray buffer;
    QHostAddress sender;
    quint16 senderPort;

    while (1){Sleep(1);

          //this_thread::sleep_for(chrono::microseconds(SLEEP_TIME));

          if (param->udp->hasPendingDatagrams()){
          //SuspendThread(param->side_thread);   //pause main program, until reading messages in FIFO;
          fifo_mutex.lock();
              //receive data from udp
           buffer.resize(param->udp->pendingDatagramSize());
           param->udp->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);
           fifo->put(buffer);
           fifo_mutex.unlock();
          //ResumeThread(param->side_thread);

    }
  }
}


DWORD WINAPI main_thread(CONST LPVOID Param) {
  th_params *param=(th_params *)Param;

  node_msg msg_send;
  node_data_item msg_send_value;
  char srcip[80];
  msg_send.src.src_ip=srcip;   //char * initialization
  msg_send.value=&msg_send_value;

  node_msg msg_rec;
  node_hash_table node(param->node_id, param->node_ip, param->node_port,param->udp, param->tst_var);
    cout << "node created. thread message \n";

    if (param->node_id==0) {      //add files to node data list

    file_list *flist;
    int flist_size = load_files("C:\\projects\\p2p_video\\video\\chunk-stream0-",&flist);

    for (int i=0;i<flist_size;i++){
      strcpy(msg_send.src.src_ip,"127.0.0.1");
      msg_send.value->Value=flist[i].value;
      msg_send.value->Key=flist[i].id;
      msg_send.value->size=flist[i].size;   // here size !=0, in ordinary STORE size=0;
      //for (int i=0;i<100;i++) cout <<  *((msg_send.value->Value)+i);

      msg_send.command=STORE;
      msg_send.src.src_id=0;
      msg_send.src.src_port=param->node_port;
      msg_send.dst=0;
      msg_rec=node.process_query(&msg_send);
    }
    }

    //QByteArray buffer;
    while (1){
    msg_rec= node.udp_listen(2);  //2 = from_main loop
   }
  //ExitThread(0);
}

int main(int argc, char* argv[])
{//-------------  new version --------------
    int arg_id=THIS_ID,arg_port=1236,arg_port_rec=1236;
    QString arg_ipS("127.0.0.1"), arg_ip2S("127.0.0.1");


    if (argc>=3) {
     QString arg_idS(argv[1]);
     QString arg_portS(argv[2]);
     arg_ipS=QString(argv[3]);
     arg_ip2S=QString(argv[3]);//QString(argv[4]);

    arg_id=arg_idS.toInt();
    arg_port=arg_portS.toInt();
    arg_port_rec=arg_portS.toInt();//arg_port_recS.toInt();
    }

    qDebug() <<"node id: " <<arg_id << endl;
    qDebug() << "port: "<<arg_port << endl;
    qDebug() <<"node ip: " <<arg_ipS << endl;
    qDebug() <<"node rec_ip: " <<arg_ip2S << endl;
    qDebug() << "receiver port: "<<arg_port_rec << endl;

    //UDP init
    QUdpSocket *my_udp;
    my_udp= new QUdpSocket();
    my_udp->bind(QHostAddress(arg_ipS),arg_port);
    udp_fifo *FIFO=new udp_fifo;

    DWORD thread_id,thread_id2;
    HANDLE hThread, udp_Thread;

    th_params param;
    th_params udp_param;

    param.udp=my_udp;
    param.node_id=arg_id;
    strcpy(param.node_ip,arg_ipS.toStdString().c_str());
    param.node_port=arg_port;
    param.port_rec=arg_port_rec;
    param.tst_var=FIFO;


    udp_param.udp=my_udp;
    udp_param.tst_var=FIFO;

    hThread=CreateThread(NULL,2097152,&main_thread,&param,CREATE_SUSPENDED,&thread_id);// stack_size = 4MB
    if (hThread==NULL) {cout << "thread create error\n"; return 0;}

    udp_param.side_thread=hThread;
    udp_Thread=CreateThread(NULL,2097152,&Udp_Listen,&udp_param,CREATE_SUSPENDED,&thread_id2);// stack_size = 4MB
    if (udp_Thread==NULL) {cout << "thread create error\n"; return 0;}


    cout << "main flow message\n";
    if (ResumeThread(hThread)==-1) {
        cout << "Thread resume error \n";
        return 0;
    }

    if (ResumeThread(udp_Thread)==-1) {
        cout << "Thread resume error \n";
        return 0;
    }

        //system("pause");
        char data_to_udp[80];
        QByteArray Data;

        while (strcmp(data_to_udp,"exit")!=0){
        cin.getline(data_to_udp,80);
        Data.clear();
        Data.append(data_to_udp);
        my_udp->writeDatagram(Data, QHostAddress(arg_ipS), arg_port_rec);
        }

        //system("pause");
        TerminateThread(hThread,-1);
        TerminateThread(udp_Thread,-1);
    //*******
//    for (int i=0;i<NODES_SIZE;i++){
//        delete nodes[i];
//    }
//    free(nodes);
    return 0;
}

