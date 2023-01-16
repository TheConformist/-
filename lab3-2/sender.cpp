#include <iostream>
#include <cstring>
#include <WINSOCK2.h>
#include <time.h>
#include <fstream>
#include <process.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;
double MAX_TIME = 0.5 * CLOCKS_PER_SEC;
const int MTU = 10500;//���仺������󳤶�
SOCKET socket_sender;
SOCKADDR_IN  receiver_address;
int receiver_address_len = sizeof(receiver_address);
int totallength = 0;
clock_t start_time; //�ܿ�ʼʱ��
clock_t end_time; //�ܽ���ʱ��
clock_t onepacket_start; //һ�η����Ŀ�ʼʱ�� 
clock_t onepacket_end; //һ�η����Ľ���ʱ��

string File_src_path = "C:\\Users\\xumin\\Desktop\\2022������ѧ��\\���������\\lab3\\�����ļ�\\";
int base = 0; //�����
int nextseqnum = 0; //��һ�����
int window = 20; //���ڴ�С
int packetnum = 0;//�����������
int seq;//���������к�
int filesize;//һ�η��͵��ļ���С
char* TheFile;//һ�η��͵��ļ��Ļ�����

//��UDP��TCP���ĸ�ʽ�����һ�£�һ��10�ֽ�
struct MyPacketHeader
{
	//UDP��TCP��ͷ����ԴĿ�Ķ˿ڣ�����Ͳ�����
	u_short datalength;//���ݰ������ݵĳ���
	u_short verification;//У���
	u_short seq;//���кţ�������16λ
	u_short ack;//ack��������16λ
	char flag; //Ϊ1��SYN��Ϊ2��FIN��Ϊ3�Ƿ����ļ����֣�Ϊ4�Ƿ����ļ������һ�����ݰ�
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
	shake_header.verification = 0;
	int seq = rand() % 100;//����ʱseq���������
	shake_header.seq = seq;//����ʱseq���������
	shake_header.flag = 1;
	u_short veri = checkerror((u_short*)&shake_header, sizeof(shake_header));
	shake_header.verification = veri;
	char* buffer = new char[sizeof(shake_header)];

	memcpy(buffer, &shake_header, sizeof(shake_header));//ת��char*
	//��һ������
	sendto(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, receiver_address_len);

	/*int go = 0;
	while (true)
	{
		go = recvfrom(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len);
		if (go == SOCKET_ERROR)
		{
			Sleep(2000);
			continue;
		}
		else
			break;
	}*/

	clock_t start = clock();
	//sender����receiver�ĵڶ�������
	while (recvfrom(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
	{
		if (clock() - start > 500)//��ʱ�����µ�һ������
		{
			shake_header.flag = 1;
			shake_header.verification = 0;
			shake_header.seq = seq;
			shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

			memcpy(buffer, &shake_header, sizeof(shake_header));
			sendto(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
			start = clock();
			cout << "���ֳ�ʱ�������ش���";
		}
	}

	cout << "��һ�����ֳɹ���" << endl;
	memcpy(&shake_header, buffer, sizeof(shake_header));
	if (shake_header.flag_ack == 1 && shake_header.flag == 1 && shake_header.ack == seq + 1) //���շ���������SYN��ACK���ģ�����ack����seq+1
	{
		if (checkerror((u_short*)&shake_header, sizeof(shake_header) == 0))
		{
			cout << "�յ��ڶ������֡�" << endl;
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
	int seq_new = shake_header.seq;
	//���������ֿ�ʼ
	shake_header.flag_ack = 1;
	shake_header.seq = seq + 1;
	shake_header.ack = seq_new + 1;
	shake_header.verification = 0;
	shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header)); //����У���

	memcpy(buffer, &shake_header, sizeof(shake_header));//ת��char*
	sendto(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
	cout << "����������ɣ��������������ݡ�" << endl;
	return true;
}

bool wave_hand()
{
	MyPacketHeader wave_header;
	//wave_header = recvheader;
	char* buffer = new char[sizeof(wave_header)];
	//memcpy(&wave_header, buffer, sizeof(wave_header));
	//cout << "�ɹ����յ�һ�λ�����Ϣ" << endl;


	while (true)
	{

		if (recvfrom(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) == SOCKET_ERROR)
		{
			Sleep(1000);
			continue;
		}

		memcpy(&wave_header, buffer, sizeof(wave_header));
		if (wave_header.flag == 2 && checkerror((u_short*)&wave_header, sizeof(wave_header)) == 0)
		{
			cout << "�ɹ����յ�һ�λ�����Ϣ" << endl;
			break;
		}
	}
	int old_seq = wave_header.seq;
	//���͵ڶ��λ�����Ϣ
	wave_header.flag_ack = 1;
	wave_header.flag = 2;//ACK��FIN����
	wave_header.verification = 0;
	int new_seq = rand() % 100;
	wave_header.seq = new_seq;
	wave_header.datalength = 0;
	wave_header.ack = old_seq + 1;
	wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));
	memcpy(buffer, &wave_header, sizeof(wave_header));
	if (sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len))
		cout << "���͵ڶ��λ���" << endl;

	cout << "���λ��ֽ��������ӶϿ���" << endl;
	return true;
}

bool one_recv()
{
	//���Ϸ�ʧ����o(�i�n�i)o
	
	fd_set rdfds;
	FD_ZERO(&rdfds);
	FD_SET(socket_sender, &rdfds);
	struct timeval tv;
	tv.tv_sec =  0.5;
	tv.tv_usec = (500 % 1000) * 1000;
	int ret = 1;
	ret = select(socket_sender + 1, &rdfds, NULL, NULL, &tv);
	if (ret == 0)
	{
		//cout << "fucku" << endl;
		cout << "���ճ�ʱ��" << endl;
		return 0;
	}
	else if (ret < 0)
	{
		//cout << "fucku" << endl;
		cout << "���ճ���" << endl;
		return 0;
	}
	char* recv_buffer = new char[sizeof(MyPacketHeader)];//����10�ֽڻ�����	
	int recv_return = recvfrom(socket_sender, recv_buffer, sizeof(MyPacketHeader), 0, (sockaddr*)&receiver_address, &receiver_address_len);

	MyPacketHeader recvpacketheader;
	memcpy(&recvpacketheader, recv_buffer, sizeof(recvpacketheader));
	if (recvpacketheader.flag_ack == 1)
	{
		base = recvpacketheader.ack;
		cout << "baseŲ��Ϊ" << base << endl;
		return 1;
	}

	//�����Ҳ����
	//char* recv_buffer = new char[sizeof(MyPacketHeader)];//����10�ֽڻ�����	
	//u_long mode = 1;
	//ioctlsocket(socket_sender, FIONBIO, &mode);
	//if (recvfrom(socket_sender, recv_buffer, sizeof(MyPacketHeader), 0, (sockaddr*)&receiver_address, &receiver_address_len))
	//{
	//	MyPacketHeader recvpacketheader;
	//	memcpy(&recvpacketheader, recv_buffer, sizeof(recvpacketheader));
	//	if ((checkerror((u_short*)&recvpacketheader, sizeof(recvpacketheader)) == 0) && recvpacketheader.flag_ack == 1)
	//	{
	//		base = recvpacketheader.ack;
	//		return 1;
	//	}
	//}


}




void mytimechecker(void*)
{
	while (base!= packetnum)
	{
		onepacket_end = clock();
		//��ʱ���Σ���ͼ3-20�ұߵ����
		if (onepacket_end - onepacket_start >= MAX_TIME)
		{
			onepacket_start = clock();
			//��ʱҪ�ش����ش�����Ҫ��base��nextseqnum֮��ı���ȫ���ٷ���ȥ
			char* resendpacket = new char[MTU];//�ְ�����

			for (int i = base;i < nextseqnum && base < nextseqnum;i++)
			{
				MyPacketHeader resendpakcetheader;
				memcpy(resendpacket, &resendpakcetheader, sizeof(resendpakcetheader));
				//һ�����ݰ��ĳ��ȣ�1490���߲���1490�����
				int packetsize = min(filesize - i * (MTU - 10), MTU - 10);
				resendpakcetheader.datalength = packetsize;

				for (int j = 0;j < packetsize;j++)
				{
					resendpacket[j + 10] = TheFile[j + i * (MTU - 10)];
				}

				resendpakcetheader.seq = i;
				resendpakcetheader.ack = 1;
				if (i == packetnum - 1)//����������һ����
					resendpakcetheader.flag = 4;
				resendpakcetheader.verification = checkerror((u_short*)&resendpakcetheader, sizeof(resendpakcetheader));//У���
				memcpy(resendpacket, &resendpakcetheader, sizeof(resendpakcetheader));
				cout << "�ش����к�Ϊ" << resendpakcetheader.seq << "�ı���" << endl;
				sendto(socket_sender, resendpacket, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			}
		}
	}
	cout << "�˳�ʱ���߳�" << endl;
	_endthread();
}

void myrecv(void*)
{
	char* recv_buffer = new char[sizeof(MyPacketHeader)];//����10�ֽڻ�����
	int myrecv = 0;
	while (base != packetnum)
	{

		if (one_recv())
		{
			onepacket_start = clock();

		}
	}

	//if (one_recv())
	//{
	//	onepacket_start = clock();
	//}

	cout << "�˳������߳�" << endl;
	_endthread();
}



void sendfilename(string filename)
{
	MyPacketHeader filenameheader;
	filenameheader.datalength = filename.size();
	filenameheader.flag = 3;//�����ļ�����
	filenameheader.verification = checkerror((u_short*)&filenameheader, sizeof(filenameheader));
	char* sendname = new char[filename.size() + 10 + 1];
	//strcpy(sendname, filename.c_str());
	memcpy(sendname, &filenameheader, sizeof(filenameheader));
	for (int i = 0;i < filename.size(); i++)
		sendname[10 + i] = filename[i];
	sendto(socket_sender, sendname, filename.size() + 1 + 10, 0, (sockaddr*)&receiver_address, receiver_address_len);

	u_long mode = 1;
	ioctlsocket(socket_sender, FIONBIO, &mode);
	clock_t start = clock();
	char* recv = new char[MTU];
	while (recvfrom(socket_sender, recv, sizeof(recv), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
	{
		if (clock() - start > MAX_TIME)  //���ÿ�һ���ش�
		{
			filenameheader.datalength = sizeof(filename);
			filenameheader.verification = 0;
			filenameheader.flag = 3;//�����ļ�����
			filenameheader.verification = checkerror((u_short*)&filenameheader, sizeof(filenameheader));
			memcpy(sendname, &filenameheader, sizeof(filenameheader));
			for (int i = 0;i < filename.size(); i++)
				sendname[10 + i] = filename[i];
			sendto(socket_sender, sendname, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			cout << "�����ļ�����ʧ�ܣ������ش�" << endl;
		}
	}
	cout << "�����ļ����ɹ�" << endl;
}

void send(string filename)
{
	cout << "��ʼ����" << filename << endl;
	seq = 0;//seq��С0-2^16
	base = 0;
	nextseqnum = 0;
	sendfilename(filename);

	ifstream fin((File_src_path + filename).c_str(), ifstream::binary);//�Զ����Ʒ�ʽ���ļ�
	fin.seekg(0, fin.end);//ָ������ļ���β
	filesize = fin.tellg();//��ȡ�ļ���С
	fin.seekg(0, fin.beg);//ָ�������ļ�ͷ��
	TheFile = new char[filesize];
	memset(TheFile, 0, sizeof(char) * filesize);
	fin.read(TheFile, filesize);//�ļ����ݷ���TheFile��
	fin.close();
	packetnum = filesize / (MTU - sizeof(MyPacketHeader)); //��Ҫ���͵ķ������� sizeof(MyPacketHeader)=10
	if (filesize % (MTU - sizeof(MyPacketHeader)) == 0)
		packetnum += 0;
	else
		packetnum += 1;

	cout << "���ļ���Ҫ����" << packetnum << "�����ݰ�" << endl;
	char* sendpacket = new char[MTU];//�ְ�����

	while (base!= packetnum)
	{
		//����rdt_send(data)������nextseqnum���ķ��飬��Ӧͼ3-20���������������
		//�Ⱦ�������base��nextseqnum�ڴ������ڣ�nextseqnumû�г����ܰ���
		if (nextseqnum < base + window && nextseqnum < packetnum)
		{
			MyPacketHeader sendpacketheader;
			memcpy(sendpacket, &sendpacketheader, sizeof(sendpacketheader));
			//һ�����ݰ��ĳ��ȣ�1490���߲���1490�����
			int packetsize = min(filesize - nextseqnum * (MTU - 10), MTU - 10);
			sendpacketheader.datalength = packetsize;

			for (int i = 0;i < packetsize;i++)
			{
				sendpacket[i + 10] = TheFile[i + nextseqnum * (MTU - 10)];
			}

			//sendpacketheader.seq = seq;//���к�
			//sendpacketheader.ack = seq + 1;
			if (nextseqnum == packetnum - 1)
			{
				sendpacketheader.flag = 4; //���������һ����ʱ��flag��4
			}

			sendpacketheader.seq = nextseqnum;
			sendpacketheader.verification = checkerror((u_short*)&sendpacketheader, sizeof(sendpacketheader));//У���
			memcpy(sendpacket, &sendpacketheader, sizeof(sendpacketheader));
			sendto(socket_sender, sendpacket, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			cout << "�������ͱ��ġ�";
			cout << "���кź�nextseqnum��" << nextseqnum << " " << "base��" << base << endl;
			//		clock_t start = clock();//��¼����ʱ��
			if (base == nextseqnum)//�򿪼�ʱ��
				onepacket_start = clock();

			//��һ�ΰ���nextseqnum��һ��
			nextseqnum++;//���кŵ���

		}

		//��һ�η���ʱ�������̣߳�һ�������ʱ��һ���������
		if (nextseqnum == 1)
		{
			_beginthread(mytimechecker, 0, NULL);
			_beginthread(myrecv, 0, NULL);//����������Ϣ���߳�
		}

	}



	cout << "�ɹ����� " << filename << endl;
}


int main()
{
	srand((unsigned)time(NULL));
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "winsockʧ�ܡ�" << endl;
		return 0;
	}

	socket_sender = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_sender == INVALID_SOCKET)
	{
		cout << "�����socket����ʧ�ܣ�" << WSAGetLastError() << endl << endl;
		WSACleanup();
		return false;
	}


	SOCKADDR_IN sender_address;
	sender_address.sin_family = AF_INET;
	sender_address.sin_port = htons(4395); //����˿ں�
	sender_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//�����Ͷ��Ȱ���
	if (SOCKET_ERROR == bind(socket_sender, (SOCKADDR*)&sender_address, sizeof(sender_address)))
	{
		cout << "��ʧ��:" << WSAGetLastError() << endl;
		closesocket(socket_sender);
		WSACleanup();
		return false;
	}

	//��Ҫ����·�����ĵ�ַ�������ļ�ʱ����
	receiver_address.sin_family = AF_INET;
	receiver_address.sin_port = htons(4399); //����˿ں�
	receiver_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");



	//��ִ������
	if (shake_hand())
	//if (1)
	{
		//���ֳɹ��󣬷�������
		start_time = clock();

		send("1.jpg");
		cout << "����ʱ��base��" << base << " nextseqnum��" << nextseqnum << " packetnum��" << packetnum << endl;

		end_time = clock();
		double time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
		cout << "����ʱ�䣺" << time << " s" << endl;
		cout << "�����ܴ�С��" << filesize << " Bytes" << endl;
		cout << "�����ʣ�" << (double)((filesize * 8 / 1000) / time) << " Kbps" << endl;

		//wave_hand();
	}



	closesocket(socket_sender);
	WSACleanup();



	return 0;
}
