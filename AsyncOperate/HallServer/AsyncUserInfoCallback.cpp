#include "AsyncUserInfoCallback.h"
#include "PlayerMng.h"
#include "../../OuterFactoryImp.h"
#include "Room.h"

AsyncUserInfoCallback::AsyncUserInfoCallback(const userinfo::GetUserBasicReq &req): m_reqUserBasic(req)
{
    loginType = -1;
    nMsgID = 0;
    servantImp = NULL;
    m_createTime = TNOWMS;
}

AsyncUserInfoCallback::AsyncUserInfoCallback(const userinfo::GetUserAccountReq &req) : m_reqGetAccount(req)
{
    m_createTime = TNOWMS;
}

AsyncUserInfoCallback::AsyncUserInfoCallback(const userinfo::ModifyUserWealthReq &req) : m_reqModifyWealth(req)
{
    m_createTime = TNOWMS;
}

AsyncUserInfoCallback::AsyncUserInfoCallback(const GoodsManager::GetTypeGoodsCountReq &req) : m_reqGetUserTypeProps(req)
{
    m_createTime = TNOWMS;
}

AsyncUserInfoCallback::~AsyncUserInfoCallback()
{

}


//获取用户个人信息
void AsyncUserInfoCallback::callback_getUserBasic(tars::Int32 ret,  const userinfo::GetUserBasicResp &resp)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    if (ret != 0)
    {
        LOG_ERROR << resp.userinfo.uid << "|" << "get user basic callback error, ret:" << ret << endl;
        return;
    }

    long lPlayerID = resp.userinfo.uid;
    if (!CPlayerMngSingleton::getInstance()->isExistPlayerMap(lPlayerID))
    {
        CPlayerMngSingleton::getInstance()->insertPlayer2UMap(lPlayerID);
    }

    //
    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(lPlayerID);
    if (!pPlayer)
    {
        LOG_ERROR << lPlayerID << "|" << stHeadInfo.sRoomID << " enter(1) room failed." << endl;
        return;
    }

    LOG_DEBUG << "resp: "<< printTars(resp)<< endl;

    //
    UInfo stUInfo;
    stUInfo.iPlayerGender = resp.userinfo.gender;
    stUInfo.lLastPauseGameTime = resp.userinfo.last_pause_game_time;
    stUInfo.iVipLevel = resp.userinfo.vip_level;
    stUInfo.bCanGame = !resp.userinfo.user_lock;
    strncpy(stUInfo.sNickName, resp.userinfo.name.c_str(), sizeof(stUInfo.sNickName) - 1);
    strncpy(stUInfo.sHeadStr, resp.userinfo.head.c_str(), sizeof(stUInfo.sHeadStr) - 1);
    strncpy(stUInfo.sIp, resp.userinfo.last_login_ip.c_str(), sizeof(stUInfo.sIp) - 1);
    strncpy(stUInfo.sLongitude, resp.userinfo.longitude.c_str(), sizeof(stUInfo.sLongitude) - 1);
    strncpy(stUInfo.sLatitude, resp.userinfo.latitude.c_str(), sizeof(stUInfo.sLatitude) - 1);
    CPlayerMngSingleton::getInstance()->insertUInfo2CPlayer(lPlayerID, stUInfo);

    //
    UWealthInfo stUWealthInfo;
    stUWealthInfo.lGoldCoin  = resp.userinfo.gold;
    stUWealthInfo.iDiamond = resp.userinfo.diamond;
    stUWealthInfo.lClubCoin = resp.userinfo.club_gold;
    stUWealthInfo.lPoint = resp.userinfo.point;
    pPlayer->setUWealthInfo(stUWealthInfo);
    pPlayer->setUpdateGoldFlag(true);

    pPlayer->setCheckSitDownTime(resp.userinfo.lCheckSitDownTime);

    OuterFactorySingleton::getInstance()->async2GetPropsInfo(lPlayerID);
    OuterFactorySingleton::getInstance()->async2GetUserAccount(lPlayerID);
    OuterFactorySingleton::getInstance()->asyncGetCollectGame(lPlayerID);

    if (loginType == 1)
    {
        pPlayer->updateMsgHead(stHeadInfo);
        servantImp->toClientLoginMsg(lPlayerID, nMsgID, 0);
        LOG_DEBUG << lPlayerID << "|" << stHeadInfo.sRoomID << " enter(1) room success." << endl;
    }
    else if (loginType == 2)
    {
        //
        pPlayer->updateMsgHead(stHeadInfo);
        //
        pPlayer->updateBaseInfoExt(stUserBaseInfo);

        ReqInfo stReqInfo;
        stReqInfo.lUID = lPlayerID;
        stReqInfo.nMsgID = nMsgID;
        stReqInfo.vecMsgData = vecMsgData;
        auto pTableMng = CRoomSingleton::getInstance()->findTMngByRID(sRoomID);
        if (!pTableMng)
        {
            LOG_ERROR << lPlayerID << "|" << sRoomID << "|" << nMsgID << "|" << "find pTableMng error." << endl;
            return;
        }

        LOG_DEBUG << lPlayerID << "|" << sRoomID << "|" << nMsgID << "|" << "enter(2) room message." << endl;

        int iRet = pTableMng->onMessage(stReqInfo, stClientParam, stUserBaseInfo);
        if (iRet != 0)
        {
            LOG_ERROR << lPlayerID << "|" << sRoomID << "|" << nMsgID << "|" << "process msg error, ret : " << iRet << endl;
            return;
        }
    }
    else
    {
        LOG_ERROR << lPlayerID << "|" << sRoomID << "|" << nMsgID << "|" << "enter(3) room." << endl;
    }
}

void AsyncUserInfoCallback::callback_getUserBasic_exception(tars::Int32 ret)
{
    LOG_ERROR << m_reqUserBasic.uid << "|" << "get user basic callback exception... ret: " << ret << endl;
}

//获取用户财富信息
void AsyncUserInfoCallback::callback_getUserAccount(tars::Int32 ret,  const userinfo::GetUserAccountResp &resp)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    //LOG_DEBUG << "resp: "<< printTars(resp)<< endl;

    if (ret != 0)
    {
        LOG_ERROR << m_reqGetAccount.uid << "|" << "get user basic callback error, ret:" << ret  << ", uid: " << m_reqGetAccount.uid << endl;
        return;
    }

    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(m_reqGetAccount.uid);
    if (pPlayer)
    {
        UAccountInfo setUAccountInfo;
        strncpy(setUAccountInfo.userName, resp.useraccount.userName.c_str(), sizeof(setUAccountInfo.userName) - 1);
        strncpy(setUAccountInfo.deviceID, resp.useraccount.deviceID.c_str(), sizeof(setUAccountInfo.deviceID) - 1);
        strncpy(setUAccountInfo.deviceType, resp.useraccount.deviceType.c_str(), sizeof(setUAccountInfo.deviceType) - 1);
        strncpy(setUAccountInfo.countryID, resp.useraccount.countryId.c_str(), sizeof(setUAccountInfo.countryID) - 1);
        strncpy(setUAccountInfo.tgId, resp.useraccount.bindTgId.c_str(), sizeof(setUAccountInfo.tgId) - 1);
        setUAccountInfo.platform = resp.useraccount.platform;
        setUAccountInfo.channnelID = resp.useraccount.channnelID;
        setUAccountInfo.areaID = resp.useraccount.areaID;
        setUAccountInfo.regType = resp.useraccount.regType;
        setUAccountInfo.isForbidden = resp.useraccount.isForbidden;
        pPlayer->setUAccountInfo(setUAccountInfo);
        LOG_DEBUG << m_reqGetAccount.uid << "|" << "get user detail callback success, uid: " << m_reqGetAccount.uid  << printTars(resp) << endl;
    }
    else
    {
        LOG_DEBUG << m_reqGetAccount.uid << "|" << "get user detail callback failed, uid: " << m_reqGetAccount.uid << endl;
    }
}

void AsyncUserInfoCallback::callback_getUserAccount_exception(tars::Int32 ret)
{
    LOG_ERROR << m_reqGetAccount.uid << "|" << "get user account callback exception... ret: " << ret << endl;
}

//修改用户财富信息
void AsyncUserInfoCallback::callback_modifyUserWealth(tars::Int32 ret, const userinfo::ModifyUserWealthResp &resp)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    if (ret != 0)
    {
        LOG_ERROR << m_reqModifyWealth.uid << "|" << "modify account resp callback error, ret : " << ret << endl;
        return;
    }

    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(m_reqModifyWealth.uid);
    if (pPlayer)
    {
        if (resp.resultCode == 0)
        {
            UWealthInfo &wealthInfo = pPlayer->getWealthInfo();
            wealthInfo.lGoldCoin = resp.userinfo.gold;
            wealthInfo.iDiamond = resp.userinfo.diamond;
            wealthInfo.lPoint   = resp.userinfo.point;
            pPlayer->setUpdateGoldFlag(true);
            LOG_DEBUG << m_reqModifyWealth.uid << "|" << "modify account resp callback success." << endl;
        }
        else
        {
            LOG_DEBUG << m_reqModifyWealth.uid << "|" << "modify account resp callback fatal error. iResult: " << resp.resultCode << endl;
        }
    }
    else
    {
        LOG_ERROR << m_reqModifyWealth.uid << "|" << "modify account resp callback failed." << endl;
    }
}

//
void AsyncUserInfoCallback::callback_modifyUserWealth_exception(tars::Int32 ret)
{
    LOG_ERROR << m_reqModifyWealth.uid << "|" << "modify user account callback exception. ret : " << ret << endl;
}

//获取道具信息
void AsyncUserInfoCallback::callback_getTypeGoodsCount(tars::Int32 ret, const GoodsManager::GetTypeGoodsCountRsp &resp)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    if (ret != 0)
    {
        LOG_ERROR << m_reqGetUserTypeProps.uid << "|" << "get user props callback error, ret : " << ret << endl;
        return;
    }

    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(m_reqGetUserTypeProps.uid);
    if (pPlayer)
    {
        if (resp.resultCode == 0)
        {
            for (auto &it : resp.goodsVec)
            {
                UPropsInfo info;
                info.iPropsID = it.goodsID;
                info.iPropsNum = it.count;
                pPlayer->updatePropsInfo(m_reqGetUserTypeProps.goodsType, info);
                LOG_DEBUG << m_reqGetUserTypeProps.uid << "|"
                          << "get user props callback success. goodsID: " << it.goodsID 
                          << ", goodsType: " << m_reqGetUserTypeProps.goodsType 
                          << ", count: " << it.count << "" << endl;
            }
        }
        else
        {
            LOG_DEBUG << m_reqGetUserTypeProps.uid << "|" << "get user props callback fatal error. iResult: " << resp.resultCode << endl;
        }
    }
    else
    {
        LOG_ERROR << m_reqGetUserTypeProps.uid << "|" << "get user propscallback failed." << endl;
    }
}

//
void AsyncUserInfoCallback::callback_getTypeGoodsCount_exception(tars::Int32 ret)
{
    LOG_ERROR << m_reqGetUserTypeProps.uid << "|" << "get user props callback exception. ret : " << ret << endl;
}




AsyncUserStatCallback::AsyncUserStatCallback(const gamerecord::CollectGameListReq &req) : m_reqCollectGameList(req)
{
    m_createTime = TNOWMS;
}

AsyncUserStatCallback::AsyncUserStatCallback(const int64_t lUid) : lPlayerID(lUid)
{
    m_createTime = TNOWMS;
}

AsyncUserStatCallback::~AsyncUserStatCallback()
{

}

//获取收藏信息
void AsyncUserStatCallback::callback_getCollectGame(tars::Int32 ret, const gamerecord::CollectGameListResp &resp)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    if (ret != 0)
    {
        LOG_ERROR << m_reqCollectGameList.uid << "|" << "get user collect game callback error, ret : " << ret << endl;
        return;
    }

    LOG_ERROR << m_reqCollectGameList.uid << "| resp: " << printTars(resp) << endl;

    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(m_reqCollectGameList.uid);
    if (pPlayer)
    {
        if (resp.resultCode == 0)
        {
            map<string, vector<int>> mapCollectGame;
            for(auto item : resp.vecCollectGame)
            {
                auto it = mapCollectGame.find(item.sBriefId);
                if(it == mapCollectGame.end())
                {
                    mapCollectGame.insert(std::make_pair(item.sBriefId, vector<int>(1, item.iRound)));
                }
                else
                {
                    it->second.push_back(item.iRound);
                }
            }
            pPlayer->setUCollectGames(mapCollectGame);
        }
        else
        {
            LOG_ERROR << m_reqCollectGameList.uid << "|" << "get collect game callback fatal error. iResult: " << resp.resultCode << endl;
        }
    }
    else
    {
        LOG_ERROR << m_reqCollectGameList.uid << "|" << "get user propscallback failed." << endl;
    }
}

//
void AsyncUserStatCallback::callback_getCollectGame_exception(tars::Int32 ret)
{
    LOG_ERROR << m_reqCollectGameList.uid << "|" << "get user collect game callback exception. ret : " << ret << endl;
}

//获取打牌信息
void AsyncUserStatCallback::callback_getUserGameStat(tars::Int32 ret, const string &sData)
{
    int64_t iCostTime = TNOWMS - m_createTime;
    if (iCostTime > COST_MS)
    {
        LOG_WARN << "@iCostTime: " << iCostTime << endl;
    }

    if (ret != 0)
    {
        LOG_ERROR << lPlayerID << "|" << "get user game stat error, ret : " << ret << endl;
        return;
    }

    auto pPlayer = CPlayerMngSingleton::getInstance()->findPlayerPtrByUID(lPlayerID);
    if (pPlayer)
    {
        Pb::UserGameStat userStat;
        userStat.ParseFromString(sData);

        LOG_DEBUG << "game round count: "<< userStat.iroundcount() << ", into pool count: "<< userStat.iintopoolcount()<< endl;
        if(userStat.iroundcount() > 300)
        {
            pPlayer->setUIntoPoolRate(userStat.iintopoolcount() * 100 / userStat.iroundcount());
        }
        CPlayerMngSingleton::getInstance()->setUpdateStatFlag(lPlayerID, true);
    }
    else
    {
        LOG_ERROR << lPlayerID << "|" << "get user propscallback failed." << endl;
    }
}

//
void AsyncUserStatCallback::callback_getUserGameStat_exception(tars::Int32 ret)
{
    LOG_ERROR << lPlayerID << "|" << "get user collect game callback exception. ret : " << ret << endl;
}
