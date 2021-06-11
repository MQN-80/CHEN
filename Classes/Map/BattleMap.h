#pragma once
#include"cocos2d.h"
#include"Monster.h"
#include"MonsterPistolAmmo.h"
#include"AllTag.h"
#include"Item.h"
#include"TreasureBoxes.h"

class NormalBattleScene;
class TreasureBoxes;
USING_NS_CC;
class BattleMap :public Node
{
public:
	virtual bool init();

	void createMonster(int MonsterNum);
	void bindScene(NormalBattleScene* scene);
	virtual void MonsterUpdate(float dt);

	TMXTiledMap* getBattleMap();
	int getNumber();
	void setNumber(int i);
	Vector<Monster*> getMonster();
	std::vector <TreasureBoxes*> getBox();
	std::vector<Item*> getItems();
	std::vector<Item*> getDrops();
	void BoxInit();
	void BoxCreate();
	void ItemInit();
	void ItemCreate();
	virtual void ItemInBoxUpdate(float dt);

	void DropsInit();
	void DropsCreate();
	void DropsUpdate(float dt);
	void MapInit(int mode);

	CREATE_FUNC(BattleMap);
private:
	int Number;//��¼��ͼ�ڳ����ı��
	TMXTiledMap* m_map;
	Vector<Monster*> m_monster;
	std::vector <TreasureBoxes*> m_box; //����ֺ���ֵ�box
	std::vector<Item*>m_Items;//���������Item
	std::vector<Item*>m_Drops;//��ֵĵ�����
	NormalBattleScene* m_scene;
};