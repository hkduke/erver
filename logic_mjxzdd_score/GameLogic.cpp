#include "GameLogic.h"
#include <time.h>
#include "math.h"
#include <random>
#include <string.h>
#include "MSG_mjxzdd.h"
#include "glog_wrapper.h"
#include <algorithm>


//静态变量

//扑克数据
const CT_BYTE CGameLogic::m_cbCardDataArray[MAX_REPERTORY]= {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,						//索子
        0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
        0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
        0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
        0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,						//同子
/*                0x31, 0x31, 0x31, 0x31,
                0x32, 0x32, 0x32, 0x32,
                0x33, 0x33, 0x33, 0x33,
                0x34, 0x34, 0x34, 0x34,
                0x35, 0x35, 0x35, 0x35,
                0x36, 0x36, 0x36, 0x36,
                0x37, 0x37, 0x37, 0x37,

                0x01, 0x01, 0x01, 0x01,
                0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03,
                0x04, 0x04, 0x04, 0x04,
                0x05, 0x05, 0x05, 0x05,
                0x06, 0x06, 0x06, 0x06,
                0x07, 0x07, 0x07, 0x07,
                0x08, 0x08, 0x08, 0x08,
                0x09, 0x09, 0x09, 0x09,*/

                //0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
                //0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
                //0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
                //0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,						//万子
                //0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//番子
                //0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//番子
                //0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//番子
                //0x31,0x32,0x33,0x34,0x35,0x36,0x37,									//番子
};


//静态变量
CT_BOOL		CChiHuRight::m_bInit = CT_FALSE;
CT_DWORD		CChiHuRight::m_dwRightMask[MAX_RIGHT_COUNT];

//构造函数
CChiHuRight::CChiHuRight()
{
    //memset( m_dwRight,sizeof(m_dwRight) );
    memset(m_dwRight, 0 , sizeof(m_dwRight) );

    if( !m_bInit )
    {
        m_bInit = CT_TRUE;
        for( CT_BYTE i = 0; i < CountArray(m_dwRightMask); i++ )
        {
            if( 0 == i )
                m_dwRightMask[i] = 0;
            else
                m_dwRightMask[i] = (CT_DWORD(pow((float)2,i-1)))<<28;
        }
    }
}

//赋值符重载
CChiHuRight & CChiHuRight::operator = ( CT_DWORD dwRight )
{
    CT_DWORD dwOtherRight = 0;
    //验证权位
    if( !IsValidRight( dwRight ) )
    {
        //验证取反权位
        //assert( IsValidRight( ~dwRight ) );
        assert(IsValidRight( ~dwRight ));
        if( !IsValidRight( ~dwRight ) ) return *this;
        dwRight = ~dwRight;
        dwOtherRight = MASK_CHI_HU_RIGHT;
    }

    for( CT_BYTE i = 0; i < CountArray(m_dwRightMask); i++ )
    {
        if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
            m_dwRight[i] = dwRight&MASK_CHI_HU_RIGHT;
        else m_dwRight[i] = dwOtherRight;
    }

    return *this;
}

//与等于
CChiHuRight & CChiHuRight::operator &= ( CT_DWORD dwRight )
{
    CT_BOOL bNavigate = CT_FALSE;
    //验证权位
    if( !IsValidRight( dwRight ) )
    {
        //验证取反权位
        //assert( IsValidRight( ~dwRight ) );
        assert(IsValidRight( ~dwRight ));
        if( !IsValidRight( ~dwRight ) ) return *this;
        //调整权位
        CT_DWORD dwHeadRight = (~dwRight)&0xF0000000;
        CT_DWORD dwTailRight = dwRight&MASK_CHI_HU_RIGHT;
        dwRight = dwHeadRight|dwTailRight;
        bNavigate = CT_TRUE;
    }

    for( CT_BYTE i = 0; i < CountArray(m_dwRightMask); i++ )
    {
        if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
        {
            m_dwRight[i] &= (dwRight&MASK_CHI_HU_RIGHT);
        }
        else if( !bNavigate )
            m_dwRight[i] = 0;
    }

    return *this;
}

//或等于
CChiHuRight & CChiHuRight::operator |= ( CT_DWORD dwRight )
{
    //验证权位
    if( !IsValidRight( dwRight ) ) return *this;

    for( CT_BYTE i = 0; i < CountArray(m_dwRightMask); i++ )
    {
        if( (dwRight&m_dwRightMask[i]) || (i==0&&dwRight<0x10000000) )
        {
            m_dwRight[i] |= (dwRight&MASK_CHI_HU_RIGHT);
            break;
        }
    }

    return *this;
}

//与
CChiHuRight CChiHuRight::operator & ( CT_DWORD dwRight )
{
    CChiHuRight chr = *this;
    return (chr &= dwRight);
}

//与
CChiHuRight CChiHuRight::operator & ( CT_DWORD dwRight ) const
{
    CChiHuRight chr = *this;
    return (chr &= dwRight);
}

//或
CChiHuRight CChiHuRight::operator | ( CT_DWORD dwRight )
{
    CChiHuRight chr = *this;
    return chr |= dwRight;
}

//或
CChiHuRight CChiHuRight::operator | ( CT_DWORD dwRight ) const
{
    CChiHuRight chr = *this;
    return chr |= dwRight;
}

//相等
CT_BOOL CChiHuRight::operator == ( CT_DWORD dwRight ) const
{
    CChiHuRight chr;
    chr = dwRight;
    return (*this)==chr;
}

//相等
CT_BOOL CChiHuRight::operator == ( const CChiHuRight chr ) const
{
    for( CT_WORD i = 0; i < CountArray( m_dwRight ); i++ )
    {
        if( m_dwRight[i] != chr.m_dwRight[i] ) return CT_FALSE;
    }
    return CT_TRUE;
}

//不相等
CT_BOOL CChiHuRight::operator != ( CT_DWORD dwRight ) const
{
    CChiHuRight chr;
    chr = dwRight;
    return (*this)!=chr;
}

//不相等
CT_BOOL CChiHuRight::operator != ( const CChiHuRight chr ) const
{
    return !((*this)==chr);
}

//是否权位为空
CT_BOOL CChiHuRight::IsEmpty()
{
    for( CT_BYTE i = 0; i < CountArray(m_dwRight); i++ )
        if( m_dwRight[i] ) return CT_FALSE;
    return CT_TRUE;
}

//设置权位为空
CT_VOID CChiHuRight::SetEmpty()
{
    //memset( m_dwRight,sizeof(m_dwRight) );
    memset(m_dwRight, 0, sizeof(m_dwRight) );
    return;
}

//获取权位数值
CT_BYTE CChiHuRight::GetRightData( CT_DWORD dwRight[], CT_BYTE cbMaxCount )
{
    //assert( cbMaxCount >= CountArray(m_dwRight) );
    assert( cbMaxCount >= CountArray(m_dwRight) );
    if( cbMaxCount < CountArray(m_dwRight) ) return 0;

    //memcpy( dwRight,m_dwRight,sizeof(CT_DWORD)*CountArray(m_dwRight) );
    memcpy( dwRight,m_dwRight,sizeof(CT_DWORD)*CountArray(m_dwRight) );
    return CountArray(m_dwRight);
}

//设置权位数值
CT_BOOL CChiHuRight::SetRightData( const CT_DWORD dwRight[], CT_BYTE cbRightCount )
{
    //assert( cbRightCount <= CountArray(m_dwRight) );
    assert( cbRightCount <= CountArray(m_dwRight) );
    if( cbRightCount > CountArray(m_dwRight) ) return CT_FALSE;

   // memset( m_dwRight,sizeof(m_dwRight) );
   memset(m_dwRight, 0, sizeof(m_dwRight));
    //memcpy( m_dwRight,dwRight,sizeof(CT_DWORD)*cbRightCount );
    memcpy( m_dwRight,dwRight,sizeof(CT_DWORD)*cbRightCount );
    return CT_TRUE;
}

//检查仅位是否正确
CT_BOOL CChiHuRight::IsValidRight( CT_DWORD dwRight )
{
    CT_DWORD dwRightHead = dwRight & 0xF0000000;
    for( CT_BYTE i = 0; i < CountArray(m_dwRightMask); i++ )
        if( m_dwRightMask[i] == dwRightHead ) return CT_TRUE;
    return CT_FALSE;
}

//混乱扑克
    CT_VOID CGameLogic::RandCardData(CT_BYTE cbCardData[], CT_BYTE cbMaxCount)
    {
        //混乱准备
        CT_BYTE cbCardDataTemp[CountArray(m_cbCardDataArray)];
        //memcpy(cbCardDataTemp,m_cbCardDataArray,sizeof(m_cbCardDataArray));
        memcpy(cbCardDataTemp,m_cbCardDataArray,sizeof(m_cbCardDataArray));

        //NiqiuSrand();
        //混乱扑克
        CT_BYTE cbRandCount=0,cbPosition=0;
        do
        {
            cbPosition=rand()%(cbMaxCount-cbRandCount);
            cbCardData[cbRandCount++]=cbCardDataTemp[cbPosition];
            cbCardDataTemp[cbPosition]=cbCardDataTemp[cbMaxCount-cbRandCount];
        } while (cbRandCount<cbMaxCount);

        return;
    }

//混乱扑克
    CT_VOID CGameLogic::RandCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbRandData[])
    {
        if( cbCardCount == 0 ) return;

        //混乱准备
        CT_BYTE cbCardDataTemp[MAX_COUNT];
        //memcpy(cbCardDataTemp,cbCardData,sizeof(CT_BYTE)*cbCardCount);
        memcpy(cbCardDataTemp,cbCardData,sizeof(CT_BYTE)*cbCardCount);

        //NiqiuSrand();
        //混乱扑克
        CT_BYTE cbRandCount=0,cbPosition=0;
        do
        {
            cbPosition=rand()%(cbCardCount-cbRandCount);
            cbRandData[cbRandCount++]=cbCardDataTemp[cbPosition];
            cbCardDataTemp[cbPosition]=cbCardDataTemp[cbCardCount-cbRandCount];
        } while (cbRandCount<cbCardCount);

        return;
    }

//检查扑克
    CT_BOOL CGameLogic::CheckCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbRemoveCard)
    {
        //效验扑克
        //assert(IsValidCard(cbRemoveCard));
        //assert(IsValidCard(cbRemoveCard));
        //assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard)]>0);
        //assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard)]>0);

        CT_BYTE cbRemoveIndex=SwitchToCardIndex(cbRemoveCard);
        return cbCardIndex[cbRemoveIndex]>0;
    }
//添加扑克
    CT_BOOL CGameLogic::AddCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbAddCard)
    {
        CT_BYTE cbAddIndex = SwitchToCardIndex(cbAddCard);
        if(cbAddIndex >= MAX_INDEX)
        {
            return CT_FALSE;
        }

        cbCardIndex[cbAddIndex]++;
        return CT_TRUE;
    }

//删除扑克
    CT_BOOL CGameLogic::RemoveCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbRemoveCard)
    {
        //效验扑克
        //assert(IsValidCard(cbRemoveCard));
        //assert(IsValidCard(cbRemoveCard));
        //assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard)]>0);
        //assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard)]>0);

        //删除扑克
        CT_BYTE cbRemoveIndex=SwitchToCardIndex(cbRemoveCard);
        if (cbCardIndex[cbRemoveIndex]>0)
        {
            cbCardIndex[cbRemoveIndex]--;
            return CT_TRUE;
        }

        //失败效验
        //assert(CT_FALSE);
        //assert(CT_FALSE);

        return CT_FALSE;
    }

//删除扑克
    CT_BOOL CGameLogic::RemoveCard(CT_BYTE cbCardIndex[MAX_INDEX], const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount)
    {
        //删除扑克
        for (CT_BYTE i=0;i<cbRemoveCount;i++)
        {
            //效验扑克
           // assert(IsValidCard(cbRemoveCard[i]));
           assert(IsValidCard(cbRemoveCard[i]));
            //assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard[i])]>0);
            assert(cbCardIndex[SwitchToCardIndex(cbRemoveCard[i])]>0);

            //删除扑克
            CT_BYTE cbRemoveIndex=SwitchToCardIndex(cbRemoveCard[i]);
            if (cbCardIndex[cbRemoveIndex]==0)
            {
                //错误断言
                //assert(CT_FALSE);
                assert(CT_FALSE);

                //还原删除
                for (CT_BYTE j=0;j<i;j++)
                {
                    //assert(IsValidCard(cbRemoveCard[j]));
                    assert(IsValidCard(cbRemoveCard[j]));
                    cbCardIndex[SwitchToCardIndex(cbRemoveCard[j])]++;
                }

                return CT_FALSE;
            }
            else
            {
                //删除扑克
                --cbCardIndex[cbRemoveIndex];
            }
        }

        return CT_TRUE;
    }

//删除扑克
    CT_BOOL CGameLogic::RemoveCard(CT_BYTE cbCardData[], CT_BYTE cbCardCount, const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount)
    {
        //检验数据
        //assert(cbCardCount<=MAX_COUNT);
        assert(cbCardCount<=MAX_COUNT);
        //assert(cbRemoveCount<=cbCardCount);
        assert(cbRemoveCount<=cbCardCount);

        //定义变量
        CT_BYTE cbDeleteCount=0,cbTempCardData[MAX_COUNT];
        if (cbCardCount>CountArray(cbTempCardData))
            return CT_FALSE;
        //memcpy(cbTempCardData,cbCardData,cbCardCount*sizeof(cbCardData[0]));
        memcpy(cbTempCardData,cbCardData,cbCardCount*sizeof(cbCardData[0]));

        //置零扑克
        for (CT_BYTE i=0;i<cbRemoveCount;i++)
        {
            for (CT_BYTE j=0;j<cbCardCount;j++)
            {
                if (cbRemoveCard[i]==cbTempCardData[j])
                {
                    cbDeleteCount++;
                    cbTempCardData[j]=0;
                    break;
                }
            }
        }

        //成功判断
        if (cbDeleteCount!=cbRemoveCount)
        {
            //assert(CT_FALSE);
            assert(CT_FALSE);
            return CT_FALSE;
        }

        //清理扑克
        CT_BYTE cbCardPos=0;
        for (CT_BYTE i=0;i<cbCardCount;i++)
        {
            if (cbTempCardData[i]!=0)
                cbCardData[cbCardPos++]=cbTempCardData[i];
        }

        return CT_TRUE;
    }

    CT_BOOL CGameLogic::IsValidCard(CT_BYTE cbCardData)
    {
        CT_BYTE cbValue=(cbCardData&MASK_VALUE);
        CT_BYTE cbColor=(cbCardData&MASK_COLOR)>>4;
        return (((cbValue>=1)&&(cbValue<=9)&&(cbColor<=2))||((cbValue>=1)&&(cbValue<=7)&&(cbColor==3)));
    }

    CT_BYTE CGameLogic::GetCardColor(CT_BYTE cbCardData)
    {
        //CT_BYTE cbValue=(cbCardData&MASK_VALUE);
        CT_BYTE cbColor=(cbCardData&MASK_COLOR)>>4;

        return cbColor;
    }

//扑克数目
    CT_BYTE CGameLogic::GetCardCount(const CT_BYTE cbCardIndex[MAX_INDEX])
    {
        //数目统计
        CT_BYTE cbCardCount=0;
        for (CT_BYTE i=0;i<MAX_INDEX;i++)
            cbCardCount+=cbCardIndex[i];

        return cbCardCount;
    }

    CT_BYTE CGameLogic::GetCardCount(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbOneCardIndex)
    {
        CT_BYTE count = 0;

        if(cbOneCardIndex >= MAX_INDEX)
        {
            count = 0;
        }
        else
        {
            count = cbCardIndex[cbOneCardIndex];
        }

        return count;
    }

    CT_BYTE CGameLogic::GetCardCountByCardData(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbOneCardData)
    {
        CT_BYTE cbTempCardIndex = SwitchToCardIndex(cbOneCardData);
        if(cbTempCardIndex >= MAX_INDEX)
        {
            return 0;
        }
        else
        {
            return cbCardIndex[cbTempCardIndex];
        }
    }

//获取组合
    CT_BYTE CGameLogic::GetWeaveCard(CT_BYTE cbWeaveKind, CT_BYTE cbCenterCard, CT_BYTE cbCardBuffer[4])
    {
        //组合扑克
        switch (cbWeaveKind)
        {
            case WIK_LEFT:		//上牌操作
            {
                //设置变量
                cbCardBuffer[0]=cbCenterCard;
                cbCardBuffer[1]=cbCenterCard+1;
                cbCardBuffer[2]=cbCenterCard+2;

                return 3;
            }
            case WIK_RIGHT:		//上牌操作
            {
                //设置变量
                cbCardBuffer[0]=cbCenterCard-2;
                cbCardBuffer[1]=cbCenterCard-1;
                cbCardBuffer[2]=cbCenterCard;

                return 3;
            }
            case WIK_CENTER:	//上牌操作
            {
                //设置变量
                cbCardBuffer[0]=cbCenterCard-1;
                cbCardBuffer[1]=cbCenterCard;
                cbCardBuffer[2]=cbCenterCard+1;

                return 3;
            }
            case WIK_PENG:		//碰牌操作
            {
                //设置变量
                cbCardBuffer[0]=cbCenterCard;
                cbCardBuffer[1]=cbCenterCard;
                cbCardBuffer[2]=cbCenterCard;

                return 3;
            }
            case WIK_GANG:		//杠牌操作
            {
                //设置变量
                cbCardBuffer[0]=cbCenterCard;
                cbCardBuffer[1]=cbCenterCard;
                cbCardBuffer[2]=cbCenterCard;
                cbCardBuffer[3]=cbCenterCard;

                return 4;
            }
            default:
            {
                LOG(ERROR) << "GameLogic.cpp GetWeaveCard() cbWeaveKind:" << (int)cbWeaveKind;
                assert(CT_FALSE);
            }
        }

        return 0;
    }

//动作等级
    CT_BYTE CGameLogic::GetUserActionRank(CT_BYTE cbUserAction)
    {
        //胡牌等级
        if (cbUserAction&WIK_CHI_HU) { return 4; }

        //杠牌等级
        if (cbUserAction&WIK_GANG) { return 3; }

        //碰牌等级
        if (cbUserAction&WIK_PENG) { return 2; }

        //上牌等级
        if (cbUserAction&(WIK_RIGHT|WIK_CENTER|WIK_LEFT)) { return 1; }

        return 0;
    }

//胡牌等级
    CT_WORD CGameLogic::GetChiHuActionRank(const CChiHuRight & ChiHuRight)
    {
        CT_WORD wFanShu = 0;

        return wFanShu;
    }

//吃牌判断
    CT_BYTE CGameLogic::EstimateEatCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard)
    {
        //参数效验
        //assert(IsValidCard(cbCurrentCard));
        assert(IsValidCard(cbCurrentCard));

        //过滤判断
        if ( cbCurrentCard>=0x31 || IsMagicCard(cbCurrentCard) )
            return WIK_NULL;

        //变量定义
        CT_BYTE cbExcursion[3]={0,1,2};
        CT_BYTE cbItemKind[3]={WIK_LEFT,WIK_CENTER,WIK_RIGHT};

        //吃牌判断
        CT_BYTE cbEatKind=0,cbFirstIndex=0;
        CT_BYTE cbCurrentIndex=SwitchToCardIndex(cbCurrentCard);
        for (CT_BYTE i=0;i<CountArray(cbItemKind);i++)
        {
            CT_BYTE cbValueIndex=cbCurrentIndex%9;
            if ((cbValueIndex>=cbExcursion[i])&&((cbValueIndex-cbExcursion[i])<=6))
            {
                //吃牌判断
                cbFirstIndex=cbCurrentIndex-cbExcursion[i];

                //吃牌不能包含有王霸
                if( m_cbMagicIndex != MAX_INDEX &&
                    m_cbMagicIndex >= cbFirstIndex && m_cbMagicIndex <= cbFirstIndex+2 ) continue;

                if ((cbCurrentIndex!=cbFirstIndex)&&(cbCardIndex[cbFirstIndex]==0))
                    continue;
                if ((cbCurrentIndex!=(cbFirstIndex+1))&&(cbCardIndex[cbFirstIndex+1]==0))
                    continue;
                if ((cbCurrentIndex!=(cbFirstIndex+2))&&(cbCardIndex[cbFirstIndex+2]==0))
                    continue;

                //设置类型
                cbEatKind|=cbItemKind[i];
            }
        }

        return cbEatKind;
    }

//碰牌判断
    CT_BYTE CGameLogic::EstimatePengCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard)
    {
        //参数效验
        assert(IsValidCard(cbCurrentCard));

        //过滤判断
        if ( IsMagicCard(cbCurrentCard) )
            return WIK_NULL;

        //碰牌判断
        return (cbCardIndex[SwitchToCardIndex(cbCurrentCard)]>=2)?WIK_PENG:WIK_NULL;

       /* //参数效验
        //assert(IsValidCard(cbCurrentCard));
        assert(IsValidCard(cbCurrentCard));

        //过滤判断
        if ( IsMagicCard(cbCurrentCard) || IsHuaCard(cbCurrentCard) )
            return WIK_NULL;

        //碰牌判断
        return (cbCardIndex[SwitchToCardIndex(cbCurrentCard)]>=2)?WIK_PENG:WIK_NULL;*/
    }

//杠牌判断
    CT_BYTE CGameLogic::EstimateGangCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard)
    {
        //参数效验
       // assert(IsValidCard(cbCurrentCard));
       assert(IsValidCard(cbCurrentCard));

        //过滤判断
        if ( IsMagicCard(cbCurrentCard))
            return WIK_NULL;

        //杠牌判断
        return (cbCardIndex[SwitchToCardIndex(cbCurrentCard)]==3)?WIK_GANG:WIK_NULL;
    }

//杠牌分析
    CT_BYTE CGameLogic::AnalyseGangCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, tagGangCardResult & GangCardResult)
    {
        //设置变量
        CT_BYTE cbActionMask=WIK_NULL;
        //memset(&GangCardResult,sizeof(GangCardResult));
        memset(&GangCardResult,0,sizeof(GangCardResult));

        //手上杠牌
        for (CT_BYTE i=0;i<MAX_INDEX;i++)
        {
            if( i == m_cbMagicIndex ) continue;
            if (cbCardIndex[i]==4)
            {
                cbActionMask|=WIK_GANG;
                CT_BYTE nIndex = GangCardResult.cbCardCount++;
                GangCardResult.cbCardData[nIndex]=SwitchToCardData(i);
                GangCardResult.dwGangCardType[nIndex] |= WIK_GANG_MY;
                GangCardResult.dwGangCardType[nIndex] |= WIK_GANG_AN_MY;
            }
        }

        //组合杠牌
        for (CT_BYTE i=0;i<cbWeaveCount;i++)
        {
            if (WeaveItem[i].cbWeaveKind==WIK_PENG)
            {
                if (cbCardIndex[SwitchToCardIndex(WeaveItem[i].cbCenterCard)]==1)
                {
                    cbActionMask|=WIK_GANG;
                    CT_BYTE nIndex = GangCardResult.cbCardCount++;
                    GangCardResult.cbCardData[nIndex]=WeaveItem[i].cbCenterCard;
                    GangCardResult.dwGangCardType[nIndex] |= WIK_GANG_MY;
                    GangCardResult.dwGangCardType[nIndex] |= WIK_GANG_MING_PENG_MY;
                }
            }
        }

        return cbActionMask;
    }

//吃胡分析
    CT_BYTE CGameLogic::AnalyseChiHuCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, CT_BYTE cbCurrentCard, CChiHuRight &ChiHuRight, CAnalyseItemArray& AnalyseItemArray)
    {
        //变量定义
        CT_BYTE cbChiHuKind=WIK_NULL;

        //设置变量
        AnalyseItemArray.RemoveAll();
        ChiHuRight.SetEmpty();

        //构造扑克
        CT_BYTE cbCardIndexTemp[MAX_INDEX];
        //memcpy(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));
        memcpy(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

        //cbCurrentCard一定不为0			!!!!!!!!!
        //assert( cbCurrentCard != 0 );
        assert( cbCurrentCard != 0 );
        if( cbCurrentCard == 0 )
        {
            return WIK_NULL;
        }

        //插入扑克
        if (cbCurrentCard!=0) cbCardIndexTemp[SwitchToCardIndex(cbCurrentCard)]++;

        //分析扑克
        AnalyseCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,AnalyseItemArray);

        //胡牌分析/无番和
        if (AnalyseItemArray.GetCount()>0) cbChiHuKind = WIK_CHI_HU;

        return cbChiHuKind;
    }

//听牌分析
    CT_BYTE CGameLogic::AnalyseTingCard(CT_WORD wChairID, CT_WORD wBankerUser, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
                                     const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount, const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
                                     const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng, CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
                                     CT_BYTE cbListenCardData[MAX_COUNT][2+MAX_INDEX*4], CT_BYTE& cbListenCardCount, CT_BOOL bOnce /*= CT_FALSE*/ )
    {
        cbListenCardCount = 0;
        //memset(cbListenCardData, 0, sizeof(cbListenCardData));
        memset(cbListenCardData, 0, (sizeof(CT_BYTE) * MAX_COUNT * (2 + MAX_INDEX * 4)));

        CT_BOOL hasHu = CT_FALSE;
        //复制数据
        CT_BYTE cbCardIndexTemp[GAME_PLAYER][MAX_INDEX];
        memcpy( cbCardIndexTemp,cbHandCardIndex,sizeof(cbCardIndexTemp) );

        CChiHuRight chr;
        CT_BOOL bIsZiMo = CT_FALSE;
        tagAnalyseItem LastAnalyseItem;
        CAnalyseItemArray AnalyseItemArray;
        for( CT_BYTE i = 0; i < MAX_INDEX-MAX_HUA_CARD; i++ )
        {
            if( cbCardIndexTemp[wChairID][i] == 0 ) continue;
            cbCardIndexTemp[wChairID][i]--;

            //听牌数据
            hasHu = CT_FALSE;
            //memset(cbListenCardData[cbListenCardCount], sizeof(cbListenCardData[cbListenCardCount]));
            memset(cbListenCardData[cbListenCardCount], 0, sizeof(cbListenCardData[cbListenCardCount]));
            tagListenCardData* pLCardData = (tagListenCardData*)cbListenCardData[cbListenCardCount];
            pLCardData->cbOutCard = SwitchToCardData(i);

            for( CT_BYTE j = 0; j < MAX_INDEX-MAX_HUA_CARD; j++ )
            {
                CT_BYTE cbCurrentCard = SwitchToCardData(j);
                if( WIK_CHI_HU == AnalyseChiHuCard(cbCardIndexTemp[wChairID],WeaveItemArray[wChairID],cbWeaveItemCount[wChairID],cbCurrentCard,chr,AnalyseItemArray) )
                {
                    if (bOnce)
                    {
                        return WIK_LISTEN;
                    }

                    tagHuCardData* pHuCardData = (tagHuCardData*)(pLCardData->cbHuCardData+pLCardData->cbHuCardCount*sizeof(tagHuCardData));
                    pHuCardData->cbListenCard = cbCurrentCard;

                    cbCardIndexTemp[wChairID][j]++;
                    GetHuType(wChairID, wBankerUser, INVALID_CHAIR, INVALID_CHAIR, cbCurrentCard, cbCardIndexTemp, cbDiscardCard, cbDiscardCount, cbOutCardCount,
                              WeaveItemArray,cbWeaveItemCount, cbListenStatus, bIsTianTing, cbQuanFeng, cbMenFeng, cbLeftCardCount, bGangStatus, szGameRoomName,
                              AnalyseItemArray, pHuCardData->wMultipleCount, LastAnalyseItem, bIsZiMo, CT_TRUE);
                    cbCardIndexTemp[wChairID][j]--;

                    pHuCardData->cbRemainCount = 4-GetCardRemain(wChairID,cbHandCardIndex, cbListenStatus, cbDiscardCard, cbDiscardCount, WeaveItemArray, cbWeaveItemCount, cbCurrentCard);
                    pLCardData->cbHuCardCount++;

                    hasHu = CT_TRUE;
                }
            }

            if (hasHu)
            {
                cbListenCardCount++;
            }

            cbCardIndexTemp[wChairID][i]++;
        }

        return cbListenCardCount>0?WIK_LISTEN:WIK_NULL;
    }

    CT_VOID CGameLogic::GetHuCardData(CT_WORD wChairID, CT_WORD wBankerUser, CT_WORD wOutCardUser, CT_WORD wProvideUser, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
                                   const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount, const CAnalyseItemArray& AnalyseItemArray, const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
                                   const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng, CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
                                   CT_BYTE cbListenCardData[2+MAX_INDEX*4], CT_BYTE cbCard)
    {
        memset(cbListenCardData, 0,(2+MAX_INDEX*4)*sizeof(CT_BYTE));
        tagListenCardData* pLCardData = (tagListenCardData*)cbListenCardData;

        pLCardData->cbOutCard = 0x00;
        pLCardData->cbHuCardCount = 1;

        tagHuCardData* pHuCardData = (tagHuCardData*)(pLCardData->cbHuCardData);
        pHuCardData->cbListenCard = cbCard;

        //复制数据
        CT_BYTE cbCardIndexTemp[GAME_PLAYER][MAX_INDEX];
        memcpy( cbCardIndexTemp,cbHandCardIndex,sizeof(cbCardIndexTemp) );

        CT_BOOL bIsZiMo = CT_FALSE;
        tagAnalyseItem LastAnalyseItem;
        //cbCardIndexTemp[wChairID][SwitchToCardIndex(cbCard)]++;
        GetHuType(wChairID, wBankerUser, wOutCardUser, wProvideUser, cbCard, cbCardIndexTemp, cbDiscardCard, cbDiscardCount, cbOutCardCount,
                  WeaveItemArray,cbWeaveItemCount, cbListenStatus, bIsTianTing, cbQuanFeng, cbMenFeng, cbLeftCardCount, bGangStatus, szGameRoomName,
                  AnalyseItemArray, pHuCardData->wMultipleCount, LastAnalyseItem, bIsZiMo,CT_FALSE);
        //cbCardIndexTemp[wChairID][j]--;

        pHuCardData->cbRemainCount = 4-GetCardRemain(wChairID, cbHandCardIndex, cbListenStatus,cbDiscardCard, cbDiscardCount, WeaveItemArray, cbWeaveItemCount, cbCard);
    }

    CT_BYTE CGameLogic::AnalyseTingCard13Ex( const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, CT_BYTE cbCurrentCardIndex )
    {
        //复制数据
        CT_BYTE cbCardIndexTemp[MAX_INDEX];
        memcpy( cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp) );

        CChiHuRight chr;
        CAnalyseItemArray AnalyseItemArray;

        CT_BYTE cbCurCardData = SwitchToCardData(cbCurrentCardIndex);
        if( WIK_CHI_HU == AnalyseChiHuCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,cbCurCardData,chr,AnalyseItemArray) )
            return WIK_LISTEN;

        return WIK_NULL;
    }

//听牌分析
    CT_BYTE CGameLogic::AnalyseTingCard13( const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount )
    {
        //复制数据
        CT_BYTE cbCardIndexTemp[MAX_INDEX];
        memcpy( cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp) );

        CChiHuRight chr;
        CAnalyseItemArray AnalyseItemArray;

        //暂时屏蔽
       // for( CT_BYTE j = 0; j < MAX_INDEX-MAX_HUA_CARD; j++ )
       //0-8:1-9万 9-15:东南西北中发白
        for(CT_BYTE j = 0 ; j < 16; j++)
        {
            CT_BYTE cbCurrentCard = SwitchToCardData(j);
            if( WIK_CHI_HU == AnalyseChiHuCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,cbCurrentCard,chr,AnalyseItemArray) )
                return WIK_LISTEN;
        }

        return WIK_NULL;
    }

//扑克转换
    CT_BYTE CGameLogic::SwitchToCardData(CT_BYTE cbCardIndex)
    {
        assert(cbCardIndex<MAX_INDEX);
        if( cbCardIndex < 9 )
            return ((cbCardIndex/9)<<4)|(cbCardIndex%9+1);
        else return (0x30|(cbCardIndex%9+1));
    }

//扑克转换
    CT_BYTE CGameLogic::SwitchToCardIndex(CT_BYTE cbCardData)
    {
        //assert(IsValidCard(cbCardData));
        if(cbCardData<=0x09)return (cbCardData-1);
        else return (8+(cbCardData&MASK_VALUE));
    }

//扑克转换
    CT_BYTE CGameLogic::SwitchToCardData(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCardData[MAX_COUNT])
    {
        //memset(cbCardData, 0, sizeof(cbCardData));
        memset(cbCardData, 0, sizeof(CT_BYTE) * MAX_COUNT);
        //转换扑克
        CT_BYTE cbPosition=0;
        /*//钻牌
        if( m_cbMagicIndex != MAX_INDEX )
        {
            for( CT_BYTE i = 0; i < cbCardIndex[m_cbMagicIndex]; i++ )
                cbCardData[cbPosition++] = SwitchToCardData(m_cbMagicIndex);
        }
        for (CT_BYTE i=0;i<MAX_INDEX;i++)
        {
            if( i == m_cbMagicIndex && m_cbMagicIndex != INDEX_REPLACE_CARD )
            {
                //如果财神有代替牌，则代替牌代替财神原来的位置
                if( INDEX_REPLACE_CARD != MAX_INDEX )
                {
                    for( CT_BYTE j = 0; j < cbCardIndex[INDEX_REPLACE_CARD]; j++ )
                        cbCardData[cbPosition++] = SwitchToCardData(INDEX_REPLACE_CARD);
                }
                continue;
            }
            if( i == INDEX_REPLACE_CARD ) continue;
            if (cbCardIndex[i]!=0)
            {
                for (CT_BYTE j=0;j<cbCardIndex[i];j++)
                {
                    assert(cbPosition<MAX_COUNT);
                    cbCardData[cbPosition++]=SwitchToCardData(i);
                }
            }
        }*/

        for(CT_BYTE i = 0; i < MAX_INDEX - MAX_HUA_CARD; i++)
        {
            if(cbCardIndex[i] != 0)
            {
                for (CT_BYTE j=0;j<cbCardIndex[i];j++)
                {
                    assert(cbPosition<MAX_COUNT);
                    cbCardData[cbPosition++]=SwitchToCardData(i);
                }
            }
        }

        return cbPosition;
    }

//扑克转换
    CT_BYTE CGameLogic::SwitchToCardIndex(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbCardIndex[MAX_INDEX])
    {
        //设置变量
        //memset(cbCardIndex,sizeof(CT_BYTE)*MAX_INDEX);
        memset(cbCardIndex,0,sizeof(CT_BYTE)*MAX_INDEX);

        //转换扑克
        for (CT_BYTE i=0;i<cbCardCount;i++)
        {
            assert(IsValidCard(cbCardData[i]));
            cbCardIndex[SwitchToCardIndex(cbCardData[i])]++;
        }

        return cbCardCount;
    }

//分析扑克
    CT_BOOL CGameLogic::AnalyseCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, CAnalyseItemArray & AnalyseItemArray)
    {
        //计算数目
        CT_BYTE cbCardCount=GetCardCount(cbCardIndex);

        //效验数目
        if(cbCardCount < 2 || cbCardCount > MAX_COUNT || (cbCardCount-2)%3 !=0)
        {
            LOG(ERROR) << "AnalyseCard: cbCardCount error:"<< (int)cbCardCount;
        }
        assert((cbCardCount>=2)&&(cbCardCount<=MAX_COUNT)&&((cbCardCount-2)%3==0));
        if ((cbCardCount<2)||(cbCardCount>MAX_COUNT)||((cbCardCount-2)%3!=0))
            return CT_FALSE;

        //变量定义
        CT_BYTE cbKindItemCount=0;
        tagKindItem KindItem[27*2+28];
        memset(KindItem,0,sizeof(KindItem));

        //需求判断
        CT_BYTE cbLessKindItem=(cbCardCount-2)/3;
        if(cbLessKindItem + cbWeaveCount != MAX_WEAVE)
        {
            LOG(ERROR) << " cbLessKindItem: "<< cbLessKindItem << " cbWeaveCount:"<<cbWeaveCount;
        }
        assert((cbLessKindItem+cbWeaveCount)==MAX_WEAVE);

        //单吊判断
        if (cbLessKindItem==0)
        {
            //效验参数
            assert((cbCardCount==2)&&(cbWeaveCount==MAX_WEAVE));

            //牌眼判断
            for (CT_BYTE i=0;i<MAX_INDEX-MAX_HUA_CARD;i++)
            {
                if (cbCardIndex[i]==2 ||
                    ( m_cbMagicIndex != MAX_INDEX && i != m_cbMagicIndex && cbCardIndex[m_cbMagicIndex]+cbCardIndex[i]==2 ) )
                {
                    //变量定义
                    tagAnalyseItem AnalyseItem;
                    memset(&AnalyseItem,0,sizeof(AnalyseItem));

                    //设置结果
                    for (CT_BYTE j=0;j<cbWeaveCount;j++)
                    {
                        AnalyseItem.cbWeaveKind[j]=WeaveItem[j].cbWeaveKind;
                        AnalyseItem.cbCenterCard[j]=WeaveItem[j].cbCenterCard;
                        AnalyseItem.cbPublicCard[j]=WeaveItem[i].cbPublicCard;
                        memcpy( AnalyseItem.cbCardData[j],WeaveItem[j].cbCardData,sizeof(WeaveItem[j].cbCardData) );
                        AnalyseItem.cbWeaveSource[j] = PengChiGangZone;
                    }
                    AnalyseItem.cbCardEye=SwitchToCardData(i);
                    if( cbCardIndex[i] < 2 || i == m_cbMagicIndex )
                        AnalyseItem.bMagicEye = CT_TRUE;
                    else AnalyseItem.bMagicEye = CT_FALSE;

                    //插入结果
                    AnalyseItemArray.Add(AnalyseItem);

                    return CT_TRUE;
                }
            }

            return CT_FALSE;
        }

        CT_BYTE cbIsQiDui = CT_FALSE;
        if (cbLessKindItem == 4 && CheckQiDui(WeaveItem, cbWeaveCount, cbCardIndex)) // 检查下七对
        {
            cbIsQiDui=CT_TRUE;
        }

        //拆分分析
        CT_BYTE cbMagicCardIndex[MAX_INDEX];
        memcpy(cbMagicCardIndex,cbCardIndex,sizeof(cbMagicCardIndex));
        //如果有财神
        CT_BYTE cbMagicCardCount = 0;
        if( m_cbMagicIndex != MAX_INDEX )
        {
            cbMagicCardCount = cbCardIndex[m_cbMagicIndex];
            //如果财神有代替牌，财神与代替牌转换
            if( INDEX_REPLACE_CARD != MAX_INDEX )
            {
                cbMagicCardIndex[m_cbMagicIndex] = cbMagicCardIndex[INDEX_REPLACE_CARD];
                cbMagicCardIndex[INDEX_REPLACE_CARD] = cbMagicCardCount;
            }
        }
        if (cbCardCount>=3)
        {
            for (CT_BYTE i=0;i<MAX_INDEX-MAX_HUA_CARD;i++)
            {
                //同牌判断
                //如果是财神,并且财神数小于3,则不进行组合
                if( cbMagicCardIndex[i] >= 3 || ( cbMagicCardIndex[i]+cbMagicCardCount >= 3 &&
                                                  ( ( INDEX_REPLACE_CARD!=MAX_INDEX && i != INDEX_REPLACE_CARD ) || ( INDEX_REPLACE_CARD==MAX_INDEX && i != m_cbMagicIndex ) ) )
                        )
                {
                    int nTempIndex = cbMagicCardIndex[i];
                    do
                    {
                        assert( cbKindItemCount < CountArray(KindItem) );
                        CT_BYTE cbIndex = i;
                        CT_BYTE cbCenterCard = SwitchToCardData(i);
                        //如果是财神且财神有代替牌,则换成代替牌
                        if( i == m_cbMagicIndex && INDEX_REPLACE_CARD != MAX_INDEX )
                        {
                            cbIndex = INDEX_REPLACE_CARD;
                            cbCenterCard = SwitchToCardData(INDEX_REPLACE_CARD);
                        }
                        KindItem[cbKindItemCount].cbWeaveKind=WIK_PENG;
                        KindItem[cbKindItemCount].cbCenterCard=cbCenterCard;
                        KindItem[cbKindItemCount].cbValidIndex[0] = nTempIndex>0?cbIndex:m_cbMagicIndex;
                        KindItem[cbKindItemCount].cbValidIndex[1] = nTempIndex>1?cbIndex:m_cbMagicIndex;
                        KindItem[cbKindItemCount].cbValidIndex[2] = nTempIndex>2?cbIndex:m_cbMagicIndex;
                        cbKindItemCount++;

                        //如果是财神,则退出
                        if( i == INDEX_REPLACE_CARD || (i == m_cbMagicIndex && INDEX_REPLACE_CARD == MAX_INDEX) )
                            break;

                        nTempIndex -= 3;
                        //如果刚好搭配全部，则退出
                        if( nTempIndex == 0 ) break;

                    }while( nTempIndex+cbMagicCardCount >= 3 );
                }

                //连牌判断
                if ((i<(MAX_INDEX-MAX_HUA_CARD-9))&&((i%9)<7))
                {
                    //只要财神牌数加上3个顺序索引的牌数大于等于3,则进行组合
                    if( cbMagicCardCount+cbMagicCardIndex[i]+cbMagicCardIndex[i+1]+cbMagicCardIndex[i+2] >= 3 )
                    {
                        CT_BYTE cbIndex[3] = { cbMagicCardIndex[i],cbMagicCardIndex[i+1],cbMagicCardIndex[i+2] };
                        int nMagicCountTemp = cbMagicCardCount;
                        CT_BYTE cbValidIndex[3];
                        while( nMagicCountTemp+cbIndex[0]+cbIndex[1]+cbIndex[2] >= 3 )
                        {
                            for( CT_BYTE j = 0; j < CountArray(cbIndex); j++ )
                            {
                                if( cbIndex[j] > 0 )
                                {
                                    cbIndex[j]--;
                                    cbValidIndex[j] = (i+j==m_cbMagicIndex&&INDEX_REPLACE_CARD!=MAX_INDEX)?INDEX_REPLACE_CARD:i+j;
                                }
                                else
                                {
                                    nMagicCountTemp--;
                                    cbValidIndex[j] = m_cbMagicIndex;
                                }
                            }
                            if( nMagicCountTemp >= 0 )
                            {
                                assert( cbKindItemCount < CountArray(KindItem) );
                                KindItem[cbKindItemCount].cbWeaveKind=WIK_LEFT;
                                KindItem[cbKindItemCount].cbCenterCard=SwitchToCardData(i);
                                memcpy( KindItem[cbKindItemCount].cbValidIndex,cbValidIndex,sizeof(cbValidIndex) );
                                cbKindItemCount++;
                            }
                            else break;
                        }
                    }
                }
            }
        }

        //组合分析
        if (cbKindItemCount>=cbLessKindItem)
        {
            //变量定义
            CT_BYTE cbCardIndexTemp[MAX_INDEX];
            memset(cbCardIndexTemp,0,sizeof(cbCardIndexTemp));

            //变量定义
            CT_BYTE cbIndex[MAX_WEAVE];
            for( CT_BYTE i = 0; i < CountArray(cbIndex); i++ )
                cbIndex[i] = i;
            tagKindItem * pKindItem[MAX_WEAVE];
            memset(&pKindItem,0,sizeof(pKindItem));
            tagKindItem KindItemTemp[CountArray(KindItem)];
            if( m_cbMagicIndex != MAX_INDEX )
                memcpy( KindItemTemp,KindItem,sizeof(KindItem) );

            //开始组合
            do
            {
                //设置变量
                memcpy(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));
                cbMagicCardCount = 0;
                if( m_cbMagicIndex != MAX_INDEX )
                {
                    memcpy( KindItem,KindItemTemp,sizeof(KindItem) );
                }

                for (CT_BYTE i=0;i<cbLessKindItem;i++)
                    pKindItem[i]=&KindItem[cbIndex[i]];

                //数量判断
                CT_BOOL bEnoughCard=CT_TRUE;

                for (CT_BYTE i=0;i<cbLessKindItem*3;i++)
                {
                    //存在判断
                    CT_BYTE cbCardIndex=pKindItem[i/3]->cbValidIndex[i%3];
                    if (cbCardIndexTemp[cbCardIndex]==0)
                    {
                        if( m_cbMagicIndex != MAX_INDEX && cbCardIndexTemp[m_cbMagicIndex] > 0 )
                        {
                            cbCardIndexTemp[m_cbMagicIndex]--;
                            pKindItem[i/3]->cbValidIndex[i%3] = m_cbMagicIndex;
                        }
                        else
                        {
                            bEnoughCard=CT_FALSE;
                            break;
                        }
                    }
                    else cbCardIndexTemp[cbCardIndex]--;
                }

                //胡牌判断
                if (bEnoughCard==CT_TRUE)
                {
                    //牌眼判断
                    CT_BYTE cbCardEye=0;
                    CT_BOOL bMagicEye = CT_FALSE;
                    if( GetCardCount(cbCardIndexTemp) == 2 )
                    {
                        for (CT_BYTE i=0;i<MAX_INDEX-MAX_HUA_CARD;i++)
                        {
                            if (cbCardIndexTemp[i]==2)
                            {
                                cbCardEye=SwitchToCardData(i);
                                if( m_cbMagicIndex != MAX_INDEX && i == m_cbMagicIndex ) bMagicEye = CT_TRUE;
                                break;
                            }
                            else if( i!=m_cbMagicIndex &&
                                     m_cbMagicIndex != MAX_INDEX && cbCardIndexTemp[i]+cbCardIndexTemp[m_cbMagicIndex]==2 )
                            {
                                cbCardEye = SwitchToCardData(i);
                                bMagicEye = CT_TRUE;
                            }
                        }
                    }

                    //组合类型
                    if (cbCardEye!=0)
                    {
                        //变量定义
                        tagAnalyseItem AnalyseItem;
                        memset(&AnalyseItem,0,sizeof(AnalyseItem));

                        //设置组合
                        for (CT_BYTE i=0;i<cbWeaveCount;i++)
                        {
                            AnalyseItem.cbWeaveKind[i]=WeaveItem[i].cbWeaveKind;
                            AnalyseItem.cbCenterCard[i]=WeaveItem[i].cbCenterCard;
                            GetWeaveCard( WeaveItem[i].cbWeaveKind,WeaveItem[i].cbCenterCard,AnalyseItem.cbCardData[i] );
                            AnalyseItem.cbWeaveSource[i] = PengChiGangZone;
                        }

                        //设置牌型
                        for (CT_BYTE i=0;i<cbLessKindItem;i++)
                        {
                            AnalyseItem.cbWeaveKind[i+cbWeaveCount]=pKindItem[i]->cbWeaveKind;
                            AnalyseItem.cbCenterCard[i+cbWeaveCount]=pKindItem[i]->cbCenterCard;
                            AnalyseItem.cbCardData[cbWeaveCount+i][0] = SwitchToCardData(pKindItem[i]->cbValidIndex[0]);
                            AnalyseItem.cbCardData[cbWeaveCount+i][1] = SwitchToCardData(pKindItem[i]->cbValidIndex[1]);
                            AnalyseItem.cbCardData[cbWeaveCount+i][2] = SwitchToCardData(pKindItem[i]->cbValidIndex[2]);
                            AnalyseItem.cbWeaveSource[i+cbWeaveCount] = HandCardZone;
                        }

                        //设置牌眼
                        AnalyseItem.cbCardEye=cbCardEye;
                        AnalyseItem.bMagicEye = bMagicEye;
                        AnalyseItem.cbIsQiDui=cbIsQiDui;

                        //插入结果
                        AnalyseItemArray.Add(AnalyseItem);
                    }
                }

                //设置索引
                if (cbIndex[cbLessKindItem-1]==(cbKindItemCount-1))
                {
                    int i=cbLessKindItem-1;
                    for (;i>0;i--)
                    {
                        if ((cbIndex[i-1]+1)!=cbIndex[i])
                        {
                            CT_BYTE cbNewIndex=cbIndex[i-1];
                            for (CT_BYTE j=(i-1);j<cbLessKindItem;j++)
                                cbIndex[j]=cbNewIndex+j-i+2;
                            break;
                        }
                    }
                    if (i==0)
                        break;
                }
                else
                    cbIndex[cbLessKindItem-1]++;
            } while (CT_TRUE);

        }

        //七对
        if (AnalyseItemArray.GetCount()==0 && cbIsQiDui==CT_TRUE)
        {
            //变量定义
            tagAnalyseItem AnalyseItem;
            memset(&AnalyseItem,0,sizeof(AnalyseItem));

            AnalyseItem.cbIsQiDui = cbIsQiDui;
            //插入结果
            AnalyseItemArray.Add(AnalyseItem);
        }

        return (AnalyseItemArray.GetCount()>0);
    }

//钻牌
    CT_BOOL CGameLogic::IsMagicCard( CT_BYTE cbCardData )
    {
        if( m_cbMagicIndex != MAX_INDEX )
            return SwitchToCardIndex(cbCardData) == m_cbMagicIndex;
        return CT_FALSE;
    }

//花牌判断
    CT_BOOL CGameLogic::IsHuaCard( CT_BYTE cbCardData )
    {
        assert( IsValidCard(cbCardData) );

        return cbCardData>=0x38?CT_TRUE:CT_FALSE;
    }

//排序,根据牌值排序
    CT_BOOL CGameLogic::SortCardList( CT_BYTE cbCardData[MAX_COUNT], CT_BYTE cbCardCount )
    {
        //数目过虑
        if (cbCardCount==0||cbCardCount>MAX_COUNT) return CT_FALSE;

        //排序操作
        CT_BOOL bSorted=CT_TRUE;
        CT_BYTE cbSwitchData=0;
        int cbLast=std::min(cbCardCount-1, MAX_COUNT-1);
        do
        {
            bSorted=CT_TRUE;
            for (CT_BYTE i=0;i<cbLast;i++)
            {
                if (cbCardData[i]>cbCardData[i+1])
                {
                    //设置标志
                    bSorted=CT_FALSE;

                    //扑克数据
                    cbSwitchData=cbCardData[i];
                    cbCardData[i]=cbCardData[i+1];
                    cbCardData[i+1]=cbSwitchData;
                }
            }
            cbLast--;
        } while(bSorted==CT_FALSE);

        return CT_TRUE;
    }

//////////////////////////////////////////////////////////////////////////
//取得手牌索引
    CT_BYTE CGameLogic::GetHandcardIndex(const tagAnalyseItem& AnalyseItem, CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        CT_BYTE cbHandcardCount = 0;
        memset(cbHandcardIndex, 0, MAX_INDEX*sizeof(CT_BYTE));
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            for (CT_BYTE j = 0; j < 4; ++j)
            {
                if (AnalyseItem.cbCardData[i][j] == 0x00)
                {
                    continue;
                }
                cbHandcardIndex[SwitchToCardIndex(AnalyseItem.cbCardData[i][j])] += 1;
                cbHandcardCount += 1;
            }
        }

        if (AnalyseItem.cbCardEye != 0x00)
        {
            CT_BYTE cbCardIndex = SwitchToCardIndex(AnalyseItem.cbCardEye);
            cbHandcardIndex[cbCardIndex] += 2;
            cbHandcardCount += 2;
        }

       // CString str = _T("取得手牌索引 ");
        //CString FunctionName;
        //GetFunctionName;
        //str.AppendFormat(_T("麻将:0x%x cbHandcardCount:%d"), AnalyseItem.cbCardEye, cbHandcardCount);
        if (cbHandcardCount < MAX_COUNT)
        {
            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                //str.AppendFormat(_T("\r\n动作:0x%x "), AnalyseItem.cbWeaveKind[i]);
                for (CT_BYTE j = 0; j < 4; ++j)
                {
                    if (AnalyseItem.cbCardData[i][j] == 0x00)
                    {
                        continue;
                    }
                    //str.AppendFormat(_T("0x%x "), AnalyseItem.cbCardData[i][j]);
                }
            }
           // PrintLog(str, _T("二人麻将|分析牌"));
        }

        return cbHandcardCount;
    }

//是否清一色
    CT_BOOL CGameLogic::IsQingYiSe(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        for (CT_BYTE i = 9; i < MAX_INDEX; ++i)
        {
            if (cbHandcardIndex[i] > 0)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//大四喜
    CT_BOOL CGameLogic::CheckDaSiXi(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
            {
                continue;
            }

            if (AnalyseItem.cbCenterCard[i] == 0x31 || AnalyseItem.cbCenterCard[i] == 0x32 || AnalyseItem.cbCenterCard[i] == 0x33 || AnalyseItem.cbCenterCard[i] == 0x34)
            {
                ++count;
            }
        }

        return count==4;
    }

//大三元
    CT_BOOL CGameLogic::CheckDaSanYuan(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
            {
                continue;
            }

            if (AnalyseItem.cbCenterCard[i] == 0x35 || AnalyseItem.cbCenterCard[i] == 0x36 || AnalyseItem.cbCenterCard[i] == 0x37)
            {
                ++count;
            }
        }

        return count==3;
    }

//九莲宝灯
    CT_BOOL CGameLogic::CheckJiuLianBaoDeng(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        CT_BYTE count = cbHandcardIndex[0]+cbHandcardIndex[8];
        if (cbHandcardIndex[0] < 3 || cbHandcardIndex[8] < 3 || count < 6 || count >= 8)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 1; i < 8; ++i)
        {
            if (cbHandcardIndex[i] == 0)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//四杠
    CT_BOOL CGameLogic::CheckSiGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount != 4)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind != WIK_GANG)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//连七对
    CT_BOOL CGameLogic::CheckLianQiDui(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (cbItemCount != 0)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        CT_BYTE n = 0xFF;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if(cbHandcardIndex[i] == 2)
            {
                if (n == 0xFF)
                {
                    ++count;
                    n = i;
                    continue;
                }

                if (i-n != 1)
                {
                    return CT_FALSE;
                }
                n = i;
                ++count;
            }
        }

        return count==7;
    }

//天和
    CT_BOOL CGameLogic::CheckTianHu(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
        {
            if (cbHandcardIndex[i] == 4)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//百万石
    CT_BOOL CGameLogic::CheckBaiWanShi(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            count += cbHandcardIndex[i]*(i+1);
        }

        return count >= 100;
    }

//小四喜
    CT_BOOL CGameLogic::CheckXiaoSiXi(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
            {
                continue;
            }

            if (AnalyseItem.cbCenterCard[i] == 0x31 || AnalyseItem.cbCenterCard[i] == 0x32 || AnalyseItem.cbCenterCard[i] == 0x33 || AnalyseItem.cbCenterCard[i] == 0x34)
            {
                ++count;
            }
        }

        return count==4||(AnalyseItem.cbCardEye>=0x31&&AnalyseItem.cbCardEye<=0x34&&count==3);
    }

//小三元
    CT_BOOL CGameLogic::CheckXiaoSanYuan(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
            {
                continue;
            }

            if (AnalyseItem.cbCenterCard[i] == 0x35 || AnalyseItem.cbCenterCard[i] == 0x36 || AnalyseItem.cbCenterCard[i] == 0x37)
            {
                ++count;
            }
        }

        return count==3||(AnalyseItem.cbCardEye>=0x35&&count==2);
    }

//字一色
    CT_BOOL CGameLogic::CheckZiYiSe(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if (cbHandcardIndex[i] > 0)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//四暗刻
    CT_BOOL CGameLogic::CheckSiAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                ++count;
            }
        }

        for (CT_BYTE i = cbItemCount; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] == WIK_PENG)
            {
                ++count;
            }
        }

        return count==4;
    }

//一色双龙会
    CT_BOOL CGameLogic::CheckYiSeShuangLong(const tagAnalyseItem& AnalyseItem)
    {
        if (AnalyseItem.cbCardEye != 0x05)
        {
            return CT_FALSE;
        }

        CT_BYTE cbHas1 = 0;
        CT_BYTE cbHas7 = 0;
        CT_BYTE cbCards[3];
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                return CT_FALSE;
            }

            memset(cbCards, 0,sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);
            if (cbCards[0] != 0x01 && cbCards[0] != 0x07)
            {
                return CT_FALSE;
            }

            if (cbCards[0]==0x01)
            {
                cbHas1 += 1;
                if (cbHas1 > 2)
                {
                    return CT_FALSE;
                }
            }
            else if (cbCards[0]==0x07)
            {
                cbHas7 += 1;
                if (cbHas7 > 2)
                {
                    return CT_FALSE;
                }
            }
        }

        return cbHas1==2&&cbHas7==2;
    }

//一色四同顺
    CT_BOOL CGameLogic::CheckYiSeSiTongShun(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE card = 0x00;
        CT_BYTE cbCards[3];
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                return CT_FALSE;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);
            if (card == 0x00)
            {
                card = cbCards[0];
                continue;
            }

            if (cbCards[0] != card)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//一色四节高
    CT_BOOL CGameLogic::CheckYiSeSiJieGao(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG) || AnalyseItem.cbCenterCard[i] >= 0x31)
            {
                return CT_FALSE;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            count++;
            cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
        }

        if (count < 4)
        {
            return CT_FALSE;
        }

        CT_BYTE n = 0xFF;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if (cbCardIndex[i] == 0)
            {
                //断链了
/*                if (n != 0xFF)
                {
                    return CT_FALSE;
                }*/
                continue;
            }

            if (n == 0xFF)
            {
                n = i;
                continue;
            }

            if (i-n != 1)
            {
                return CT_FALSE;
            }
            n = i;
        }

        return CT_TRUE;
    }

//一色四步高
    CT_BOOL CGameLogic::CheckYiSeSiBuGao(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                return CT_FALSE;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);
            count++;
            cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
        }

        if (count < 4)
        {
            return CT_FALSE;
        }

        count = 0;
        //CT_BYTE def = 0xFF;
        CT_BYTE n = 0xFF;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if (cbCardIndex[i] == 0)
            {
                continue;
            }

            if (n == 0xFF)
            {
                count++;
                n = i;
                continue;
            }

  /*          if (def == 0xFF)
            {
                def = i-n;
            }*/

            if (i-n != 1 && i-n != 2)
            {
                return CT_FALSE;
            }
            n = i;
            count++;
        }

        return count==4;
    }

//三杠
    CT_BOOL CGameLogic::CheckSanGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount < 3)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG)
            {
                ++count;
            }
        }

        return count==3;
    }

//混幺九
    CT_BOOL CGameLogic::CheckHunYaoJiu(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (AnalyseItem.cbIsQiDui)
        {
            for (CT_BYTE i = 1; i < 8; ++i)
            {
                if (cbHandcardIndex[i] > 0)
                {
                    return CT_FALSE;
                }
            }
        }
        else
        {
            if (AnalyseItem.cbCardEye > 0x01 && AnalyseItem.cbCardEye < 0x09)
            {
                return CT_FALSE;
            }

            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
                {
                    return CT_FALSE;
                }

                if (AnalyseItem.cbCenterCard[i] > 0x01 && AnalyseItem.cbCenterCard[i] < 0x09)
                {
                    return CT_FALSE;
                }
            }
        }

        return CT_TRUE;
    }

//七对
    CT_BOOL CGameLogic::CheckQiDui(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (cbItemCount != 0)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
        {
            if (cbHandcardIndex[i]%2 != 0)
            {
                return CT_FALSE;
            }

            count += cbHandcardIndex[i]/2;
        }

        return count==7;
    }

//一色三同顺
    CT_BOOL CGameLogic::CheckYiSeSanTongShun(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE cbIndex = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            cbIndex = SwitchToCardIndex(cbCards[0]);
            cbCardIndex[cbIndex]++;
            if (cbCardIndex[cbIndex]==3)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//一色三节高
    CT_BOOL CGameLogic::CheckYiSeSanJieGao(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG) || AnalyseItem.cbCenterCard[i] >= 0x31)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            count++;
            cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
        }

        if (count < 3)
        {
            return CT_FALSE;
        }
        count = 0;
        CT_BYTE n = 0xFF;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if (cbCardIndex[i] == 0)
            {
                n = 0xFF;
                continue;
            }

            if (n == 0xFF)
            {
                n = i;
                count = 1;
                continue;
            }

            if (i-n != 1)
            {
                n = i;
                count = 1;
                continue;
            }
            n = i;
            ++count;
            if(count==3)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//清龙
    CT_BOOL CGameLogic::CheckQingLong(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex,0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards,0,  sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
        }

        return cbCardIndex[0]>0&&cbCardIndex[3]>0&&cbCardIndex[6]>0;
    }

//一色三步高
    CT_BOOL CGameLogic::CheckYiSeSanBuGao(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            count++;
            cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
        }

        if (count < 3)
        {
            return CT_FALSE;
        }

        CT_BYTE def = 0xFF;
        CT_BYTE n = 0xFF;
        count = 0;
        for (CT_BYTE i = 0; i < 9; ++i)
        {
            if (cbCardIndex[i] == 0)
            {
                continue;
            }

            if (n == 0xFF)
            {
                n = i;
                count = 1;
                continue;
            }

            if (def == 0xFF)
            {
                def = i-n;
            }

            if (def > 2 || def == 0 || i-n != def)
            {
                def = i-n;
                n = i;
                count = 2;
                continue;
            }
            n = i;
            ++count;
            if(count==3)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//三暗刻
    CT_BOOL CGameLogic::CheckSanAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                ++count;
            }
        }

        for (CT_BYTE i = cbItemCount; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] == WIK_PENG)
            {
                ++count;
            }
        }

        return count==3;
    }

//大于5
    CT_BOOL CGameLogic::CheckDaYu5(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (AnalyseItem.cbIsQiDui)
        {
            CT_BYTE count = 0;
            for (CT_BYTE i = 5; i < 9; ++i)
            {
                count += cbHandcardIndex[i];
            }

            return count==MAX_COUNT;
        }
        else
        {
            if (AnalyseItem.cbCardEye < 0x06 || AnalyseItem.cbCardEye > 0x09)
            {
                return CT_FALSE;
            }

            CT_BYTE cbCards[3];
            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                memset(cbCards, 0, sizeof(cbCards));
                memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
                SortCardList(cbCards, 3);

                if (cbCards[0] < 0x06 || cbCards[0] > 0x09)
                {
                    return CT_FALSE;
                }
            }
        }

        return CT_TRUE;
    }

//小于5
    CT_BOOL CGameLogic::CheckXiaoYu5(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (AnalyseItem.cbIsQiDui)
        {
            CT_BYTE cbHandCardData[MAX_COUNT];
            memset(cbHandCardData, 0, sizeof(cbHandCardData));
            SwitchToCardData(cbHandcardIndex, cbHandCardData);

            for(int i= 0; i < MAX_COUNT; i++)
            {
                if(cbHandCardData[i] >= 0x05)
                {
                    return CT_FALSE;
                }
            }

            return CT_TRUE;
        }
        else
        {
            if (AnalyseItem.cbCardEye >= 0x05)
            {
                return CT_FALSE;
            }

            CT_BYTE cbCards[3];
            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                memset(cbCards,0,  sizeof(cbCards));
                memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
                SortCardList(cbCards, 3);

                if (cbCards[0] >= 0x05)
                {
                    return CT_FALSE;
                }
            }
        }

        return CT_TRUE;
    }

//三风刻
    CT_BOOL CGameLogic::CheckSanFengKe(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        /*for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG) || AnalyseItem.cbCenterCard[i] < 0x31 || AnalyseItem.cbCenterCard[i] > 0x34)
            {
                return CT_FALSE;
            }

            ++count;
        }*/
        for(CT_BYTE i = 0; i < MAX_WEAVE; i++)
        {
            if(AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG)
            {
                if(AnalyseItem.cbCenterCard[i] >= 0x31 && AnalyseItem.cbCenterCard[i] <= 0x34)
                {
                    count++;
                }
            }
        }

        return count==3;
    }

//碰碰胡
    CT_BOOL CGameLogic::CheckPengPengHu(const tagAnalyseItem& AnalyseItem)
    {
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//全求人
    CT_BOOL CGameLogic::CheckQuanQiuRen(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount != 4)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//双暗杠
    CT_BOOL CGameLogic::CheckShuangAnGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount < 2)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                ++count;
            }
        }

        return count==2;
    }

//双箭刻
    CT_BOOL CGameLogic::CheckShuangJianKe(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] != WIK_PENG && AnalyseItem.cbWeaveKind[i] != WIK_GANG) || AnalyseItem.cbCenterCard[i] < 0x35)
            {
                continue;
            }

            ++count;
        }

        return count==2;
    }

//全带幺
    CT_BOOL CGameLogic::CheckQuanDaiYao(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        //杠牌 刻字 顺子 将牌 带有1万或9万
        if(AnalyseItem.cbIsQiDui)
        {
            //七对肯定不是全带幺
            return CT_FALSE;
        }

        if(AnalyseItem.cbCardEye != 0x01 && AnalyseItem.cbCardEye != 0x09)
        {
            //将牌必须是1万或者9万
            return CT_FALSE;
        }

        CT_BYTE cbCards[3];
        CT_BOOL bFind = CT_FALSE;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            bFind = CT_FALSE;
            for(int k = 0; k < 3; k++)
            {
                if(cbCards[k] == 0x01 || cbCards[k] == 0x09)
                {
                    bFind = CT_TRUE;
                    break;
                }
            }

            if(bFind == CT_FALSE)
            {
                return CT_FALSE;
            }
        }

        /*if (AnalyseItem.cbIsQiDui)
        {
            for (CT_BYTE i = 1; i < 8; ++i)
            {
                if (cbHandcardIndex[i] > 0)
                {
                    return CT_FALSE;
                }
            }
        }
        else
        {
            if (AnalyseItem.cbCardEye > 0x01 && AnalyseItem.cbCardEye < 0x09)
            {
                return CT_FALSE;
            }

            CT_BYTE cbCards[3];
            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                if (AnalyseItem.cbCenterCard[i] >= 0x31)
                {
                    continue;
                }

                memset(cbCards, 0, sizeof(cbCards));
                memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
                SortCardList(cbCards, 3);

                if ((cbCards[0] == 0x07 || cbCards[0] == 0x01) && (AnalyseItem.cbWeaveKind[i] == WIK_LEFT || AnalyseItem.cbWeaveKind[i] == WIK_CENTER || AnalyseItem.cbWeaveKind[i] == WIK_RIGHT))
                {
                    continue;
                }

                if (AnalyseItem.cbCenterCard[i] != 0x01 && AnalyseItem.cbCenterCard[i] != 0x09)
                {
                    return CT_FALSE;
                }
            }
        }*/

        return CT_TRUE;
    }

//不求人
    CT_BOOL CGameLogic::CheckBuQiuRen(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbPublicCard == CT_TRUE)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//双明杠
    CT_BOOL CGameLogic::CheckShuangMingGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount < 2)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_TRUE)
            {
                ++count;
            }
        }

        return count==2;
    }

//取牌剩余张数
    CT_BYTE CGameLogic::GetCardRemain(CT_BYTE cbChairID, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX], const CT_BYTE bTing[GAME_PLAYER],const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER], CT_BYTE cbCard)
    {
        CT_BYTE cbCardIndexTmp[MAX_INDEX];
        memset(cbCardIndexTmp, 0, sizeof(cbCardIndexTmp));
        memcpy(cbCardIndexTmp, cbHandCardIndex[cbChairID], sizeof(cbCardIndexTmp));

        CT_BYTE cbCardIndexOther[MAX_INDEX];
        memset(cbCardIndexOther, 0, sizeof(cbCardIndexOther));
        for(int k = 0; k < GAME_PLAYER; k++)
        {
            if(k == cbChairID)
            {
                continue;
            }

            if(bTing[k] == 1)
            {
                //另外一个玩家听牌了
                memcpy(cbCardIndexOther, cbHandCardIndex[k], sizeof(cbCardIndexOther));
            }
        }

        for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
        {
            for (CT_BYTE j = 0; j < cbDiscardCount[i]; ++j)
            {
                if (cbDiscardCard[i][j] == 0x00)
                {
                    break;
                }

                cbCardIndexTmp[SwitchToCardIndex(cbDiscardCard[i][j])]++;
            }

            for (CT_BYTE j = 0; j < cbWeaveItemCount[i]; ++j)
            {
                for (CT_BYTE m = 0; m < 4; ++m)
                {
                    if (WeaveItemArray[i][j].cbCardData[m] == 0x00)
                    {
                        break;
                    }

                    cbCardIndexTmp[SwitchToCardIndex(WeaveItemArray[i][j].cbCardData[m])]++;
                }
            }
        }

        return cbCardIndexTmp[SwitchToCardIndex(cbCard)] + cbCardIndexOther[SwitchToCardIndex(cbCard)];
    }

//门前清
    CT_BOOL CGameLogic::CheckMenQianQing(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbPublicCard == CT_TRUE)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//箭刻
    CT_BOOL CGameLogic::CheckJianKe(const tagAnalyseItem& AnalyseItem)
    {
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG) && AnalyseItem.cbCenterCard[i] >= 0x35)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//圈风刻
    CT_BOOL CGameLogic::CheckQuanFengKe(const tagAnalyseItem& AnalyseItem, CT_BYTE cbQuanFendIndex)
    {
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG) && AnalyseItem.cbCenterCard[i] == 0x31+cbQuanFendIndex)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//门风刻
    CT_BOOL CGameLogic::CheckMenFengKe(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard)
    {
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG) && AnalyseItem.cbCenterCard[i] == cbCard)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//平胡
    CT_BOOL CGameLogic::CheckPingHu(const tagAnalyseItem& AnalyseItem)
    {
        //平胡是用万字牌做将
        if (AnalyseItem.cbCardEye >= 0x31)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//四归一
    CT_BOOL CGameLogic::CheckSiGuiYi(const CT_BYTE cbHandcardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        std::vector<CT_BYTE> vec;
        for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
        {
            if (cbHandcardIndex[i] == 4)
            {
                vec.push_back(SwitchToCardData(i));
            }
        }

        if (vec.size() == 0)
        {
            return CT_FALSE;
        }

        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && std::count(vec.begin(), vec.end(), WeaveItem[i].cbCenterCard) != 0)
            {
                ++count;
            }
        }

        return vec.size()!=count;
    }

//双暗刻
    CT_BOOL CGameLogic::CheckShuangAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE count = 0;
        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                ++count;
            }
        }

        for (CT_BYTE i = cbItemCount; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] == WIK_PENG)
            {
                ++count;
            }
        }

        return count==2;
    }

//暗杠
    CT_BOOL CGameLogic::CheckAnGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount < 1)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_FALSE)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//断幺
    CT_BOOL CGameLogic::CheckDuanYao(const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (cbHandcardIndex[0] > 0 || cbHandcardIndex[8] > 0)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 9; i < MAX_INDEX; ++i)
        {
            if (cbHandcardIndex[i] > 0)
            {
                return CT_FALSE;
            }
        }

        return CT_TRUE;
    }

//258将
    CT_BOOL CGameLogic::Check258Jiang(const CAnalyseItemArray& AnalyseItemArray)
    {
        if (AnalyseItemArray.GetCount() < 3)
        {
            return CT_FALSE;
        }

        CT_BOOL bHas2 = CT_FALSE;
        CT_BOOL bHas5 = CT_FALSE;
        CT_BOOL bHas8 = CT_FALSE;
        for (CT_BYTE i = 0; i < AnalyseItemArray.GetCount(); ++i)
        {
            if (AnalyseItemArray[i].cbCardEye==0x02)
            {
                bHas2 = CT_TRUE;
            }
            else if (AnalyseItemArray[i].cbCardEye==0x05)
            {
                bHas5 = CT_TRUE;
            }
            else if (AnalyseItemArray[i].cbCardEye==0x08)
            {
                bHas8 = CT_TRUE;
            }

            if (bHas2 && bHas5 && bHas8)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//幺九头
    CT_BOOL CGameLogic::CheckYaoJiuTou(const tagAnalyseItem& AnalyseItem)
    {
        return AnalyseItem.cbCardEye==0x01 || AnalyseItem.cbCardEye==0x09;
        /*if (AnalyseItemArray.GetCount() < 2)
        {
            return CT_FALSE;
        }

        CT_BOOL bHas1 = CT_FALSE;
        CT_BOOL bHas9 = CT_FALSE;
        for (CT_BYTE i = 0; i < AnalyseItemArray.GetCount(); ++i)
        {
            if (AnalyseItemArray[i].cbCardEye==0x01)
            {
                bHas1 = CT_TRUE;
            }
            else if (AnalyseItemArray[i].cbCardEye==0x09)
            {
                bHas9 = CT_TRUE;
            }

            if (bHas1 && bHas9)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;*/
    }

//一般高
    CT_BOOL CGameLogic::CheckYiBanGao(const tagAnalyseItem& AnalyseItem)
    {
        CT_BYTE cbIndex = 0;
        CT_BYTE cbCards[3];
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex,0, sizeof(cbCardIndex));

        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            cbIndex = SwitchToCardIndex(cbCards[0]);
            cbCardIndex[cbIndex]++;
            if (cbCardIndex[cbIndex] >= 2)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//连六
    CT_BOOL CGameLogic::CheckLianLiu(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX])
    {
        if (AnalyseItem.cbIsQiDui)
        {
            CT_BYTE n = 0xFF;
            CT_BYTE count = 0;
            for (CT_BYTE i = 0; i < 9; ++i)
            {
                if (cbHandcardIndex[i] == 0)
                {
                    n = 0xFF;
                    continue;
                }

                if (n == 0xFF)
                {
                    n = i;
                    count = 1;
                    continue;
                }

                if (i-n == 1)
                {
                    n = i;
                    ++count;
                }
            }

            return count==6;
        }
        else
        {
            CT_BYTE cbCards[3];
            CT_BYTE cbCardIndex[MAX_INDEX];
            memset(cbCardIndex, 0,sizeof(cbCardIndex));
            for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
            {
                if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
                {
                    continue;
                }

                memset(cbCards, 0, sizeof(cbCards));
                memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
                SortCardList(cbCards, 3);

                cbCardIndex[SwitchToCardIndex(cbCards[0])]++;
            }

            if ((cbCardIndex[0]>=1&&cbCardIndex[3]>=1) ||
                (cbCardIndex[1]>=1&&cbCardIndex[4]>=1) ||
                (cbCardIndex[2]>=1&&cbCardIndex[5]>=1) ||
                (cbCardIndex[3]>=1&&cbCardIndex[6]>=1))
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//老少副
    CT_BOOL CGameLogic::CheckLaoShaoFu(const tagAnalyseItem& AnalyseItem)
    {
        CT_BOOL bHas1 = CT_FALSE;
        CT_BOOL bHas7 = CT_FALSE;
        CT_BYTE cbCards[3];
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            if (cbCards[0] == 0x01)
            {
                bHas1 = CT_TRUE;
            }
            else if (cbCards[0] == 0x07)
            {
                bHas7 = CT_TRUE;
            }

            if (bHas1 && bHas7)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//幺九刻
    CT_BOOL CGameLogic::CheckYaoJiuKe(const tagAnalyseItem& AnalyseItem)
    {
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if ((AnalyseItem.cbWeaveKind[i] == WIK_PENG || AnalyseItem.cbWeaveKind[i] == WIK_GANG) &&
                (AnalyseItem.cbCenterCard[i] == 0x01 || AnalyseItem.cbCenterCard[i] >= 0x09))
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//明杠
    CT_BOOL CGameLogic::CheckMingGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount)
    {
        if (cbItemCount < 1)
        {
            return CT_FALSE;
        }

        for (CT_BYTE i = 0; i < cbItemCount; ++i)
        {
            if (WeaveItem[i].cbWeaveKind == WIK_GANG && WeaveItem[i].cbPublicCard == CT_TRUE)
            {
                return CT_TRUE;
            }
        }

        return CT_FALSE;
    }

//边张
    CT_BOOL CGameLogic::CheckBianZhang(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard)
    {
        if (cbCard != 0x03 && cbCard != 0x07)
        {
            return CT_FALSE;
        }

        CT_BYTE cbCards[3];
        CT_BOOL bHasCard = CT_FALSE;
        CT_BOOL bHasCardNo = CT_FALSE;
        //检查牌
        CT_BYTE cbCardNo = (cbCard==0x03)?cbCard:0x05;
        cbCard = (cbCard==0x03)?0x01:cbCard;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT && AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT)
            {
                continue;
            }

            memset(cbCards, 0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            if (cbCards[0] == cbCard)
            {
                bHasCard = CT_TRUE;
            } else if (cbCards[0] == cbCardNo)
            {
                bHasCardNo = CT_TRUE;
            }
        }

        return bHasCard && !bHasCardNo;
    }

//坎张
    CT_BOOL CGameLogic::CheckKanZhang(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard)
    {
        if (cbCard == 0x01 || cbCard >= 0x09)
        {
            return CT_FALSE;
        }

        CT_BYTE cbCards[3];
        CT_BOOL bHasCard = CT_FALSE;
        CT_BOOL bHasCardNo1 = CT_FALSE;
        CT_BOOL bHasCardNo2 = CT_FALSE;
        //检查牌
        CT_BYTE cbCardNo1 = cbCard;
        CT_BYTE cbCardNo2 = 0x00;
        if (cbCard-2 > 0x00)
        {
            cbCardNo2 = cbCard-2;
        }
        cbCard = cbCard-1;
        for (CT_BYTE i = 0; i < MAX_WEAVE; ++i)
        {
            //胡坎张一定是手牌中能胡，所以要排除组合区中的中吃
            if (AnalyseItem.cbWeaveKind[i] != WIK_LEFT || AnalyseItem.cbWeaveSource[i] == PengChiGangZone/*&& AnalyseItem.cbWeaveKind[i] != WIK_CENTER && AnalyseItem.cbWeaveKind[i] != WIK_RIGHT*/)
            {
                //胡中间那张一定是手牌中的左吃
                continue;
            }

            memset(cbCards,0, sizeof(cbCards));
            memcpy(cbCards, AnalyseItem.cbCardData[i], sizeof(cbCards));
            SortCardList(cbCards, 3);

            if (cbCards[0] == cbCard)
            {
                bHasCard = CT_TRUE;
            } else if (cbCards[0] == cbCardNo1)
            {
                bHasCardNo1 = CT_TRUE;
            } else if (cbCards[0] == cbCardNo2)
            {
                bHasCardNo2 = CT_TRUE;
            }
        }

        return bHasCard && !bHasCardNo1 && !bHasCardNo2;
    }

    CT_BOOL CGameLogic::CheckDanDiaoJiang(const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX], CT_BYTE cbCard)
    {
        if (AnalyseItem.cbCardEye != cbCard)
        {
            return CT_FALSE;
        }

        // 大神的只要胡的是麻将牌就是单调将

        //if (cbCard >= 0x31)
        //{
        //	return CT_TRUE;
        //}

        ////复制数据
        //CT_BYTE cbCardIndexTemp[MAX_INDEX];
        //memcpy( cbCardIndexTemp,cbHandcardIndex,sizeof(cbCardIndexTemp) );

        //CChiHuRight chr;
        //CAnalyseItemArray AnalyseItemArray;
        //CT_BYTE cbCardTmp = 0x00;
        //CT_BYTE cbCardIndex = SwitchToCardIndex(cbCard);
        //cbCardIndexTemp[cbCardIndex]--;

        //for( int i = max(0,cbCard-3); i < min(cbCard+3,9); i++ )
        //{
        //	if (i == cbCardIndex)
        //	{
        //		continue;
        //	}

        //	cbCardTmp = SwitchToCardData(i);
        //	if( WIK_CHI_HU == AnalyseChiHuCard(cbCardIndexTemp,WeaveItem,cbWeaveCount,cbCardTmp,chr,AnalyseItemArray) )
        //	{
        //		return CT_FALSE;
        //	}
        //}

        return CT_TRUE;
    }

//分析扑克
    CT_BYTE CGameLogic::AnalyseGoodCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE& cbFengCardCount)
    {
        cbFengCardCount = 0;
        CAnalyseItemArray AnalyseItemArray;

        //计算数目
        CT_BYTE cbCardCount=GetCardCount(cbCardIndex);

        //效验数目
        assert((cbCardCount>=2)&&(cbCardCount<=MAX_COUNT)&&((cbCardCount-2)%3==0));
        if ((cbCardCount<2)||(cbCardCount>MAX_COUNT)||((cbCardCount-2)%3!=0))
            return CT_FALSE;

        //变量定义
        CT_BYTE cbKindItemCount=0;
        tagKindItem KindItem[27*2+28];
        memset(KindItem,0,sizeof(KindItem));

        //需求判断
        CT_BYTE cbLessKindItem=(cbCardCount-2)/3;

        if (cbLessKindItem == 4 && CheckQiDui(NULL, 0, cbCardIndex)) // 检查下七对
        {
            return 4;
        }

        //拆分分析
        CT_BYTE cbMagicCardIndex[MAX_INDEX];
        memcpy(cbMagicCardIndex,cbCardIndex,sizeof(cbMagicCardIndex));
        //如果有财神
        CT_BYTE cbMagicCardCount = 0;
        if( m_cbMagicIndex != MAX_INDEX )
        {
            cbMagicCardCount = cbCardIndex[m_cbMagicIndex];
            //如果财神有代替牌，财神与代替牌转换
            if( INDEX_REPLACE_CARD != MAX_INDEX )
            {
                cbMagicCardIndex[m_cbMagicIndex] = cbMagicCardIndex[INDEX_REPLACE_CARD];
                cbMagicCardIndex[INDEX_REPLACE_CARD] = cbMagicCardCount;
            }
        }
        if (cbCardCount>=3)
        {
            for (CT_BYTE i=0;i<MAX_INDEX-MAX_HUA_CARD;i++)
            {
                //同牌判断
                //如果是财神,并且财神数小于3,则不进行组合
                if( cbMagicCardIndex[i] >= 3 || ( cbMagicCardIndex[i]+cbMagicCardCount >= 3 &&
                                                  ( ( INDEX_REPLACE_CARD!=MAX_INDEX && i != INDEX_REPLACE_CARD ) || ( INDEX_REPLACE_CARD==MAX_INDEX && i != m_cbMagicIndex ) ) )
                        )
                {
                    int nTempIndex = cbMagicCardIndex[i];
                    do
                    {
                        assert( cbKindItemCount < CountArray(KindItem) );
                        CT_BYTE cbIndex = i;
                        CT_BYTE cbCenterCard = SwitchToCardData(i);
                        //如果是财神且财神有代替牌,则换成代替牌
                        if( i == m_cbMagicIndex && INDEX_REPLACE_CARD != MAX_INDEX )
                        {
                            cbIndex = INDEX_REPLACE_CARD;
                            cbCenterCard = SwitchToCardData(INDEX_REPLACE_CARD);
                        }
                        KindItem[cbKindItemCount].cbWeaveKind=WIK_PENG;
                        KindItem[cbKindItemCount].cbCenterCard=cbCenterCard;
                        KindItem[cbKindItemCount].cbValidIndex[0] = nTempIndex>0?cbIndex:m_cbMagicIndex;
                        KindItem[cbKindItemCount].cbValidIndex[1] = nTempIndex>1?cbIndex:m_cbMagicIndex;
                        KindItem[cbKindItemCount].cbValidIndex[2] = nTempIndex>2?cbIndex:m_cbMagicIndex;
                        cbKindItemCount++;

                        //如果是财神,则退出
                        if( i == INDEX_REPLACE_CARD || (i == m_cbMagicIndex && INDEX_REPLACE_CARD == MAX_INDEX) )
                            break;

                        nTempIndex -= 3;
                        //如果刚好搭配全部，则退出
                        if( nTempIndex == 0 ) break;

                    }while( nTempIndex+cbMagicCardCount >= 3 );
                }

                //连牌判断
                if ((i<(MAX_INDEX-MAX_HUA_CARD-9))&&((i%9)<7))
                {
                    //只要财神牌数加上3个顺序索引的牌数大于等于3,则进行组合
                    if( cbMagicCardCount+cbMagicCardIndex[i]+cbMagicCardIndex[i+1]+cbMagicCardIndex[i+2] >= 3 )
                    {
                        CT_BYTE cbIndex[3] = { cbMagicCardIndex[i],cbMagicCardIndex[i+1],cbMagicCardIndex[i+2] };
                        int nMagicCountTemp = cbMagicCardCount;
                        CT_BYTE cbValidIndex[3];
                        while( nMagicCountTemp+cbIndex[0]+cbIndex[1]+cbIndex[2] >= 3 )
                        {
                            for( CT_BYTE j = 0; j < CountArray(cbIndex); j++ )
                            {
                                if( cbIndex[j] > 0 )
                                {
                                    cbIndex[j]--;
                                    cbValidIndex[j] = (i+j==m_cbMagicIndex&&INDEX_REPLACE_CARD!=MAX_INDEX)?INDEX_REPLACE_CARD:i+j;
                                }
                                else
                                {
                                    nMagicCountTemp--;
                                    cbValidIndex[j] = m_cbMagicIndex;
                                }
                            }
                            if( nMagicCountTemp >= 0 )
                            {
                                assert( cbKindItemCount < CountArray(KindItem) );
                                KindItem[cbKindItemCount].cbWeaveKind=WIK_LEFT;
                                KindItem[cbKindItemCount].cbCenterCard=SwitchToCardData(i);
                                memcpy( KindItem[cbKindItemCount].cbValidIndex,cbValidIndex,sizeof(cbValidIndex) );
                                cbKindItemCount++;
                            }
                            else break;
                        }
                    }
                }
            }
        }

        //变量定义
        CT_BYTE cbCardIndexTemp[MAX_INDEX];
        memset(cbCardIndexTemp,0,sizeof(cbCardIndexTemp));
        //设置变量
        memcpy(cbCardIndexTemp,cbCardIndex,sizeof(cbCardIndexTemp));

        CT_BYTE cbGoodCount = 0;
        for (CT_BYTE i = 0; i < cbKindItemCount; ++i)
        {
            //存在判断
            if (cbCardIndexTemp[KindItem[i].cbValidIndex[0]]==0 ||
                cbCardIndexTemp[KindItem[i].cbValidIndex[1]]==0 ||
                cbCardIndexTemp[KindItem[i].cbValidIndex[2]]==0)
            {
                continue;
            }

            cbCardIndexTemp[KindItem[i].cbValidIndex[0]]--;
            cbCardIndexTemp[KindItem[i].cbValidIndex[1]]--;
            cbCardIndexTemp[KindItem[i].cbValidIndex[2]]--;

            cbGoodCount++;

            if (KindItem[i].cbValidIndex[0] > 9)
            {
                ++cbFengCardCount;
            }
        }

        return cbGoodCount;
    }

    CT_DWORD CGameLogic::GetGenCount(const CT_BYTE cbHandIndex[MAX_INDEX])
    {
        CT_DWORD dwGenCount = 0;
        for(int i = 0; i < MAX_INDEX; i++)
        {
            if(cbHandIndex[i] == 4)
            {
                dwGenCount++;
            }
        }

        return dwGenCount;
    }

    CT_LONGLONG CGameLogic::GetHuType(CT_WORD wChairID, CT_WORD wBankerUser, CT_WORD wOutCardUser, CT_WORD wProvideUser, CT_BYTE cbProvideCard, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
                                   const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount, const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
                                   const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng, CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
                                   const CAnalyseItemArray& AnalyseItemArray, CT_WORD& wHuMultiple, tagAnalyseItem& LastAnalyseItem, CT_BOOL& cbIsZiMo,CT_BOOL bCheckTing /*= CT_FALSE*/)
    {
        CT_BOOL bQiDui = CT_FALSE;
        CT_BOOL bQingYiSe = CT_FALSE;
        CT_BOOL bPengPengHu = CT_FALSE;
        CT_WORD wMultiple = 0;
        EnumFlag<HUTYPES> eFlag;
        CT_BYTE cbCardIndex[MAX_INDEX];
        memset(cbCardIndex, 0, sizeof(cbCardIndex));
        memcpy(cbCardIndex, cbHandCardIndex[wChairID], sizeof(cbCardIndex));
        wHuMultiple = 0;
        CT_LONGLONG lLastHuType = 0;
        memset(&LastAnalyseItem, 0, sizeof(LastAnalyseItem));
        CT_DWORD dwHandCardCount = GetCardCount(cbCardIndex);

        CT_BOOL bJiang258 = CT_FALSE;
        if(wProvideUser==wChairID)
        {
            cbIsZiMo = CT_TRUE;
        }
        bQingYiSe = IsQingYiSe(cbCardIndex);

        for (CT_BYTE i = 0; i < AnalyseItemArray.GetCount(); ++i)
        {
            if(MAX_COUNT > GetHandcardIndex(AnalyseItemArray[i], cbCardIndex))
            {
                assert(CT_FALSE);
                continue;
            }

            wMultiple = 0;
            eFlag.ResetData();

            //只要和牌里有2，5，8将
            bJiang258 =  AnalyseItemArray[i].cbCardEye == 0x02 || AnalyseItemArray[i].cbCardEye == 0x05 || AnalyseItemArray[i].cbCardEye == 0x08;
            bQiDui = AnalyseItemArray[i].cbIsQiDui;
            bPengPengHu = CheckPengPengHu(AnalyseItemArray[i]);

            CT_DWORD  dwGenCount = GetGenCount(cbCardIndex);
            if(bQingYiSe)
            {
                if(bQiDui && dwGenCount >= 1)
                {
                    //清龙七对
                    eFlag.SetFlag<HU_TYPE_QING_LONG_QI_DU>();
                    wMultiple += 6;
                }
                else if(bQiDui)
                {
                    //清7对
                    eFlag.SetFlag<HU_TYPE_QING_QI_DU>();
                    wMultiple += 5;
                }
                else if(bPengPengHu)
                {
                    //清对
                    eFlag.SetFlag<HU_TYPE_QING_DUI>();
                    wMultiple += 4;
                }
                else if(CheckQuanDaiYao(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
                {
                    //清幺九
                    eFlag.SetFlag<HU_TYPE_QING_YAO_JIU>();
                    wMultiple += 5;
                }
                else
                {
                    //清一色
                    eFlag.SetFlag<HU_TYPE_QING_YI_SE>();
                    wMultiple += 3;
                }
            }
            else if(bQiDui)
            {
                if(dwGenCount >= 1)
                {
                    //龙七对
                    eFlag.SetFlag<HU_TYPE_LONG_QI_DUI>();
                    wMultiple += 4;
                }
                else
                {
                    //七对
                    eFlag.SetFlag<HU_TYPE_QI_DUI>();
                    wMultiple += 3;
                }
            }
            else if(bPengPengHu)
            {
                //碰碰胡
                eFlag.SetFlag<HU_TYPE_PENG_PENG_HU>();
                wMultiple += 2;
            }
            else
            {
                //平胡
                eFlag.SetFlag<HU_TYPE_PING_HU>();
                wMultiple += 1;
            }

            if(cbIsZiMo && bGangStatus)
            {
                //杠上花
                eFlag.SetFlag<HU_TYPE_GANG_SHANG_HUA>();
                wMultiple += 1;
            }

            //杠上炮

            if (cbDiscardCount[wChairID] == 0 && cbOutCardCount == 0 && wChairID == wBankerUser && cbWeaveItemCount[wChairID]==0)
            {
                //天胡
                eFlag.SetFlag<HU_TYPE_TIAN_HU>();
                wMultiple += 6;
            }

            if (cbDiscardCount[wChairID] == 0 && cbOutCardCount == 1 && wChairID != wBankerUser)
            {
                //地胡
                eFlag.SetFlag<HU_TYPE_DI_HU>();
                wMultiple += 6;
            }

            if (cbLeftCardCount==0)
            {
                //海底捞月
                eFlag.SetFlag<HU_TYPE_HAI_DI_LAO_YUE>();
                wMultiple += 1;
            }

            if (GetCardRemain(wChairID, cbHandCardIndex, cbListenStatus,cbDiscardCard, cbDiscardCount, WeaveItemArray, cbWeaveItemCount, cbProvideCard)==4)
            {
                //胡绝张
                eFlag.SetFlag<HU_TYPE_HU_JUE_ZHANG>();
                wMultiple += 1;
            }

            if(dwHandCardCount == 1)
            {
                //大单钓
                eFlag.SetFlag<HU_TYPE_DA_DAN_DIAO>();
                wMultiple += 1;
            }

            //抢杠胡

            //带幺九
            if (!bQingYiSe && CheckQuanDaiYao(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_DAI_YAO_JIU>();
                wMultiple += 3;
            }

            //将对

            if(cbIsZiMo)
            {
                //自摸
                eFlag.SetFlag<HU_TYPE_ZI_MO>();
                wMultiple += 1;
            }

            if(dwGenCount >= 1)
            {
                //加根
                eFlag.SetFlag<HU_TYPE_GEN>();
                wMultiple += dwGenCount;
            }

            /*
            if (!AnalyseItemArray[i].cbIsQiDui && CheckDaSiXi(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_DA_SI_XI>();
                wMultiple += 88;

                //str.AppendFormat(_T("大四喜 "));
                //LOG(ERROR) << "能胡:大四喜";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckDaSanYuan(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_DA_SAN_YUAN>();
                wMultiple += 88;

                //str.AppendFormat(_T("大三元 "));
                //LOG(ERROR) << "能胡:大三元";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckJiuLianBaoDeng(cbCardIndex))
            {
                eFlag.SetFlag<HU_TYPE_JIU_LIAN_BAO_DENG>();
                wMultiple += 88;

                //str.AppendFormat(_T("九莲宝灯 "));
                //LOG(ERROR) << "能胡:九莲宝灯";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckSiGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_SI_GANG>();
                wMultiple += 88;

               // LOG(ERROR) << "能胡:四杠";
                //str.AppendFormat(_T("四杠 "));
            }

            if (AnalyseItemArray[i].cbIsQiDui && CheckLianQiDui(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID], cbCardIndex))
            {
                eFlag.SetFlag<HU_TYPE_LIAN_QI_DUI>();
                wMultiple += 88;

                //LOG(ERROR) << "能胡:连七对";
                //str.AppendFormat(_T("连七对 "));
            }

            if (!bCheckTing && cbDiscardCount[wChairID] == 0 && cbOutCardCount == 0 && wChairID == wBankerUser && cbWeaveItemCount[wChairID]==0)
            {
                eFlag.SetFlag<HU_TYPE_TIAN_HU>();
                wMultiple += 88;

                //LOG(ERROR) << "能胡:天胡";
                //str.AppendFormat(_T("天胡 "));
            }

            if (!bCheckTing && cbDiscardCount[wChairID] == 0 && cbOutCardCount == 1 && wChairID != wBankerUser)
            {
                eFlag.SetFlag<HU_TYPE_DI_HU>();
                wMultiple += 88;

                //LOG(ERROR) << "能胡:地胡";
                //str.AppendFormat(_T("地胡 "));
            }

            if (!bCheckTing && cbDiscardCount[wBankerUser] == 1 && cbOutCardCount == 1 && wOutCardUser == wBankerUser)
            {
                eFlag.SetFlag<HU_TYPE_REN_HU>();
                wMultiple += 88;

                //str.AppendFormat(_T("人胡 "));
                //LOG(ERROR) << "能胡:人胡";
            }

            if (bQingYiSe && CheckBaiWanShi(cbCardIndex))
            {
                eFlag.SetFlag<HU_TYPE_BAI_WAN_SHI>();
                wMultiple += 88;

                //str.AppendFormat(_T("百万石 "));
               // LOG(ERROR) << "能胡:百万石";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SI_XI>() && CheckXiaoSiXi(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_XIAO_SI_XI>();
                wMultiple += 64;

                //str.AppendFormat(_T("小四喜 "));
                //LOG(ERROR) << "能胡:小四喜";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SAN_YUAN>() && CheckXiaoSanYuan(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_XIAO_SAN_YUAN>();
                wMultiple += 64;

                //str.AppendFormat(_T("小三元 "));
               // LOG(ERROR) << "能胡:小三元";
            }

            if (CheckZiYiSe(cbCardIndex))
            {
                eFlag.SetFlag<HU_TYPE_ZI_YI_SE>();
                wMultiple += 64;

                //str.AppendFormat(_T("字一色 "));
               // LOG(ERROR) << "能胡:字一色";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckSiAnKe(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID], AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_SI_AN_KE>();
                wMultiple += 64;

                //str.AppendFormat(_T("四暗刻 "));
               // LOG(ERROR) << "能胡:四暗刻";
            }

            //if (bQingYiSe && !AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_ZI_YI_SE>() && CheckYiSeShuangLong(AnalyseItemArray[i]))
            if(CheckYiSeShuangLong(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SHUANG_LONG>();
                wMultiple += 64;

                //str.AppendFormat(_T("一色双龙会 "));
                //LOG(ERROR) << "能胡:一色双龙会";
            }

            if (CheckYiSeSiTongShun(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>();
                wMultiple += 48;

                //str.AppendFormat(_T("一色四同顺 "));
               // LOG(ERROR) << "能胡:一色四同顺";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckYiSeSiJieGao(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SI_JIE_GAO>();
                wMultiple += 48;

                //str.AppendFormat(_T("一色四节高 "));
                //LOG(ERROR) << "能胡:一色四节高";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckYiSeSiBuGao(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SI_BU_GAO>();
                wMultiple += 32;

                //str.AppendFormat(_T("一色四步高 "));
                //LOG(ERROR) << "能胡:一色四步高";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckSanGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_SAN_GANG>();
                wMultiple += 32;

                //str.AppendFormat(_T("三杠 "));
                //LOG(ERROR) << "能胡:三杠";
            }

            if (!eFlag.TestFlag<HU_TYPE_ZI_YI_SE>() && CheckHunYaoJiu(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_HUN_YAO_JIU>();
                wMultiple += 32;

                //str.AppendFormat(_T("混幺九 "));
                //LOG(ERROR) << "能胡:混幺九";
            }

            if (!eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SHUANG_LONG>() && AnalyseItemArray[i].cbIsQiDui)
            {
                eFlag.SetFlag<HU_TYPE_QI_DUI>();
                wMultiple += 24;

                //str.AppendFormat(_T("七对 "));
                //LOG(ERROR) << "能胡:七对";
            }

            if (bQingYiSe && !eFlag.TestFlag<HU_TYPE_JIU_LIAN_BAO_DENG>() && !eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() &&
                !eFlag.TestFlag<HU_TYPE_BAI_WAN_SHI>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SHUANG_LONG>())
            {
                eFlag.SetFlag<HU_TYPE_QING_YI_SE>();
                wMultiple += 24;

                //str.AppendFormat(_T("清一色 "));
                //LOG(ERROR) << "能胡:清一色";
            }

            if (!eFlag.TestFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_JIE_GAO>() && !AnalyseItemArray[i].cbIsQiDui && CheckYiSeSanJieGao(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SAN_JIE_GAO>();
                wMultiple += 24;

                //str.AppendFormat(_T("一色三节高 "));
                //LOG(ERROR) << "能胡:一色三节高";
            }

            if (!eFlag.TestFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_JIE_GAO>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SAN_JIE_GAO>() && !AnalyseItemArray[i].cbIsQiDui &&
                CheckYiSeSanTongShun(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SAN_TONG_SHUN>();
                wMultiple += 24;

                //str.AppendFormat(_T("一色三同顺 "));
                //LOG(ERROR) << "能胡:一色三同顺";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckQingLong(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_QING_LONG>();
                wMultiple += 16;

                //str.AppendFormat(_T("清龙 "));
                //LOG(ERROR) << "能胡:清龙";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_BU_GAO>() && CheckYiSeSanBuGao(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_SE_SAN_BU_GAO>();
                wMultiple += 16;

                //str.AppendFormat(_T("一色三步高 "));
                //LOG(ERROR) << "能胡:一色三步高";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_AN_KE>() && CheckSanAnKe(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID], AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_SAN_AN_KE>();
                wMultiple += 16;

                //str.AppendFormat(_T("三暗刻 "));
               // LOG(ERROR) << "能胡:三暗刻";
            }

            if (bIsTianTing[wChairID] && cbListenStatus[wChairID])
            {
                eFlag.SetFlag<HU_TYPE_TIAN_TING>();
                wMultiple += 16;

                //str.AppendFormat(_T("天听 "));
                //LOG(ERROR) << "能胡:天听";
            }

            if (CheckDaYu5(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_DA_YU_5>();
                wMultiple += 12;

                //str.AppendFormat(_T("大于5 "));
                //LOG(ERROR) << "能胡:大于5";
            }

            if (CheckXiaoYu5(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_XIAO_YU_5>();
                wMultiple += 12;

                //str.AppendFormat(_T("小于5 "));
                //LOG(ERROR) << "能胡:小于5";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SI_XI>() && !eFlag.TestFlag<HU_TYPE_XIAO_SI_XI>() && CheckSanFengKe(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_SAN_FENG_KE>();
                wMultiple += 12;

                //str.AppendFormat(_T("三风刻 "));
                //LOG(ERROR) << "能胡:三风刻";
            }

            if (!bCheckTing && cbLeftCardCount==0)
            {
                if (wProvideUser==wChairID)
                {
                    eFlag.SetFlag<HU_TYPE_MIAO_SHOU_HUI_CHUN>();
                    wMultiple += 8;

                    cbIsZiMo = CT_FALSE;
                   // LOG(ERROR) << "能胡:妙手回春";
                    //str.AppendFormat(_T("妙手回春 "));
                }
                else
                {
                    eFlag.SetFlag<HU_TYPE_HAI_DI_LAO_YUE>();
                    wMultiple += 8;

                    //str.AppendFormat(_T("海底捞月 "));
                    //LOG(ERROR) << "能胡:海底捞月";
                }
            }

            if (!bCheckTing && bGangStatus)
            {
                if (wProvideUser==wChairID)
                {
                    eFlag.SetFlag<HU_TYPE_GANG_SHANG_HUA>();
                    wMultiple += 8;

                    cbIsZiMo = CT_FALSE;

                    //str.AppendFormat(_T("杠上花 "));
                    //LOG(ERROR) << "能胡:杠上花";
                }
                else if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_HU_JUE_ZHANG>())
                {
                    eFlag.SetFlag<HU_TYPE_QIANG_GANG_HU>();
                    wMultiple += 8;

                    //str.AppendFormat(_T("抢杠胡 "));
                    //LOG(ERROR) << "能胡:抢杠胡";
                }
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SI_XI>() && !eFlag.TestFlag<HU_TYPE_ZI_YI_SE>() && !eFlag.TestFlag<HU_TYPE_SI_AN_KE>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_JIE_GAO>() &&
                !eFlag.TestFlag<HU_TYPE_HUN_YAO_JIU>() && CheckPengPengHu(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_PENG_PENG_HU>();
                wMultiple += 6;

                //str.AppendFormat(_T("碰碰胡 "));
                //LOG(ERROR) << "能胡:碰碰胡";
            }

            if (!bQingYiSe && !eFlag.TestFlag<HU_TYPE_ZI_YI_SE>())
            {
                eFlag.SetFlag<HU_TYPE_HUN_YI_SE>();
                wMultiple += 6;

                //str.AppendFormat(_T("混一色 "));
                //LOG(ERROR) << "能胡:混一色";
            }

            if (!bCheckTing && !AnalyseItemArray[i].cbIsQiDui && wProvideUser!=wChairID && CheckQuanQiuRen(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_QUAN_QIU_REN>();
                wMultiple += 6;

                //str.AppendFormat(_T("全求人 "));
               // LOG(ERROR) << "能胡:全求人";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_GANG>() && !eFlag.TestFlag<HU_TYPE_SAN_GANG>() && CheckShuangAnGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_SHUANG_AN_GANG>();
                wMultiple += 6;

                //str.AppendFormat(_T("双暗杠 "));
               // LOG(ERROR) << "能胡:双暗杠";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SAN_YUAN>() && !eFlag.TestFlag<HU_TYPE_XIAO_SAN_YUAN>() && CheckShuangJianKe(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_SHUANG_JIAN_KE>();
                wMultiple += 6;

                //str.AppendFormat(_T("双箭刻 "));
                //LOG(ERROR) << "能胡:双箭刻";
            }

            if (!eFlag.TestFlag<HU_TYPE_ZI_YI_SE>() && !eFlag.TestFlag<HU_TYPE_HUN_YAO_JIU>() && CheckQuanDaiYao(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_QUAN_DAI_YAO>();
                wMultiple += 4;

                //str.AppendFormat(_T("全带幺 "));
                //LOG(ERROR) << "能胡:全带幺";
            }

            if (!bCheckTing && wChairID==wProvideUser && !eFlag.TestFlag<HU_TYPE_JIU_LIAN_BAO_DENG>() && !eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_SI_AN_KE>() &&
                !eFlag.TestFlag<HU_TYPE_QI_DUI>() && CheckBuQiuRen(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_BU_QIU_REN>();
                wMultiple += 4;

                cbIsZiMo = CT_FALSE;

                //str.AppendFormat(_T("不求人 "));
                //LOG(ERROR) << "能胡:不求人";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_GANG>() && !eFlag.TestFlag<HU_TYPE_SAN_GANG>() && CheckShuangMingGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_SHUANG_MING_GANG>();
                wMultiple += 4;

                //str.AppendFormat(_T("双明杠 "));
                //LOG(ERROR) << "能胡:双明杠";
            }

            if (!eFlag.TestFlag<HU_TYPE_QIANG_GANG_HU>())
            {
                //把对家的牌放进去算是不是绝张
                memset(cbCardIndexTmp, 0, sizeof(cbCardIndexTmp));
                if (cbListenStatus[wChairID] == CT_TRUE)
                {
                    CT_BYTE peer = (wChairID+1)%GAME_PLAYER;
                    memcpy(cbCardIndexTmp, cbHandCardIndex[peer], sizeof(cbCardIndexTmp));
                }

                //把自己的牌也放进去
                for (CT_BYTE n = 0; n < MAX_INDEX; ++n)
                {
                    cbCardIndexTmp[n] += cbHandCardIndex[wChairID][n];
                }

                if (GetCardRemain(wChairID, cbHandCardIndex, cbListenStatus,cbDiscardCard, cbDiscardCount, WeaveItemArray, cbWeaveItemCount, cbProvideCard)==4)
                {
                    eFlag.SetFlag<HU_TYPE_HU_JUE_ZHANG>();
                    wMultiple += 4;

                    //str.AppendFormat(_T("胡绝张 "));
                   // LOG(ERROR) << "能胡:胡绝张";
                }
            }

            if (*//*!bCheckTing && *//*wChairID!=wProvideUser && CheckMenQianQing(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                if (cbListenStatus[wChairID] && !eFlag.TestFlag<HU_TYPE_TIAN_TING>())
                {
                    eFlag.SetFlag<HU_TYPE_LI_ZHI>();
                    wMultiple += 4;

                    //str.AppendFormat(_T("立直 "));
                    //LOG(ERROR) << "能胡:立直";
                } else if (!eFlag.TestFlag<HU_TYPE_JIU_LIAN_BAO_DENG>() && !eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_SI_AN_KE>() && !eFlag.TestFlag<HU_TYPE_QI_DUI>() &&
                           !eFlag.TestFlag<HU_TYPE_BU_QIU_REN>())
                {
                    eFlag.SetFlag<HU_TYPE_MEN_QIAN_QING>();
                    wMultiple += 2;

                    //str.AppendFormat(_T("门前清 "));
                    //LOG(ERROR) << "能胡:门前清";
                }
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SAN_YUAN>() && !eFlag.TestFlag<HU_TYPE_XIAO_SAN_YUAN>() && !eFlag.TestFlag<HU_TYPE_SHUANG_JIAN_KE>() && CheckJianKe(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_JIAN_KE>();
                wMultiple += 2;

                //str.AppendFormat(_T("箭刻 "));
                //LOG(ERROR) << "能胡:箭刻";
            }

            if(!AnalyseItemArray[i].cbIsQiDui && CheckPingHu(AnalyseItemArray[i]))
            {
                //4组顺子和万字牌做将
                eFlag.SetFlag<HU_TYPE_PING_HU>();
                wMultiple += 2;

                //str.AppendFormat(_T("平胡 "));
                //LOG(ERROR) << "能胡:平胡";
            }

            if (!eFlag.TestFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>() && CheckSiGuiYi(cbCardIndex, WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_SI_GUI_YI>();
                wMultiple += 2;

                //str.AppendFormat(_T("四归一 "));
                //LOG(ERROR) << "能胡:四归一";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_AN_KE>() && !eFlag.TestFlag<HU_TYPE_SAN_AN_KE>() && !eFlag.TestFlag<HU_TYPE_SHUANG_AN_GANG>() && CheckShuangAnKe(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID], AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_SHUANG_AN_KE>();
                wMultiple += 2;

                //str.AppendFormat(_T("双暗刻 "));
                //LOG(ERROR) << "能胡:双暗刻";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_GANG>() && !eFlag.TestFlag<HU_TYPE_SAN_GANG>() && !eFlag.TestFlag<HU_TYPE_SHUANG_AN_GANG>() && CheckAnGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_AN_GANG>();
                wMultiple += 2;

                //str.AppendFormat(_T("暗杠 "));
               // LOG(ERROR) << "能胡:暗杠";
            }

            if (CheckDuanYao(cbCardIndex))
            {
                eFlag.SetFlag<HU_TYPE_DUAN_YAO>();
                wMultiple += 2;

                //str.AppendFormat(_T("断幺 "));
                //LOG(ERROR) << "能胡:断幺";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && bJiang258)
            {
                eFlag.SetFlag<HU_TYPE_ER_WU_BA_JIANG>();
                wMultiple += 1;

                //str.AppendFormat(_T("二五八将 "));
                //LOG(ERROR) << "能胡:二五八将";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && CheckYaoJiuTou(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YAO_JIU_TOU>();
                wMultiple += 1;

                //str.AppendFormat(_T("幺九头 "));
                //LOG(ERROR) << "能胡:幺九头";
            }

            if (cbListenStatus[wChairID] && !eFlag.TestFlag<HU_TYPE_TIAN_TING>() && !eFlag.TestFlag<HU_TYPE_LI_ZHI>())
            {
                eFlag.SetFlag<HU_TYPE_BAO_TING>();
                wMultiple += 1;

                //str.AppendFormat(_T("报听 "));
               // LOG(ERROR) << "能胡:报听";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SHUANG_LONG>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_TONG_SHUN>() &&
                !eFlag.TestFlag<HU_TYPE_YI_SE_SAN_TONG_SHUN>() && CheckYiBanGao(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YI_BAN_GAO>();
                wMultiple += 1;

                //str.AppendFormat(_T("一般高 "));
                //LOG(ERROR) << "能胡:一般高";
            }

            if (!eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_BU_GAO>() && !eFlag.TestFlag<HU_TYPE_QING_LONG>() &&
                CheckLianLiu(AnalyseItemArray[i], cbHandCardIndex[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_LIAN_LIU>();
                wMultiple += 1;

                //str.AppendFormat(_T("连六 "));
                //LOG(ERROR) << "能胡:连六";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_YI_SE_SHUANG_LONG>() && !eFlag.TestFlag<HU_TYPE_YI_SE_SI_BU_GAO>() && !eFlag.TestFlag<HU_TYPE_QING_LONG>() && CheckLaoShaoFu(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_LAO_SHAO_FU>();
                wMultiple += 1;

                //str.AppendFormat(_T("老少副 "));
                //LOG(ERROR) << "能胡:老少副";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_DA_SI_XI>() && !eFlag.TestFlag<HU_TYPE_DA_SAN_YUAN>() && !eFlag.TestFlag<HU_TYPE_JIU_LIAN_BAO_DENG>() && !eFlag.TestFlag<HU_TYPE_XIAO_SAN_YUAN>() &&
                !eFlag.TestFlag<HU_TYPE_ZI_YI_SE>() && !eFlag.TestFlag<HU_TYPE_HUN_YAO_JIU>() && !eFlag.TestFlag<HU_TYPE_SAN_FENG_KE>() && !eFlag.TestFlag<HU_TYPE_JIAN_KE>() &&
                !eFlag.TestFlag<HU_TYPE_QUAN_FENG_KE>() && !eFlag.TestFlag<HU_TYPE_MEN_FENG_KE>() && CheckYaoJiuKe(AnalyseItemArray[i]))
            {
                eFlag.SetFlag<HU_TYPE_YAO_JIU_KE>();
                wMultiple += 1;

                //str.AppendFormat(_T("幺九刻 "));
                //LOG(ERROR) << "能胡:幺九刻";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_SI_GANG>() && !eFlag.TestFlag<HU_TYPE_SAN_GANG>() &&  !eFlag.TestFlag<HU_TYPE_SHUANG_MING_GANG>() && CheckMingGang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID]))
            {
                eFlag.SetFlag<HU_TYPE_MING_GANG>();
                wMultiple += 1;

                //str.AppendFormat(_T("明杠 "));
                //LOG(ERROR) << "能胡:明杠";
            }

            if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_TIAN_HU>() && CheckBianZhang(AnalyseItemArray[i], cbProvideCard))
            {
                eFlag.SetFlag<HU_TYPE_BIAN_ZHANG>();
                wMultiple += 1;

                //str.AppendFormat(_T("边张 "));
                //LOG(ERROR) << "能胡:边张";
            }
            else if (!AnalyseItemArray[i].cbIsQiDui && !eFlag.TestFlag<HU_TYPE_TIAN_HU>() && CheckKanZhang(AnalyseItemArray[i], cbProvideCard))
            {
                eFlag.SetFlag<HU_TYPE_KAN_ZHANG>();
                wMultiple += 1;

                //str.AppendFormat(_T("坎张 "));
                //LOG(ERROR) << "能胡:坎张";
            }
            else if (!eFlag.TestFlag<HU_TYPE_SI_GANG>() && !eFlag.TestFlag<HU_TYPE_LIAN_QI_DUI>() && !eFlag.TestFlag<HU_TYPE_TIAN_HU>() && !eFlag.TestFlag<HU_TYPE_QI_DUI>() &&
                     !eFlag.TestFlag<HU_TYPE_QUAN_QIU_REN>() && CheckDanDiaoJiang(WeaveItemArray[wChairID], cbWeaveItemCount[wChairID], AnalyseItemArray[i], cbHandCardIndex[wChairID], cbProvideCard))
            {
                eFlag.SetFlag<HU_TYPE_DAN_DIAO>();
                wMultiple += 1;

                //str.AppendFormat(_T("单调将 "));
                //LOG(ERROR) << "能胡:单调将";
            }

            if (cbIsZiMo)
            {
                wMultiple += 1;

                //str.AppendFormat(_T("自摸胡 "));
                //LOG(ERROR) << "能胡:自摸胡";
            }*/

            //取得最大牌型
            if (wHuMultiple < wMultiple)
            {
                LastAnalyseItem = AnalyseItemArray[i];
                lLastHuType = eFlag.GetData();
                wHuMultiple = wMultiple;
            }
        }

        //LOG(ERROR) << "能胡的总番数:" << wHuMultiple;
        return lLastHuType;
    }














































CGameLogic::CGameLogic()
{
    m_cbMagicIndex = MAX_INDEX;
}

CGameLogic::~CGameLogic()
{

}

CT_VOID CGameLogic::RandCard(CT_BYTE cbCardData[], CT_WORD cbMaxCount)
{
    memset(cbCardData, 0, cbMaxCount);
    //混乱准备
    CT_BYTE cbCardDataTemp[MAX_CARD_COUNT];
    memcpy(cbCardDataTemp, m_cbCardDataArray,sizeof(m_cbCardDataArray));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);
    //int n = dis(gen);

    //混乱扑克
    CT_BYTE cbRandCount=0,cbPosition=0;
    do
    {
        std::uniform_int_distribution<> dis(0, cbMaxCount-cbRandCount);
        cbPosition = dis(gen);
        cbCardData[cbRandCount++]=cbCardDataTemp[cbPosition];
        cbCardDataTemp[cbPosition]=cbCardDataTemp[cbMaxCount-cbRandCount];
    } while (cbRandCount<cbMaxCount);

    m_wCardBeginIndex = 0;
}

CT_VOID CGameLogic::GetSendCard(CT_BYTE getCard[], CT_WORD wGetCardCount)
{
    for(int i = 0; i < wGetCardCount; i++)
    {
        getCard[i] = m_cbCurCardData[m_wCardBeginIndex];
        m_cbCurCardData[m_wCardBeginIndex] = INVALID_CARD_DATA;
        m_wCardBeginIndex++;
    }
}






