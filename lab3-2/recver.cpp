#include <iostream>
#include <WINSOCK2.h>
#include <time.h>
#include <fstream>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
SOCKADDR_IN sender_address = { 0 }; //服务器地址
int sender_address_len = sizeof(sender_address);
SOCKADDR_IN  receiver_address;
int receiver_address_len = sizeof(receiver_address);
SOCKET socket_receiver;
double MAX_TIME = 0.5 * CLOCKS_PER_SEC;
const int MTU = 10500;//传输缓冲区最大长度
string File_des_path = "C:\\Users\\xumin\\Desktop\\2022大三上学期\\计算机网络\\lab3\\测试结果2\\";


struct MyPacketHeader
{
    //UDP、TCP开头都是源目的端口，这里就不用了
    u_short datalength;//数据包里数据的长度
    u_short verification;//校验和
    u_short seq;//序列号，这里是16位
    u_short ack;//ack，这里是16位
    char flag; //为1是SYN，为2是FIN，为3是发送文件名，为4是发送文件的最后一个数据包
    char flag_ack;//ACK标志位
    MyPacketHeader()
    {
        datalength = 0;
        verification = 0;
        seq = 0;
        ack = 0;
        flag = 0;
        flag_ack = 0;
    }
};

u_short checkerror(u_short* message, int size)
{
    int count = (size + 1) / 2;
    u_short* check_array = (u_short*)malloc(size + 1);
    memset(check_array, 0, size + 1);
    memcpy(check_array, message, size);
    u_long sum = 0;
    while (count--)
    {
        sum += *check_array;
        check_array++;
        if (sum & 0xffff0000)  //若进位
        {
            sum &= 0xffff; //保留后八位
            sum++;  //加1
        }
    }
    return ~(sum & 0xffff);
}


bool shake_hand()
{
    MyPacketHeader shake_header;
    char* buffer = new char[sizeof(shake_header)];

    //接收第一次握手信息
    while (true)
    {
        if (recvfrom(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) == SOCKET_ERROR)
        {
            Sleep(1000);
            continue;
        }
        memcpy(&shake_header, buffer, sizeof(shake_header));

        if (shake_header.flag == 1) //syn报文
        {
            if (checkerror((u_short*)&shake_header, sizeof(shake_header)) == 0)//校验和没问题
            {
                cout << "第一次握手成功！" << endl;
                break;
            }
        }
    }

    //准备第二次握手信息
    shake_header.flag_ack = 1;
    shake_header.flag = 1;  //准备SYN、ACK报文
    int seq_old = shake_header.seq;
    shake_header.ack = seq_old + 1;
    int seq_new = rand() % 100;
    shake_header.seq = seq_new;//接收方也随机生成一个随机序列号

    shake_header.verification = 0;
    shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

    memcpy(buffer, &shake_header, sizeof(shake_header));
    sendto(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //clock_t start = clock();//记录第二次握手发送时间

    int go = 0;
    while (true)
    {
        go = recvfrom(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len);
        if (go == SOCKET_ERROR)
        {
            Sleep(2000);
            continue;
        }
        else
            break;
    }

    //接收第三次握手
    //while (recvfrom(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    //{
    //    if (clock() - start > MAX_TIME) //超时重发
    //    {
    //        shake_header.flag_ack = 1;
    //        shake_header.flag = 1;  //准备SYN、ACK报文
    //        shake_header.ack = seq_old + 1;
    //        shake_header.seq = seq_new;
    //        shake_header.verification = 0;
    //        shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

    //        memcpy(buffer, &shake_header, sizeof(shake_header));
    //        sendto(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&sender_address, sender_address_len);
    //        cout << "握手超时，尝试重传" << endl;
    //    }
    //}

    memcpy(&shake_header, buffer, sizeof(shake_header));
    if (shake_header.flag_ack == 1 && shake_header.ack == seq_new + 1)//ack报文且ack为seq+1
    {
        if (checkerror((u_short*)&shake_header, sizeof(shake_header)) == 0)
        {
            cout << "收到第三次握手。成功建立通信！" << endl;
        }
        else
        {
            cout << "寄！建立连接失败！" << endl;
            return false;
        }
    }
    else
    {
        cout << "寄！建立连接失败！" << endl;
        return false;
    }
    return true;
}


bool wave_hand()
{
    MyPacketHeader wave_header;
    wave_header.verification = 0;
    wave_header.flag = 2;
    wave_header.flag_ack = 1;  //FIN、ACK报文
    int seq = rand() % 100;//握手时seq得是随机数
    wave_header.seq = seq;//握手时seq得是随机数
    wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));

    char* buffer = new char[sizeof(wave_header)];

    memcpy(buffer, &wave_header, sizeof(wave_header));//转成char*
    //第一次握手
    sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);



    cout << "开始第一次挥手" << endl;


    /* int go = 0;
     while (true)
     {
         go = recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len);
         if (go == SOCKET_ERROR)
         {
             Sleep(2000);
             continue;
         }
         else
             break;
     }*/


    u_long mode = 1;
    ioctlsocket(socket_receiver, FIONBIO, &mode);
    clock_t start = clock(); //记录发送第一次挥手时间
    //接收第二次挥手
    while (recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    {
        if (clock() - start > MAX_TIME)//超时，重新传输第一次挥手
        {
            wave_header.flag_ack = 1;//ack为1
            wave_header.seq = seq;
            wave_header.verification = 0;//校验和置0
            wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));//计算校验和
            memcpy(buffer, &wave_header, sizeof(wave_header));//将首部放入缓冲区
            //sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
            sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

            start = clock();
            cout << "第一次挥手超时，尝试重传" << endl;
        }
    }

    cout << "第一次挥手成功！" << endl;
    //进行校验和检验
    memcpy(&wave_header, buffer, sizeof(wave_header));
    int new_seq = wave_header.seq;
    if (wave_header.flag_ack == 1 && wave_header.flag == 2 && checkerror((u_short*)&wave_header, sizeof(wave_header)) == 0 && wave_header.ack == seq + 1)
    {
        cout << "收到第二次挥手信息，连接断开！" << endl;
        return true;
    }
    else
    {
        cout << "寄！挥手失败！" << endl;
        return false;
    }

    ////第三次挥手
    //wave_header.verification = 0;
    //wave_header.flag_ack = 1;  //ACK报文
    ////int seq = rand() % 100;//握手时seq得是随机数
    //wave_header.seq = seq + 1;//握手时seq得是随机数
    //wave_header.ack = new_seq + 1;//ack是接收方发来的seq+1
    //wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));


    //memcpy(buffer, &wave_header, sizeof(wave_header));//转成char*
    ////第一次握手
    ///*sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);*/
    //sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //mode = 1;
    //ioctlsocket(socket_receiver, FIONBIO, &mode);
    //start = clock(); //记录发送第一次挥手时间
    ////接收第二次挥手
    //while (recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    //{
    //    if (clock() - start > MAX_TIME)//超时，重新传输第一次挥手
    //    {
    //        wave_header.flag_ack = 1;  //ACK报文
    //        //int seq = rand() % 100;//握手时seq得是随机数
    //        wave_header.seq = seq + 1;//握手时seq得是随机数
    //        wave_header.ack = new_seq + 1;//ack是接收方发来的seq+1
    //        wave_header.verification = 0;//校验和置0
    //        wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));

    //  
    //        memcpy(buffer, &wave_header, sizeof(wave_header));//将首部放入缓冲区
    //        //sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
    //        sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //        start = clock();
    //        cout << "第三次挥手超时，尝试重传" << endl;
    //    }
    //}

    //int go = 0;
    //while (true)
    //{
    //    go = recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len);
    //    if (go == SOCKET_ERROR)
    //    {
    //        Sleep(2000);
    //        continue;
    //    }
    //    else
    //        break;
    //}

    //cout << "四次挥手完成，断开连接。" << endl;
    //return true;
}

void recvdata()
{
    MyPacketHeader recv_header;
    int expectedseqnum = 0;
    string filename;
    char* recv_buffer = new char[MTU];
    char* response_buffer = new char[sizeof(recv_header)];
    while (true)
    {
        //char filename = '0' + filenum;
        if (recvfrom(socket_receiver, recv_buffer, MTU, 0, (sockaddr*)&receiver_address, &receiver_address_len) == SOCKET_ERROR)
        {
            Sleep(2000);
            continue;
        }
        memcpy(&recv_header, recv_buffer, sizeof(recv_header));
        //if (recv_header.flag == 2 && checkerror((u_short*)&recv_header, sizeof(recv_header)) == 0) //进入挥手
        //{
        //    cout << "进入挥手" << endl;
        //    wave_hand(recv_header);
        //    return;
        //}
        if (recv_header.flag == 3)//如果收到的是文件名
        {
            int namelen = recv_header.datalength;
            char* namebuffer = new char[namelen + 1];
            for (int i = 0;i < namelen;i++)
            {
                namebuffer[i] = recv_buffer[i + 10];
            }
            namebuffer[namelen] = '\0';
            MyPacketHeader nameresponse;
            nameresponse.flag_ack = 1;
            nameresponse.flag == 3;
            nameresponse.verification = checkerror((u_short*)&recv_header, sizeof(recv_header));
            char* nameresponse_buffer = new char[10];
            memcpy(nameresponse_buffer, &nameresponse, sizeof(nameresponse));
            sendto(socket_receiver, nameresponse_buffer, sizeof(nameresponse_buffer), 0, (sockaddr*)&sender_address, sender_address_len);
            filename = namebuffer;
            continue;
        }

        ofstream fout((File_des_path + filename).c_str(), ifstream::out | ios::binary | ios::app);


        MyPacketHeader response;
        u_short tocheck = checkerror((u_short*)&recv_header, sizeof(recv_header));
        bool seqright = 0;

        //和上次作业多了一个expectedseqnum的判断
        if (checkerror((u_short*)&recv_header, sizeof(recv_header)) == 0 && expectedseqnum == recv_header.seq)
        {
            seqright = 1;
            //cout << checkerror((u_short*)&recv_header, sizeof(recv_header)) << endl;
            cout << "收到有序报文" << recv_header.seq << endl;
            //cout << "ACK为" << recv_header.ack << endl;
            int datasize = recv_header.datalength;
            int recv_seq = recv_header.seq;
            //expectedseqnum = recv_header.seq + 1;
            //int ack = recv_seq + 1;
            response.ack = expectedseqnum + 1;
            response.flag_ack = 1;//ACK报文
            response.verification = checkerror((u_short*)&response, sizeof(response));

            char* writebuffer = new char[datasize];
            for (int j = 0;j < datasize;j++)
            {
                writebuffer[j] = recv_buffer[10 + j];
            }
            fout.write(writebuffer, datasize);
            expectedseqnum++;
        }
        else  //报文传输有错误，需要重传
        {
            seqright = 0;
            // cout << checkerror((u_short*)&recv_header, sizeof(recv_header) ) << endl;
            cout << "expectedseqnum:" << expectedseqnum << " recv_header.seq" << recv_header.seq << endl;
            //cout << recv_header.ack << endl;
            cout << "收到无序报文" << recv_header.seq << endl;
            int recv_seq = recv_header.seq;
            //int ack = recv_seq + 1;
            response.ack = expectedseqnum;
            response.flag_ack = 0;//ACK为0，则为NAK
            response.verification = checkerror((u_short*)&response, sizeof(response));
        }
        memcpy(response_buffer, &response, sizeof(response));

        sendto(socket_receiver, response_buffer, sizeof(response), 0, (sockaddr*)&sender_address, sender_address_len);
        if (recv_header.flag == 4 &&seqright==1)//收到结束文件报文，且序列号正确
        {
            expectedseqnum = 0;
            cout << "成功接收文件" << filename << endl;
            fout.close();
            return;
        }
    }
}


int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "winsock失败" << endl << endl;
        return false;
    }


    socket_receiver = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_receiver == INVALID_SOCKET)
    {
        cout << "客户端socket创建失败：" << WSAGetLastError() << endl << endl;
        WSACleanup();
        return false;
    }



    //服务端

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(4396); //定义端口号
    receiver_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //路由器端
    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(4399); //定义端口号
    sender_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //给自己绑上
    if (SOCKET_ERROR == bind(socket_receiver, (SOCKADDR*)&receiver_address, sizeof(receiver_address)))
    {
        cout << "绑定失败:" << WSAGetLastError() << endl;
        closesocket(socket_receiver);
        WSACleanup();
        return false;
    }

    if (shake_hand())
    //if (1)
    {
        recvdata();
        Sleep(2000);
        //wave_hand();
    }

    closesocket(socket_receiver);
    WSACleanup();

    return 0;
}

