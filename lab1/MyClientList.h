//感恩github
#pragma once
#include<iostream>
#include<cstring>
#include<string>
#include<ctime>
#include<stdlib.h>
#include<WinSock2.h>
#include<stdio.h>
using namespace std;

typedef struct client
{
public:
	SOCKET the_socket;      //客户端套接字
	char buffer[296];		 //数据缓冲区
	char username[33];   //客户端用户名
	char IP[20];		 //客户端IP
	unsigned short port; // 客户端端口
	client* next;
}*Clients;

Clients client_head = (Clients)malloc(sizeof(client));
int client_num = 0;
void init()
{
	client_head->next = NULL;
}

Clients GetMyHead()
{
	return client_head;
}

void AddClient(Clients new_c)
{
	if (client_num == 0)//新客户端是第一个，直接做表头
	{
		client_head = new_c;
		new_c->next = nullptr;
	}
	else//新客户不是第一个
	{
		Clients p0 = client_head;
		while (p0->next != nullptr)//当p0的next是nullptr时，我们把新客户端插它后面
		{
			p0 = p0->next;
		}
		p0->next = new_c;
		new_c->next = nullptr;

		string systeminfo = "【系统消息】";
		for (int i = systeminfo.length();i < 32;i++)
			systeminfo += " ";

		time_t timep;
		time(&timep);
		char tmp[256];
		strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep));
		string now_time = tmp;
		string one_happeynews = new_c->username;
		one_happeynews += "加入聊天";

		string message_to_old = systeminfo;
		message_to_old += now_time;
		message_to_old += one_happeynews;
		char the_message_to_old[256];
		strcpy_s(the_message_to_old, message_to_old.c_str());


		string message_to_new = systeminfo;
		message_to_new += now_time;
		message_to_new += "已在聊天室的有：";
		

		Clients p1 = client_head;
		message_to_new += p1->username;
		send(p1->the_socket, the_message_to_old, sizeof(the_message_to_old), 0);
		p1 = p1->next;
		//每一次新加入一位用户，就向已加入的其余客户端广播。new_c默认就是链表最后一个。
		while (p1 != new_c && p1!= nullptr)
		{
			message_to_new += ',';
			message_to_new += p1->username;
			send(p1->the_socket, the_message_to_old, sizeof(the_message_to_old), 0);
			p1 = p1->next;
		}
		char the_message_to_new[256];
		strcpy_s(the_message_to_new, message_to_new.c_str());
		send(new_c->the_socket, the_message_to_new, sizeof(the_message_to_new), 0);
	}
	client_num++;
}

void DeleteClient(Clients gone_c)
{
	Clients p1 = client_head;
	if (p1 == gone_c)//如果要删的客户端就是头
	{
		client_head = gone_c->next;
	}
	else
	{
		while (p1->next != gone_c)
		{
			p1 = p1->next;
		}
		p1->next = gone_c->next;
	}

	string systeminfo = "【系统消息】";
	for (int i = systeminfo.length();i < 32;i++)
		systeminfo += " ";

	time_t timep;
	time(&timep);
	char tmp[256];
	strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep));
	string now_time = tmp;

	string message_to_stay = systeminfo;
	message_to_stay += now_time;
	message_to_stay += gone_c->username;
	message_to_stay += "已经离开";
	char the_message_to_stay[256];
	strcpy_s(the_message_to_stay, message_to_stay.c_str());
	
	Clients p2 = client_head;
	while (p2 != nullptr)
	{
		send(p2->the_socket, the_message_to_stay, sizeof(the_message_to_stay), 0);
		p2 = p2->next;
	}
	client_num--;
}
