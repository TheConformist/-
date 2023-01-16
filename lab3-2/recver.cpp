#include <iostream>
#include <WINSOCK2.h>
#include <time.h>
#include <fstream>
#include<cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
SOCKADDR_IN sender_address = { 0 }; //��������ַ
int sender_address_len = sizeof(sender_address);
SOCKADDR_IN  receiver_address;
int receiver_address_len = sizeof(receiver_address);
SOCKET socket_receiver;
double MAX_TIME = 0.5 * CLOCKS_PER_SEC;
const int MTU = 10500;//���仺������󳤶�
string File_des_path = "C:\\Users\\xumin\\Desktop\\2022������ѧ��\\���������\\lab3\\���Խ��2\\";


struct MyPacketHeader
{
    //UDP��TCP��ͷ����ԴĿ�Ķ˿ڣ�����Ͳ�����
    u_short datalength;//���ݰ������ݵĳ���
    u_short verification;//У���
    u_short seq;//���кţ�������16λ
    u_short ack;//ack��������16λ
    char flag; //Ϊ1��SYN��Ϊ2��FIN��Ϊ3�Ƿ����ļ�����Ϊ4�Ƿ����ļ������һ�����ݰ�
    char flag_ack;//ACK��־λ
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
        if (sum & 0xffff0000)  //����λ
        {
            sum &= 0xffff; //�������λ
            sum++;  //��1
        }
    }
    return ~(sum & 0xffff);
}


bool shake_hand()
{
    MyPacketHeader shake_header;
    char* buffer = new char[sizeof(shake_header)];

    //���յ�һ��������Ϣ
    while (true)
    {
        if (recvfrom(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) == SOCKET_ERROR)
        {
            Sleep(1000);
            continue;
        }
        memcpy(&shake_header, buffer, sizeof(shake_header));

        if (shake_header.flag == 1) //syn����
        {
            if (checkerror((u_short*)&shake_header, sizeof(shake_header)) == 0)//У���û����
            {
                cout << "��һ�����ֳɹ���" << endl;
                break;
            }
        }
    }

    //׼���ڶ���������Ϣ
    shake_header.flag_ack = 1;
    shake_header.flag = 1;  //׼��SYN��ACK����
    int seq_old = shake_header.seq;
    shake_header.ack = seq_old + 1;
    int seq_new = rand() % 100;
    shake_header.seq = seq_new;//���շ�Ҳ�������һ��������к�

    shake_header.verification = 0;
    shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

    memcpy(buffer, &shake_header, sizeof(shake_header));
    sendto(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //clock_t start = clock();//��¼�ڶ������ַ���ʱ��

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

    //���յ���������
    //while (recvfrom(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    //{
    //    if (clock() - start > MAX_TIME) //��ʱ�ط�
    //    {
    //        shake_header.flag_ack = 1;
    //        shake_header.flag = 1;  //׼��SYN��ACK����
    //        shake_header.ack = seq_old + 1;
    //        shake_header.seq = seq_new;
    //        shake_header.verification = 0;
    //        shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

    //        memcpy(buffer, &shake_header, sizeof(shake_header));
    //        sendto(socket_receiver, buffer, sizeof(shake_header), 0, (sockaddr*)&sender_address, sender_address_len);
    //        cout << "���ֳ�ʱ�������ش�" << endl;
    //    }
    //}

    memcpy(&shake_header, buffer, sizeof(shake_header));
    if (shake_header.flag_ack == 1 && shake_header.ack == seq_new + 1)//ack������ackΪseq+1
    {
        if (checkerror((u_short*)&shake_header, sizeof(shake_header)) == 0)
        {
            cout << "�յ����������֡��ɹ�����ͨ�ţ�" << endl;
        }
        else
        {
            cout << "�ģ���������ʧ�ܣ�" << endl;
            return false;
        }
    }
    else
    {
        cout << "�ģ���������ʧ�ܣ�" << endl;
        return false;
    }
    return true;
}


bool wave_hand()
{
    MyPacketHeader wave_header;
    wave_header.verification = 0;
    wave_header.flag = 2;
    wave_header.flag_ack = 1;  //FIN��ACK����
    int seq = rand() % 100;//����ʱseq���������
    wave_header.seq = seq;//����ʱseq���������
    wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));

    char* buffer = new char[sizeof(wave_header)];

    memcpy(buffer, &wave_header, sizeof(wave_header));//ת��char*
    //��һ������
    sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);



    cout << "��ʼ��һ�λ���" << endl;


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
    clock_t start = clock(); //��¼���͵�һ�λ���ʱ��
    //���յڶ��λ���
    while (recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    {
        if (clock() - start > MAX_TIME)//��ʱ�����´����һ�λ���
        {
            wave_header.flag_ack = 1;//ackΪ1
            wave_header.seq = seq;
            wave_header.verification = 0;//У�����0
            wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));//����У���
            memcpy(buffer, &wave_header, sizeof(wave_header));//���ײ����뻺����
            //sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
            sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

            start = clock();
            cout << "��һ�λ��ֳ�ʱ�������ش�" << endl;
        }
    }

    cout << "��һ�λ��ֳɹ���" << endl;
    //����У��ͼ���
    memcpy(&wave_header, buffer, sizeof(wave_header));
    int new_seq = wave_header.seq;
    if (wave_header.flag_ack == 1 && wave_header.flag == 2 && checkerror((u_short*)&wave_header, sizeof(wave_header)) == 0 && wave_header.ack == seq + 1)
    {
        cout << "�յ��ڶ��λ�����Ϣ�����ӶϿ���" << endl;
        return true;
    }
    else
    {
        cout << "�ģ�����ʧ�ܣ�" << endl;
        return false;
    }

    ////�����λ���
    //wave_header.verification = 0;
    //wave_header.flag_ack = 1;  //ACK����
    ////int seq = rand() % 100;//����ʱseq���������
    //wave_header.seq = seq + 1;//����ʱseq���������
    //wave_header.ack = new_seq + 1;//ack�ǽ��շ�������seq+1
    //wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));


    //memcpy(buffer, &wave_header, sizeof(wave_header));//ת��char*
    ////��һ������
    ///*sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);*/
    //sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //mode = 1;
    //ioctlsocket(socket_receiver, FIONBIO, &mode);
    //start = clock(); //��¼���͵�һ�λ���ʱ��
    ////���յڶ��λ���
    //while (recvfrom(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
    //{
    //    if (clock() - start > MAX_TIME)//��ʱ�����´����һ�λ���
    //    {
    //        wave_header.flag_ack = 1;  //ACK����
    //        //int seq = rand() % 100;//����ʱseq���������
    //        wave_header.seq = seq + 1;//����ʱseq���������
    //        wave_header.ack = new_seq + 1;//ack�ǽ��շ�������seq+1
    //        wave_header.verification = 0;//У�����0
    //        wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));

    //  
    //        memcpy(buffer, &wave_header, sizeof(wave_header));//���ײ����뻺����
    //        //sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
    //        sendto(socket_receiver, buffer, sizeof(wave_header), 0, (sockaddr*)&sender_address, sender_address_len);

    //        start = clock();
    //        cout << "�����λ��ֳ�ʱ�������ش�" << endl;
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

    //cout << "�Ĵλ�����ɣ��Ͽ����ӡ�" << endl;
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
        //if (recv_header.flag == 2 && checkerror((u_short*)&recv_header, sizeof(recv_header)) == 0) //�������
        //{
        //    cout << "�������" << endl;
        //    wave_hand(recv_header);
        //    return;
        //}
        if (recv_header.flag == 3)//����յ������ļ���
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

        //���ϴ���ҵ����һ��expectedseqnum���ж�
        if (checkerror((u_short*)&recv_header, sizeof(recv_header)) == 0 && expectedseqnum == recv_header.seq)
        {
            seqright = 1;
            //cout << checkerror((u_short*)&recv_header, sizeof(recv_header)) << endl;
            cout << "�յ�������" << recv_header.seq << endl;
            //cout << "ACKΪ" << recv_header.ack << endl;
            int datasize = recv_header.datalength;
            int recv_seq = recv_header.seq;
            //expectedseqnum = recv_header.seq + 1;
            //int ack = recv_seq + 1;
            response.ack = expectedseqnum + 1;
            response.flag_ack = 1;//ACK����
            response.verification = checkerror((u_short*)&response, sizeof(response));

            char* writebuffer = new char[datasize];
            for (int j = 0;j < datasize;j++)
            {
                writebuffer[j] = recv_buffer[10 + j];
            }
            fout.write(writebuffer, datasize);
            expectedseqnum++;
        }
        else  //���Ĵ����д�����Ҫ�ش�
        {
            seqright = 0;
            // cout << checkerror((u_short*)&recv_header, sizeof(recv_header) ) << endl;
            cout << "expectedseqnum:" << expectedseqnum << " recv_header.seq" << recv_header.seq << endl;
            //cout << recv_header.ack << endl;
            cout << "�յ�������" << recv_header.seq << endl;
            int recv_seq = recv_header.seq;
            //int ack = recv_seq + 1;
            response.ack = expectedseqnum;
            response.flag_ack = 0;//ACKΪ0����ΪNAK
            response.verification = checkerror((u_short*)&response, sizeof(response));
        }
        memcpy(response_buffer, &response, sizeof(response));

        sendto(socket_receiver, response_buffer, sizeof(response), 0, (sockaddr*)&sender_address, sender_address_len);
        if (recv_header.flag == 4 &&seqright==1)//�յ������ļ����ģ������к���ȷ
        {
            expectedseqnum = 0;
            cout << "�ɹ������ļ�" << filename << endl;
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
        cout << "winsockʧ��" << endl << endl;
        return false;
    }


    socket_receiver = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_receiver == INVALID_SOCKET)
    {
        cout << "�ͻ���socket����ʧ�ܣ�" << WSAGetLastError() << endl << endl;
        WSACleanup();
        return false;
    }



    //�����

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(4396); //����˿ں�
    receiver_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //·������
    sender_address.sin_family = AF_INET;
    sender_address.sin_port = htons(4399); //����˿ں�
    sender_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    //���Լ�����
    if (SOCKET_ERROR == bind(socket_receiver, (SOCKADDR*)&receiver_address, sizeof(receiver_address)))
    {
        cout << "��ʧ��:" << WSAGetLastError() << endl;
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

