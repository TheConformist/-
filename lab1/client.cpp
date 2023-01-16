#include<stdlib.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<cstring>
#include<string>
#include<ctime>
#include "message.h"
#pragma comment(lib,"ws2_32.lib")
using namespace std;
SOCKET socket_client;
char username[33];
string str_user;


DWORD WINAPI RecvThread(LPVOID lpParameter)
{
	char buf[296] = { 0 };
	int mes_res = 0;
	while (1) {
		mes_res = recv(*(SOCKET*)lpParameter, buf, sizeof(buf), 0);
		Message myrecv = string_to_message(buf);
		// 如果没有收到消息，就2s之后再次接收消息
		if (mes_res == SOCKET_ERROR)
		{
			Sleep(2000);
			continue;
		}
		// 非空消息时直接输出
		if (strlen(buf) != 0)
		{
			cout << "【接收消息】";
			cout << myrecv.now_time << " ";
			//用户名是32位的字符串，要删去后面多余的空格
			cout << myrecv.username.erase(myrecv.username.find_last_not_of(" ") + 1) << "：";
			cout << myrecv.msg << endl << endl;
		}
	}
	return 0;
}

DWORD WINAPI SendThread(LPVOID lpParameter)
{
	string message;
	char message_to_send[296] = { 0 };
	int mes_res = 0;
	while (1)
	{
		//用户输入message
		getline(cin, message);
		//获取时间
		time_t timep;
		time(&timep);
		char tmp[256];
		strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep));
		string now_time = tmp;
		string totalmessage = str_user;
		totalmessage += now_time;
		//totalmessage += " ";

		totalmessage += message;
		strcpy_s(message_to_send, totalmessage.c_str());


		//用户输入exit，推出聊天
		if (message == "exit")
		{
			mes_res = send(*(SOCKET*)lpParameter, message_to_send, sizeof(message_to_send), 0);
			closesocket(socket_client);
			cout << endl;
			cout << now_time << "  ";
			cout << "byebye" << endl << endl;
			WSACleanup();
			return 0;
		}
		else //正常聊天
		{
			mes_res = send(*(SOCKET*)lpParameter, message_to_send, sizeof(message_to_send), 0);
			cout << endl;
			cout << "【发送消息】";
			cout << now_time << " " << username << "：" << message << endl << endl;
			//cout << totalmessage << endl << endl;
		}
		if (mes_res == SOCKET_ERROR) {
			cout << endl;
			cout << "发送消息失败！" << endl << endl;
			return false;
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
	socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_client == INVALID_SOCKET)
	{
		cout << "客户端socket创建失败：" << WSAGetLastError() << endl << endl;
		return false;
	}

	//接下来要与服务器建立连接]
	SOCKADDR_IN server_address;
	USHORT port = 4396;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port); //定义端口号
	char IP[20];
	cout << "请输入IP地址：";
	cin.getline(IP, 20);
	server_address.sin_addr.S_un.S_addr = inet_addr(IP);
	if (SOCKET_ERROR == connect(socket_client, (SOCKADDR*)&server_address, sizeof(server_address)))
	{
		cout << "连接失败：" << WSAGetLastError() << endl << endl;
		closesocket(socket_client);
		WSACleanup();
		return false;
	}
	string abuffer;
	cout << "连接成功!" << endl << endl;
	//连接上之后，输入username
	cout << "请选择你的用户名：";
	cin >> str_user;
	getline(cin, abuffer);
	strcpy_s(username, str_user.c_str());
	//cin.getline(username, 32);

	//填充字符串，满足报文定义中前32位均是username。
	for (int i = str_user.length();i < 32;i++)
		str_user += " ";

	cout << endl << endl;
	send(socket_client, username, sizeof(username), 0);//向服务器发送用户名
	CreateThread(NULL, 0, &RecvThread, &socket_client, 0, NULL);
	// 创建一个数据发送线程，可以在聊天室任意时刻发送任意消息；
	CreateThread(NULL, 0, &SendThread, &socket_client, 0, NULL);
	// 让主线程休眠，不让它关闭TCP连接
	for (int k = 0;k < 1000;k++)
	{
		Sleep(1000000000);
	}
	closesocket(socket_client);
	WSACleanup();
	return 0;
}