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
//clock_t start_time; //�ܿ�ʼʱ��
//clock_t end_time; //�ܽ���ʱ��
//clock_t onepacket_start; //һ�η����Ŀ�ʼʱ�� 
//clock_t onepacket_end; //һ�η����Ľ���ʱ��
//
//string File_des_path = "C:\\Users\\xumin\\Desktop\\2022������ѧ��\\���������\\lab3\\���Խ��2\\";
//string File_src_path = "C:\\Users\\xumin\\Desktop\\2022������ѧ��\\���������\\lab3\\�����ļ�\\";
//int base = 0; //�����
//int nextseqnum = 0; //��һ�����
//int window = 20; //���ڴ�С
//int packetnum = 0;//�����������
//int seq;//���������к�
//int filesize;//һ�η��͵��ļ���С
//char* TheFile;//һ�η��͵��ļ��Ļ�����
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
//    u_short datalength;//���ݰ������ݵĳ���
//    u_short verification;//У���
//    u_short seq;//���кţ�������16λ
//    u_short ack;//ack��������16λ
//    char flag; //Ϊ1��SYN��Ϊ2��FIN��Ϊ3�Ƿ����ļ����֣�Ϊ4�Ƿ����ļ������һ�����ݰ�
//    char flag_ack;//ACK��־λ
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
//    // rdt����
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
