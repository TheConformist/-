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
		// ���û���յ���Ϣ����2s֮���ٴν�����Ϣ
		if (mes_res == SOCKET_ERROR)
		{
			Sleep(2000);
			continue;
		}
		// �ǿ���Ϣʱֱ�����
		if (strlen(buf) != 0)
		{
			cout << "��������Ϣ��";
			cout << myrecv.now_time << " ";
			//�û�����32λ���ַ�����Ҫɾȥ�������Ŀո�
			cout << myrecv.username.erase(myrecv.username.find_last_not_of(" ") + 1) << "��";
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
		//�û�����message
		getline(cin, message);
		//��ȡʱ��
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


		//�û�����exit���Ƴ�����
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
		else //��������
		{
			mes_res = send(*(SOCKET*)lpParameter, message_to_send, sizeof(message_to_send), 0);
			cout << endl;
			cout << "��������Ϣ��";
			cout << now_time << " " << username << "��" << message << endl << endl;
			//cout << totalmessage << endl << endl;
		}
		if (mes_res == SOCKET_ERROR) {
			cout << endl;
			cout << "������Ϣʧ�ܣ�" << endl << endl;
			return false;
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
	socket_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_client == INVALID_SOCKET)
	{
		cout << "�ͻ���socket����ʧ�ܣ�" << WSAGetLastError() << endl << endl;
		return false;
	}

	//������Ҫ���������������]
	SOCKADDR_IN server_address;
	USHORT port = 4396;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port); //����˿ں�
	char IP[20];
	cout << "������IP��ַ��";
	cin.getline(IP, 20);
	server_address.sin_addr.S_un.S_addr = inet_addr(IP);
	if (SOCKET_ERROR == connect(socket_client, (SOCKADDR*)&server_address, sizeof(server_address)))
	{
		cout << "����ʧ�ܣ�" << WSAGetLastError() << endl << endl;
		closesocket(socket_client);
		WSACleanup();
		return false;
	}
	string abuffer;
	cout << "���ӳɹ�!" << endl << endl;
	//������֮������username
	cout << "��ѡ������û�����";
	cin >> str_user;
	getline(cin, abuffer);
	strcpy_s(username, str_user.c_str());
	//cin.getline(username, 32);

	//����ַ��������㱨�Ķ�����ǰ32λ����username��
	for (int i = str_user.length();i < 32;i++)
		str_user += " ";

	cout << endl << endl;
	send(socket_client, username, sizeof(username), 0);//������������û���
	CreateThread(NULL, 0, &RecvThread, &socket_client, 0, NULL);
	// ����һ�����ݷ����̣߳�����������������ʱ�̷���������Ϣ��
	CreateThread(NULL, 0, &SendThread, &socket_client, 0, NULL);
	// �����߳����ߣ��������ر�TCP����
	for (int k = 0;k < 1000;k++)
	{
		Sleep(1000000000);
	}
	closesocket(socket_client);
	WSACleanup();
	return 0;
}