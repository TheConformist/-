#pragma once
#include<stdlib.h>
#include<WinSock2.h>
#include<stdio.h>
#include<iostream>
#include<cstring>
#include<string>
#include<ctime>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
class Message
{
public:
	//char username[33];
	string username;
	string now_time;
	char msg[256];
	Message()
	{
		memset(msg, 0, 256);
	}
	Message(string user, string thetime, char themsg[256])
	{
		username = user;
		//strcpy_s(username, user);
		now_time = thetime;
		strcpy_s(msg, themsg);
	}
};

//char数组转为类
Message string_to_message(char buf[])
{
	string text = buf;
	string username;
	Message res_msg;
	//数组的前十位是用户名，中间八位是时间，最后都是message
	res_msg.username = text.substr(0, 32);//从第0个字符串开始，连着取32个字符，就是用户名
	//strcpy_s(res_msg.username, username.c_str());

	res_msg.now_time = text.substr(32, 8);
	strcpy_s(res_msg.msg, text.substr(40).c_str());
	return res_msg;
}

char* message_to_string(Message a)
{
	string str_msg = a.msg;
	string str_res = a.username + a.now_time + str_msg;
	char res[296];
	strcpy_s(res, str_res.c_str());
	return res;
}