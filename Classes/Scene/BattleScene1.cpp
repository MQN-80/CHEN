#include"BattleScene1.h"
#include"music/music.h"
#include"Map/BattleMap.h"
#include"Player.h"
#include"PlayerMove.h"
#include"PlayerAttribute.h"
#include"Pistol.h"
#include"PistolAmmo.h"
#include"Monster.h"
#include"RemoteMonster.h"
#include"MonsterDashAmmo.h"
#include"TreasureBoxes.h"
#include"miniMapTab.h"
#include"SafeScene.h"
#include"HealingVial.h"
#include"WeaponManager.h"
#include"Coin.h"
#include"PitchForkAmmo.h"
#include"StartScene.h"
#define PI 3.1415926f
static long long SwordEnd;
static long long PistolEnd;
//��¼��һ���ӵ���ʧ��ʱ�̣������������ӵ�ʱ����������˼��0.3s
bool BattleScene1::init()
{
	
	srand((unsigned)time(NULL));//����ʱ��ȡ�������
	auto visibleSize = Director::getInstance()->getVisibleSize();
	
	if (initWithPhysics()) {
		getPhysicsWorld()->setGravity(Vec2::ZERO);
		getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
		getPhysicsWorld()->setSubsteps(20);
	}
	getmusicManager()->changeMusic("bgm/Room.mp3");
	Mapinit();
	//���ð�ť
	auto settings = MenuItemImage::create("ui/settings.png", "ui/settings.png", [&](Ref* sender) {
		getmusicManager()->effectPlay("effect/button.mp3");
		getmusicManager()->menu(this);
	});
	//λ��λ�����Ͻ�
	settings->setPosition(visibleSize.width - settings->getContentSize().width / 2, visibleSize.height - settings->getContentSize().height / 2);
	//�˵�
	auto menu = Menu::create(settings, nullptr);
	menu->setPosition(Vec2::ZERO);
	addChild(menu, 3);

	//��Ҵ���
	auto physicsBody = PhysicsBody::createBox(Size(40.0f, 40.0f),
		PhysicsMaterial(0.0f, 0.0f, 0.0f));
	physicsBody->setDynamic(false);
	bindPlayer(Player::create());
	getPlayer()->setTag(AllTag::Player_TAG);
//	getPlayer()->PistolInit();//��ǹ
//	getPlayer()->SwordInit();//��
	/*
	bindWeaponManager(WeaponManager::create());
	getWeaponManager()->bindPlayer(getPlayer());
	getWeaponManager()->getWeapon(getPlayer()->getWeapon1(), getPlayer()->getWeapon2());//����WeaponManager�����ɫ��
	*/
	getPlayer()->addComponent(physicsBody);
	getPlayer()->getPhysicsBody()->setCategoryBitmask(0x0010);
	getPlayer()->getPhysicsBody()->setCollisionBitmask(0x0010);
	getPlayer()->getPhysicsBody()->setContactTestBitmask(0x0010);
	getPlayer()->setPosition(64 * 2 + 32, 64 * 2 + 32);
	getPlayer()->getplayermove()->bindMap(parentMap->getBattleMap());//PlayerMove�������ͼ��
	getPlayer()->getPlayerAttribute()->setPosition(getPlayer()->getPlayerAttribute()->getSprite()->getContentSize().width / 2,
		visibleSize.height - getPlayer()->getPlayerAttribute()->getSprite()->getContentSize().height / 2);//����UIλ������
	this->addChild(getPlayer()->getPlayerAttribute(), 4);
	this->addChild(getPlayer(), 2);
	//���䴴��,Ѫƿ����
	if (parentMap->getBattleMap()->getTag() == NormalRoom_TAG)
	{
		parentMap->BoxInit();
		parentMap->getBox().back()->bindScene(this);
		parentMap->BoxCreate();
		parentMap->getBox().back()->BoxBirth(1);

		parentMap->ItemInit();//���������Ʒ����
		parentMap->getItems().back()->bindScene(this);
		parentMap->ItemCreate();

		parentMap->schedule(CC_SCHEDULE_SELECTOR(BattleMap::ItemInBoxUpdate), 0.1f);
	}


	parentMap->DropsInit();//��ֵĵ�����
	for (int i = 0; i < parentMap->getDrops().size(); i++)
	{
		parentMap->getDrops().at(i)->bindScene(this);
	}
	parentMap->DropsCreate();
	parentMap->schedule(CC_SCHEDULE_SELECTOR(BattleMap::DropsUpdate), 0.1f);

	//С��ͼ����
	MiniMap = miniMapTab::create();
	MiniMap->bindBattleScene(this);
	this->addChild(MiniMap, 10);
	MiniMap->setScale(1);
	MiniMap->setPosition(Vec2(visibleSize.width-MiniMap->getSprite()->getContentSize().width/2
	, MiniMap->getSprite()->getContentSize().height / 2));

	

	////eventlistener,���̼����������ƶ�����
	auto myKeyListener = EventListenerKeyboard::create();
	myKeyListener->onKeyPressed = [=](EventKeyboard::KeyCode keycode, cocos2d::Event* event)//���̰���ʱ��Ӧ
	{
		getPlayer()->getplayermove()->TrueKeyCode(keycode);//PlayerMove��keyMap�Ķ�Ӧ������true
		if (keycode==EventKeyboard::KeyCode::KEY_E)//��E����
		{
			inGate();
			for (int i = 0; i < parentMap->getBox().size(); i++)
			{
				if(parentMap->getBattleMap()->getTag()==NormalRoom_TAG)
					parentMap->getBox().at(i)->Interact(" ");
				else if(parentMap->getBattleMap()->getTag()==ShopRoom_TAG)
					parentMap->getBox().at(i)->Interact("$10");
			}


			
		}
		if (keycode == EventKeyboard::KeyCode::KEY_Q)//��Q�л�����
		{
			
			if (getPlayer()->getWeapon2()!=nullptr)
			{
				getPlayer()->swapWeapon();
			}

			

		}
		if (keycode == EventKeyboard::KeyCode::KEY_SPACE)//���ո񿪼���
		{
			getPlayer()->HeroSkill(0);
		}
	};
	
	myKeyListener->onKeyReleased = [=](EventKeyboard::KeyCode keycode, cocos2d::Event* event)//�����ɿ�ʱ��Ӧ
	{
		getPlayer()->getplayermove()->FalseKeyCode(keycode);
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(myKeyListener, this);

	//������
	auto myMouseListener = EventListenerMouse::create();
	myMouseListener->onMouseMove = [=](cocos2d::Event* event)//�ƶ�
	{
		EventMouse* e = (EventMouse*)event;
		getPlayer()->changeMouseLocation(e->getLocation());


	};
	myMouseListener->onMouseDown = [=](cocos2d::Event* event)//����
	{
		EventMouse* e = (EventMouse*)event;
		getPlayer()->trueMouseMap(e->getMouseButton());

	};
	myMouseListener->onMouseUp = [=](cocos2d::Event* event)//�ɿ�
	{
		EventMouse* e = (EventMouse*)event;
		getPlayer()->flaseMouseMap(e->getMouseButton());
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(myMouseListener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(BattleScene1::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	this->scheduleUpdate();
//	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::test), 1.0f);
	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::DeleteAmmo), 0.001f);
	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::Ammoupdate), 0.01f);//ÿ0.01s�������ӵ��Ƿ���Ҫ����
	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::AmmoUpdate_Monster), 0.3f);//�����ӵ�
	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::LandDamageUpdate), 0.5f);//�����˺�
	this->schedule(CC_SCHEDULE_SELECTOR(BattleScene1::PlayerDeatheUpdate), 1.0f);//
	MapGateInit();
	return 1;
}
void BattleScene1::MapGateInit()//��ͼ�ŵĳ�ʼ��
{
	for (int i = 1; i <= 4; i++)
	{
		m_mapgate.pushBack(MapGate::create());
		m_mapgate.back()->bindPlayer(getPlayer());
		this->addChild(m_mapgate.back(), 5);
		m_mapgate.back()->IsAble(false);//��ʼĬ�ϸ��Ų�����
	}

	//��
	m_mapgate.at(0)->setPosition(Director::getInstance()->getVisibleSize().width / 2,
		Director::getInstance()->getVisibleSize().height - m_mapgate.back()->getSprite()->getContentSize().height * 0.5 - 64);
	//��
	m_mapgate.at(1)->setPosition(Director::getInstance()->getVisibleSize().width / 2, m_mapgate.back()->getSprite()->getContentSize().height * 0.5 + 64);

	//��
	m_mapgate.at(2)->setPosition(m_mapgate.back()->getSprite()->getContentSize().width/2+64, Director::getInstance()->getVisibleSize().height / 2);
	//��
	m_mapgate.at(3)->setPosition(Director::getInstance()->getVisibleSize().width- m_mapgate.back()->getSprite()->getContentSize().width / 2-64,
		Director::getInstance()->getVisibleSize().height / 2);
}
void BattleScene1::inGate()//���ŵ��жϺʹ��͵�ʵ��
{
	for (int i = 0; i < 4; i++)
	{
		if (m_mapgate.at(i)->getAble())//��ȡ�����Ƿ����
		{
			if (m_mapgate.at(i)->isAround(getPlayer()->getPositionX(), getPlayer()->getPositionY()) )//�ж��������Χ
			{
//				CCLOG("%d", parentMap->getNumber());
				switch (i)
				{
				case 0://�ı��ͼ���ı����λ�ã�������
					changeMap(parentMap->getNumber() + 4);
					getPlayer()->setPosition(Director::getInstance()->getVisibleSize().width / 2, m_mapgate.back()->getSprite()->getContentSize().height * 0.5 + 64);
					break;
				case 1:
					changeMap(parentMap->getNumber() - 4);
					getPlayer()->setPosition(Director::getInstance()->getVisibleSize().width / 2,
						Director::getInstance()->getVisibleSize().height - m_mapgate.back()->getSprite()->getContentSize().height * 0.5 - 64);
					break;
				case 2:
					changeMap(parentMap->getNumber() - 1);
					getPlayer()->setPosition(Director::getInstance()->getVisibleSize().width - m_mapgate.back()->getSprite()->getContentSize().width / 2 - 64,
						Director::getInstance()->getVisibleSize().height / 2);
					break;
				case 3:
					changeMap(parentMap->getNumber() + 1);
					getPlayer()->setPosition(m_mapgate.back()->getSprite()->getContentSize().width / 2 + 64,
						Director::getInstance()->getVisibleSize().height / 2);
					break;
				default:
					break;
				}
				break;
			}
			
		}
	}
}
void BattleScene1::update(float dt)
{
	int able = 1;
	for (int i = 0; i < parentMap->getMonster().size(); i++)
	{
		if (parentMap->getMonster().at(i)->getIsDead()==0)//��������ͼ����һֻ��û��������Ϊ������ able=0
			able = 0;
	}
	int x = parentMap->getNumber()%4+1;
	int y = parentMap->getNumber() / 4+1;
	//���ݵ�ͼ�ı��λ��ѡ�����ŵĿ���������������ͼ���Ὺ����
	for (int i = 0; i < 4; i++)
	{
		m_mapgate.at(i)->IsAble(0);
	}
	if (x<=3 && m_battleMap.at(parentMap->getNumber()+1)->getName()!="no")
	{
		m_mapgate.at(3)->IsAble(able);
	}
	if (x>=2 && m_battleMap.at(parentMap->getNumber() - 1)->getName() != "no")
	{
		m_mapgate.at(2)->IsAble(able);
	}
	if (y>=2 && m_battleMap.at(parentMap->getNumber() - 4)->getName() != "no")
	{
		m_mapgate.at(1)->IsAble(able);
	}
	if (y<=3 && m_battleMap.at(parentMap->getNumber() + 4)->getName() != "no")
	{
		m_mapgate.at(0)->IsAble(able);
	}
	for (int i = 0; i < parentMap->getBox().size(); i++)
		parentMap->getBox().at(i)->setIsCanSee(able);
}
//����Pistol�ӵ�
void BattleScene1::AmmoUpdate_Monster(float dt)
{
	//��ȡ�������
	float Px = getPlayer()->getPositionX() - getPlayer()->getContentSize().width / 2;
	float Py = getPlayer()->getPositionY() - getPlayer()->getContentSize().height / 2;
	for (int i = 0; i < parentMap->getMonster().size(); i++)
	{
		bool CanShoot = 0;
		float Mx = parentMap->getMonster().at(i)->getPositionX();//��ȡ��������
		float My = parentMap->getMonster().at(i)->getPositionY();
		Vec2 m = Vec2(Vec2(Px - Mx, Py - My) / sqrt((Px - Mx) * (Px - Mx) + (Py - My) * (Py - My)));
//		CCLOG("%d", parentMap->getMonster().at(i)->getTag());
		if (parentMap->getMonster().at(i)->getIsDead() == 0 && //û��
			parentMap->getMonster().at(i)->CanSee()&&			//�������
			((parentMap->getMonster().at(i)->getTag()==AllTag::O_small_monster_TAG)||//��һ��Զ�̹�
			(parentMap->getMonster().at(i)->getTag() == AllTag::Y_craw_monster_TAG)))
		{

			m_monsterAmmoList.push_back(parentMap->getMonster().at(i)->MonsterAttack());//�����ӵ�
			m_monsterAmmoList.back()->start = clock();									//��ǳ���ʱ��
			m_monsterAmmoList.back()->setTag(AllTag::MonsterAmmo_PistolTAG);
			this->addChild(m_monsterAmmoList.back(), 3);
			auto physicBody = PhysicsBody::createBox(Size(28.0f, 28.0f), PhysicsMaterial(0, 0, 0));//���Ӹ���
			m_monsterAmmoList.back()->addComponent(physicBody);
			m_monsterAmmoList.back()->getPhysicsBody()->setDynamic(false);
			m_monsterAmmoList.back()->setPosition(parentMap->getMonster().at(i)->getPosition());
			m_monsterAmmoList.back()->getPhysicsBody()->setVelocity(m * 500);

			m_monsterAmmoList.back()->getPhysicsBody()->setCategoryBitmask(0x0010);//���룬�����ӵ��������ͬ
			m_monsterAmmoList.back()->getPhysicsBody()->setCollisionBitmask(0x0010);
			m_monsterAmmoList.back()->getPhysicsBody()->setContactTestBitmask(0x0010);
		}
	}
}
//����ӵ�������
void BattleScene1::Ammoupdate(float dt)
{

	float x = getPlayer()->getMouseLocation().x;
	float y = Director::getInstance()->getVisibleSize().height - getPlayer()->getMouseLocation().y;
	float Px = getPlayer()->getPositionX() - getPlayer()->getContentSize().width / 2;
	float Py = getPlayer()->getPositionY() - getPlayer()->getContentSize().height / 2;
	float Wx = (Px + getPlayer()->getWeapon1()->getPositionX() + getPlayer()->getWeapon1()->getContentSize().width / 2);
	float Wy = (Py + getPlayer()->getWeapon1()->getPositionY());
	Vec2 v = Vec2(Vec2(x - Wx, y - Wy) / sqrt((x - Wx) * (x - Wx) + (y - Wy) * (y - Wy)));//������ָ�����λ�õĵ�λ����

	//���Pistol�ӵ�
	if (getPlayer()->getMouseMap()[EventMouse::MouseButton::BUTTON_LEFT]		//����������
		&&getPlayer()->getPlayerAttribute()->getHp()>0							//Playerû��
		&&(getPlayer()->getWeapon1()->getTag()==AllTag::PlayerWeapon_Pistol_TAG
			|| getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Shotgun_TAG))//Weapon��ǹ
	{
		bool CanShoot = 0;


		if (AmmoList.size() == 0)
		{
			if (clock() - PistolEnd > 
				getPlayer()->getWeapon1 ()->getWeaponSpeed()
				+getPlayer()->getPlayerAttribute()->getShootSpeed()
				- getPlayer()->getPlayerAttribute()->getShootSpeed_Buff())
			{
				CanShoot = 1;
			}

		}
		else if (clock() - AmmoList.back()->start > 
			getPlayer()->getWeapon1()->getWeaponSpeed()
			+getPlayer()->getPlayerAttribute()->getShootSpeed()
			- getPlayer()->getPlayerAttribute()->getShootSpeed_Buff())
			CanShoot = 1;

		if (AmmoList.size() == 0)
		{

		}
		if (CanShoot)
		{
			int Num = 1;
			if (getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Shotgun_TAG)//������������
				Num = 5;
			for (int i = 1; i <= Num; i++)
			{
				AmmoList.push_back(getPlayer()->getWeapon1()->Attack());


				AmmoList.back()->setTag(AllTag::PlayerAmmo_Pistol_TAG);
				AmmoList.back()->start = clock();//ÿ���ӵ��ڲ���start�����¼����ʱ��
				addChild(AmmoList.back(), 3);
				auto physicBody = PhysicsBody::createBox(Size(20.0f, 20.0f), PhysicsMaterial(0, 0, 0));
				AmmoList.back()->addComponent(physicBody);
				AmmoList.back()->getPhysicsBody()->setDynamic(false);
				AmmoList.back()->setPosition(getPlayer()->getPosition() + getPlayer()->getWeapon1()->getPosition());
				if (i == 1)
					AmmoList.back()->getPhysicsBody()->setVelocity(v * 500);
				else if (i == 2)
				{					
					float ang_v = v.getAngle() + 4 * (PI / 180);
					Vec2 v2 = Vec2(cos(ang_v), sin(ang_v));

					AmmoList.back()->getPhysicsBody()->setVelocity(v2 * 500);
				}
				else if (i == 3)
				{
					float ang_v = v.getAngle() -4  * (PI / 180);
					Vec2 v2 = Vec2(cos(ang_v), sin(ang_v));

					AmmoList.back()->getPhysicsBody()->setVelocity(v2 * 500);
				}
				else if (i == 4)
				{
					float ang_v = v.getAngle() - 8 * (PI / 180);
					Vec2 v2 = Vec2(cos(ang_v), sin(ang_v));

					AmmoList.back()->getPhysicsBody()->setVelocity(v2 * 500);
				}
				else if (i == 5)
				{
					float ang_v = v.getAngle() - 8 * (PI / 180);
					Vec2 v2 = Vec2(cos(ang_v), sin(ang_v));

					AmmoList.back()->getPhysicsBody()->setVelocity(v2 * 500);
				}
				AmmoList.back()->getPhysicsBody()->setCategoryBitmask(0x0001);//0001
				AmmoList.back()->getPhysicsBody()->setCollisionBitmask(0x0001);//0001
				AmmoList.back()->getPhysicsBody()->setContactTestBitmask(0x0001);
			}
		
		}
	
	}
	//���Sword�ӵ� &PitchFor�ӵ�
	if (getPlayer()->getMouseMap()[EventMouse::MouseButton::BUTTON_LEFT] &&
		getPlayer()->getPlayerAttribute()->getHp() > 0 && 
		(getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Sword_TAG
			|| getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapn_PitchFork_TAG))
	{
		bool CanShoot = 0;


		if (AmmoList.size() == 0)
		{
			if (clock() - SwordEnd > 
				getPlayer()->getWeapon1()->getWeaponSpeed()
				+getPlayer()->getPlayerAttribute()->getShootSpeed()
				- getPlayer()->getPlayerAttribute()->getShootSpeed_Buff())
			{
				CanShoot = 1;
			}

		}
		else if (clock() - AmmoList.back()->start > 
			getPlayer()->getWeapon1()->getWeaponSpeed()
			+getPlayer()->getPlayerAttribute()->getShootSpeed()
			- getPlayer()->getPlayerAttribute()->getShootSpeed_Buff())
			CanShoot = 1;
		if (CanShoot)
		{

			AmmoList.push_back(getPlayer()->getWeapon1()->Attack());
			AmmoList.back()->start = clock();
			AmmoList.back()->setTag(AllTag::PlayerAmmo_Sword_TAG);
			addChild(AmmoList.back(), 3);
			auto physicBody = PhysicsBody::createBox(Size(60.0f, 60.0f), PhysicsMaterial(0, 0, 0));
			AmmoList.back()->addComponent(physicBody);
			AmmoList.back()->getPhysicsBody()->setDynamic(false);
			AmmoList.back()->setPosition(getPlayer()->getPosition() + getPlayer()->getWeapon1()->getPosition());
			AmmoList.back()->getPhysicsBody()->setVelocity(v * 1500);
			AmmoList.back()->getPhysicsBody()->setCategoryBitmask(0x0001);//0001
			AmmoList.back()->getPhysicsBody()->setCollisionBitmask(0x0001);//0001
			AmmoList.back()->getPhysicsBody()->setContactTestBitmask(0x0001);
			if (getPlayer()->getIsFlip()&&getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Sword_TAG)
				AmmoList.back()->getSprite()->setFlippedX(1);
			if (getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapn_PitchFork_TAG)
			{
				AmmoList.back()->getSprite()->setRotation(getPlayer()->getWeapon1()->getSprite()->getRotation());
			}
		}

	}
	
	
}

bool BattleScene1::isWall(float x, float y)//�жϴ������������Ӧ�ڵ�ͼ���ǲ���ǽ��
{
	int mapX = (int)(x / 64);
	int mapY = (int)(11 - int(y / 64));
	int tileGid = parentMap->getBattleMap()->getLayer("wall")->getTileGIDAt(Vec2(mapX, mapY));
	if (tileGid)
		return true;	//��ǽ

	else
		return false;	//����ǽ
}
bool BattleScene1::isDamagingLand(float x, float y)
{
	if (parentMap->getBattleMap()->getTag() == NormalRoom_TAG)
	{
		int mapX = (int)(x / 64);
		int mapY = (int)(11 - int(y / 64));
		int tileGid = parentMap->getBattleMap()->getLayer("DamagingLand")->getTileGIDAt(Vec2(mapX, mapY));
		if (tileGid)
			return true;

		else
			return false;
	}
	else
		return false;

}
//����ӵ�
void BattleScene1::DeleteAmmo(float dt)
{
	if (AmmoList.size() != 0)
	{
		int i = 0;
		auto ix = AmmoList.begin();
		int size = AmmoList.size();
		//Sword&&PitchFork���ӵ�����
		if (getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Sword_TAG
			|| getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapn_PitchFork_TAG)
		{

			
			for (; i < size; ix++, i++)
			{
				
				if (AmmoList[i]->getTag() == AllTag::PlayerAmmo_Sword_TAG)
				{
					AmmoList[i]->setPosition(getPlayer()->getPosition() + getPlayer()->getWeapon1()->getPosition());
					double NowTime = clock();

					if (NowTime - AmmoList[i]->start >= getPlayer()->getPlayerAttribute()->getShootSpeed()
						- getPlayer()->getPlayerAttribute()->getShootSpeed_Buff())
					{
						removeChild(AmmoList[i]);
						AmmoList.erase(ix);
						SwordEnd = clock();
						break;
					}
				}

			}
		}
		i = 0;
		ix = AmmoList.begin();
		size = AmmoList.size();
//�ӵ�ײǽ����
			for (; i < size; ix++, i++)
			{

				if (isWall((*ix)->getPositionX(), (*ix)->getPositionY()))
				{
					removeChild(AmmoList[i]);
					AmmoList.erase(ix);
					PistolEnd = clock();
					break;
				}
				


			}
		

	}
	//������ǹ�ӵ���ײǽ���� 
	if (m_monsterAmmoList.size() != 0)
	{
		int i = 0;
		auto ix = m_monsterAmmoList.begin();
		int size = m_monsterAmmoList.size();
		for (; i < size; ix++, i++)
		{
			if (isWall((*ix)->getPositionX(), (*ix)->getPositionY()))
			{
				removeChild(m_monsterAmmoList[i]);
				m_monsterAmmoList.erase(ix);
				break;
			}

		}
	}
	if (getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapon_Sword_TAG
		|| getPlayer()->getWeapon1()->getTag() == AllTag::PlayerWeapn_PitchFork_TAG)
	{
		if (AmmoList.size())
			getPlayer()->getWeapon1()->getSprite()->setOpacity(0);
		else
			getPlayer()->getWeapon1()->getSprite()->setOpacity(255);
	}

}
//��ײ���
bool BattleScene1::onContactBegin(PhysicsContact& contact)
{
	auto nodeA = contact.getShapeA()->getBody()->getNode();
	auto nodeB = contact.getShapeB()->getBody()->getNode();
	if (nodeA && nodeB)
	{
		Ammo* ammo;
		MonsterPistolAmmo* ammo_M;
		MonsterDashAmmo* dash_M;
		Monster* monster;
		Player* player;

		int TagA = nodeA->getTag();
		int TagB = nodeB->getTag();

		if (TagA == AllTag::PlayerAmmo_Pistol_TAG || TagA == AllTag::PlayerAmmo_Sword_TAG)
			ammo = dynamic_cast<Ammo*>(nodeA);
		else if (TagA>AllTag::CloseMonster_TAG&&TagA<AllTag::RemoteMonster_TAG)
			monster = dynamic_cast<Monster*>(nodeA);
		else if (TagA == 0)
			player = dynamic_cast<Player*>(nodeA);
		else if (TagA == AllTag::MonsterAmmo_PistolTAG)
			ammo_M = dynamic_cast<MonsterPistolAmmo*>(nodeA);
		else if (TagA == AllTag::MonsterAmmo_CloseTAG)
			dash_M = dynamic_cast<MonsterDashAmmo*>(nodeA);



		if (TagB == AllTag::PlayerAmmo_Pistol_TAG || TagB == AllTag::PlayerAmmo_Sword_TAG)
			ammo = dynamic_cast<Ammo*>(nodeB);
		else if (TagB > AllTag::CloseMonster_TAG && TagB < AllTag::RemoteMonster_TAG)
			monster = dynamic_cast<Monster*>(nodeB);
		else if (TagB == 0)
			player = dynamic_cast<Player*>(nodeB);
		else if (TagB == AllTag::MonsterAmmo_PistolTAG)
			ammo_M = dynamic_cast<MonsterPistolAmmo*>(nodeB);
		else if (TagB == AllTag::MonsterAmmo_CloseTAG)
			dash_M = dynamic_cast<MonsterDashAmmo*>(nodeB);
		//����ײ�����ǹ�ӵ�
		if ((TagA > AllTag::CloseMonster_TAG && TagA < AllTag::RemoteMonster_TAG) &&(TagB==AllTag::PlayerAmmo_Pistol_TAG)||
			(TagB > AllTag::CloseMonster_TAG && TagB < AllTag::RemoteMonster_TAG) && (TagA == AllTag::PlayerAmmo_Pistol_TAG))
		{
			int i = 0;
			auto ix = AmmoList.begin();
			int size = AmmoList.size();
			for (; i < size; ix++, i++)
			{
				if (*ix == ammo)
				{
					removeChild(AmmoList[i]);
					AmmoList.erase(ix);
					PistolEnd = clock();
					break;
				}
			}
			monster->takingDamage(getPlayer()->getPlayerAttribute()->getDamage()
			+ getPlayer()->getPlayerAttribute()->getDamage_Buff());
		}
		//����ײ��ҵ����ӵ�
		if ((TagA > AllTag::CloseMonster_TAG && TagA < AllTag::RemoteMonster_TAG) && (TagB == AllTag::PlayerAmmo_Sword_TAG) ||
			(TagB > AllTag::CloseMonster_TAG && TagB < AllTag::RemoteMonster_TAG) && (TagA == AllTag::PlayerAmmo_Sword_TAG))
		{
			int i = 0;
			auto ix = AmmoList.begin();
			int size = AmmoList.size();
			for (; i < size; ix++, i++)
			{
				if (*ix == ammo&&clock()-(*ix)->start>400)
				{
					removeChild(AmmoList[i]);
					AmmoList.erase(ix);
					SwordEnd = clock();	
					break;
				}
			}
			monster->takingDamage(getPlayer()->getPlayerAttribute()->getDamage()
				+ getPlayer()->getPlayerAttribute()->getDamage_Buff());
		}
		//���ײ����Pistol�ӵ�
		if ((TagA==AllTag::Player_TAG&&TagB==AllTag::MonsterAmmo_PistolTAG)||
			(TagB == AllTag::Player_TAG && TagA == AllTag::MonsterAmmo_PistolTAG))
		{
			int i = 0;
			auto ix = m_monsterAmmoList.begin();
			int size = m_monsterAmmoList.size();
			for (; i < size; ix++, i++)
			{
				if (*ix == ammo_M)
				{
					removeChild(m_monsterAmmoList[i]);
					m_monsterAmmoList.erase(ix);
					break;
				}
			}
			getPlayer()->getPlayerAttribute()->takeDamage(1);
		}
		//���ײ����Close�ӵ�
		if ((TagA == AllTag::Player_TAG && TagB == AllTag::MonsterAmmo_CloseTAG) ||
			(TagB == AllTag::Player_TAG && TagA == AllTag::MonsterAmmo_CloseTAG))
		{
			getPlayer()->getPlayerAttribute()->takeDamage(4);
		}

		return 1;
	}
	return 0;
}
void BattleScene1::LandDamageUpdate(float dt)
{
	
	if (isDamagingLand(getPlayer()->getPositionX(), getPlayer()->getPositionY()))
	{
		getPlayer()->getPlayerAttribute()->takeDamage(1);
	}
}
void BattleScene1::PlayerDeatheUpdate(float dt)
{
	int Hp=getPlayer()->getPlayerAttribute()->getHp();
	if (Hp <= 0)
	{	

		Director::getInstance()->replaceScene(StartScene::create());
	}
}