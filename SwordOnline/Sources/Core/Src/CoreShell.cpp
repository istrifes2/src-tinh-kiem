/*****************************************************************************************
//	外界访问Core接口方法
//	Copyright : Kingsoft 2002
//	Author	:   
//	CreateTime:	2002-9-12
------------------------------------------------------------------------------------------
*****************************************************************************************/
#include "KCore.h"
#include "GameDataDef.h"
#include "CoreShell.h"
#include "KPlayerTask.h"
#include "KEngine.h"
#include "KTaskFuns.h"

#include "CoreDrawGameObj.h"
#include "ImgRef.h"

#include "KPlayer.h"
#include "KPlayerSet.h"
#include "KObjSet.h"
#include "KItemList.h"
#include "KSubWorldSet.h"
#include "KProtocolProcess.h"

#include "KNpcResList.h"
#include "Scene/KScenePlaceC.h"
#include "kskills.h"
#include "GameDataDef.h"
#include "MsgGenreDef.h"
#include "KOption.h"
#include "KSubWorld.h"
#include "KViewItem.h"
#include "KSellItem.h"
#include "KItemSet.h"
#include "KTongData.h"
#include "KTongProtocol.h"
#include "KMagicDesc.h"
#include "malloc.h"

#include "KBuySell.h"
#include "KThiefSkill.h"
#include "KItemChangeRes.h"

#define	NPC_TRADE_BOX_WIDTH		6
#define	NPC_TRADE_BOX_HEIGHT	10
#define	MAX_TRADE_ITEM_WIDTH	2
#define	MAX_TRADE_ITEM_HEIGHT	4
IClientCallback* l_pDataChangedNotifyFunc = 0;

class KCoreShell : public iCoreShell
{
public:
	int	 GetProtocolSize(BYTE byProtocol);
	int	 OperationRequest(unsigned int uOper, unsigned int uParam, int nParam);
	void ProcessInput(unsigned int uMsg, unsigned int uParam, int nParam);
	int GetTargetNPC();
	int	 FindSelectNPC(int x, int y, int nRelation, bool bSelect, void* pReturn, int& nKind, bool bFind);
	int FindSelectObject(int x, int y, bool bSelect, int& nObjectIdx, int& nKind);
	int FindSpecialNPC(char* Name, void* pReturn, int& nKind);
	int ChatSpecialPlayer(void* pPlayer, const char* pMsgBuff, unsigned short nMsgLength);
	void ApplyAddTeam(void* pPlayer);
	void TradeApplyStart(void* pPlayer, bool bFolkGame);
	void JoinTongReply(void* pPlayer);
	int UseSkill(int x, int y, int nSkillID);
	int LockSomeoneUseSkill(int nTargetIndex, int nSkillID);
	int LockSomeoneAction(int nTargetIndex);
	int LockObjectAction(int nTargetIndex);
	void GotoWhere(int x, int y, int mode);	//mode 0 is auto, 1 is walk, 2 is run
	void GetWhere(int x, int y, void* pInfo);
	void Goto(int nDir, int mode);	//nDir 0~63, mode 0 is auto, 1 is walk, 2 is run
	void Turn(int nDir);	//nDir 0 is left, 1 is right, 2 is back
	int ThrowAwayItem();
	int GetNPCRelation(int nIndex);
	BOOL CheckMouse();
		
	
		
	//与地图相关的操作,uOper的取值来自 GAME_SCENE_MAP_OPERATION_INDEX
	int	SceneMapOperation(unsigned int uOper, unsigned int uParam, int nParam);
	//与帮会相关的操作, uOper的取值来自 GAME_TONG_OPERATION_INDEX
	int	TongOperation(unsigned int uOper, unsigned int uParam, int nParam);
	//与组队相关的操作，uOper的取值来自 GAME_TEAM_OPERATION_INDEX
	int TeamOperation(unsigned int uOper, unsigned int uParam, int nParam);

	int PAIOperation(unsigned int uOper, unsigned int uParam, int nParam, int nParam1);

	int	 GetGameData(unsigned int uDataId, unsigned int uParam, int nParam);

	void DrawGameObj(unsigned int uObjGenre, unsigned int uId, int x, int y, int Width, int Height, int nParam);
	void DrawGameSpace();
	DWORD GetPing();
	//void SendPing();
	int	 SetCallDataChangedNofify(IClientCallback* pNotifyFunc);
	void NetMsgCallbackFunc(void* pMsgData);
	void SetRepresentShell(struct iRepresentShell* pRepresent);
	void SetMusicInterface(void* pMusicInterface);
	void SetRepresentAreaSize(int nWidth, int nHeight);
	int  Breathe();
	void Release();	//释放接口对象
	void SetClient(LPVOID pClient);
	void SendNewDataToServer(void* pData, int nLength);
	void DirectFindPos(unsigned int uParam, int nParam, BOOL bSync, BOOL bPaintLine);
	BOOL GetPaintMode();
	void SetPaintMode(BOOL nIndex);
	BOOL GetFlagMode();
	void SetFlagMode(BOOL nIndex);
	void GetSizeItem(unsigned int uId, int *nW, int *nH);
	int	GetExtPoint();
	int GetNatureItem(unsigned int uItemId, unsigned int uGenre);
	int GetGenreItem(unsigned int uItemId, unsigned int uGenre);
	int IsDamage(unsigned int uItemId);
	BOOL GetLockState();
	int GetStackNum(unsigned int uItemId);
	BOOL GetSkillData(int nSkillId, int *nLevel);
	void GetSkillName(int nSkillId, char* szSkillName);
	int GetTradePrice(unsigned int uItemId);
	int GetTradeState(int nIndex, int uId);
	int	GetFightState();
	void BreakItem(unsigned int uParam, int nParam, BOOL bIsBreakAll);
	DWORD GetdwIDPlayer(unsigned int uId);
	int	GetOwnValue(int nMoneyUnit);
	int GetDataSuperShop(int nSaleId, unsigned int uParam, int nParam);
	int GetDataDynamicShop(int nSaleId, unsigned int uParam, int nParam);
	int GetNextSkill(int nType, int nIndex);
	BOOL IsSkillAura(int nSkillId);
};

static KCoreShell	g_CoreShell;

CORE_API void g_InitCore();
#ifndef _STANDALONE
extern "C" __declspec(dllexport)
#endif
iCoreShell* CoreGetShell()
{
	g_InitCore();
	return &g_CoreShell;
}

//--------------------------------------------------------------------------
//	功能：发出游戏世界数据改变的通知函数
//	返回：如未被注册通知函数，则直接返回0，否则返回通知函数执行结果。
//--------------------------------------------------------------------------

//--------------------------------------------------------------------
void CoreDataChanged(unsigned int uDataId, unsigned int uParam, int nParam)
{
	if (l_pDataChangedNotifyFunc)
		l_pDataChangedNotifyFunc->CoreDataChanged(uDataId, uParam, nParam);
}


void KCoreShell::Release()
{
	g_ReleaseCore();
}

//--------------------------------------------------------------------------
//	功能：接受与分派处理网络消息
//--------------------------------------------------------------------------
void KCoreShell::NetMsgCallbackFunc(void* pMsgData)
{
	g_ProtocolProcess.ProcessNetMsg((BYTE *)pMsgData);
}
//--------------------------------------------------------------------------
//	功能：设置游戏世界数据改变的通知函数
//	参数：fnCoreDataChangedCallback pNotifyFunc --> 通知函数的指针。
//	返回：返回值为非0值表示注册成功，否则表示失败。
//--------------------------------------------------------------------------
int	KCoreShell::SetCallDataChangedNofify(IClientCallback* pNotifyFunc)
{
	l_pDataChangedNotifyFunc = pNotifyFunc;
	return true;
}

//--------------------------------------------------------------------------
//	功能：从游戏世界获取数据
//	参数：unsigned int uDataId --> 表示获取游戏数据的数据项内容索引，其值为梅举类型
//							GAMEDATA_INDEX的取值之一。
//		  unsigned int uParam  --> 依据uDataId的取值情况而定
//		  int nParam --> 依据uDataId的取值情况而定
//	返回：依据uDataId的取值情况而定。
//--------------------------------------------------------------------------
int	KCoreShell::GetGameData(unsigned int uDataId, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uDataId)
	{
	case GDI_PLAYER_IS_MALE:
		{
			int nIndex = 0;
			if (nParam == 0)
				nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;
			else
				nIndex = NpcSet.SearchID(nParam);

			if (nIndex)
				nRet = (Npc[nIndex].m_NpcSettingIdx == PLAYER_MALE_NPCTEMPLATEID);
			else
				nRet = 1;	//出错时是男性
		}
		break;
	case GDI_REPAIR_ITEM_PRICE:
		if (uParam)
		{
			KUiObjAtContRegion *pObj = (KUiObjAtContRegion *)uParam;
			KItem*	pItem = NULL;

			switch(pObj->Obj.uGenre)
			{
			case CGOG_ITEM:
				{
					if (pObj->Obj.uId > 0)
					{
						pItem = &Item[pObj->Obj.uId];
					}
				}
				break;
			default:
				break;
			}

			if (!pItem)
				break;

			KUiItemBuySelInfo *pInfo = (KUiItemBuySelInfo *)nParam;
		
			if (pObj->eContainer == UOC_NPC_SHOP)
				break;
			pInfo->nItemNature = pItem->GetNature();
			pInfo->bNewArrival = FALSE;
			pInfo->nMoneyUnit = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nMoneyUnit;
			pInfo->nCurPrice = pItem->GetRepairPrice();
			strcpy(pInfo->szItemName, pItem->GetName());
			nRet = pItem->CanBeRepaired();
		}
		else
		{
			nParam = 0;
			nRet = 0;
		}
		break;

	case GDI_TRADE_ITEM_PRICE:
		if (uParam	&& !Player[CLIENT_PLAYER_INDEX].m_ItemList.IsLockOperation())
		{
			KUiObjAtContRegion *pObj = (KUiObjAtContRegion *)uParam;
			KItem*	pItem = NULL;

			switch(pObj->Obj.uGenre)
			{
			case CGOG_PLAYERSELLITEM:
				{
					if (pObj->Obj.uId > 0)
					{
						KUiItemBuySelInfo *pInfo = (KUiItemBuySelInfo *)nParam;
						pInfo->nItemNature = Item[pObj->Obj.uId].GetNature();
						pInfo->bNewArrival = FALSE;
						pInfo->nMoneyUnit = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nMoneyUnit;
						pInfo->nCurPrice = Item[pObj->Obj.uId].GetTradePrice();
						strcpy(pInfo->szItemName,Item[pObj->Obj.uId].GetName());
						nRet = 1;
						return nRet;
					}
				}
				break;
			case CGOG_ITEM:
				{
					if (pObj->Obj.uId > 0)
					{
						pItem = &Item[pObj->Obj.uId];
					}
				}
				break;
			case CGOG_NPCSELLITEM:
				{
					int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop];
					if (nBuyIdx != -1)
					{
						int nIndex = BuySell.GetItemIndex(nBuyIdx, pObj->Obj.uId);
						if (nIndex >= 0)
							pItem = BuySell.GetItem(nIndex);
					}
				}
				break;
			default:
				break;
			}

			if (!pItem)
				break;

			KUiItemBuySelInfo *pInfo = (KUiItemBuySelInfo *)nParam;
			pInfo->nItemNature = pItem->GetNature();
			pInfo->nOldPrice = pItem->GetOrgPrice();
			if (pObj->eContainer == UOC_NPC_SHOP)
				pInfo->nCurPrice = pItem->GetCurPrice();
			else
				pInfo->nCurPrice = pItem->GetSalePrice();
			pInfo->bNewArrival = pItem->GetNewArrival();
			strcpy(pInfo->szItemName, pItem->GetName());
			pInfo->nMoneyUnit = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nMoneyUnit;
			nRet = 1;
		}
		else
		{
			nParam = 0;
			nRet = 0;
		}
		break;
	//游戏对象描述说明文本串
	//uParam = (KUiGameObject*) 描述游戏对象的结构数据的指针
	//nParam = (char*) 指向一个缓冲区的指针，其空间不少于256字节。
	case GDI_GAME_OBJ_DESC_INCLUDE_REPAIRINFO:
	case GDI_GAME_OBJ_DESC_INCLUDE_TRADEINFO:
		if (nParam && uParam)
		{
			KUiObjAtContRegion* pObj = (KUiObjAtContRegion *)uParam;
			char* pszDescript = (char *)nParam;
			pszDescript[0] = 0;
			switch(pObj->Obj.uGenre)
			{
			case CGOG_PLAYERSELLITEM:
					Item[pObj->Obj.uId].GetDesc(pszDescript, true, true);
				break;
			case CGOG_ITEM:
				{
					if (pObj->eContainer == UOC_EQUIPTMENT)
					{
						Item[pObj->Obj.uId].GetDesc(pszDescript, true, true, Player[CLIENT_PLAYER_INDEX].m_ItemList.GetActiveAttribNum(pObj->Obj.uId),
							Player[CLIENT_PLAYER_INDEX].m_ItemList.GetGoldActiveAttribNum(pObj->Obj.uId));
					}
					else
						Item[pObj->Obj.uId].GetDesc(pszDescript, true, true);
				}
				break;		
			case CGOG_NPCSELLITEM:
				{
					int nIdx = -1;
					if (-1 == Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop])
						break;
					nIdx = BuySell.GetItemIndex(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop], pObj->Obj.uId);

					KItem* pItem = NULL;
					if (nIdx < 0)
						break;
					pItem = BuySell.GetItem(nIdx);

					if (!pItem)
						break;
					pItem->GetDesc(pszDescript, true);
				}
				break;
			}
		}
		break;
case GDI_GAME_OBJ_DESC:
		if (nParam && uParam)
		{
			KUiObjAtContRegion* pObj = (KUiObjAtContRegion *)uParam;
			char* pszDescript = (char *)nParam;
			pszDescript[0] = 0;
			switch(pObj->Obj.uGenre)
			{
			case CGOG_ITEM:
				{
					if (pObj->eContainer == UOC_EQUIPTMENT)
					{
						Item[pObj->Obj.uId].GetDesc(pszDescript, false, false, Player[CLIENT_PLAYER_INDEX].m_ItemList.GetActiveAttribNum(pObj->Obj.uId),
							Player[CLIENT_PLAYER_INDEX].m_ItemList.GetGoldActiveAttribNum(pObj->Obj.uId));
					}
					else
						Item[pObj->Obj.uId].GetDesc(pszDescript);
				}
				break;
			case CGOG_SKILL:
			case CGOG_SKILL_FIGHT:
			case CGOG_SKILL_SHORTCUT:
				{
					int nLevel = 0;
					if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetLevel(pObj->Obj.uId) > 0)
						nLevel = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetCurrentLevel(pObj->Obj.uId);
					_ASSERT(nLevel >= 0);
					if (pObj->Obj.uId >0)
					{
						
						ISkill * pISkill = g_SkillManager.GetSkill(pObj->Obj.uId, 1);
						if (!pISkill)
							break;
						eSkillStyle eStyle = (eSkillStyle) pISkill->GetSkillStyle();
						
						switch(eStyle)
						{
						case SKILL_SS_Missles:			//	子弹类		本技能用于发送子弹类
						case SKILL_SS_Melee:
						case SKILL_SS_InitiativeNpcState:	//	主动类		本技能用于改变当前Npc的主动状态
						case SKILL_SS_PassivityNpcState:		//	被动类		本技能用于改变Npc的被动状态
							{
								KSkill::GetDesc(
									pObj->Obj.uId,
									nLevel,
									pszDescript,
									Player[CLIENT_PLAYER_INDEX].m_nIndex,
									(pObj->Obj.uGenre == CGOG_SKILL_SHORTCUT)?false:true
									);
							}
							break;
						case SKILL_SS_Thief:
							{
								((KThiefSkill *)pISkill)->GetDesc(
									pObj->Obj.uId,
									nLevel,
									pszDescript,
									Player[CLIENT_PLAYER_INDEX].m_nIndex,
									(pObj->Obj.uGenre == CGOG_SKILL_SHORTCUT)?false:true
								);
			
							}break;
							
						}
					}
					
				}
				break;
			case CGOG_NPCSELLITEM:
				{
					int nIdx = -1;
					if (-1 == Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop])
						break;
					nIdx = BuySell.GetItemIndex(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop], pObj->Obj.uId);

					KItem* pItem = NULL;
					if (nIdx < 0)
						break;
					pItem = BuySell.GetItem(nIdx);

					if (!pItem)
						break;
					pItem->GetDesc(pszDescript);
				}
				break;
			case CGOG_IME_ITEM:
				{
				if (pObj->Obj.uId <= 0)
					return 0;
				Item[pObj->Obj.uId].GetDesc(pszDescript);
				}
				break;
			case CGOG_PLAYERSELLITEM:
					Item[pObj->Obj.uId].GetDesc(pszDescript, true, true, 0);
				break;
			case CGOG_PLAYERVIEWITEM:
					Item[pObj->Obj.uId].GetDesc(pszDescript);
				break;
			}
		}
		break;
	case GDI_GAME_OBJ_DESC_INCLUDE_MOUSEHOVER:
		if (nParam && uParam)
		{
			KUiObjAtContRegion* pObj = (KUiObjAtContRegion *)uParam;
			char* pszFileName = (char*)nParam;
			switch(pObj->Obj.uGenre)
			{
			case CGOG_PLAYERVIEWITEM:
			case CGOG_PLAYERSELLITEM:
			case CGOG_ITEM:
					if (Item[pObj->Obj.uId].GetGenre() == item_equip && 
						Item[pObj->Obj.uId].GetDetailType() == equip_mask)
					{
						int nNpcSettingIdx;
						g_MaskChangeRes.GetInteger(Item[pObj->Obj.uId].GetParticular() + 2, 2, 0, &nNpcSettingIdx);
						Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetNpcResFile(nNpcSettingIdx, pszFileName);
					}
				break;
			case CGOG_NPCSELLITEM:
				{
					int nIdx = -1;
					if (-1 == Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop])
						break;
					nIdx = BuySell.GetItemIndex(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop], pObj->Obj.uId);

					KItem* pItem = NULL;
					if (nIdx < 0)
						break;
					pItem = BuySell.GetItem(nIdx);

					if (!pItem)
						break;
					if (pItem->GetGenre() == item_equip && 
						pItem->GetDetailType() == equip_mask)
					{
						int nNpcSettingIdx;
						g_MaskChangeRes.GetInteger(pItem->GetParticular() + 2, 2, 0, &nNpcSettingIdx);
						Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetNpcResFile(nNpcSettingIdx, pszFileName);
					}
				}
				break;
			}
			if(pszFileName[0])
				nRet = 1;
		}
		break;
	case GDI_PLAYER_UID:
			nRet = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_dwID;
		break;
	case GDI_PLAYER_CURCAMP:
			nRet = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentCamp;
		break;
	//主角的一些不易变的数据
	//uParam = (KUiPlayerBaseInfo*)pInfo	
	case GDI_PLAYER_BASE_INFO:
		if (uParam)
		{
			KUiPlayerBaseInfo* pInfo = (KUiPlayerBaseInfo*)uParam;
			int nIndex = 0;
			if (nParam == 0)
			{
				nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;
				pInfo->nCurFaction = Player[CLIENT_PLAYER_INDEX].m_cFaction.m_nCurFaction;
				pInfo->nCurTong = Player[CLIENT_PLAYER_INDEX].m_cTong.GetTongNameID();
				pInfo->nMissionGroup = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_nMissionGroup;
				pInfo->nRoomId = Player[CLIENT_PLAYER_INDEX].m_cRoom.m_nID;
				pInfo->nRankInSect = Player[CLIENT_PLAYER_INDEX].m_nSectStat;
			}
			else
			{
				nIndex = NpcSet.SearchID(nParam);
				pInfo->nCurFaction = -1;
				pInfo->nCurTong = 0;
				pInfo->nMissionGroup = -1;
				pInfo->nRoomId = -1;
				pInfo->nRankInSect = 0;
			}
			if (nIndex)
			{
				strcpy(pInfo->Name, Npc[nIndex].Name);
				strcpy(pInfo->szMateName, Npc[nIndex].MateName);
				strcpy(pInfo->szTongName, Npc[nIndex].m_szTongName);
				pInfo->Title[0] = 0;
				if (Npc[nIndex].m_RankID)
				{
					char szRankId[5];
					itoa(Npc[nIndex].m_RankID, szRankId, 10);
					g_RankTabSetting.GetString(szRankId, "RANKSTR", "", pInfo->Title, sizeof(pInfo->Title));
				}
				pInfo->nRankInWorld = Npc[nIndex].m_nRankInWorld;
				pInfo->nFuYuan = Npc[nIndex].m_nFuYuan;
				pInfo->nRepute = Npc[nIndex].m_nRepute;
				pInfo->nPKValue = Npc[nIndex].m_nPKValue;
				pInfo->nTranslife = Npc[nIndex].m_byTranslife;
				memset(pInfo->StatusDesc, 0, sizeof(pInfo->StatusDesc));

				switch (Npc[nIndex].m_Series)
				{
					case series_metal:
						strcpy(pInfo->StatusDesc, "H� Kim");
					break;
					case series_wood:
						strcpy(pInfo->StatusDesc, "H� M閏");
					break;
					case series_water:
						strcpy(pInfo->StatusDesc, "H� Th駓");
					break;
					case series_fire:
						strcpy(pInfo->StatusDesc, "H� H醓");
					break;
					case series_earth:
						strcpy(pInfo->StatusDesc, "H� Th�");
					break;
					default:
						strcpy(pInfo->StatusDesc, "V� H�");
					break;
				}
			}
				
		}
		break;

	//主角的一些易变的数据
	//uParam = (KUiPlayerRuntimeInfo*)pInfo
	case GDI_PLAYER_RT_INFO:
		if (uParam)
		{
			KUiPlayerRuntimeInfo* pInfo = (KUiPlayerRuntimeInfo*)uParam;
			pInfo->nLifeFull = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentLifeMax;		//生命满值
			pInfo->nLife = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentLife;				//生命
			pInfo->nManaFull = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentManaMax;		//内力满值
			pInfo->nMana = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentMana;				//内力
			pInfo->nStaminaFull = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentStaminaMax;//体力满值
			pInfo->nStamina = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_CurrentStamina;		//体力	
			pInfo->nExperienceFull = Player[CLIENT_PLAYER_INDEX].m_nNextLevelExp;		//经验满值
			pInfo->nExperience = Player[CLIENT_PLAYER_INDEX].m_nExp;					//经验
			pInfo->nCurLevelExperience = Player[CLIENT_PLAYER_INDEX].m_nNextLevelExp;
		
			pInfo->byAction = PA_NONE;

			if (Player[CLIENT_PLAYER_INDEX].m_RunStatus)
				pInfo->byAction |= PA_RUN;

			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_Doing == do_sit)
				pInfo->byAction |= PA_SIT;
				
			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_bRideHorse == TRUE)
				pInfo->byAction |= PA_RIDE;

		}
		break;

	//主角的一些易变的属性数据
	//uParam = (KUiPlayerAttribute*)pInfo
	case GDI_PLAYER_RT_ATTRIBUTE:
		if (uParam)
		{
			KUiPlayerAttribute* pInfo = (KUiPlayerAttribute*)uParam;
			KNpc*	pNpc = &Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex];
			pInfo->nMoney = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_equipment);				//银两
			pInfo->nBARemainPoint = Player[CLIENT_PLAYER_INDEX].m_nAttributePoint;						//基本属性剩余点数
			pInfo->nStrength = Player[CLIENT_PLAYER_INDEX].m_nCurStrength;								//力量
			pInfo->nDexterity = Player[CLIENT_PLAYER_INDEX].m_nCurDexterity;								//敏捷
			pInfo->nVitality = Player[CLIENT_PLAYER_INDEX].m_nCurVitality;								//活力
			pInfo->nEnergy = Player[CLIENT_PLAYER_INDEX].m_nCurEngergy;									//精力
			Player[CLIENT_PLAYER_INDEX].GetEchoDamage(&pInfo->nKillMIN, &pInfo->nKillMAX, 0);				//最大最小杀伤力
			Player[CLIENT_PLAYER_INDEX].GetEchoDamage(&pInfo->nRightKillMin , &pInfo->nRightKillMax, 1);

			int nLeftSkillId = Player[CLIENT_PLAYER_INDEX].GetLeftSkill();
			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetLevel(nLeftSkillId) > 0)
			{
				KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(nLeftSkillId, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetCurrentLevel(nLeftSkillId));
				if (pOrdinSkill->IsUseAR())
					pInfo->nLeftAttack = pNpc->m_CurrentAttackRating;				//攻击力
				else
					pInfo->nLeftAttack = 0;
			}
			else
				pInfo->nLeftAttack = 0;
			int nRightSkillId = Player[CLIENT_PLAYER_INDEX].GetRightSkill();
			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetLevel(nRightSkillId) > 0)
			{
				KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(nRightSkillId, Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetCurrentLevel(nRightSkillId));
				if (pOrdinSkill->IsUseAR())
					pInfo->nRightAttack = pNpc->m_CurrentAttackRating;				//攻击力
				else
					pInfo->nRightAttack = 0;
			}
			else
				pInfo->nRightAttack = 0;
			pInfo->nDefence = pNpc->m_CurrentDefend;					//防御力
			if (Player[CLIENT_PLAYER_INDEX].m_RunStatus)
				pInfo->nMoveSpeed = pNpc->m_CurrentRunSpeed;				//移动速度
			else
				pInfo->nMoveSpeed = pNpc->m_CurrentWalkSpeed;				//移动速度
			pInfo->nAttackSpeed = pNpc->m_CurrentAttackSpeed;
			pInfo->nCastSpeed = pNpc->m_CurrentCastSpeed;//攻击速度
			//物理防御
			if (pNpc->m_CurrentPhysicsResistMax >= pNpc->m_CurrentPhysicsResist)
			{
				pInfo->nPhyDef = pNpc->m_CurrentPhysicsResist;
				pInfo->nPhyDefPlus = 0;
			}
			else
			{
				pInfo->nPhyDef = pNpc->m_CurrentPhysicsResistMax;
				pInfo->nPhyDefPlus = (pNpc->m_CurrentPhysicsResist - pNpc->m_CurrentPhysicsResistMax) / RESIST_PLUS_SCALE;
			}
			//冰冻防御
			if (pNpc->m_CurrentColdResistMax >= pNpc->m_CurrentColdResist)
			{
				pInfo->nCoolDef = pNpc->m_CurrentColdResist;
				pInfo->nCoolDefPlus = 0;
			}
			else
			{
				pInfo->nCoolDef = pNpc->m_CurrentColdResistMax;
				pInfo->nCoolDefPlus = (pNpc->m_CurrentColdResist - pNpc->m_CurrentColdResistMax) / RESIST_PLUS_SCALE;
			}
			//闪电防御
			if (pNpc->m_CurrentLightResistMax >= pNpc->m_CurrentLightResist)
			{
				pInfo->nLightDef = pNpc->m_CurrentLightResist;
				pInfo->nLightDefPlus = 0;
			}
			else
			{
				pInfo->nLightDef = pNpc->m_CurrentLightResistMax;
				pInfo->nLightDefPlus = (pNpc->m_CurrentLightResist - pNpc->m_CurrentLightResistMax) / RESIST_PLUS_SCALE;
			}
			//火焰防御
			if (pNpc->m_CurrentFireResistMax >= pNpc->m_CurrentFireResist)
			{
				pInfo->nFireDef = pNpc->m_CurrentFireResist;
				pInfo->nFireDefPlus = 0;
			}
			else
			{
				pInfo->nFireDef = pNpc->m_CurrentFireResistMax;
				pInfo->nFireDefPlus = (pNpc->m_CurrentFireResist - pNpc->m_CurrentFireResistMax) / RESIST_PLUS_SCALE;
			}
			//毒素防御
			if (pNpc->m_CurrentPoisonResistMax >= pNpc->m_CurrentPoisonResist)
			{
				pInfo->nPoisonDef = pNpc->m_CurrentPoisonResist;
				pInfo->nPoisonDefPlus = 0;
			}
			else
			{
				pInfo->nPoisonDef = pNpc->m_CurrentPoisonResistMax;
				pInfo->nPoisonDefPlus = (pNpc->m_CurrentPoisonResist - pNpc->m_CurrentPoisonResistMax) / RESIST_PLUS_SCALE;
			}
			pInfo->nLevel = pNpc->m_Level;
		
			// 根据玩家状态显示 状态文字 not end 不应该是状态，应该是五行属性 spe
			memset(pInfo->StatusDesc, 0, sizeof(pInfo->StatusDesc));
			switch(pNpc->m_Series)
			{
			case series_water:
				strcpy(pInfo->StatusDesc, "H� Th駓");
				break;
			case series_wood:
				strcpy(pInfo->StatusDesc, "H� M閏");
				break;
			case series_metal:
				strcpy(pInfo->StatusDesc, "H� Kim");
				break;
			case series_fire:
				strcpy(pInfo->StatusDesc, "H� H醓");
				break;
			case series_earth:
				strcpy(pInfo->StatusDesc, "H� Th�");
				break;
			}

			pInfo->nPKValue = Player[CLIENT_PLAYER_INDEX].m_cPK.GetPKValue();
			pInfo->nRepute = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_REPUTE);
			pInfo->nFuYuan = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_FUYUAN);
			pInfo->nTranslife = pNpc->m_byTranslife;
			pInfo->nRankInWorld = Player[CLIENT_PLAYER_INDEX].m_nWorldStat;
			pInfo->Title[0] = 0;
			if (pNpc->m_RankID)
			{
				char szRankId[5];
				itoa(pNpc->m_RankID, szRankId, 10);
				g_RankTabSetting.GetString(szRankId, "RANKSTR", "", pInfo->Title, sizeof(pInfo->Title));
			}
		}
		break;

	//主角的立即使用物品与武功
	//uParam = (KUiPlayerImmedItemSkill*)pInfo
	case GDI_PLAYER_IMMED_ITEMSKILL:
		if (uParam)
		{
			KUiPlayerImmedItemSkill* pInfo = (KUiPlayerImmedItemSkill*)uParam;
			memset(pInfo,0,sizeof(KUiPlayerImmedItemSkill));
			pInfo->IMmediaSkill[0].uGenre	= CGOG_SKILL_SHORTCUT;
			pInfo->IMmediaSkill[0].uId		= Player[CLIENT_PLAYER_INDEX].GetLeftSkill();
			pInfo->IMmediaSkill[1].uGenre	= CGOG_SKILL_SHORTCUT;
			pInfo->IMmediaSkill[1].uId		= Player[CLIENT_PLAYER_INDEX].GetRightSkill();

			for (int i = 0; i < MAX_IMMEDIACY_ITEM; i++)
			{
				pInfo->ImmediaItem[i].uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.m_Room[room_immediacy].FindItem(i, 0);
				if (pInfo->ImmediaItem[i].uId > 0)
				{
					pInfo->ImmediaItem[i].uGenre = CGOG_ITEM;
				}
				else
				{
					pInfo->ImmediaItem[i].uGenre = CGOG_NOTHING;
				}
			}
		}
		break;

	//主角随身携带的钱
	//nRet = 主角随身携带的钱
	case GDI_PLAYER_HOLD_MONEY:	
		nRet = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_equipment);
		break;
	//主角随身携带的物品等
	//uParam = (KUiObjAtRegion*) pInfo -> KUiObjAtRegion结构数组的指针，KUiObjAtRegion
	//				结构用于存储物品或者武功的数据及其放置区域位置信息。
	//nParam = pInfo数组中包含KUiObjAtRegion结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pInfo数组中的前多少个KUiObjAtRegion
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiObjAtRegion结构的数组
	//			才够存储全部的随身携带的物品信息。

	// flying添加了一个是否可以骑马的请求，在这里实现，gg可乐一罐，mm请吃KFC哦
	case GDI_PLAYER_CAN_RIDE:
		// 实现分支的代码在这里添加
		break;

	case GDI_ITEM_TAKEN_WITH:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_equiproom)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_equiproom)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_equiproom)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_equiproom)
					nCount++;
			}
			nRet = nCount;
		}
		break;

	//主角装备物品
	//uParam = (KUiObjAtRegion*)pInfo -> 包含10个元素的KUiObjAtRegion结构数组指针，
	//			KUiObjAtRegion结构用于存储装备的数据和放置位置信息。
	//			KUiObjAtRegion::Region::h 表示属于第几套装备
	//			KUiObjAtRegion::Region::v 表示属于哪个位置的装备,其值为梅举类型
	//			UI_EQUIPMENT_POSITION的取值之一。请参看UI_EQUIPMENT_POSITION的注释。
	//nParam =	要获取的是第几套装备信息
	//Return =  其值表示pInfo数组中的前多少个KUiObjAtRegion结构被填充了有效的数据。
	// new add
		//
	case GDI_EQUIPMENT:
		nRet = 0;
		if (uParam)
		{
			// TODO：暂时没有做第二套装备
			if (nParam == 1)
				break;

			int PartConvert[itempart_num] = 
			{
				UIEP_HEAD,		UIEP_BODY,
				UIEP_WAIST,		UIEP_HAND,
				UIEP_FOOT,		UIEP_FINESSE,
				UIEP_NECK,		UIEP_FINGER1,
				UIEP_FINGER2,	UIEP_WAIST_DECOR,
				UIEP_HORSE,		UIEP_MASK,
				UIEP_MANTLE,	UIEP_SIGNET,
				UIEP_SHIPIN,	UIEP_HOODS,
				UIEP_CLOAK,
			};

			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;

			for (int i = 0; i < itempart_num; i++)
			{
				pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetEquipment(i);
				if (pInfo->Obj.uId)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
				
					pInfo->Region.Width = Item[pInfo->Obj.uId].GetWidth();
					pInfo->Region.Height = Item[pInfo->Obj.uId].GetHeight();
					pInfo->Region.h = 0;
					pInfo->Region.v = PartConvert[i];
				}
				else
				{
					pInfo->Obj.uGenre = CGOG_NOTHING;
				}
				nCount++;
				pInfo++;
			}
			nRet = nCount;
		}
		break;
	case GDI_PARADE_EQUIPMENT:
		nRet = 0;
		if (uParam)
		{
			// TODO：暂时没有做第二套装备
			if (nParam == 1)
				break;

			int PartConvert[itempart_num] = 
			{
				UIEP_HEAD,		UIEP_BODY,
				UIEP_WAIST,		UIEP_HAND,
				UIEP_FOOT,		UIEP_FINESSE,
				UIEP_NECK,		UIEP_FINGER1,
				UIEP_FINGER2,	UIEP_WAIST_DECOR,
				UIEP_HORSE,		UIEP_MASK,
				UIEP_MANTLE,	UIEP_SIGNET,
				UIEP_SHIPIN,	UIEP_HOODS,
				UIEP_CLOAK,

			};

			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;

			for (int i = 0; i < itempart_num; i++)
			{
				pInfo->Obj.uId = g_cViewItem.m_sItem[i].nIdx;
				if (pInfo->Obj.uId)
				{
					pInfo->Obj.uGenre = CGOG_PLAYERVIEWITEM;
				
					pInfo->Region.Width = Item[pInfo->Obj.uId].GetWidth();
					pInfo->Region.Height = Item[pInfo->Obj.uId].GetHeight();
					pInfo->Region.h = 0;
					pInfo->Region.v = PartConvert[i];
				}
				else
				{
					pInfo->Obj.uGenre = CGOG_NOTHING;
				}
				nCount++;
				pInfo++;
			}
			nRet = nCount;
		}
		break;
		
	//交易操作相关的数据
	//uParam = (UI_TRADE_OPER_DATA)eOper 具体含义见UI_TRADE_OPER_DATA
	//nParam 具体应用与含义由uParam的取值状况决定,见UI_TRADE_OPER_DATA的说明
	//Return 具体含义由uParam的取值状况决定,见UI_TRADE_OPER_DATA的说明
	//主角的生活技能数据
	case GDI_TRADE_OPER_DATA:
		if (uParam == UTOD_IS_LOCKED)
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeLock;
		else if (uParam == UTOD_IS_TRADING)
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeState;
		else if (uParam == UTOD_IS_OTHER_LOCKED)
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeDestLock;
		else if (uParam == UTOD_IS_OTHER_TRADING)
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeDestState;
		else if (uParam == UTOD_IS_WILLING)
		{
			nRet = (Player[CLIENT_PLAYER_INDEX].m_cMenuState.m_nState == PLAYER_MENU_STATE_TRADEOPEN);
		}
		break;
	//主角掌握的各项生活技能
	//uParam = (unsigned int*) pSkills -> 包含10个unsigned int的数组用于存储各项生活技能的id。

	//剩余战斗技能点数
	//Return = 剩余战斗技能点数
	case GDI_FIGHT_SKILL_POINT:
		//to do:no implements in this version;
		nRet = Player[CLIENT_PLAYER_INDEX].m_nSkillPoint;
		break;

	//主角掌握的各项战斗技能
	//uParam = (unsigned int*) pSkills -> 包含30个unsigned int的数组用于存储各项战斗技能的id。
	case GDI_FIGHT_SKILLS:
		if (uParam)
		{
			KUiSkillData* pSkills = (KUiSkillData*)uParam;
			Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetSkillSortList(pSkills);
		}
		break;
	//显示左键技能列表
	//uParam = (KUiSkillData*) pSkills -> 包含65个KUiSkillData的数组用于存储各技能的数据。
	//								KUiSkillData::nLevel用来表述技能显示在第几行
	//Return = 返回有效数据的Skills的数目
	case GDI_LEFT_ENABLE_SKILLS:
		{
			KUiSkillData * pSkills = (KUiSkillData*)uParam;
			int nCount = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetLeftSkillSortList(pSkills);
			return nCount;

		}
		break;
	//显示右键技能列表
	//uParam = (KUiSkillData*) pSkills -> 包含65个KUiSkillData的数组用于存储各技能的数据。
	//								KUiSkillData::nLevel用来表述技能显示在第几行
	//Return = 返回有效数据的Skills的数目
	case GDI_RIGHT_ENABLE_SKILLS:
		{
			KUiSkillData * pSkills = (KUiSkillData*)uParam;
			int nCount = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetRightSkillSortList(pSkills);
			return nCount;
		}
		break;
	//主角的自创武功
	//uParam = (unsigned int*) pSkills -> 包含5个unsigned int的数组用于存储各项自创武功的id。
	case GDI_CUSTOM_SKILLS:
		if (uParam)
		{
			//to do: no implements in this version;
			KUiSkillData* pSkills = (KUiSkillData*)uParam;
			memset(pSkills,0,sizeof(KUiSkillData)*5);
		}
		break;
	//获取周围玩家的列表
	//uParam = (KUiPlayerItem*)pList -> 人员信息列表
	//			KUiPlayerItem::nData = 0
	//nParam = pList数组中包含KUiPlayerItem结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pList数组中的前多少个KUiPlayerItem
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiPlayerItem结构的数组
	//			才够存储全部人员信息。
	case GDI_NEARBY_PLAYER_LIST:
		nRet = NpcSet.GetAroundPlayer((KUiPlayerItem*)uParam, nParam);
		break;

	//获取周围孤单可受邀请的玩家的列表
	//参数含义同GDI_NEARBY_PLAYER_LIST
	case GDI_NEARBY_IDLE_PLAYER_LIST:
		nRet = NpcSet.GetAroundPlayerForTeamInvite((KUiPlayerItem*)uParam, nParam);
		break;

	//主角统帅能力相关的数据
	//uParam = (KUiPlayerLeaderShip*) -> 主角统帅能力相关的数据结构指针
	case GDI_PLAYER_LEADERSHIP:
		if (uParam)
		{
			KUiPlayerLeaderShip* pInfo = (KUiPlayerLeaderShip*)uParam ;
			pInfo->nLeaderShipExperience = Player[CLIENT_PLAYER_INDEX].m_nLeadExp ;		//统帅力经验值
			//to do: waiting for...;
			pInfo->nLeaderShipExperienceFull = Player[CLIENT_PLAYER_INDEX].m_nNextLevelLeadExp;//统帅力经验值
			pInfo->nLeaderShipLevel = Player[CLIENT_PLAYER_INDEX].m_nLeadLevel ;			//统帅力等级
		}
		break;
	//获得物品在某个环境位置的属性状态
	//uParam = (KUiGameObject*)pObj（当nParam==0时）物品的信息
	//uParam = (KUiObjAtContRegion*)pObj（当nParam!=0时）物品的信息
	//nParam = (int)(bool)bJustTry  是否只是尝试放置
	//Return = (ITEM_IN_ENVIRO_PROP)eProp 物品的属性状态
	case GDI_ITEM_IN_ENVIRO_PROP:
		{
			if (!nParam)
			{
				KUiGameObject *pObj = (KUiGameObject *)uParam;
				if (pObj->uGenre != CGOG_ITEM && pObj->uGenre != CGOG_NPCSELLITEM && pObj->uGenre != CGOG_PLAYERSELLITEM && pObj->uGenre != CGOG_PLAYERVIEWITEM)
					break;

				KItem* pItem = NULL;

				if (pObj->uGenre == CGOG_ITEM || pObj->uGenre == CGOG_PLAYERSELLITEM || pObj->uGenre == CGOG_PLAYERVIEWITEM && pObj->uId > 0 && pObj->uId < MAX_ITEM)
				{
					pItem = &Item[pObj->uId];
				}
				else if (pObj->uGenre == CGOG_NPCSELLITEM)
				{
					int nIdx = BuySell.GetItemIndex(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop], pObj->uId);
					pItem = BuySell.GetItem(nIdx);
				}
				_ASSERT(pItem);
				if (!pItem || pItem->GetGenre() != item_equip)
					break;

				if (Player[CLIENT_PLAYER_INDEX].m_ItemList.CanEquip(pItem))
				{
					nRet = IIEP_NORMAL;
				}
				else
				{
					nRet = IIEP_NOT_USEABLE;
				}
			}
			else
			{
				KUiObjAtContRegion *pObj = (KUiObjAtContRegion *)uParam;
				if (pObj->Obj.uGenre != CGOG_ITEM && pObj->Obj.uGenre != CGOG_NPCSELLITEM && pObj->Obj.uGenre != CGOG_PLAYERSELLITEM && pObj->Obj.uGenre != CGOG_PLAYERVIEWITEM && pObj->Obj.uId >= MAX_ITEM)
					break;

				int PartConvert[itempart_num] = 
				{ 
					itempart_head,		itempart_weapon,
					itempart_amulet,	itempart_cuff,
					itempart_body,		itempart_belt,
					itempart_ring1,		itempart_ring2,
					itempart_pendant,	itempart_foot,
					itempart_horse,		itempart_mask,
					itempart_mantle,	itempart_signet,
					itempart_shipin,	itempart_hoods,
					itempart_cloak,
				};

				_ASSERT(pObj->eContainer < itempart_num);
				if (pObj->eContainer >= itempart_num || pObj->eContainer < 0)
					break;

				if (Item[pObj->Obj.uId].GetGenre() != item_equip)
					break;

				int nPlace = PartConvert[pObj->eContainer];
				if (Player[CLIENT_PLAYER_INDEX].m_ItemList.CanEquip(pObj->Obj.uId, nPlace))
				{
					nRet = IIEP_NORMAL;
				}
				else
				{
					nRet = IIEP_NOT_USEABLE;
				}
			}
		}
		break;
	case GDI_IMMEDIATEITEM_NUM:
		if (uParam >= 0 && uParam < IMMEDIACY_ROOM_WIDTH)
			nRet = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetSameDetailItemNum(uParam);
		break;
	//与NPC的买卖
	//uParam = (KUiObjAtContRegion*) pInfo -> KUiObjAtRegion结构数组的指针，KUiObjAtRegion
	//				结构用于存储物品的数据及其放置区域位置信息。
	//				其中KUiObjAtContRegion::nContainer值表示第几页的物品
	//nParam = pInfo数组中包含KUiObjAtRegion结构的数目
	//Return = 如果返回值小于等于传入参数nParam，其值表示pInfo数组中的前多少个KUiObjAtRegion
	//			结构被填充了有效的数据；否则表示需要传入包含多少个KUiObjAtRegion结构的数组
	//			才够存储全部的npc列出来交易的物品信息。
	case GDI_TRADE_NPC_ITEM:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			int nPage = 0;
			int nIndex = 0;
			KUiObjAtContRegion* pInfo = (KUiObjAtContRegion *)uParam;
			int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop];

			if (nBuyIdx == -1)
				break;
			if (nBuyIdx >= BuySell.GetHeight())
				break;
			if (!BuySell.m_pShopRoom)
				break;
			BuySell.m_pShopRoom->Clear();
			for (int i = 0; i < BuySell.GetWidth(); i++)
			{
				nIndex = BuySell.GetItemIndex(nBuyIdx, i);
				KItem* pItem = BuySell.GetItem(nIndex);
				
				if (nIndex >= 0 && pItem)
				{
					// Set pInfo->Obj.uGenre
					pInfo->Obj.uGenre = CGOG_NPCSELLITEM;
					// Set pInfo->Obj.uId
					pInfo->Obj.uId = i;

					POINT	Pos;
					if (BuySell.m_pShopRoom->FindRoom(pItem->GetWidth(), pItem->GetHeight(), &Pos))
					{
						// nIndex + 1保证不为0
						BuySell.m_pShopRoom->PlaceItem(Pos.x, Pos.y, nIndex + 1, pItem->GetWidth(), pItem->GetHeight());
					}
					else
					{
						nPage++;
						BuySell.m_pShopRoom->Clear();
						// Clear完成后必然成功，所以没有判断
						BuySell.m_pShopRoom->FindRoom(pItem->GetWidth(), pItem->GetHeight(), &Pos);
						BuySell.m_pShopRoom->PlaceItem(Pos.x, Pos.y, nIndex + 1, pItem->GetWidth(), pItem->GetHeight());
					}
					pInfo->Region.h = Pos.x;
					pInfo->Region.v = Pos.y;
					pInfo->Region.Width = pItem->GetWidth();
					pInfo->Region.Height = pItem->GetHeight();
					pInfo->nContainer = nPage;
					nCount++;
					pInfo++;
				}
			}			
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			int nIndex = 0;
			int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop];
			if (nBuyIdx == -1)
				break;
			if (nBuyIdx >= BuySell.GetHeight())
				break;
			for (int i = 0; i < BuySell.GetWidth(); i++)
			{
				nIndex = BuySell.GetItemIndex(nBuyIdx, i);
				KItem* pItem = BuySell.GetItem(nIndex);
				
				if (nIndex >= 0 && pItem)
				{
					nCount++;
				}
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX1:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom1)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom1)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom1)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom1)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX2:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom2)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom2)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom2)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom2)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX3:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom3)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom3)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom3)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom3)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX4:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom4)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom4)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom4)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom4)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_IN_STORE_BOX5:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_repository);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom5)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom5)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_repositoryroom5)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_repositoryroom5)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_ITEM_EXPANDITEM:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			pInfo->Obj.uGenre = CGOG_MONEY;
			pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_equipmentex);
			nCount++;
			pInfo++;
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_equiproomex)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_equiproomex)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_equiproomex)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_equiproomex)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_PK_SETTING:					//获取pk设置
		nRet = Player[CLIENT_PLAYER_INDEX].m_cPK.GetNormalPKState();
		break;
	case GDI_SHOW_PLAYERS_NAME:			//获取显示各玩家人名
		nRet = NpcSet.CheckShowName();
		break;
	case GDI_SHOW_PLAYERS_LIFE:			//获取显示各玩家生命
		nRet = NpcSet.CheckShowLife();
		break;
	case GDI_ITEM_GIVE:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_give)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_give)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;

//			nCount++;

			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_give)
			{
				nCount++;
			}	
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_give)
				{
					nCount++;
				}	
			}
			nRet = nCount;
		}
		break;
	case GDI_EXBOX_NUM:
			nRet = Player[CLIENT_PLAYER_INDEX].m_btRepositoryNum;
			break;
	case GDI_EQUIPEX_TIME:
			nRet = Player[CLIENT_PLAYER_INDEX].m_dwEquipExpandTime - KSG_GetCurSec();
			break;
	case GDI_ITEM_NAME:
		if(uParam)
		{
			char* pzName = (char*)uParam;
			if(nParam)
				strcpy(pzName, Item[nParam].GetName());
		}
		break;
	case GDI_ITEM_QUALITY:
		if(uParam)
		{
			nRet = Item[uParam].GetQuality();
		}
		break;
	case GDI_ITEM_DESC:
		if (nParam && uParam)
		{
			char* pszDescript = (char *)uParam;
			pszDescript[0] = 0;
			Item[nParam].GetDesc(pszDescript);
		}
		break;
	case GDI_GET_ITEM_PARAM:
		if(uParam && nParam)
		{
			char *CIParam = (char *)uParam;
			
			char Buffer[16];
			int nOffset = 0;
			itoa(Item[nParam].GetNature(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetGenre(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			if(Item[nParam].GetNature() >= NATURE_GOLD)
				itoa(Item[nParam].GetRow(), Buffer, 10);
			else
				itoa(Item[nParam].GetDetailType(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetParticular(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetSeries(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetLevel(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].m_GeneratorParam.nLuck, Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].m_GeneratorParam.uRandomSeed, Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].m_GeneratorParam.nVersion, Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetDurability(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			itoa(Item[nParam].GetExpireTime(), Buffer, 10);
			strcat(&CIParam[nOffset],Buffer);
			nOffset += strlen(Buffer);
			CIParam[nOffset] = ',';
			nOffset++;
			for (int j = 0; j < MAX_ITEM_MAGICLEVEL; j++)
			{
				sprintf(Buffer, "%d", Item[nParam].m_GeneratorParam.nGeneratorLevel[j]);
				strcat(&CIParam[nOffset],Buffer);
				nOffset += strlen(Buffer);
				if (j == MAX_ITEM_MAGICLEVEL - 1) break;
				CIParam[nOffset] = ',';
				nOffset++;
			}
			CIParam[nOffset] = '\0';
			nRet = 1;
		}
		break;
	case GDI_ITEM_CHAT:
		if(uParam)
		{
			if(nParam)
			{
				char *CIParam = (char *)nParam;
				int CIParamV[11], nMagicParam[MAX_ITEM_MAGICLEVEL];
				char* cParam = strtok(CIParam, ",");
				int i = 0, j = 0;
				while(cParam)
				{
					if (i>=10+MAX_ITEM_MAGICLEVEL)break;
					else if (i>= 11)
					{
						if (j>=MAX_ITEM_MAGICLEVEL)break;
						nMagicParam[j] = atoi(cParam);
						j++;
					}
					else
						CIParamV[i] = atoi(cParam);
					cParam = strtok(NULL, ",");
					i ++;
				}
				nRet = ItemSet.Add(
						CIParamV[0],
						CIParamV[1],
						CIParamV[4],
						CIParamV[5],
						CIParamV[6],
						CIParamV[2],
						CIParamV[3],
						nMagicParam,
						CIParamV[8],
						CIParamV[7]);
				if(nRet)
				{
					Item[nRet].SetDurability(CIParamV[9]);
					Item[nRet].SetExpireTime(CIParamV[10]);
				}
			}
		}
		else
			ItemSet.Remove((int)nParam);
		break;
	case GDI_SHOW_OBJ_NAME:
			nRet = NpcSet.CheckShowObj();
			break;
	case GDI_TRADE_PLAYER_ITEM:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtContRegion* pInfo = (KUiObjAtContRegion*)uParam;
			
			for (int i = 0; i < MAX_EQUIPMENT_ITEM; i++)
			{
				if (g_cSellItem.m_sItem[i].nIdx)
				{
					pInfo->Obj.uGenre = CGOG_PLAYERSELLITEM;
					pInfo->Obj.uId = g_cSellItem.m_sItem[i].nIdx;
					
					pInfo->Region.Width = Item[pInfo->Obj.uId].GetWidth();
					pInfo->Region.Height = Item[pInfo->Obj.uId].GetHeight();
					pInfo->Region.h = g_cSellItem.m_sItem[i].nX;
					pInfo->Region.v = g_cSellItem.m_sItem[i].nY;
					nCount++;
					pInfo++;
				}		
			}
			
			nRet = nCount;
		} 
		else
		{
			nRet = g_cSellItem.GetCount();
		}
		break;
	case GDI_COMPOUND:
		nRet = 0;
		if (uParam)
		{
			// TODO：暂时没有做第二套装备
			if (nParam == 1)
				break;

			int PartCompoundConvert[compound_num] = 
			{
				MOSAICENCRUSTED_UIEP_BOX_1,
				MOSAICENCRUSTED_UIEP_BOX_2,
				MOSAICENCRUSTED_UIEP_BOX_3,
			};

			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;

			for (int i = 0; i < MAX_COMPOUND_ITEM; i++)
			{	
				pInfo->Obj.uId = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetCompoundItem(i);
				if (pInfo->Obj.uId)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
				
					pInfo->Region.Width = Item[pInfo->Obj.uId].GetWidth();
					pInfo->Region.Height = Item[pInfo->Obj.uId].GetHeight();
					pInfo->Region.h = 0;
					pInfo->Region.v = PartCompoundConvert[i];
				}
				else
				{
					pInfo->Obj.uGenre = CGOG_NOTHING;
				}
				nCount++;
				pInfo++;
			}
			nRet = nCount;
		}
		break;
	case GDI_COMPOUND_BOX:
		nRet = 0;
		if (uParam)
		{
			int nCount = 0;
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*)uParam;
			
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_compoundroom)
			{
				pInfo->Obj.uGenre = CGOG_ITEM;
				pInfo->Obj.uId = pItem->nIdx;
				pInfo->Region.h = pItem->nX;
				pInfo->Region.v = pItem->nY;
				pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
				pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
				nCount++;
				pInfo++;
			}
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_compoundroom)
				{
					pInfo->Obj.uGenre = CGOG_ITEM;
					pInfo->Obj.uId = pItem->nIdx;		
					pInfo->Region.h = pItem->nX;
					pInfo->Region.v = pItem->nY;
					pInfo->Region.Width = Item[pItem->nIdx].GetWidth();
					pInfo->Region.Height = Item[pItem->nIdx].GetHeight();
					nCount++;
					pInfo++;
				}
				if (nCount > nParam)
					break;
			}
			nRet = nCount;
		}
		else
		{
			int nCount = 0;
			// 第一个是钱
			nCount++;
			// 后面是装备
			PlayerItem* pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetFirstItem();
			if (pItem && pItem->nPlace == pos_compoundroom)
				nCount++;
			while(pItem)
			{
				pItem = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetNextItem();
				if (pItem && pItem->nPlace == pos_compoundroom)
					nCount++;
			}
			nRet = nCount;
		}
		break;
	case GDI_LOCKMOVE:
		nRet = Player[CLIENT_PLAYER_INDEX].GetLockMove()->bLock;
		break;
	case GDI_NPC_STATE_SKILL:
		nRet = 0;
		if (uParam)
		{
			KStateNode* pNode;
			pNode = (KStateNode *)Npc[CLIENT_PLAYER_INDEX].m_StateSkillList.GetHead();
			KStateControl* pInfo = (KStateControl *)uParam;
			
			while(pNode)
			{
				if (pNode->m_SkillID > 0 && pNode->m_SkillID < MAX_SKILL && pNode->m_Level > 0 && pNode->m_Level < MAX_SKILLLEVEL)
				{
					KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(pNode->m_SkillID, pNode->m_Level);
					if (pOrdinSkill->GetSkillStyle() == SKILL_SS_Missles || 
						pOrdinSkill->GetSkillStyle() == SKILL_SS_InitiativeNpcState)
					{
						pInfo->nSkillId = pNode->m_SkillID;
						pInfo->nLeftTime = pNode->m_LeftTime;
						pInfo++;
					}
				}
				pNode = (KStateNode *)pNode->GetNext();
			}	
		}
		else
		{
			int nCount = 0;
			KStateNode* pNode;
			pNode = (KStateNode *)Npc[CLIENT_PLAYER_INDEX].m_StateSkillList.GetHead();
			while(pNode)
			{
				if (pNode->m_SkillID > 0 && pNode->m_SkillID < MAX_SKILL && pNode->m_Level > 0 && pNode->m_Level < MAX_SKILLLEVEL)
				{
					KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(pNode->m_SkillID, pNode->m_Level);
					if (pOrdinSkill->GetSkillStyle() == SKILL_SS_Missles || 
						pOrdinSkill->GetSkillStyle() == SKILL_SS_InitiativeNpcState)
					{
						nCount++;
					}
				}
				pNode = (KStateNode *)pNode->GetNext();
			}
			nRet = nCount;
		}
		break;	
	case GDI_MISSION_SELFDATA:
		if (uParam)
		{
			TMissionLadderSelfInfo *pRankData = (TMissionLadderSelfInfo *)uParam;
			*pRankData = Player[CLIENT_PLAYER_INDEX].m_MissionData;
		}
		break;	
	case GDI_MISSION_RANKDATA:
		if (uParam)
		{
			TMissionLadderInfo *pRankData = (TMissionLadderInfo *)uParam;
			*pRankData = Player[CLIENT_PLAYER_INDEX].m_MissionRank[nParam];
		}
		break;		
	case GDI_CHATROOM_NAME:
		if (uParam)
		{
			char *pRoomName = (char *)uParam;
			strcpy(pRoomName, Player[CLIENT_PLAYER_INDEX].m_cRoom.m_szRoomName);
		}
		break;
	
	case GDI_IS_CHECK_IMAGE:			//Kiem tra hinh anh nguoi choi da chon
		nRet = Player[CLIENT_PLAYER_INDEX].m_ImagePlayer;
		break;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：向游戏发送操作
//	参数：unsigned int uDataId --> Core外部客户对core的操作请求的索引定义
//							其值为梅举类型GAMEOPERATION_INDEX的取值之一。
//		  unsigned int uParam  --> 依据uOperId的取值情况而定
//		  int nParam --> 依据uOperId的取值情况而定
//	返回：如果成功发送操作请求，函数返回非0值，否则返回0值。
//--------------------------------------------------------------------------
int	KCoreShell::OperationRequest(unsigned int uOper, unsigned int uParam, int nParam)
{
	int nRet = 1;
	switch(uOper)
	{
	case GOI_QUERY_RANK_INFORMATION:
		SendClientCmdQueryLadder(uParam);
		break;
	//uParam = (const char*)pszFileName
	case GOI_PLAY_SOUND:
		if (uParam)
		{
			static KCacheNode* pSndNode = NULL;
			KWavSound* pSound = NULL;
			pSndNode	= (KCacheNode*)g_SoundCache.GetNode((char *)uParam, (KCacheNode * )pSndNode);
			pSound		= (KWavSound*)pSndNode->m_lpData;
			if (pSound)
			{
				if (pSound->IsPlaying())
					break;
				pSound->Play(0, -10000 + Option.GetSndVolume() * 100, 0);
			}
		}
		break;
	case GOI_PLAYER_RENASCENCE:
		{
			int nReviveType;
			if (nParam)	// bBackTown
			{
				nReviveType = REMOTE_REVIVE_TYPE;
			}
			else
			{
				nReviveType = LOCAL_REVIVE_TYPE;
			}
//			SendClientCmdRevive(nReviveType);
			SendClientCmdRevive();
		}
		break;
	case GOI_MONEY_INOUT_STORE_BOX:
		{
			BOOL	bIn = (BOOL)uParam;
			int		nMoney = nParam;
			int		nSrcRoom, nDesRoom;


			if (bIn)
			{
				nSrcRoom = room_equipment;
				nDesRoom = room_repository;
			}
			else
			{
				nDesRoom = room_equipment;
				nSrcRoom = room_repository;
			}
			Player[CLIENT_PLAYER_INDEX].m_ItemList.ExchangeMoney(nSrcRoom, nDesRoom, nMoney);
		}
		break;
		//离开游戏
	case GOI_EXIT_GAME:
		g_SubWorldSet.Close();
		g_ScenePlace.ClosePlace();
		break;
	case GOI_GAMESPACE_DISCONNECTED:
		g_SubWorldSet.Close();
		break;
	case GOI_TRADE_NPC_BUY:
		{
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;
			if (pObject1->Obj.uGenre != CGOG_NPCSELLITEM)
				break;

			int nIdx = 0;
			if (nParam <= 0)
				break;

			SendClientCmdBuy(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop, pObject1->Obj.uId, nParam);
		}
		break;
	case GOI_TRADE_NPC_SELL:
		{
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;

			if (CGOG_ITEM != pObject1->Obj.uGenre)
				break;
			//放下去的东西不为空，所以是卖东西
			int nIdx = pObject1->Obj.uId;	//Player[CLIENT_PLAYER_INDEX].m_ItemList.Hand();
			if (nIdx > 0 && nIdx < MAX_ITEM)
			{
				SendClientCmdSell(Item[nIdx].GetID(), nParam);
				return 1;
			}
			else
			{
				return 0;
			}
		}
		break;
	case GOI_TRADE_NPC_REPAIR:
		{
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;

			if (CGOG_ITEM != pObject1->Obj.uGenre)
				break;
			//放下去的东西不为空，所以是卖东西
			int nIdx = pObject1->Obj.uId;	//Player[CLIENT_PLAYER_INDEX].m_ItemList.Hand();
			if (nIdx > 0 && nIdx < MAX_ITEM)
			{		
				if (Item[nIdx].GetGenre() != item_equip)
					return 0;
				else if (Item[nIdx].GetDurability() == -1 || Item[nIdx].GetDurability() >= Item[nIdx].GetMaxDurability())
				{
					return 0;
				}
				else if (Item[nIdx].GetRepairPrice() <= Player[CLIENT_PLAYER_INDEX].m_ItemList.GetEquipmentMoney())
				{
					SendClientCmdRepair(Item[nIdx].GetID());
				}
				else
				{
					KSystemMessage	sMsg;
					sprintf(sMsg.szMessage, MSG_SHOP_NO_MONEY);
					sMsg.eType = SMT_NORMAL;
					sMsg.byConfirmType = SMCT_NONE;
					sMsg.byPriority = 0;
					sMsg.byParamSize = 0;
					CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
					return 0;
				}
				
				return 1;
			}
			else
			{
				return 0;
			}
		}
		break;

	case GOI_SWITCH_OBJECT:
		//uParam = (KUiObjAtContRegion*)pObject1 -> 拿起的物品操作前的信息
		//nParam = (KUiObjAtContRegion*)pObject2 -> 放下的物品操作后的信息
		{
			ItemPos	P1, P2;
			int PartConvert[itempart_num] = 
			{ 
				itempart_head,		itempart_weapon,
				itempart_amulet,	itempart_cuff,
				itempart_body,		itempart_belt,
				itempart_ring1,		itempart_ring2,
				itempart_pendant,	itempart_foot,
				itempart_horse,		itempart_mask,
				itempart_mantle,	itempart_signet,
				itempart_shipin,	itempart_hoods,
				itempart_cloak,
			};
			int PartCompoundConvert[MAX_COMPOUND_ITEM] = 
			{ 
				compound_box1,	compound_box2,
				compound_box3,
			};
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;
			KUiObjAtContRegion* pObject2 = (KUiObjAtContRegion*)nParam;
			
			if (!pObject1 && !pObject2)
				break;
			
			if (pObject1)
			{
				switch(pObject1->eContainer)
				{
				case UOC_STORE_BOX:
					P1.nPlace = pos_repositoryroom;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_STORE_BOX1:
					P1.nPlace = pos_repositoryroom1;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_STORE_BOX2:
					P1.nPlace = pos_repositoryroom2;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_STORE_BOX3:
					P1.nPlace = pos_repositoryroom3;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_STORE_BOX4:
					P1.nPlace = pos_repositoryroom4;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_STORE_BOX5:
					P1.nPlace = pos_repositoryroom5;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_IMMEDIA_ITEM:
					P1.nPlace = pos_immediacy;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_ITEM_TAKE_WITH:
					P1.nPlace = pos_equiproom;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_ITEM_TAKE_WITH_EX:
					P1.nPlace = pos_equiproomex;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_EQUIPTMENT:
					{
						// TODO:暂时没有第二装备
						if (pObject1->Region.h == 1)
							break;
						P1.nPlace = pos_equip;
						P1.nX = PartConvert[pObject1->Region.v];
					}
					break;
				case UOC_TO_BE_TRADE:
					P1.nPlace = pos_traderoom;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_ITEM_GIVE:
					P1.nPlace = pos_give;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_COMPOUND:
					{
						if (pObject1->Region.h == 1)
							break;
						P1.nPlace = pos_compound;
						P1.nX = PartCompoundConvert[pObject1->Region.v];
					}
					break;
				case UOC_COMPOUND_BOX:
					P1.nPlace = pos_compoundroom;
					P1.nX = pObject1->Region.h;
					P1.nY = pObject1->Region.v;
					break;
				case UOC_NPC_SHOP:
					if (CGOG_NPCSELLITEM != pObject1->Obj.uGenre)
						break;

					SendClientCmdBuy(Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop, pObject1->Obj.uId, nParam);
					break;
				}
			}
			
			if (pObject2)
			{
				switch(pObject2->eContainer)
				{
				case UOC_STORE_BOX:
					P2.nPlace = pos_repositoryroom;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_STORE_BOX1:
					P2.nPlace = pos_repositoryroom1;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_STORE_BOX2:
					P2.nPlace = pos_repositoryroom2;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_STORE_BOX3:
					P2.nPlace = pos_repositoryroom3;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_STORE_BOX4:
					P2.nPlace = pos_repositoryroom4;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_STORE_BOX5:
					P2.nPlace = pos_repositoryroom5;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_IMMEDIA_ITEM:
					P2.nPlace = pos_immediacy;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_ITEM_TAKE_WITH:
					P2.nPlace = pos_equiproom;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_ITEM_TAKE_WITH_EX:
					P2.nPlace = pos_equiproomex;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_EQUIPTMENT:
					{
						// TODO:暂时没有第二装备
						if (pObject2->Region.h == 1)
							break;
						P2.nPlace = pos_equip;
						P2.nX = PartConvert[pObject2->Region.v];
					}
					break;
				case UOC_TO_BE_TRADE:
					P2.nPlace = pos_traderoom;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_ITEM_GIVE:
					P2.nPlace = pos_give;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_COMPOUND:
					{
						if (pObject2->Region.h == 1)
							break;
						P2.nPlace = pos_compound;
						P2.nX = PartCompoundConvert[pObject2->Region.v];
					}
					break;
				case UOC_COMPOUND_BOX:
					P2.nPlace = pos_compoundroom;
					P2.nX = pObject2->Region.h;
					P2.nY = pObject2->Region.v;
					break;
				case UOC_NPC_SHOP:
					break;
				}
				if (Player[CLIENT_PLAYER_INDEX].GetLockMove()->bLock && 
					P2.nPlace == Item[pObject2->Obj.uId].GetBackLocal()->nPlace)
				{
					P2.nPlace = Item[pObject2->Obj.uId].GetBackLocal()->nPlace;
					P2.nX = Item[pObject2->Obj.uId].GetBackLocal()->nX;
					P2.nY = Item[pObject2->Obj.uId].GetBackLocal()->nY;
				}
			}
			if (!pObject1)
			{
				memcpy(&P1, &P2, sizeof(P1));
			}
			if (!pObject2)
			{
				memcpy(&P2, &P1, sizeof(P1));
			}
			Player[CLIENT_PLAYER_INDEX].MoveItem(P1, P2);
		}
		break;

	//玩家点完对话框
	case GOI_INFORMATION_CONFIRM_NOTIFY:
	{
		PLAYER_SELECTUI_COMMAND command;
		command.nSelectIndex = nParam;
		command.nSelectUi = SELECT_TALKUI;
		Player[CLIENT_PLAYER_INDEX].OnSelectFromUI(&command, UI_TALKDIALOG);
		break;
	}
	
	//问题选择答案
	//nParma = nAnswerIndex
	case GOI_QUESTION_CHOOSE:
		if (g_bUISelLastSelCount == 0 )
			break;	
		{
			PLAYER_SELECTUI_COMMAND command;
			command.nSelectIndex = nParam;
			command.nSelectUi = SELECT_SELECTUI;
			Player[CLIENT_PLAYER_INDEX].OnSelectFromUI(&command, UI_SELECTDIALOG);
		}
		break;
	//设置立即技能
	//uParam = (KUiGameObject*)pSKill, 技能信息
	//nParam = 立即位置，0表示为右键技能，1至4表示为F1至F4技能
	//使用物品
	//uParam = (KUiObjAtRegion*)pInfo -> 物品的数据以及物品原来摆放的位置
	//nParam = 物品使用前放置的位置，取值为枚举类型UIOBJECT_CONTAINER。
	case GOI_USE_ITEM:
		//to do: waiting for...
		if (uParam)
		{
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*) uParam;
			int nPlace = nParam;
			ItemPos	Pos;
			switch(nPlace)
			{
			case UOC_ITEM_TAKE_WITH:
				Pos.nPlace = pos_equiproom;
				break;
			case UOC_ITEM_TAKE_WITH_EX:
				Pos.nPlace = pos_equiproomex;
				break;
			case UOC_IMMEDIA_ITEM:
				Pos.nPlace = pos_immediacy;
				break;
			default:
				Pos.nPlace = -1;
				break;
			}
			Pos.nX = pInfo->Region.h;
			Pos.nY = pInfo->Region.v;
			if ((pInfo->Obj.uGenre == CGOG_ITEM || pInfo->Obj.uGenre == CGOG_IME_ITEM) && pInfo->Obj.uId > 0 && Pos.nPlace != -1)
				Player[CLIENT_PLAYER_INDEX].ApplyUseItem(pInfo->Obj.uId, Pos);
		}
		break;
	case GOI_SET_IMMDIA_SKILL:
		if (uParam)
		{
			KUiGameObject* pSkill = (KUiGameObject*)uParam;
			if (nParam == 0)
				//to do : modify;
			{
				if ( (int)pSkill->uId > 0 )
					Player[CLIENT_PLAYER_INDEX].SetLeftSkill((int)pSkill->uId);
				else if ((int)(pSkill->uId) == -1) //设置当前物理技能为左技能
				{
					int nDetailType = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetWeaponType();
					int nParticularType = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetWeaponParticular();
					
					//近身武器
					if (nDetailType == 0)
					{
						Player[CLIENT_PLAYER_INDEX].SetLeftSkill(g_nMeleeWeaponSkill[nParticularType]);
					}//远程武器
					else if (nDetailType == 1)
					{
						Player[CLIENT_PLAYER_INDEX].SetLeftSkill(g_nRangeWeaponSkill[nParticularType]);
					}//空手
					else if (nDetailType == -1)
					{
						Player[CLIENT_PLAYER_INDEX].SetLeftSkill(g_nHandSkill);
					}
					
				}
			}
			else if (nParam == 1)
			{
				if ((int)pSkill->uId > 0)
					Player[CLIENT_PLAYER_INDEX].SetRightSkill((int)pSkill->uId);
				else if ((int)(pSkill->uId) == -1)
				{
					int nDetailType = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetWeaponType();
					int nParticularType = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetWeaponParticular();
					
					if (nDetailType == 0)
					{
						Player[CLIENT_PLAYER_INDEX].SetRightSkill(g_nMeleeWeaponSkill[nParticularType]);
					}
					else if (nDetailType == 1)
					{
						Player[CLIENT_PLAYER_INDEX].SetRightSkill(g_nRangeWeaponSkill[nParticularType]);
					}
					else if (nDetailType == -1)
					{
						Player[CLIENT_PLAYER_INDEX].SetRightSkill(g_nHandSkill);
					}
				}
			}
		}
		break;

	//增强一种技能，，一次加一点
	//uParam = 技能类属
	//nParam = (uint)技能id
	case GOI_TONE_UP_SKILL:
		Player[CLIENT_PLAYER_INDEX].ApplyAddSkillLevel((int)nParam, 1);
		break;

	//增强一些属性的值，一次加一点
	//uParam = 表示要增强的是哪个属性，取值为UI_PLAYER_ATTRIBUTE的梅举值之一
	case GOI_TONE_UP_ATTRIBUTE:
			if (nParam)
			Player[CLIENT_PLAYER_INDEX].ApplyAddBaseAttribute(uParam, nParam);
		break;	

	//答应/拒绝交易请求
	//uParam = (KUiPlayerItem*)pRequestPlayer 发出请求的玩家
	//nParam = (int)(bool)bAccept 是否接受请求
	case GOI_TRADE_INVITE_RESPONSE:
		if (uParam)
		{
			KTrade::ReplyInvite(((KUiPlayerItem*)uParam)->nIndex, nParam);
		}
		break;
	case GOI_TRADE_FOLKGAME_INVITE_RESPONSE:
		if (uParam)
		{
			KTrade::ReplyInvite(((KUiPlayerItem*)uParam)->nIndex, nParam, true);
		}
		break;
	//增减一个欲卖出的物品
	//uParam = (KUiObjAtRegion*) pObject -> 物品信息，其中坐标信息为在交易界面中的坐标
	//nParam = bAdd -> 0值表示减少，1值表示增加
	//Remark : 如果物品是金钱的话，则KUiObjAtRegion::Obj::uId表示把金钱额调整为这个值，且nParam无意义。
	case GOI_TRADE_DESIRE_ITEM:
		if (uParam)
		{
			KUiObjAtRegion* pInfo = (KUiObjAtRegion*) uParam;
			if (pInfo->Obj.uGenre != CGOG_MONEY)
				break;
			Player[CLIENT_PLAYER_INDEX].TradeMoveMoney(pInfo->Obj.uId);
		}
		break;

	//有无交易意向
	//nParam = bWilling
	case GOI_TRADE_WILLING:
		if (!nParam || Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetMenuState() == PLAYER_MENU_STATE_TRADEOPEN)
			Player[CLIENT_PLAYER_INDEX].TradeApplyClose();
		else
			Player[CLIENT_PLAYER_INDEX].TradeApplyOpen((char*)uParam, nParam);
		break;

	//锁定交易
	//nParam = (int)(book)bLock 是否锁定
	case GOI_TRADE_LOCK:
		if ( !Player[CLIENT_PLAYER_INDEX].CheckTrading() )
			break;
			Player[CLIENT_PLAYER_INDEX].TradeApplyLock();
		break;

	//交易
	case GOI_TRADE:
		if ( !Player[CLIENT_PLAYER_INDEX].CheckTrading() )
			break;
		if (Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeLock != 1 || Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeDestLock != 1)
			break;
		if (Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeState == 0)
			Player[CLIENT_PLAYER_INDEX].TradeDecision(1, uParam);		// 交易确定

		break;
	case GOI_TRADE_FOLKGAMEID:
		if ( !Player[CLIENT_PLAYER_INDEX].CheckTrading() )
			break;
		if (Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeLock != 1 || Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeDestLock != 1)
			break;
		if (Player[CLIENT_PLAYER_INDEX].m_cTrade.m_nTradeState)
			Player[CLIENT_PLAYER_INDEX].TradeDecision(2, uParam);
		break;
	//交易取消
	case GOI_TRADE_CANCEL:
		if ( !Player[CLIENT_PLAYER_INDEX].CheckTrading() )
			break;
		Player[CLIENT_PLAYER_INDEX].TradeDecision(0);		// 交易取消
		break;
	case GOI_SPAR_INVITE_RESPONSE:
		if (uParam)
			Player[CLIENT_PLAYER_INDEX].m_cPK.ReplyInvite(((KUiPlayerItem*)uParam)->uId, nParam);
		break;
	//新添加聊天好友
	//uParam = (KUiPlayerItem*)pFriend
	//			KUiPlayerItem::nData = 0
	case GOI_CHAT_FRIEND_ADD:
		{
			if (g_pClient)
			{
				size_t pckgsize = sizeof(tagExtendProtoHeader) + sizeof(ASK_ADDFRIEND_CMD);

				tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
				pExHeader->ProtocolType = c2s_extendfriend;
				pExHeader->wLength = pckgsize - 1;

				ASK_ADDFRIEND_CMD* pAafCmd = (ASK_ADDFRIEND_CMD*)(pExHeader + 1);
				pAafCmd->ProtocolFamily = pf_friend;
				pAafCmd->ProtocolID = friend_c2c_askaddfriend;
				pAafCmd->pckgid = -1;
				strncpy(pAafCmd->dstrole, ((KUiPlayerItem*)uParam)->Name, _NAME_LEN);
				g_pClient->SendPackToServer(pExHeader, pckgsize);

				KSystemMessage	sMsg;
				sprintf(sMsg.szMessage, MSG_CHAT_APPLY_ADD_FRIEND, ((KUiPlayerItem*)uParam)->Name);
				sMsg.eType = SMT_NORMAL;
				sMsg.byConfirmType = SMCT_NONE;
				sMsg.byPriority = 0;
				sMsg.byParamSize = 0;
				CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
			}
		}
		break;
	//对别人要加自己为好友的回复
	//uParam = (KUiPlayerItem*)pRequestPlayer 发出请求的玩家
	//nParam = (int)(bool)bAccept 是否接受请求
	case GOI_CHAT_FRIEND_INVITE:
		if (uParam)
		{
			if (g_pClient)
			{
				size_t pckgsize = sizeof(tagExtendProtoHeader) + sizeof(REP_ADDFRIEND_CMD);

				tagExtendProtoHeader* pExHeader = (tagExtendProtoHeader*)_alloca(pckgsize);
				pExHeader->ProtocolType = c2s_extendfriend;
				pExHeader->wLength = pckgsize - 1;
				
				REP_ADDFRIEND_CMD* pRafCmd = (REP_ADDFRIEND_CMD*)(pExHeader + 1);
				pRafCmd->ProtocolFamily = pf_friend;
				pRafCmd->ProtocolID = friend_c2c_repaddfriend;
				pRafCmd->pckgid = -1;
				strncpy(pRafCmd->dstrole, ((KUiPlayerItem*)uParam)->Name, _NAME_LEN);
				pRafCmd->answer = nParam ? answerAgree : answerDisagree;
				g_pClient->SendPackToServer(pExHeader, pckgsize);
			}
		}
		break;

	case GOI_OPTION_SETTING:			//选项设置
		if (uParam == OPTION_DYNALIGHT)
		{
			g_ScenePlace.EnableDynamicLights(nParam != 0);
			if (g_pRepresent)
				g_pRepresent->SetOption(DYNAMICLIGHT, nParam != 0);
		}
		else if (uParam == OPTION_PERSPECTIVE)
		{
			if (g_pRepresent)
				g_pRepresent->SetOption(PERSPECTIVE, nParam != 0);
		}
		else if (uParam == OPTION_MUSIC_VALUE)
			Option.SetMusicVolume(nParam);
		else if (uParam == OPTION_SOUND_VALUE)
			Option.SetSndVolume(nParam);
		else if (uParam == OPTION_BRIGHTNESS)
			Option.SetGamma(nParam);
		else if (uParam == OPTION_WEATHER)
			g_ScenePlace.EnableWeather(nParam);
		break;
	case GOI_VIEW_PLAYERITEM:
		g_cViewItem.ApplyViewEquip(uParam);
		break;
	case GOI_VIEW_PLAYERITEM_END:
		g_cViewItem.DeleteAll();
		break;
	case GOI_PLAYER_ACTION:
		{
			if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
				return 0;

			switch(uParam)
			{
			case PA_RUN:
				Player[CLIENT_PLAYER_INDEX].m_RunStatus = !Player[CLIENT_PLAYER_INDEX].m_RunStatus;
				break;
			case PA_SIT:
				if (!Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_bRideHorse)
				{
					if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_Doing != do_sit)
					{
						Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].SendCommand(do_sit);
						SendClientCmdSit(true);
					}
					else
					{
						Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].SendCommand(do_stand);
						SendClientCmdSit(false);
					}
				}
				else
				{
					KSystemMessage	sMsg;
					strcpy(sMsg.szMessage, MSG_CANT_SIT_RIDE);
					sMsg.eType = SMT_NORMAL;
					sMsg.byConfirmType = SMCT_NONE;
					sMsg.byPriority = 0;
					sMsg.byParamSize = 0;
					CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
				}
				break;
			case PA_RIDE:	
				SendClientCmdRide();
				break;
			}
		}
		break;
	case GOI_PK_SETTING:		//设置PK
		{
			Player[CLIENT_PLAYER_INDEX].m_cPK.ApplySetNormalPKState(nParam);
		}
		break;
	//仇杀某人
	//uParam = (KUiPlayerItem*) pTarget	仇杀目标
	case GOI_REVENGE_SOMEONE:
		if (uParam)
		{
			KUiPlayerItem	*pTarget = (KUiPlayerItem*)uParam;
			Player[CLIENT_PLAYER_INDEX].m_cPK.ApplyEnmityPK(pTarget->uId);
		}
		break;
	case GOI_SPAR_SOMEONE:
		if (uParam)
		{
			KUiPlayerItem	*pTarget = (KUiPlayerItem*)uParam;
			Player[CLIENT_PLAYER_INDEX].m_cPK.SparApplyStart(pTarget->uId);
		}
		break;
	//跟随某人
	//uParam = (KUiPlayerItem*) pTarget	跟随目标
	case GOI_FOLLOW_SOMEONE:
		if (uParam)
		{
			KUiPlayerItem	*pTarget = (KUiPlayerItem*)uParam;
			if (Npc[pTarget->nIndex].m_Kind == kind_player && 
				!Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_PTrade.nTrade)
				Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_nPeopleIdx = pTarget->nIndex;
		}
		break;
	case GOI_VIEW_PLAYERSHOP:
		if (uParam)
		{
			if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
				return 0;

			KUiPlayerItem	*pTarget = (KUiPlayerItem*)uParam;
			if (Npc[pTarget->nIndex].m_Kind == kind_player)
				g_cSellItem.Update(TRUE, Npc[pTarget->nIndex].m_dwID);
		}
		break;
	//显示各玩家人名
	//nParam = (int)(bool)bShow	是否显示
	case GOI_SHOW_PLAYERS_NAME:
		NpcSet.SetShowNameFlag(nParam);
		break;
	//显示各玩家生命
	//nParam = (int)(bool)bShow	是否显示
	case GOI_SHOW_PLAYERS_LIFE:
		NpcSet.SetShowLifeFlag(nParam);
		break;
	case GOI_INPUT:
		if(uParam)
		{
			C2S_BUF_COMMAND sIP;
			sIP.ProtocolType = c2s_inputcommand;
			strcpy(sIP.szBuf, (char*)uParam);
			g_pClient->SendPackToServer((BYTE*)&sIP, sizeof(C2S_BUF_COMMAND));
		}
		break;
	case GOI_LOCKSTATE:
		{
			PLAYER_COMMAND	sLS;
			sLS.ProtocolType = c2s_playercommand;
			sLS.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_LOCKSTATE;
			sLS.m_lpBuf = 0;
			sLS.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
			g_pClient->SendPackToServer((BYTE*)&sLS, sLS.m_wLength + 1);
		}
		break;
	case GOI_GIVE:
		{
			PLAYER_COMMAND	sGv;
			sGv.ProtocolType = c2s_playercommand;
			sGv.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_GIVE;
			sGv.m_lpBuf = (void *)uParam;
			sGv.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
			g_pClient->SendPackToServer((BYTE*)&sGv, sizeof(PLAYER_COMMAND));
		}
		break;
	case GOI_LOCKITEM:
		if (uParam)
		{
			KUiObjAtContRegion *pObj = (KUiObjAtContRegion *)uParam;
			if (pObj->Obj.uId > 0 && pObj->Obj.uId < MAX_ITEM)
			{
				KItem*	pItem = &Item[pObj->Obj.uId];
				if (pItem)
				{
					PLAYER_COMMAND	sLI;
					sLI.ProtocolType = c2s_playercommand;
					sLI.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_LOCKITEM;
					sLI.m_lpBuf = (void *)MAKELONG(pItem->GetID(), nParam);
					sLI.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
					g_pClient->SendPackToServer((BYTE*)&sLI, sLI.m_wLength + 1);
				}
			}
		}
		break;
	case GOI_SUPERSHOP:
		{
			PLAYER_COMMAND	sSS;
			sSS.ProtocolType = c2s_playercommand;
			sSS.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_SUPERSHOP;
			sSS.m_lpBuf = 0;
			sSS.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
			g_pClient->SendPackToServer((BYTE*)&sSS, sSS.m_wLength + 1);
		}
		break;
	case GOI_SHOW_OBJ_NAME:
		NpcSet.SetShowObjFlag(nParam);
		break;
	case GOI_PICKUP_OBJ:
		Player[CLIENT_PLAYER_INDEX].PickObjectNear();
		break;
	case GOI_OPTIMAL:
		Option.SetLow((int)uParam, (BOOL)nParam);
		break;
	case GOI_OFFLINE:
		{
			PLAYER_COMMAND	sOffL;
			sOffL.ProtocolType = c2s_playercommand;
			sOffL.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_OFFLINE;
			sOffL.m_lpBuf = 0;
			sOffL.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
			g_pClient->SendPackToServer((BYTE*)&sOffL, sOffL.m_wLength + 1);
		}
		break;
	case GOI_MASKFEATURE:
		{
			PLAYER_COMMAND	sMF;
			sMF.ProtocolType = c2s_playercommand;
			sMF.m_wMsgID = enumC2S_PLAYERCOMMAND_ID_MASKFEATURE;
			sMF.m_wLength = sizeof(SHOW_MSG_SYNC) - 1;
			sMF.m_lpBuf = 0;
			g_pClient->SendPackToServer((BYTE*)&sMF, sMF.m_wLength + 1);
		}
		break;
	case GOI_PLAYER_SETPRICE:
		if(uParam)
		{
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;	
			PLAYER_TRADE_SET_COMMAND sTrade;
			sTrade.ProtocolType = c2s_playertradeset;
			sTrade.m_dwID = Item[pObject1->Obj.uId].GetID();
			sTrade.m_nPrice = nParam;
			if (g_pClient)
				g_pClient->SendPackToServer((BYTE*)&sTrade, sizeof(PLAYER_TRADE_SET_COMMAND));
		}
	break;
	case GOI_PLAYER_DOTRADE:
		{
			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].GetMenuState() == PLAYER_MENU_STATE_TRADEOPEN)
				Player[CLIENT_PLAYER_INDEX].TradeApplyClose();

			PLAYER_TRADE_START_COMMAND sTrade;
			sTrade.ProtocolType = c2s_playertradestart;
			strcpy(sTrade.Name, (char *)uParam);
			if (g_pClient)
				g_pClient->SendPackToServer((BYTE*)&sTrade, sizeof(PLAYER_TRADE_START_COMMAND));
		}
		break;
	case GOI_PLAYER_BUYTRADE:
		{
			if (Player[CLIENT_PLAYER_INDEX].m_ItemList.IsLockOperation())
				return 0;
			KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;
			if (CGOG_PLAYERSELLITEM != pObject1->Obj.uGenre)
				break;			
			
			int nWidth, nHeight;
			ItemPos	Pos;
			
			nWidth = Item[pObject1->Obj.uId].GetWidth();
			nHeight = Item[pObject1->Obj.uId].GetHeight();
			if (!Player[CLIENT_PLAYER_INDEX].m_ItemList.SearchPosition(nWidth, nHeight, &Pos))
			{
				nRet = 0;
				break;
			}
			if (Pos.nPlace != pos_equiproom)
			{
				nRet = 0;
				
				KSystemMessage	sMsg;
				
				strcpy(sMsg.szMessage, MSG_SHOP_NO_ROOM);
				sMsg.eType = SMT_SYSTEM;
				sMsg.byConfirmType = SMCT_CLICK;
				sMsg.byPriority = 1;
				sMsg.byParamSize = 0;
				CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
				break;
			}
			
			if (Player[CLIENT_PLAYER_INDEX].m_ItemList.GetEquipmentMoney() < Item[pObject1->Obj.uId].GetTradePrice())
			{
				nRet = 0;
				KSystemMessage	sMsg;
				
				strcpy(sMsg.szMessage, MSG_SHOP_NO_MONEY);
				sMsg.eType = SMT_SYSTEM;
				sMsg.byConfirmType = SMCT_CLICK;
				sMsg.byPriority = 1;
				sMsg.byParamSize = 0;
				CoreDataChanged(GDCNI_SYSTEM_MESSAGE, (unsigned int)&sMsg, 0);
				break;
				
			}		
			PLAYER_TRADE_BUY_COMMAND sBuy;
			sBuy.ProtocolType = c2s_playertradebuy;
			sBuy.m_nIdx = g_cSellItem.FindIdx(pObject1->Obj.uId);
			sBuy.m_dwID = g_cSellItem.GetSellId();
			sBuy.m_Place = pos_equiproom;
			sBuy.m_X = Pos.nX;
			sBuy.m_Y = Pos.nY;
			if (g_pClient)
				g_pClient->SendPackToServer((BYTE*)&sBuy, sizeof(PLAYER_TRADE_BUY_COMMAND));
			Player[CLIENT_PLAYER_INDEX].m_ItemList.LockOperation();
			nRet = 1;
		}
		break;

	case GOI_PLAYER_RLTRADE:
		{
			g_cSellItem.Release();
			BYTE btProtocol = c2s_playertradeviewend;
			g_pClient->SendPackToServer((BYTE*)&btProtocol, sizeof(BYTE));
		}
		break;
	case GOI_LOCKMOVE:
		PLAYER_LOCKMOVE	LockMove;
		LockMove.ProtocolType = c2s_lockmove;
		LockMove.m_LockMove.bLock = (BOOL)uParam;
		LockMove.m_LockMove.nPlace = (int)nParam;
		g_pClient->SendPackToServer((BYTE*)&LockMove, sizeof(PLAYER_LOCKMOVE));
		break;
	case GOI_UNLOCK:
		if(uParam)
		{
			C2S_BUF_COMMAND sUL;
			sUL.ProtocolType = c2s_unlockcommand;
			strcpy(sUL.szBuf, (char*)uParam);
			g_pClient->SendPackToServer((BYTE*)&sUL, sizeof(C2S_BUF_COMMAND));
		}
		break;
	case GOI_CHATROOM:
		if(uParam)
		{
			PLAYER_CHATROOM_DECISION_COMMAND	sRoom;
			sRoom.ProtocolType = c2s_chatroomdecision;
			sRoom.m_nDecision = (int)uParam;
			sRoom.m_bRoomList = (BOOL)nParam;
			g_pClient->SendPackToServer((BYTE*)&sRoom, sizeof(PLAYER_CHATROOM_DECISION_COMMAND));
		}
		break;
	
	case GOI_CP_SET_IMAGE_PLAYER:
		SendClientCPSetImageCmd(uParam);
		break;
	default:
		nRet = 0;
		break;
	}
	return nRet;
}

//--------------------------------------------------------------------------
//	功能：发送输入设备的输入操作消息
//--------------------------------------------------------------------------
void KCoreShell::ProcessInput(unsigned int uMsg, unsigned int uParam, int nParam)
{
	Player[CLIENT_PLAYER_INDEX].ProcessInputMsg(uMsg, uParam, nParam);
}

int KCoreShell::GetTargetNPC()
{
	return Player[CLIENT_PLAYER_INDEX].GetTargetNpc();
}

int KCoreShell::FindSelectNPC(int x, int y, int nRelation, bool bSelect, void* pReturn, int& nKind, bool bFind)
{
	Player[CLIENT_PLAYER_INDEX].FindSelectNpc(x, y, nRelation);
	int nT = Player[CLIENT_PLAYER_INDEX].GetTargetNpc();

	if (!bSelect)
		Player[CLIENT_PLAYER_INDEX].SetTargetNpc(0);

	if (nT > 0)
	{
		if (pReturn)
		{
			if (bFind && (Npc[nT].Name[0]==0 && (Npc[nT].m_nTeamServerID == -1 || 
				Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_nTeamServerID != Npc[nT].m_nTeamServerID)))
				return false;
			KUiPlayerItem* p = (KUiPlayerItem*)pReturn;
			strncpy(p->Name, Npc[nT].Name, 32);
			p->nIndex = Npc[nT].m_Index;
			p->uId = Npc[nT].m_dwID;
			p->nData = Npc[nT].GetMenuState();
			p->nTeamID = Npc[nT].m_nTeamServerID;
		}
		nKind = Npc[nT].m_Kind;
		return true;
	}
	return false;
}

int KCoreShell::FindSelectObject(int x, int y, bool bSelect, int& nObjectIdx, int& nKind)
{
	Player[CLIENT_PLAYER_INDEX].FindSelectObject(x, y);
	int nT = Player[CLIENT_PLAYER_INDEX].GetTargetObj();
	
	if (!bSelect)
		Player[CLIENT_PLAYER_INDEX].SetTargetObj(0);

	if (nT > 0)
	{
		nObjectIdx = nT;
		nKind = Object[nT].m_nKind;
		return true;
	}
	return false;
}


int KCoreShell::FindSpecialNPC(char* Name, void* pReturn, int& nKind)
{
	if (Name == NULL || Name[0] == 0)
		return false;
	for (int nT = 0; nT < MAX_NPC; nT++)
	{
		if	(strcmp(Npc[nT].Name, Name) == 0)
		{
			if (pReturn)
			{
				KUiPlayerItem* p = (KUiPlayerItem*)pReturn;
				strncpy(p->Name, Npc[nT].Name, 32);
				p->nIndex = Npc[nT].m_Index;
				p->uId = Npc[nT].m_dwID;
				p->nData = Npc[nT].GetMenuState();
				p->nTeamID = Npc[nT].m_nTeamServerID;
			}
			nKind = Npc[nT].m_Kind;
			return true;
		}
	}
	return false;
}

int KCoreShell::ChatSpecialPlayer(void* pPlayer, const char* pMsgBuff, unsigned short nMsgLength)
{
	KUiPlayerItem* p = (KUiPlayerItem*)pPlayer;
	if (p)
	{
		if (p->nIndex >= 0 && p->nIndex < MAX_NPC)
		{
			int nTalker = p->nIndex;
			if (Npc[nTalker].m_Kind == kind_player &&
				Npc[nTalker].m_dwID == p->uId)
			{
				Npc[nTalker].SetChatInfo(p->Name, pMsgBuff, nMsgLength);
				return true;
			}
		}
	}

	return false;
}

void KCoreShell::ApplyAddTeam(void* pPlayer)
{
	KUiPlayerItem* p = (KUiPlayerItem*)pPlayer;
	if (p)
	{
		if (p->nIndex >= 0 && p->nIndex < MAX_NPC && !Player[CLIENT_PLAYER_INDEX].CheckTrading())
		{
			Player[CLIENT_PLAYER_INDEX].ApplyAddTeam(p->nIndex);
		}
	}
}

void KCoreShell::TradeApplyStart(void* pPlayer, bool bFolkGame)
{
	KUiPlayerItem* p = (KUiPlayerItem*)pPlayer;
	if (p)
	{
		if (p->nIndex >= 0 && p->nIndex < MAX_NPC && !Player[CLIENT_PLAYER_INDEX].CheckTrading())
		{
			Player[CLIENT_PLAYER_INDEX].TradeApplyStart(p->nIndex, bFolkGame);
		}
	}
}

void KCoreShell::JoinTongReply(void* pPlayer)
{
	KUiPlayerItem* p = (KUiPlayerItem*)pPlayer;
	if (p)
	{
		if (p->nIndex >= 0 && p->nIndex < MAX_NPC && !Player[CLIENT_PLAYER_INDEX].CheckTrading())
		{
			Player[CLIENT_PLAYER_INDEX].m_cTong.JoinTongReply(p->nIndex);
		}
	}
}

int KCoreShell::UseSkill(int x, int y, int nSkillID)
{
	if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
		return 0;
	
	int nX = x;
	int nY = y;
	int nZ = 0;
	g_ScenePlace.ViewPortCoordToSpaceCoord(nX, nY, nZ);
	int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	if (Npc[nIndex].IsCanInput())
	{
		int nIdx = 0;
		
		nIdx = Npc[nIndex].m_SkillList.FindSame(nSkillID);
		Npc[nIndex].SetActiveSkill(nIdx);
	}
	else
	{
		return 0;
	}

	if (Npc[nIndex].m_ActiveSkillID > 0)
	{
		ISkill * pISkill =  g_SkillManager.GetSkill(Npc[nIndex].m_ActiveSkillID, 1);
		if (!pISkill) 
            return 0;

		if (pISkill->IsAura())
			return 0;

		int nAttackRange = pISkill->GetAttackRadius();

		int nTargetIdx = 0;
		//按照Object / Enemy / Ally 的优先级找到需要打的对象id
		
		if (pISkill->IsTargetAlly())
		{
			Player[CLIENT_PLAYER_INDEX].FindSelectNpc(x, y, relation_ally);
			if (Player[CLIENT_PLAYER_INDEX].GetTargetNpc())
			{
				nTargetIdx = Player[CLIENT_PLAYER_INDEX].GetTargetNpc();
			}
		}

		if (pISkill->IsTargetEnemy())
		{
			Player[CLIENT_PLAYER_INDEX].FindSelectNpc(x, y, relation_enemy);
			if (Player[CLIENT_PLAYER_INDEX].GetTargetNpc())
			{
				nTargetIdx = Player[CLIENT_PLAYER_INDEX].GetTargetNpc();
			}
		}

		if (pISkill->IsTargetObj())
		{
			Player[CLIENT_PLAYER_INDEX].FindSelectObject(x, y);
			if (Player[CLIENT_PLAYER_INDEX].GetTargetObj())
			{
				nTargetIdx = Player[CLIENT_PLAYER_INDEX].GetTargetObj();
			}
		}
		//如果技能必须指定对象，而当前位置无对象的话，直接退出
		if (pISkill->IsTargetOnly() && !nTargetIdx)
        {
			Npc[nIndex].m_nPeopleIdx = 0;
			Player[CLIENT_PLAYER_INDEX].SetTargetNpc(0);
			return 0;
		}
		
		if (nIndex == nTargetIdx)
		{
			Npc[nIndex].m_nPeopleIdx = 0;
			Player[CLIENT_PLAYER_INDEX].SetTargetNpc(0);
			return 0;
		}
		if ((!Npc[nIndex].m_SkillList.CanCast(Npc[nIndex].m_ActiveSkillID, SubWorld[Npc[nIndex].m_SubWorldIndex].m_dwCurrentTime))
			||
			(!Npc[nIndex].Cost(pISkill->GetSkillCostType(), pISkill->GetSkillCost(&Npc[nIndex]), TRUE))
			)
		{
			Npc[nIndex].m_nPeopleIdx = 0;
			Player[CLIENT_PLAYER_INDEX].SetTargetNpc(0);
			return 0;
		}

		//无对象，直接发坐标
		if (!nTargetIdx)
		{
			Npc[nIndex].SendCommand(do_skill, Npc[nIndex].m_ActiveSkillID, nX, nY);
			SendClientCmdSkill(Npc[nIndex].m_ActiveSkillID, nX, nY);
		}
		else
		{
			if (pISkill->IsTargetOnly())
			{
				if (NpcSet.GetDistance(nIndex , nTargetIdx) > nAttackRange)
				{
					Player[CLIENT_PLAYER_INDEX].SetTargetNpc(nTargetIdx);
					return 0;
				}
			}
			if (nIndex == nTargetIdx && pISkill->GetSkillStyle() == SKILL_SS_Missles) 
				return 0;

			if (NpcSet.GetDistance(nIndex , nTargetIdx) <= nAttackRange)
			{
				Npc[nIndex].SendCommand(do_skill, Npc[nIndex].m_ActiveSkillID, -1, nTargetIdx);	
				SendClientCmdSkill(Npc[nIndex].m_ActiveSkillID, -1, Npc[nTargetIdx].m_dwID);
			}
		}
	}
	Npc[nIndex].m_nPeopleIdx = 0;
	return 1;
}

int KCoreShell::LockSomeoneUseSkill(int nTargetIndex, int nSkillID)
{
	if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
		return 0;
	
	int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	if (nTargetIndex == nIndex)
		return 0;

	if (Npc[nIndex].IsCanInput())
	{
		int nIdx = 0;
		
		nIdx = Npc[nIndex].m_SkillList.FindSame(nSkillID);
		Npc[nIndex].SetActiveSkill(nIdx);
	}
	else
	{
		return 0;
	}

	int nRelation = NpcSet.GetRelation(nIndex, nTargetIndex);
	if (nRelation == relation_enemy)
	{
		Npc[nIndex].m_nPeopleIdx = nTargetIndex;
		return 1;
	}

	return 0;
}

int KCoreShell::LockSomeoneAction(int nTargetIndex)
{
	if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
		return 0;
	
	int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	if (nTargetIndex == nIndex)
		return 0;
	if (nTargetIndex <= 0 || nTargetIndex >= MAX_NPC)	//取消Lock
	{
		Npc[nIndex].m_nPeopleIdx = 0;
		return 1;
	}

	int nRelation = NpcSet.GetRelation(nIndex, nTargetIndex);
	if (nRelation != relation_enemy)
	{
		Npc[nIndex].m_nPeopleIdx = nTargetIndex;
		return 1;
	}

	return 0;
}

int KCoreShell::LockObjectAction(int nTargetIndex)
{
	if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
		return 0;
	
	int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	if (nTargetIndex <= 0)	//取消Lock
		Npc[nIndex].m_nObjectIdx = 0;
	else
		Npc[nIndex].m_nObjectIdx = nTargetIndex;

	return 1;
}

void KCoreShell::GotoWhere(int x, int y, int mode)
{

	if (Player[CLIENT_PLAYER_INDEX].CheckTrading())
		return;

	if (mode < 0 || mode > 2)
		return;

	if (Player[CLIENT_PLAYER_INDEX].m_nSendMoveFrames >= defMAX_PLAYER_SEND_MOVE_FRAME)
	{
		int bRun = false;

		if ((mode == 0 && Player[CLIENT_PLAYER_INDEX].m_RunStatus) ||
			mode == 2)
			bRun = true;

		int nX = x;
		int nY = y;
		int nZ = 0;

		g_ScenePlace.ViewPortCoordToSpaceCoord(nX, nY, nZ);
		int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

		if (!bRun)
		{
			Npc[nIndex].SendCommand(do_walk, nX, nY);
			// Send to Server
			SendClientCmdWalk(nX, nY);
		}
		else
		{
			Npc[nIndex].SendCommand(do_run, nX, nY);
			// Send to Server
			SendClientCmdRun(nX, nY);
		}
		Player[CLIENT_PLAYER_INDEX].m_nSendMoveFrames = 0;
	}
}

void KCoreShell::GetWhere(int x, int y, void* pInfo)
{
	KPosition3* p = (KPosition3*)pInfo;
	int nX = x;
	int nY = y;
	int nZ = 0;
	g_ScenePlace.ViewPortCoordToSpaceCoordNew(p->nX, nX, nY, nZ);
	p->nY = nX;
	p->nZ = nY;
}

void KCoreShell::Goto(int nDir, int mode)
{
	if (nDir < 0 || nDir > 63)
		return;

	if (mode < 0 || mode > 2)
		return;

	int bRun = false;

	if ((mode == 0 && Player[CLIENT_PLAYER_INDEX].m_RunStatus) ||
		mode == 2)
		bRun = true;

	int nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;
	int nSpeed;
	if (bRun)
		nSpeed = Npc[nIndex].m_CurrentRunSpeed;
	else
		nSpeed = Npc[nIndex].m_CurrentWalkSpeed;

	Player[CLIENT_PLAYER_INDEX].Walk(nDir, nSpeed);

	Player[CLIENT_PLAYER_INDEX].m_nSendMoveFrames = 0;
}

void KCoreShell::Turn(int nDir)
{
	if (nDir < 0 || nDir > 3)
		return;

	if (nDir == 0)
		Player[CLIENT_PLAYER_INDEX].TurnLeft();
	else if (nDir == 1)
		Player[CLIENT_PLAYER_INDEX].TurnRight();
	else
		Player[CLIENT_PLAYER_INDEX].TurnBack();
}

int KCoreShell::ThrowAwayItem()
{
	return Player[CLIENT_PLAYER_INDEX].ThrowAwayItem();
}

int KCoreShell::GetNPCRelation(int nIndex)
{
	return NpcSet.GetRelation(Player[CLIENT_PLAYER_INDEX].m_nIndex, nIndex);
}

BOOL KCoreShell::CheckMouse()
{
	if (Player[CLIENT_PLAYER_INDEX].m_ItemList.Hand())
		return TRUE;
	return FALSE;
}
//--------------------------------------------------------------------------
//	功能：绘制游戏对象
//--------------------------------------------------------------------------
void KCoreShell::DrawGameObj(unsigned int uObjGenre, unsigned int uId, int x, int y, int Width, int Height, int nParam)
{
	if (g_pRepresent)
		CoreDrawGameObj(uObjGenre, uId, x, y, Width, Height, nParam);
}

#include "../../Represent/iRepresent/iRepresentshell.h"

//--------------------------------------------------------------------------
//	功能：绘制游戏世界
//--------------------------------------------------------------------------
void KCoreShell::DrawGameSpace()
{
	if (g_pRepresent)
	{
		g_ScenePlace.Paint();
	}
}

//--------------------------------------------------------------------------
//	功能：设置绘图接口实例的指针
//--------------------------------------------------------------------------
void KCoreShell::SetRepresentShell(struct iRepresentShell* pRepresent)
{
	g_pRepresent = pRepresent;
	g_ScenePlace.RepresentShellReset();
	if (g_pAdjustColorTab && g_ulAdjustColorCount && g_pRepresent)
		g_pRepresent->SetAdjustColorList(g_pAdjustColorTab, g_ulAdjustColorCount);
}

void KCoreShell::SetMusicInterface(void* pMusicInterface)
{
	g_pMusic = (KMusic*)pMusicInterface;
	Option.SetMusicVolume(Option.GetMusicVolume());
}

//日常活动，core如果要寿终正寝则返回0，否则返回非0值
int KCoreShell::Breathe()
{
	g_SubWorldSet.MessageLoop();
	g_SubWorldSet.MainLoop();
	g_ScenePlace.Breathe();
	return true;
}

int KCoreShell::GetProtocolSize(BYTE byProtocol)
{
	if (byProtocol <= s2c_clientbegin || byProtocol >= s2c_end)
		return -1;
	return g_nProtocolSize[byProtocol - s2c_clientbegin - 1];
}

DWORD KCoreShell::GetPing()
{
	return g_SubWorldSet.GetPing();
}

void KCoreShell::SetRepresentAreaSize(int nWidth, int nHeight)
{
	g_ScenePlace.SetRepresentAreaSize(nWidth, nHeight);
}

void KCoreShell::SetClient(LPVOID pClient)
{
	g_SetClient(pClient);
}

void KCoreShell::SendNewDataToServer(void* pData, int nLength)
{
	if (g_pClient)
		g_pClient->SendPackToServer(pData, nLength);
}

//与地图相关的操作
int	KCoreShell::SceneMapOperation(unsigned int uOper, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uOper)
	{
	case GSMOI_SCENE_TIME_INFO:
		if (uParam)
		{
			KUiSceneTimeInfo* pInfo = (KUiSceneTimeInfo*)uParam;
			g_ScenePlace.GetSceneNameAndFocus(pInfo->szSceneName, pInfo->nSceneId,
				pInfo->nScenePos0, pInfo->nScenePos1);
			pInfo->nGameSpaceTime = (SubWorld[0].m_dwCurrentTime / 100) % 1440;
		}
		break;
	case GSMOI_SCENE_TIME_INFO_OFTEN:
		if (uParam)
		{
			KUiSceneTimeInfoOften* pInfo = (KUiSceneTimeInfoOften*)uParam;
			g_ScenePlace.GetSceneNameAndFocusOften(pInfo->szSceneName, pInfo->nSceneId,
				pInfo->nScenePos0, pInfo->nScenePos1);
			pInfo->nGameSpaceTime = (SubWorld[0].m_dwCurrentTime / 100) % 1440;
		}
		break;
	case GSMOI_SCENE_MAP_INFO:
		nRet = g_ScenePlace.GetMapInfo((KSceneMapInfo*)uParam);
		break;
	case GSMOI_IS_SCENE_MAP_SHOWING:
		g_ScenePlace.SetMapParam(uParam, nParam);
		break;
	case GSMOI_PAINT_SCENE_MAP:
		g_ScenePlace.PaintMap(uParam, nParam);
		break;
	case GSMOI_SCENE_MAP_FOCUS_OFFSET:
		g_ScenePlace.SetMapFocusPositionOffset((int)uParam, nParam);
		break;
	case GSMOI_SCENE_FOLLOW_WITH_MAP:	//设置场景是否随着地图的移动而移动
		g_ScenePlace.FollowMapMove(nParam);
		break;
	case GSMOI_IS_SCENE_DIRECT_MAP:
		nRet = g_ScenePlace.OnDirectMap((int)uParam, nParam);
		break;
	case GSMOI_IS_SCENE_DO_DIRECT_MAP:
		g_ScenePlace.DoDirectMap((int)uParam, nParam);
		break;
	}
	return nRet;
}

//与帮会相关的操作, uOper的取值来自 GAME_TONG_OPERATION_INDEX
int	KCoreShell::TongOperation(unsigned int uOper, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uOper)
	{
	case GTOI_TONG_CREATE:		//创建帮会
		//uParam = (const char*) pszTongName 帮会的名字
		//nParam = (NPCCAMP)enFaction 帮会阵营
		nRet = Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyCreateTong(nParam, (char *)uParam);
		break;
	case GTOI_TONG_FLAG:		//查询某人的招人开关
		//uParam = (KUiPlayerItem*) 要查谁
		//Return = (int)(bool)		是否开着的招人开关
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			nRet = Npc[pItem->nIndex].m_bTongFlag;
		}
		else
			nRet = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_bTongFlag;
		break;
	case GTOI_TONG_FIGURE:		//查询某人的招人开关
		//uParam = (KUiPlayerItem*) 要查谁
		//Return = (int)(bool)		是否开着的招人开关
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			nRet = Npc[pItem->nIndex].m_nFigure;
		}
		else
			nRet = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_nFigure;
		break;
	case GTOI_TONG_VICEROY:		//查询某人的招人开关
		//uParam = (KUiPlayerItem*) 要查谁
		//Return = (int)(bool)		是否开着的招人开关
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			if(Npc[pItem->nIndex].m_nFigure == enumTONG_FIGURE_MASTER)
			{
				nRet =0;
			}
		}
		else
		{
			if(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_nFigure == enumTONG_FIGURE_MASTER)
			{
				nRet = 0;
			}
		}
		break;
	case GTOI_TONG_RECRUIT:     //招人开关
		if (uParam)
			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyRecruit(1);
		else
			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyRecruit(0);
		//uParam = (int)(bool)bRecruit 是否愿意招人
		break;
	case GTOI_TONG_ACTION:         //对帮内成员做的动作，或自己与帮会的关系的改变
		//uParam = (KTongOperationParam*) pOperParam 动作时的参数
		//nParam = (KTongMemberItem*) pMember 指出了操作（帮会成员）对象，
		{
    		KTongOperationParam *Oper = (KTongOperationParam *)uParam;
		    KTongMemberItem *TargetInfo = (KTongMemberItem *)nParam;
		    KUiPlayerItem TargetPlayer;
		    int nKind;

		    switch(Oper->eOper)
		    {
		    case TONG_ACTION_DISMISS:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyKick(Oper->nData[0], Oper->nData[1], TargetInfo->Name);
    			break;
		    case TONG_ACTION_ASSIGN:
    			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInstate(Oper->nData[0], Oper->nData[1], Oper->nData[2], Oper->nData[3], TargetInfo->Name);
			    break;
		    case TONG_ACTION_DEMISE:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyChangeMaster(Oper->nData[0], Oper->nData[1], TargetInfo->Name);
    			break;
		    case TONG_ACTION_LEAVE:
    			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyLeave();
    			break;
		    case TONG_ACTION_RECRUIT:
    			break;
		    case TONG_ACTION_APPLY:
				if(!FindSpecialNPC(TargetInfo->Name, &TargetPlayer, nKind))
				{
    				break;
				}
    			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyAddTong(TargetPlayer.uId);
			    break;
			case TONG_ACTION_CHANGE_AGNAME:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyChangeAgname(Oper->nData[0], Oper->nData[1], TargetInfo->Name, Oper->szData);
				break;
			case TONG_ACTION_CHANGE_MALE_AGNAME:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyChangeSexAgname(Oper->szData, OBJ_G_MALE);
				break;
			case TONG_ACTION_CHANGE_FEMALE_AGNAME:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyChangeSexAgname(Oper->szData, OBJ_G_FEMALE);
				break;
			case TONG_ACTION_CHANGE_CAMP_JUSTIE:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyTongChangeCamp(camp_justice);
				break;
			case TONG_ACTION_CHANGE_CAMP_EVIL:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyTongChangeCamp(camp_evil);
				break;
			case TONG_ACTION_CHANGE_CAMP_BALANCE:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyTongChangeCamp(camp_balance);
				break;
		    }
		}
		break;

	//对申请加入的答复
	//uPAram = (KUiPlayerItem *) pTarget   申请方
	//nParam : != 0  同意   ==0  拒绝
	case GTOI_TONG_JOIN:
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			Player[CLIENT_PLAYER_INDEX].m_cTong.AcceptMember(pItem->nIndex, g_FileName2Id(pItem->Name), nParam);
		}
		break;

	case GTOI_TONG_JOIN_REPLY:
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			Player[CLIENT_PLAYER_INDEX].m_cTong.ReplyMember(pItem->nIndex, g_FileName2Id(pItem->Name), nParam);
		}
		break;

	case GTOI_REQUEST_PLAYER_TONG:	//查询某人是哪帮会
		//uParam = (KUiPlayerItem*) 要查谁
		//nParam = (int)(bool)bReturnTongDetail 是否要返回那个帮会的信息
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			if (pItem->nIndex == Player[CLIENT_PLAYER_INDEX].m_nIndex)
			{
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_TONG_HEAD,
					pItem->nIndex, 0, 0);
			}
			else
			{
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_TONG_HEAD,
					pItem->nIndex, 0, 0);
			}
		}
		break;
	case GTOI_REQUEST_TONG_DATA:     //要求某个帮会的各种资料
	//uParam = (KUiGameObjectWithName*)pTong 要查询的帮会
	//KUiGameObjectWithName::szName 帮会的名字
	//KUiGameObjectWithName::nData 资料的种类，值取自枚举TONG_MEMBER_FIGURE
	//			列表的种类是enumTONG_FIGURE_MASTER的话代表要求的是帮会的资讯。
	//KUiGameObjectWithName::nParam 开始的索引
		if (uParam)
		{
			if (Player[CLIENT_PLAYER_INDEX].m_cTong.CheckIn() == 0)
				break;

			KUiGameObjectWithName	*pObj = (KUiGameObjectWithName*)uParam;
			char	szTongName[32];
			DWORD	dwTongNameID;

			szTongName[0] = 0;
			Player[CLIENT_PLAYER_INDEX].m_cTong.GetTongName(szTongName);
			if (!szTongName[0])
				break;

			// 只能查询自己帮会信息
			dwTongNameID = g_FileName2Id(pObj->szName);
			if (dwTongNameID != Player[CLIENT_PLAYER_INDEX].m_cTong.GetTongNameID())
				break;
	
			switch (pObj->nData)
			{
			case enumTONG_FIGURE_MASTER:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_TONG_HEAD,
					Player[CLIENT_PLAYER_INDEX].m_nIndex, 0, 0);
				break;
			case enumTONG_FIGURE_DIRECTOR:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_TONG_HEAD,
					Player[CLIENT_PLAYER_INDEX].m_nIndex,  pObj->nParam, defTONG_ONE_PAGE_MAX_NUM);
				break;
			case enumTONG_FIGURE_MANAGER:
				if (!Player[CLIENT_PLAYER_INDEX].m_cTong.CanGetManagerInfo(dwTongNameID))
					break;
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_MANAGER,
					dwTongNameID, pObj->nParam, defTONG_ONE_PAGE_MAX_NUM);
				break;
			case enumTONG_FIGURE_MEMBER:
				if (!Player[CLIENT_PLAYER_INDEX].m_cTong.CanGetMemberInfo(dwTongNameID))
					break;
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyInfo(
					enumTONG_APPLY_INFO_ID_MEMBER,
					dwTongNameID, pObj->nParam, defTONG_ONE_PAGE_MAX_NUM);
				break;
			}
		}
		break;
	case GTOI_TONG_MONEY_ACTION:
		if (uParam)
		{
			KUiPlayerItem	*pItem = (KUiPlayerItem*)uParam;
			switch(nParam)
			{
			case TONG_ACTION_SAVE:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplySaveMoney(pItem->nData);
				break;
			case TONG_ACTION_GET:
				Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyGetMoney(pItem->nData);
				break;
			}
		}
		break;
	case GTOI_TONG_CHANGE_CREDIT:
		if (uParam)
		{
			KTongInfo	*pItem = (KTongInfo*)uParam;
			Player[CLIENT_PLAYER_INDEX].m_cTong.ApplyChangeJiyu(pItem->szName, pItem->nTongJiyuParam, pItem->szTongJiyuNotify);
		}
		break;
	case GTOI_TOTAL_EFF:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTong.m_dwTotalEff;
		break;
	case GTOI_TONG_PARAM:
			KTongParam	*pParam = (KTongParam*)uParam;
			*pParam = TongData.m_sTongParam;
		break;		
	}
	return nRet;
}

//与组队相关的操作，uOper的取值来自 GAME_TEAM_OPERATION_INDEX
int KCoreShell::TeamOperation(unsigned int uOper, unsigned int uParam, int nParam)
{
	int nRet = 0;
	switch(uOper)
	{
	case TEAM_OI_GD_INFO:		//主角所在的队伍信息
		if (uParam)
		{
			KUiPlayerTeam* pTeam = (KUiPlayerTeam*)uParam;
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTeam.GetInfo(pTeam);
		}
		break;
	case TEAM_OI_GD_FIGURE:
		if (Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFlag)
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTeam.m_nFigure;
		else
			nRet = -1;
		break;
	case TEAM_OI_GD_MEMBER_LIST://获取主角所在队伍成员列表
		nRet = g_Team[0].GetMemberInfo((KUiPlayerItem *)uParam, nParam);
		break;
	case TEAM_OI_GD_REFUSE_INVITE_STATUS://获取拒绝邀请的状态
		nRet = Player[CLIENT_PLAYER_INDEX].m_cTeam.GetAutoRefuseState();
		break;
	case TEAM_OI_COLLECT_NEARBY_LIST://获取周围队伍的列表
		NpcSet.GetAroundOpenCaptain(Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_Camp);
		break;
	case TEAM_OI_APPLY:			//申请加入她人队伍
		if (uParam)
			Player[CLIENT_PLAYER_INDEX].ApplyAddTeam(((KUiTeamItem*)uParam)->Leader.nIndex);
		break;
	case TEAM_OI_CREATE:		//新组队伍
		Player[CLIENT_PLAYER_INDEX].ApplyCreateTeam();//(char*)uParam);
		break;
	case TEAM_OI_APPOINT:		//任命队长，只有队长调用才有效果
		Player[CLIENT_PLAYER_INDEX].ApplyTeamChangeCaptain(((KUiPlayerItem*)uParam)->uId);	
		break;
	case TEAM_OI_INVITE:		//邀请别人加入队伍，只有队长调用才有效果
		if (uParam)
		{
			Player[CLIENT_PLAYER_INDEX].TeamInviteAdd(((KUiPlayerItem*)uParam)->uId);
		}
		break;
	case TEAM_OI_KICK:			//踢除队里的一个队员，只有队长调用才有效果
		Player[CLIENT_PLAYER_INDEX].TeamKickMember(((KUiPlayerItem*)uParam)->uId);
		break;
	case TEAM_OI_LEAVE:			//离开队伍
		Player[CLIENT_PLAYER_INDEX].LeaveTeam();
		break;
	case TEAM_OI_CLOSE:			//关闭组队，只有队长调用才有效果
		Player[CLIENT_PLAYER_INDEX].ApplyTeamOpenClose(nParam);
		break;
	case TEAM_OI_PKFOLLOW:			//关闭组队，只有队长调用才有效果
		Player[CLIENT_PLAYER_INDEX].ApplyTeamPKFollow(nParam);
		break;
	case TEAM_OI_MODEPICK:			//关闭组队，只有队长调用才有效果
		Player[CLIENT_PLAYER_INDEX].ApplyTeamModePick(nParam);
		break;
	case TEAM_OI_REFUSE_INVITE:		//拒绝别人邀请自己加入队伍
		Player[CLIENT_PLAYER_INDEX].m_cTeam.SetAutoRefuseInvite(nParam);
		break;
	case TEAM_OI_APPLY_RESPONSE:		//批准他人加入队伍，只有队长调用才有效果
		if (uParam)
		{
			if (nParam)
			{
				Player[CLIENT_PLAYER_INDEX].AcceptTeamMember(((KUiPlayerItem*)uParam)->uId);
			}
			else
			{
				Player[CLIENT_PLAYER_INDEX].m_cTeam.DeleteOneFromApplyList(((KUiPlayerItem*)uParam)->uId);
//				Player[CLIENT_PLAYER_INDEX].m_cTeam.UpdateInterface();
			}
		}
		break;
	case TEAM_OI_INVITE_RESPONSE://对组队邀请的回复
		if (uParam)
			Player[CLIENT_PLAYER_INDEX].m_cTeam.ReplyInvite(((KUiPlayerItem*)uParam)->nIndex, nParam);
		break;
	case TEAM_OI_GETUID:		//任命队长，只有队长调用才有效果
			nRet = ((KUiPlayerItem*)uParam)->uId;
		break;
	}
	return nRet;
}


int KCoreShell::PAIOperation(unsigned int uOper, unsigned int uParam, int nParam, int nParam1)
{
	int nRet = 0;
	switch(uOper)
	{
	case GPI_SWITCH_ACTIVE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bIsActive = !Player[CLIENT_PLAYER_INDEX].m_cAI.m_bIsActive;
			Player[CLIENT_PLAYER_INDEX].m_cAI.SetSortEquipment(FALSE);
		break;
	case GPI_ISACTIVE:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cAI.m_bIsActive;
		break;
	case GPI_PRIORITY_USE_MOUSE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bPriorityUseMouse = uParam;
		break;
	case GPI_HOLDING_SPACEBAR:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bHoldSpaceBar = uParam;
		break;
	case GPI_FOLLOW_PEOPLE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFollowPeople = uParam;
		break;
	case GPI_FOLLOW_PEOPLE_NAME:
			strcpy(Player[CLIENT_PLAYER_INDEX].m_cAI.m_szFollowName, (char*)uParam);
		break;
	case GPI_FOLLOW_PEOPLE_RADIUS:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFollowRadius = uParam;
		break;
	case GPI_FIGHTNEAR_CHECK:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFightNear = uParam;
		break;
	case GPI_ATTACK_NPC_CHECK:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bAttackNpc = uParam;
		break;
	case GPI_AUTO_PARTY:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bAutoParty = uParam;
		break;
	case GPI_AUTO_INVITE:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bAutoInvite = uParam;
		break;
	case GPI_AUTO_INVITE_LIST:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bAutoInviteList = uParam;
		break;
	case GPI_SET_INVITE_LIST:	
			if (nParam >= 0 && nParam < MAX_AUTO_LIST)
				strcpy(Player[CLIENT_PLAYER_INDEX].m_cAI.m_szPartyList[nParam], (char*)uParam);
		break;
	case GPI_AUTO_ACCEPT:	
			Player[CLIENT_PLAYER_INDEX].m_cTeam.m_bAutoAccept = uParam;
		break;
	case GPI_AUTO_ACCEPT_WITH_NAME:	
			Player[CLIENT_PLAYER_INDEX].m_cTeam.m_bAutoAcceptWithName = uParam;
		break;
	case GPI_FIGHT_AUTO:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nDistanceAuto = uParam;
		break;
	case GPI_RANGER_AUTO:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nRadiusAuto = uParam;
		break;
	case GPI_FIGHT_USE_SB:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFightUseSB = uParam;
		break;
	case GPI_SUPPORT_SKILLS:
			if (uParam >= 0 && uParam < MAX_AUTO_STATESKILL && nParam < MAX_SKILL)
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nSupportSkills[uParam] = nParam;
		break;
	case GPI_FIGHT_SKILLS:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFightSkill = uParam;
		break;
	case GPI_FIGHTBACK:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFightBack = uParam;
		break;
	case GPI_FIGHTBOSS:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFightBoss = uParam;
		break;
	case GPI_AURA_SKILLS1:
			if (nParam >= 0 && nParam < MAX_SKILL)
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nActiveAuraID1 = uParam;
		break;
	case GPI_AURA_SKILLS2:
			if (nParam >= 0 && nParam < MAX_SKILL)
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nActiveAuraID2 = uParam;
		break;
	case GPI_OVER_TARGET:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bOverTarget = uParam;
		break;
	case GPI_FOLLOW_TARGET:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFollowTarget = uParam;
		break;
	case GPI_SORT_EQUIPMENT:
			if (!Player[CLIENT_PLAYER_INDEX].m_cAI.m_bSortEquipment)
				Player[CLIENT_PLAYER_INDEX].m_cAI.SetSortEquipment(TRUE);
		break;
	case GPI_FILTER_EQUIPMENT:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFilterEquipment = uParam;
		break;
	case GPI_PICK_FIGHTNONE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bPickFightNone = uParam;
		break;
	case GPI_PICK_MONEY:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bPickMoney = uParam;
		break;
	case GPI_PICK_NOTEQUIP:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bPickNotEquip = uParam;
		break;
	case GPI_PICK_EQUIP:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bPickEquip = uParam;
		break;
	case GPI_FOLLOW_PICK:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bFollowPick = uParam;
		break;
	case GPI_PICK_KIND:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nPickEquipKind = uParam;
		break;
	case GPI_AUTO_FILTERMAGIC:
		{
			int nIndex = (int)g_MagicDesc.String2MagicID((char*)uParam);
			if (nIndex >= magic_skill_begin && nIndex < magic_normal_end)
			{
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFilterMagic[nParam1][0] = nIndex;
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFilterMagic[nParam1][1] = nParam;
			}
			else
			{
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFilterMagic[nParam1][0] = 0;
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nFilterMagic[nParam1][1] = 0;
			}
		}
		break;
	case GPI_SAVE_TRASH:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bSaveTrash = uParam;
		break;
	case GPI_SAVE_TRASH_PRICE:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nSaveTrashPrice = uParam;
		break;
	case GPI_SAVE_JEWELRY:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bSaveJewelry = uParam;
		break;
	case GPI_EAT_LIFE:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bEatLife = uParam;
		break;
	case GPI_EAT_MANA:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bEatMana = uParam;
		break;
	case GPI_LIFE_VALUE:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nLifeAutoV = uParam;
		break;
	case GPI_LIFE_PERCENT:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nLifeAutoP = uParam;
		break;
	case GPI_LIFE_TIME:	
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_uLifeTimeUse = uParam;
		break;
	case GPI_MANA_VALUE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nManaAutoV = uParam;
		break;
	case GPI_MANA_PERCENT:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nManaAutoP = uParam;
		break;
	case GPI_MANA_TIME:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_uManaTimeUse = uParam;
		break;
	case GPI_TP_LIFE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPLife = uParam;
		break;
	case GPI_TP_LIFEV:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nTPLifeV = uParam;
		break;
	case GPI_TP_MANA:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPMana = uParam;
		break;
	case GPI_TP_MANAV:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nTPManaV = uParam;
		break;
	case GPI_TP_NOT_MEDICINEBLOOD:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPNotMedicineBlood = uParam;
		break;
	case GPI_TP_NOT_MEDICINEMANA:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPNotMedicineMana = uParam;
		break;
	case GPI_TP_HIGHTMONEY:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPHightMoney = uParam;
		break;
	case GPI_TP_HIGHTMONEYV:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nTPHightMoneyV = uParam;
		break;
	case GPI_TP_DAMAGEEQUIP:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPDamageEquip = uParam;
		break;
	case GPI_TP_DAMAGEEQUIPV:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nTPDamageEquipV = uParam;
		break;
	case GPI_TP_NOT_EQUIPMENT:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bTPNotEquipment = uParam;
		break;
	case GPI_TP_NOT_EQUIPMENTV:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nTPNotEquipmentV = uParam;
		break;
	case GPI_ANTI_POISON:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bAntiPoison = uParam;
		break;
	case GPI_ENCHASEEXP:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bEnchaseExp = uParam;
		break;
	case GPI_LIFEREPLENISH:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bLifeReplenish = uParam;
		break;
	case GPI_LIFEREPLENISHP:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nLifeReplenishP = uParam;
		break;
	case GPI_INVENTORYMONEY:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bInventoryMoney = uParam;
		break;
	case GPI_INVENTORYITEM:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bInventoryItem = uParam;
		break;
	case GPI_RETURNPORTAL:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bReturnPortal = uParam;
		break;
	case GPI_RETURNPORTALSEC:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nReturnPortalSec = uParam;
		break;
	case GPI_OPEN_MEDICINE:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bOpenMedicine = uParam;					
		break;
	case GPI_AUTO_REPAIR:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bRepairEquip = uParam;
		break;
	case GPI_AUTO_MOVEMPS:
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_bMoveMps = uParam;
		break;
	case GPI_AUTO_MOVEMPSID:
		if (uParam >= 0 && uParam < MAX_AUTO_LIST)
		{
			if(uParam == 0)
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nMoveCount = 0;

			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nMoveMps[uParam][0] = nParam;
			if(nParam)
				Player[CLIENT_PLAYER_INDEX].m_cAI.m_nMoveCount++;
		}
		break;
	case GPI_AUTO_MOVEMPSX:
		if (uParam >= 0 && uParam < MAX_AUTO_LIST)
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nMoveMps[uParam][1] = nParam;
		break;
	case GPI_AUTO_MOVEMPSY:
		if (uParam >= 0 && uParam < MAX_AUTO_LIST)
			Player[CLIENT_PLAYER_INDEX].m_cAI.m_nMoveMps[uParam][2] = nParam;
		break;
	}
	return nRet;
}

void KCoreShell::DirectFindPos(unsigned int uParam, int nParam, BOOL bSync, BOOL bPaintLine)
{
	g_ScenePlace.DirectFindPos(uParam, nParam, bSync, bPaintLine);
}

BOOL KCoreShell::GetPaintMode()
{
	return g_ScenePlace.bPaintMode;
}

void KCoreShell::SetPaintMode(BOOL bFlag)
{
	g_ScenePlace.DirectFindPos(0, 0, FALSE, FALSE);
	g_ScenePlace.bPaintMode = bFlag;
}

BOOL KCoreShell::GetFlagMode()
{
	return g_ScenePlace.bFlagMode;
}

void KCoreShell::SetFlagMode(BOOL bFlag)
{
	g_ScenePlace.bFlagMode = bFlag;
}

void KCoreShell::GetSizeItem(unsigned int uItemId, int *nW, int *nH)
{
	if(nW && nH)
	{
		*nW = Item[uItemId].GetWidth();
		*nH = Item[uItemId].GetHeight();
	}
}

int KCoreShell::GetExtPoint()
{
	return  Player[CLIENT_PLAYER_INDEX].GetExtPoint();
}

int KCoreShell::GetNatureItem(unsigned int uItemId, unsigned int uGenre /* = CGOG_ITEM*/)
{
	int nIndex = uItemId;
	KItem*	pItem = NULL;
	if (uGenre == CGOG_ITEM)
		return Item[nIndex].GetNature();
	else if (uGenre == CGOG_NPCSELLITEM)
	{
		int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop];
		if (nBuyIdx != -1)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, uItemId);
			if (nIndex >= 0)
			{
				pItem = BuySell.GetItem(nIndex);
				return pItem->GetNature();
			}
		}
	}
	return Item[nIndex].GetNature();
}

int KCoreShell::GetGenreItem(unsigned int uItemId, unsigned int uGenre /* = CGOG_ITEM*/)
{
	int nIndex = uItemId;
	KItem*	pItem = NULL;
	if (uGenre == CGOG_ITEM)
		return Item[nIndex].GetGenre();
	else if (uGenre == CGOG_NPCSELLITEM)
	{
		int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop];
		if (nBuyIdx != -1)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, uItemId);
			if (nIndex >= 0)
			{
				pItem = BuySell.GetItem(nIndex);
				return pItem->GetGenre();
			}
		}
	}
	return Item[nIndex].GetGenre();
}

int KCoreShell::IsDamage(unsigned int uItemId)
{
	if(Item[uItemId].GetDurability() > 0)
		return Item[uItemId].GetDurability() < Item[uItemId].GetMaxDurability();

	return FALSE;
}

BOOL KCoreShell::GetLockState()
{
	return Player[CLIENT_PLAYER_INDEX].GetLockState();
}

int KCoreShell::GetStackNum(unsigned int uItemId)
{
	return Item[uItemId].GetStackNum();
}

BOOL KCoreShell::GetSkillData(int nSkillId, int *nLevel)
{
	int nList = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.FindSame(nSkillId);
	if (nList > 0)
	{
		if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetLevel(nSkillId) > 0)
		{
			*nLevel = Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetCurrentLevel(nSkillId);
			if (Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetAddLevelIndex(nList) > 0)
				return TRUE;
			else
				return FALSE;
		}
	}
	return FALSE;
}

void KCoreShell::GetSkillName(int nSkillId, char* szSkillName)
{
	if (nSkillId)
	{
		ISkill * pISkill = g_SkillManager.GetSkill(nSkillId, 1);
		strcpy(szSkillName, pISkill->GetSkillName());
	}
}

int KCoreShell::GetTradePrice(unsigned int uItemId)
{
	return Item[uItemId].GetTradePrice();
}

int KCoreShell::GetTradeState(int nIndex, int uId)
{
	if (nIndex || uId)
	{
		if (nIndex)
			return Npc[nIndex].m_PTrade.nTrade;
		else
		{
			int nIndex = NpcSet.SearchID(uId);
			return Npc[nIndex].m_PTrade.nTrade;
		}
	}
	else
		return Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_PTrade.nTrade;

}

int KCoreShell::GetFightState()
{
	KNpc*	pNpc = &Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex];
	return pNpc->m_FightMode;
}

void KCoreShell::BreakItem(unsigned int uParam, int nParam, BOOL bIsBreakAll)
{
		KUiObjAtContRegion* pObject1 = (KUiObjAtContRegion*)uParam;
		if (CGOG_ITEM != pObject1->Obj.uGenre)
			return;
		//放下去的东西不为空，所以是卖东西
		int nIdx = pObject1->Obj.uId;	//Player[CLIENT_PLAYER_INDEX].m_ItemList.Hand();
		if (nIdx > 0 && nIdx < MAX_ITEM)
		{
			if (nParam <= 0)
			{
				SendClientCmdBreak(Item[nIdx].GetID(), nParam, FALSE);
			}
			else
			{
				if (Item[nIdx].GetStackNum() > nParam)
					SendClientCmdBreak(Item[nIdx].GetID(), nParam, bIsBreakAll);
				else
					return;
			}
		}
}

DWORD KCoreShell::GetdwIDPlayer(unsigned int uId)
{
	int nIndex = 0;
	if (uId)
		nIndex = NpcSet.SearchID(uId);
	else
		nIndex = Player[CLIENT_PLAYER_INDEX].m_nIndex;

	if (nIndex)
		return Npc[nIndex].m_dwID;	

	return 0;
}

int KCoreShell::GetOwnValue(int nMoneyUnit)
{
	int nRet = 0;
	switch (nMoneyUnit)
	{
		case moneyunit_money:
			nRet = Player[CLIENT_PLAYER_INDEX].m_ItemList.GetMoney(room_equipment);
			break;
		case moneyunit_extpoint:
			nRet = Player[CLIENT_PLAYER_INDEX].GetExtPoint();
			break;
		case moneyunit_fuyuan:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_FUYUAN);
			break;
		case moneyunit_repute:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_REPUTE);
			break;
		case moneyunit_accum:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_ACCUM);
			break;
		case moneyunit_honor:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_HONOR);
			break;
		case moneyunit_respect:
			nRet = Player[CLIENT_PLAYER_INDEX].m_cTask.GetSaveVal(TASKVALUE_STATTASK_RESPECT);
			break;
		default:
			break;
	}
	return nRet;
}

int KCoreShell::GetDataSuperShop(int nSaleId, unsigned int uParam, int nParam)
{
	int nRet = 0;
	int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[nSaleId];
	if (nBuyIdx == -1)
		return 0;
	if (nBuyIdx >= BuySell.GetHeight())
		return 0;
	Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop = nSaleId;
	int nIndex = 0;
	int nCount = 0;
	if (uParam)
	{
		int nPage = 0;
		KUiObjAtContRegion* pInfo = (KUiObjAtContRegion *)uParam;
		for (int i = 0; i < BuySell.GetWidth(); i++)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, i);
			KItem* pItem = BuySell.GetItem(nIndex);
			
			if (nIndex >= 0 && pItem)
			{
				pInfo->Obj.uGenre = CGOG_NPCSELLITEM;
				pInfo->Obj.uId = i;
				pInfo->Region.h = 0;
				pInfo->Region.v = 0;
				pInfo->Region.Width = pItem->GetWidth();
				pInfo->Region.Height = pItem->GetHeight();
				pInfo->nContainer = nPage;
				nCount++;
				pInfo++;
				if (nCount %15 == 0)
					nPage ++;
			}
		}			
		nRet = nPage;
	}
	else
	{
		for (int i = 0; i < BuySell.GetWidth(); i++)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, i);
			KItem* pItem = BuySell.GetItem(nIndex);
			
			if (nIndex >= 0 && pItem)
			{
				nCount++;
			}
		}
		nRet = nCount;
	}
	return nRet;
}


int KCoreShell::GetDataDynamicShop(int nSaleId, unsigned int uParam, int nParam)
{
	int nRet = 0;
	int	nBuyIdx = Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nShopIdx[nSaleId];
	if (nBuyIdx == -1)
		return 0;
	if (nBuyIdx >= BuySell.GetHeight())
		return 0;
	Player[CLIENT_PLAYER_INDEX].m_BuyInfo.m_nCurShop = nSaleId;
	int nIndex = 0;
	int nCount = 0;
	if (uParam)
	{
		int nPage = 0;
		KUiObjAtContRegion* pInfo = (KUiObjAtContRegion *)uParam;
		if (!BuySell.m_pSShopRoom)
			return 0;
		BuySell.m_pSShopRoom->Clear();
		for (int i = 0; i < BuySell.GetWidth(); i++)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, i);
			KItem* pItem = BuySell.GetItem(nIndex);
			
			if (nIndex >= 0 && pItem)
			{
				pInfo->Obj.uGenre = CGOG_NPCSELLITEM;
				pInfo->Obj.uId = i;
				POINT	Pos;
				if (BuySell.m_pSShopRoom->FindRoom(pItem->GetWidth(), pItem->GetHeight(), &Pos))
				{
					BuySell.m_pSShopRoom->PlaceItem(Pos.x, Pos.y, nIndex + 1, pItem->GetWidth(), pItem->GetHeight());
				}
				else
				{
					nPage++;
					BuySell.m_pSShopRoom->Clear();
					BuySell.m_pSShopRoom->FindRoom(pItem->GetWidth(), pItem->GetHeight(), &Pos);
					BuySell.m_pSShopRoom->PlaceItem(Pos.x, Pos.y, nIndex + 1, pItem->GetWidth(), pItem->GetHeight());
				}
				pInfo->Region.h = Pos.x;
				pInfo->Region.v = Pos.y;
				pInfo->Region.Width = pItem->GetWidth();
				pInfo->Region.Height = pItem->GetHeight();
				pInfo->nContainer = nPage;
				nCount++;
				pInfo++;
			}
		}			
		nRet = nPage;
	}
	else
	{
		for (int i = 0; i < BuySell.GetWidth(); i++)
		{
			nIndex = BuySell.GetItemIndex(nBuyIdx, i);
			KItem* pItem = BuySell.GetItem(nIndex);
			
			if (nIndex >= 0 && pItem)
			{
				nCount++;
			}
		}
		nRet = nCount;
	}
	return nRet;
}

int KCoreShell::GetNextSkill(int nType, int nIndex)
{
	if(nType == 0)
		return Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetNextSkillState(nIndex);
	else if(nType == 1)
		return Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetNextSkillFight(nIndex);
	else if(nType == 2)
		return Npc[Player[CLIENT_PLAYER_INDEX].m_nIndex].m_SkillList.GetNextSkillAura(nIndex);

	return 0;
}

BOOL KCoreShell::IsSkillAura(int nSkillId)
{
	if(nSkillId > 0)
	{
		KSkill * pOrdinSkill = (KSkill *)g_SkillManager.GetSkill(nSkillId, 1);
		if (!pOrdinSkill) 
			return FALSE;

		return pOrdinSkill->IsAura();
	}
	return FALSE;
}