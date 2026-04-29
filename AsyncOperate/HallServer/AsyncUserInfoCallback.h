#ifndef _ASYNC_USER_INFO_CB_H_
#define _ASYNC_USER_INFO_CB_H_

#include "HallServant.h"
#include "GameRecordServant.h"
#include "GameDataDef.h"
#include "RoomServantImp.h"
#include "third.pb.h"

using namespace userinfo;
using namespace hall;
using namespace GoodsManager;
using namespace gamerecord;

/**
 * 异步用户信息回调类
 */
class AsyncUserInfoCallback : public HallServantPrxCallback
{
public:
    AsyncUserInfoCallback(const userinfo::GetUserBasicReq &req);
    AsyncUserInfoCallback(const userinfo::GetUserAccountReq &req);
    AsyncUserInfoCallback(const userinfo::ModifyUserWealthReq &req);
    AsyncUserInfoCallback(const GoodsManager::GetTypeGoodsCountReq &req);
    virtual ~AsyncUserInfoCallback();

public:
    //
    void callback_getUserBasic(tars::Int32 ret, const userinfo::GetUserBasicResp &resp);
    //
    void callback_getUserBasic_exception(tars::Int32 ret);
    //
    void callback_getUserAccount(tars::Int32 ret, const userinfo::GetUserAccountResp &resp);
    //
    void callback_getUserAccount_exception(tars::Int32 ret);
    //
    void callback_modifyUserWealth(tars::Int32 ret,  const userinfo::ModifyUserWealthResp &resp);
    //
    void callback_modifyUserWealth_exception(tars::Int32 ret);
    //
    void callback_getTypeGoodsCount(tars::Int32 ret, const GoodsManager::GetTypeGoodsCountRsp &resp);
    //
    void callback_getTypeGoodsCount_exception(tars::Int32 ret);

public:
    //
    RoomServantImp *servantImp;
    //
    int32_t loginType;
    //
    HeadInfo stHeadInfo;
    //
    short nMsgID;
    //
    string sRoomID;
    //
    vector<char> vecMsgData;
    //
    JFGame::TClientParam stClientParam;
    //
    JFGame::UserBaseInfoExt stUserBaseInfo;

private:
    //
    GetUserBasicReq m_reqUserBasic;
    //
    GetUserAccountReq m_reqGetAccount;
    //
    ModifyUserWealthReq m_reqModifyWealth;
    //
    GetTypeGoodsCountReq m_reqGetUserTypeProps;
    //
    int32_t i_propsType;
    //
    int64_t m_createTime;

};

class AsyncUserStatCallback: public GameRecordServantPrxCallback
{
public:
    AsyncUserStatCallback(const gamerecord::CollectGameListReq &req);
    AsyncUserStatCallback(const int64_t lUid);
    virtual ~AsyncUserStatCallback();
public:
    //
    void callback_getCollectGame(tars::Int32 ret, const gamerecord::CollectGameListResp &resp);
    //
    void callback_getCollectGame_exception(tars::Int32 ret);
    //
    void callback_getUserGameStat(tars::Int32 ret, const string &sData);
    //
    void callback_getUserGameStat_exception(tars::Int32 ret);

private:
    //
    CollectGameListReq m_reqCollectGameList;
    //
    int64_t lPlayerID;
     //
    int64_t m_createTime;
};

#endif  // _ASYNC_USER_INFO_CB_H_
