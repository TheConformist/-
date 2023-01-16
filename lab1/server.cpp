//感谢github的学姐提供的宝贵思路，本代码全系本人独立完成！
#include<stdlib.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<ctime>
#pragma comment(lib,"ws2_32.lib")
#include "message.h"
#include "MyClientList.h"
using namespace std;

#include<stdlib.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<ctime>
#pragma comment(lib,"ws2_32.lib")
#include "message.h"
using namespace std;


SOCKET socket_server;
SOCKADDR_IN new_clientaddr = { 0 }; //客户端地址
SOCKADDR_IN serveraddr = { 0 };	//服务端地址
int client_address_len = sizeof(new_clientaddr);

DWORD WINAPI SendThread(LPVOID lpParameter)
{
	Clients user = (Clients)lpParameter;
	Message myrecv = string_to_message(user->buffer);
	cout << myrecv.now_time << " ";
	cout << user->username << "：" << myrecv.msg << endl << endl;

	Clients p = GetMyHead();
	while (p != nullptr)
	{
		if (p != user)
		{
			strcpy_s(p->buffer, user->buffer);
			send(p->the_socket, p->buffer, sizeof(p->buffer), 0);
		}
		p = p->next;
	}

	return 0;
}

//接受每个用户发来的信息
DWORD WINAPI RecvThread(LPVOID lpParameter)
{
	int mes_res = 0;
	char buffer[296];
	while (1)
	{
		/*cout << "hhhhh";*/
		Clients user = (Clients)lpParameter;
		mes_res = recv(user->the_socket, buffer, sizeof(buffer), 0);
		if (mes_res == SOCKET_ERROR)
		{
			return false;
		}
		Message myrecv = string_to_message(buffer);
		//有人输入exit，则退出
		if (strcmp(myrecv.msg, "exit") == 0)
		{
			cout << user->username;
			cout << "已经退出" << endl << endl;
			DeleteClient(user);
		}
		else
		{
			strcpy_s(user->buffer, buffer);
			CreateThread(NULL, 0, &SendThread, user, 0, NULL);
		}
	}
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	init();
	while (1)
	{
		Clients new_client = (Clients)malloc(sizeof(client));
		new_client->the_socket = accept(socket_server, (SOCKADDR*)&new_clientaddr, &client_address_len);
		if (new_client->the_socket == INVALID_SOCKET)
		{
			cout << "接受失败:" << WSAGetLastError() << endl;
			closesocket(socket_server);
			WSACleanup();
			return false;
		}
		recv(new_client->the_socket, new_client->username, sizeof(new_client->username), 0); //接受输入用户名
		memcpy(new_client->IP, inet_ntoa(new_clientaddr.sin_addr), sizeof(new_client->IP));
		new_client->next = nullptr;
		new_client->port = htons(new_clientaddr.sin_port);
		AddClient(new_client);
		
		cout << "欢迎" << new_client->username << "加入聊天！" << endl << endl;
		Clients p = GetMyHead();
		while (p!=nullptr)
		{
			CreateThread(NULL, 0, &RecvThread, p, 0, NULL);
			p = p->next;
		}
	}
	return 0;
}

int main()
{
	cout << "服务器开动" << endl << endl;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout << "winsock失败。" << endl;
		return 0;
	}

	socket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_server == INVALID_SOCKET)
	{
		cout << "服务端socket创建失败:" << WSAGetLastError() << endl;
		return 0;
	}

	//bind操作
	SOCKADDR_IN server_address;
	USHORT port = 4396;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port); //定义端口号
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	//执行bind函数
	if (SOCKET_ERROR == bind(socket_server, (SOCKADDR*)&server_address, sizeof(server_address)))
	{
		cout << "绑定失败:" << WSAGetLastError() << endl;
		closesocket(socket_server);
		return false;
	}

	if (SOCKET_ERROR == listen(socket_server, 20))
	{
		cout << "监听失败:" << WSAGetLastError() << endl;
		closesocket(socket_server);
		WSACleanup();
		return false;
	}
	cout << "等待连接……" << endl << endl;
	CreateThread(NULL, 0, &AcceptThread, NULL, 0, 0);
	// 让主线程休眠，不让它关闭TCP连接
	for (int k = 0;k < 1000;k++)
	{
		Sleep(1000000000);
	}
	closesocket(socket_server);
	WSACleanup();

	return 0;
}