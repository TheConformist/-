//�ж�github
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
	SOCKET the_socket;      //�ͻ����׽���
	char buffer[296];		 //���ݻ�����
	char username[33];   //�ͻ����û���
	char IP[20];		 //�ͻ���IP
	unsigned short port; // �ͻ��˶˿�
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
	if (client_num == 0)//�¿ͻ����ǵ�һ����ֱ������ͷ
	{
		client_head = new_c;
		new_c->next = nullptr;
	}
	else//�¿ͻ����ǵ�һ��
	{
		Clients p0 = client_head;
		while (p0->next != nullptr)//��p0��next��nullptrʱ�����ǰ��¿ͻ��˲�������
		{
			p0 = p0->next;
		}
		p0->next = new_c;
		new_c->next = nullptr;

		string systeminfo = "��ϵͳ��Ϣ��";
		for (int i = systeminfo.length();i < 32;i++)
			systeminfo += " ";

		time_t timep;
		time(&timep);
		char tmp[256];
		strftime(tmp, sizeof(tmp), "%H:%M:%S", localtime(&timep));
		string now_time = tmp;
		string one_happeynews = new_c->username;
		one_happeynews += "��������";

		string message_to_old = systeminfo;
		message_to_old += now_time;
		message_to_old += one_happeynews;
		char the_message_to_old[256];
		strcpy_s(the_message_to_old, message_to_old.c_str());


		string message_to_new = systeminfo;
		message_to_new += now_time;
		message_to_new += "���������ҵ��У�";
		

		Clients p1 = client_head;
		message_to_new += p1->username;
		send(p1->the_socket, the_message_to_old, sizeof(the_message_to_old), 0);
		p1 = p1->next;
		//ÿһ���¼���һλ�û��������Ѽ��������ͻ��˹㲥��new_cĬ�Ͼ����������һ����
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
	if (p1 == gone_c)//���Ҫɾ�Ŀͻ��˾���ͷ
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

	string systeminfo = "��ϵͳ��Ϣ��";
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
	message_to_stay += "�Ѿ��뿪";
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
