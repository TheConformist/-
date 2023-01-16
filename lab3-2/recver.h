//#pragma once
//#include <iostream>
//#include <stdlib.h>
//#include <Windows.h>
//#include <Winsock.h>
//
//#include <string>
//#include <time.h>
//#pragma comment(lib,"ws2_32.lib")
//
//using namespace std;
//
//#define MTU 14650
//
//int totallength = 0;
//clock_t start_time; //总开始时间
//clock_t end_time; //总结束时间
//clock_t onepacket_start; //一次发包的开始时间 
//clock_t onepacket_end; //一次发包的结束时间
//
//string File_des_path = "C:\\Users\\xumin\\Desktop\\2022大三上学期\\计算机网络\\lab3\\测试结果2\\";
//string File_src_path = "C:\\Users\\xumin\\Desktop\\2022大三上学期\\计算机网络\\lab3\\测试文件\\";
//int base = 0; //基序号
//int nextseqnum = 0; //下一个序号
//int window = 20; //窗口大小
//int packetnum = 0;//传输包的数量
//int seq;//发包的序列号
//int filesize;//一次发送的文件大小
//char* TheFile;//一次发送的文件的缓冲区
//
//double MAX_TIME = 50 * CLOCKS_PER_SEC;
//SOCKET socket_sender;
////SOCKADDR_IN  server_addr;
////int server_addr_len = sizeof(server_addr);
//SOCKADDR_IN  client_addr;
//int client_addr_len = sizeof(client_addr);
//
//struct rdt_header
//{
//    u_short datalength;//数据包里数据的长度
//    u_short verification;//校验和
//    u_short seq;//序列号，这里是16位
//    u_short ack;//ack，这里是16位
//    char flag; //为1是SYN，为2是FIN，为3是发送文件名字，为4是发送文件的最后一个数据包
//    char flag_ack;//ACK标志位
//
//    rdt_header()
//    {
//        datalength = 0;
//        verification = 0;
//        seq = 0;
//        ack = 0;
//        flag = 0;
//        flag_ack = 0;
//    }
//};
//
//class MyRDT
//{
//public:
//    // rdt函数
//    MyRDT();
//    ~MyRDT();
//    int Listen();
//    int send(char* data, UINT32 len, char* fileName);
//    void close();
//    int Bind(SOCKADDR_IN* server_addr);
//    int farewell();
//    void recvdata();
//    MyRDT* rdt_sock;
//
//
//
//private:
//    int handshake_send(SOCKADDR_IN* addr, UINT8 flag, UINT32* _seq, UINT32 _ackseq,
//        UINT32* window = 0);
//    int handshake_recv(SOCKADDR_IN* addr, UINT8 flag, UINT32* _seq, UINT32* _ackseq,
//        UINT32* window = nullptr, bool timeLimit = true);
//    //inline USHORT checksum(char* data, UINT length);
//    static DWORD WINAPI ACK_recv(LPVOID param);
//
//
//    UINT syn_send_cnt;
//    UINT data_send_cnt;
//
//    SOCKET sock;
//    SOCKADDR_IN clientaddr;
//    SOCKADDR_IN serveraddr;
//    char* sendBuf;
//
//
//    UINT32 window;
//};
//
