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
const int MTU = 10500;//传输缓冲区最大长度
SOCKET socket_sender;
SOCKADDR_IN  receiver_address;
int receiver_address_len = sizeof(receiver_address);
int totallength = 0;
clock_t start_time; //总开始时间
clock_t end_time; //总结束时间
clock_t onepacket_start; //一次发包的开始时间 
clock_t onepacket_end; //一次发包的结束时间

string File_src_path = "C:\\Users\\xumin\\Desktop\\2022大三上学期\\计算机网络\\lab3\\测试文件\\";
int base = 0; //基序号
int nextseqnum = 0; //下一个序号
int window = 20; //窗口大小
int packetnum = 0;//传输包的数量
int seq;//发包的序列号
int filesize;//一次发送的文件大小
char* TheFile;//一次发送的文件的缓冲区

//拿UDP和TCP报文格式缝合了一下，一共10字节
struct MyPacketHeader
{
	//UDP、TCP开头都是源目的端口，这里就不用了
	u_short datalength;//数据包里数据的长度
	u_short verification;//校验和
	u_short seq;//序列号，这里是16位
	u_short ack;//ack，这里是16位
	char flag; //为1是SYN，为2是FIN，为3是发送文件名字，为4是发送文件的最后一个数据包
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
	shake_header.verification = 0;
	int seq = rand() % 100;//握手时seq得是随机数
	shake_header.seq = seq;//握手时seq得是随机数
	shake_header.flag = 1;
	u_short veri = checkerror((u_short*)&shake_header, sizeof(shake_header));
	shake_header.verification = veri;
	char* buffer = new char[sizeof(shake_header)];

	memcpy(buffer, &shake_header, sizeof(shake_header));//转成char*
	//第一次握手
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
	//sender接收receiver的第二次握手
	while (recvfrom(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, &receiver_address_len) <= 0)
	{
		if (clock() - start > 500)//超时，重新第一次握手
		{
			shake_header.flag = 1;
			shake_header.verification = 0;
			shake_header.seq = seq;
			shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header));

			memcpy(buffer, &shake_header, sizeof(shake_header));
			sendto(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
			start = clock();
			cout << "握手超时，尝试重传。";
		}
	}

	cout << "第一次握手成功！" << endl;
	memcpy(&shake_header, buffer, sizeof(shake_header));
	if (shake_header.flag_ack == 1 && shake_header.flag == 1 && shake_header.ack == seq + 1) //接收方发来的是SYN、ACK报文，并且ack得是seq+1
	{
		if (checkerror((u_short*)&shake_header, sizeof(shake_header) == 0))
		{
			cout << "收到第二次握手。" << endl;
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
	int seq_new = shake_header.seq;
	//第三次握手开始
	shake_header.flag_ack = 1;
	shake_header.seq = seq + 1;
	shake_header.ack = seq_new + 1;
	shake_header.verification = 0;
	shake_header.verification = checkerror((u_short*)&shake_header, sizeof(shake_header)); //重置校验和

	memcpy(buffer, &shake_header, sizeof(shake_header));//转成char*
	sendto(socket_sender, buffer, sizeof(shake_header), 0, (sockaddr*)&receiver_address, receiver_address_len);
	cout << "三次握手完成！接下来发送数据。" << endl;
	return true;
}

bool wave_hand()
{
	MyPacketHeader wave_header;
	//wave_header = recvheader;
	char* buffer = new char[sizeof(wave_header)];
	//memcpy(&wave_header, buffer, sizeof(wave_header));
	//cout << "成功接收第一次挥手信息" << endl;


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
			cout << "成功接收第一次挥手信息" << endl;
			break;
		}
	}
	int old_seq = wave_header.seq;
	//发送第二次挥手信息
	wave_header.flag_ack = 1;
	wave_header.flag = 2;//ACK、FIN报文
	wave_header.verification = 0;
	int new_seq = rand() % 100;
	wave_header.seq = new_seq;
	wave_header.datalength = 0;
	wave_header.ack = old_seq + 1;
	wave_header.verification = checkerror((u_short*)&wave_header, sizeof(wave_header));
	memcpy(buffer, &wave_header, sizeof(wave_header));
	if (sendto(socket_sender, buffer, sizeof(wave_header), 0, (sockaddr*)&receiver_address, receiver_address_len))
		cout << "发送第二次挥手" << endl;

	cout << "两次挥手结束，连接断开！" << endl;
	return true;
}

bool one_recv()
{
	//用老法失败了o(╥﹏╥)o
	
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
		cout << "接收超时！" << endl;
		return 0;
	}
	else if (ret < 0)
	{
		//cout << "fucku" << endl;
		cout << "接收出错！" << endl;
		return 0;
	}
	char* recv_buffer = new char[sizeof(MyPacketHeader)];//接收10字节缓冲区	
	int recv_return = recvfrom(socket_sender, recv_buffer, sizeof(MyPacketHeader), 0, (sockaddr*)&receiver_address, &receiver_address_len);

	MyPacketHeader recvpacketheader;
	memcpy(&recvpacketheader, recv_buffer, sizeof(recvpacketheader));
	if (recvpacketheader.flag_ack == 1)
	{
		base = recvpacketheader.ack;
		cout << "base挪动为" << base << endl;
		return 1;
	}

	//这个法也不行
	//char* recv_buffer = new char[sizeof(MyPacketHeader)];//接收10字节缓冲区	
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
		//超时情形，即图3-20右边的情况
		if (onepacket_end - onepacket_start >= MAX_TIME)
		{
			onepacket_start = clock();
			//此时要重传，重传就是要将base和nextseqnum之间的报文全部再发出去
			char* resendpacket = new char[MTU];//分包发送

			for (int i = base;i < nextseqnum && base < nextseqnum;i++)
			{
				MyPacketHeader resendpakcetheader;
				memcpy(resendpacket, &resendpakcetheader, sizeof(resendpakcetheader));
				//一个数据包的长度，1490或者不足1490的情况
				int packetsize = min(filesize - i * (MTU - 10), MTU - 10);
				resendpakcetheader.datalength = packetsize;

				for (int j = 0;j < packetsize;j++)
				{
					resendpacket[j + 10] = TheFile[j + i * (MTU - 10)];
				}

				resendpakcetheader.seq = i;
				resendpakcetheader.ack = 1;
				if (i == packetnum - 1)//如果发了最后一个包
					resendpakcetheader.flag = 4;
				resendpakcetheader.verification = checkerror((u_short*)&resendpakcetheader, sizeof(resendpakcetheader));//校验和
				memcpy(resendpacket, &resendpakcetheader, sizeof(resendpakcetheader));
				cout << "重传序列号为" << resendpakcetheader.seq << "的报文" << endl;
				sendto(socket_sender, resendpacket, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			}
		}
	}
	cout << "退出时间线程" << endl;
	_endthread();
}

void myrecv(void*)
{
	char* recv_buffer = new char[sizeof(MyPacketHeader)];//接收10字节缓冲区
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

	cout << "退出接收线程" << endl;
	_endthread();
}



void sendfilename(string filename)
{
	MyPacketHeader filenameheader;
	filenameheader.datalength = filename.size();
	filenameheader.flag = 3;//发送文件名字
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
		if (clock() - start > MAX_TIME)  //还得看一看重传
		{
			filenameheader.datalength = sizeof(filename);
			filenameheader.verification = 0;
			filenameheader.flag = 3;//发送文件名字
			filenameheader.verification = checkerror((u_short*)&filenameheader, sizeof(filenameheader));
			memcpy(sendname, &filenameheader, sizeof(filenameheader));
			for (int i = 0;i < filename.size(); i++)
				sendname[10 + i] = filename[i];
			sendto(socket_sender, sendname, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			cout << "发送文件名称失败，尝试重传" << endl;
		}
	}
	cout << "发送文件名成功" << endl;
}

void send(string filename)
{
	cout << "开始发送" << filename << endl;
	seq = 0;//seq大小0-2^16
	base = 0;
	nextseqnum = 0;
	sendfilename(filename);

	ifstream fin((File_src_path + filename).c_str(), ifstream::binary);//以二进制方式打开文件
	fin.seekg(0, fin.end);//指针放于文件结尾
	filesize = fin.tellg();//获取文件大小
	fin.seekg(0, fin.beg);//指针置于文件头处
	TheFile = new char[filesize];
	memset(TheFile, 0, sizeof(char) * filesize);
	fin.read(TheFile, filesize);//文件数据放入TheFile中
	fin.close();
	packetnum = filesize / (MTU - sizeof(MyPacketHeader)); //需要发送的分组数量 sizeof(MyPacketHeader)=10
	if (filesize % (MTU - sizeof(MyPacketHeader)) == 0)
		packetnum += 0;
	else
		packetnum += 1;

	cout << "该文件需要发送" << packetnum << "个数据包" << endl;
	char* sendpacket = new char[MTU];//分包发送

	while (base!= packetnum)
	{
		//这是rdt_send(data)，发送nextseqnum处的分组，对应图3-20的最上面那种情况
		//先决条件，base与nextseqnum在窗口以内，nextseqnum没有超过总包数
		if (nextseqnum < base + window && nextseqnum < packetnum)
		{
			MyPacketHeader sendpacketheader;
			memcpy(sendpacket, &sendpacketheader, sizeof(sendpacketheader));
			//一个数据包的长度，1490或者不足1490的情况
			int packetsize = min(filesize - nextseqnum * (MTU - 10), MTU - 10);
			sendpacketheader.datalength = packetsize;

			for (int i = 0;i < packetsize;i++)
			{
				sendpacket[i + 10] = TheFile[i + nextseqnum * (MTU - 10)];
			}

			//sendpacketheader.seq = seq;//序列号
			//sendpacketheader.ack = seq + 1;
			if (nextseqnum == packetnum - 1)
			{
				sendpacketheader.flag = 4; //当发送最后一个包时，flag置4
			}

			sendpacketheader.seq = nextseqnum;
			sendpacketheader.verification = checkerror((u_short*)&sendpacketheader, sizeof(sendpacketheader));//校验和
			memcpy(sendpacket, &sendpacketheader, sizeof(sendpacketheader));
			sendto(socket_sender, sendpacket, MTU, 0, (sockaddr*)&receiver_address, receiver_address_len);
			cout << "正常发送报文。";
			cout << "序列号和nextseqnum是" << nextseqnum << " " << "base是" << base << endl;
			//		clock_t start = clock();//记录发送时间
			if (base == nextseqnum)//打开计时器
				onepacket_start = clock();

			//发一次包，nextseqnum加一次
			nextseqnum++;//序列号递增

		}

		//第一次发送时开两个线程，一个负责计时，一个负责接收
		if (nextseqnum == 1)
		{
			_beginthread(mytimechecker, 0, NULL);
			_beginthread(myrecv, 0, NULL);//开启接收消息的线程
		}

	}



	cout << "成功发送 " << filename << endl;
}


int main()
{
	srand((unsigned)time(NULL));
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "winsock失败。" << endl;
		return 0;
	}

	socket_sender = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_sender == INVALID_SOCKET)
	{
		cout << "服务端socket创建失败：" << WSAGetLastError() << endl << endl;
		WSACleanup();
		return false;
	}


	SOCKADDR_IN sender_address;
	sender_address.sin_family = AF_INET;
	sender_address.sin_port = htons(4395); //定义端口号
	sender_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	//给发送端先绑上
	if (SOCKET_ERROR == bind(socket_sender, (SOCKADDR*)&sender_address, sizeof(sender_address)))
	{
		cout << "绑定失败:" << WSAGetLastError() << endl;
		closesocket(socket_sender);
		WSACleanup();
		return false;
	}

	//需要定义路由器的地址，发送文件时得用
	receiver_address.sin_family = AF_INET;
	receiver_address.sin_port = htons(4399); //定义端口号
	receiver_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");



	//先执行握手
	if (shake_hand())
	//if (1)
	{
		//握手成功后，发送数据
		start_time = clock();

		send("1.jpg");
		cout << "结束时，base是" << base << " nextseqnum是" << nextseqnum << " packetnum是" << packetnum << endl;

		end_time = clock();
		double time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
		cout << "发送时间：" << time << " s" << endl;
		cout << "传输总大小：" << filesize << " Bytes" << endl;
		cout << "吞吐率：" << (double)((filesize * 8 / 1000) / time) << " Kbps" << endl;

		//wave_hand();
	}



	closesocket(socket_sender);
	WSACleanup();



	return 0;
}
