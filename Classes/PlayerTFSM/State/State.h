#pragma once
#include"Player.h"
/*
* ����
* ��ʵ������ʵ��
*/
class State
{
public:
	virtual void execute(Player* player) = 0;
private:

};