/******************************************************************************
版    权:深圳市雁联计算系统有限公司.
模块名称:OfferingPacket.cpp
创建者	:张伟
创建日期:2008.07.22
版    本:1.0				
模块描述:封装基于报盘机报文编码解码
主要函数:Decode(...) 收到完整报文数据块后调用解码报文
	     Encode()   socket发送前按报文格式进行组装
	     GetCmdID() 获取命令字即ApiName
	     GetKey()获取报文key,即RootID
		 SetKey()设置报文key,即RootID
		 SetNodeType()设置报文头节点类型
		 SetNodeID()设置报文头节点ID
修改记录:
******************************************************************************/

#include <vector>
#include "Logger.h"
#include "strutils.h"
#include "BroadcastPacket.h"
#include "Base643Des.h"
using namespace std;
using namespace strutils;

//递增序列号
long CBroadcastPacket::m_uiSID = 0;
CGessMutex	CBroadcastPacket::m_csSID;

CBroadcastPacket::CBroadcastPacket():
m_sCmdID("")
,m_sEncode("")
,m_nEnCryptWay(1)
{
	memset(m_s3DSIvData,0x00,sizeof(m_s3DSIvData));
	memset(m_s3DSKey,0x00,sizeof(m_s3DSKey));
}

CBroadcastPacket::CBroadcastPacket(const char* pcApiName):
CPairPacket()
,m_sCmdID(pcApiName)
,m_sEncode("")
,m_nEnCryptWay(1)
{
	memset(m_s3DSIvData,0x00,sizeof(m_s3DSIvData));
	memset(m_s3DSKey,0x00,sizeof(m_s3DSKey));
	AddParameter(GESS_API_NAME,pcApiName);
}

CBroadcastPacket::~CBroadcastPacket()
{
}

/******************************************************************************
函数描述:对外提供的接口函数,socket发送前按报文格式进行组装
输出参数:unsigned int & usLen编码后的内存数据块长度
返回值  :编码后的内存数据块地址
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
const char* CBroadcastPacket::Encode(unsigned int & usLen)
{
	m_sEncode = "";
	m_sEncode += AssembleBody(GESS_FLD_SEPERATOR,GESS_VAL_SEPERATOR);

	char szTmp[128];
	memset(szTmp,0x00,128);
	sprintf(szTmp,"%c0%dd",'%',BROADCAST_LENGTH_BYTES);
	char szLen[128];
	sprintf(szLen,szTmp,m_sEncode.length());
	szLen[BROADCAST_LENGTH_BYTES] = '\0';
	
	m_sEncode = szLen + m_sEncode;

	if (2 == m_nEnCryptWay)
	{
		char * pcBuf = new char[2048];
		char * pcPwdBuf = new char[2048];
		memcpy(pcBuf,m_sEncode.c_str(),m_sEncode.length());
		usLen = m_sEncode.length();
		pcBuf[usLen] = '\0';
		int len = Base643Des::Des3Encrypt(pcBuf, m_s3DSIvData, m_s3DSKey, (unsigned char *)pcPwdBuf);
		m_sEncode.assign(pcPwdBuf,len);
		delete []pcBuf;
		delete []pcPwdBuf;
		m_sEncode = "0000000000" + m_sEncode;
		char cTemp = m_nEnCryptWay;
		m_sEncode = cTemp + m_sEncode;

		memset(szTmp,0x00,128);
		sprintf(szTmp,"%c0%dd",'%',BROADCAST_LENGTH_BYTES);
		memset(szLen,0x00,128);
		sprintf(szLen,szTmp,m_sEncode.length());
		szLen[BROADCAST_LENGTH_BYTES] = '\0';
		m_sEncode = szLen + m_sEncode;
	}
	usLen = m_sEncode.length();
	return m_sEncode.c_str();
}

/******************************************************************************
函数描述:对外提供的接口函数,收到完整报文数据块后调用解码报文
输入参数:const char * pData 报文数据块内存地址
	     unsigned int nLength 报文数据块长度
返回值  :无
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
void CBroadcastPacket::Decode(const char * pData, unsigned int nLength)
{
	char* pcBuf = new char[nLength+1];
	memcpy(pcBuf,pData,nLength);
	pcBuf[nLength] = '\0';

	if (2 == pData[BROADCAST_LENGTH_BYTES])
	{		
		char * pcSrcBuf = new char[2048];
		Base643Des::Des3UnEncrypt(pcBuf+BROADCAST_LENGTH_BYTES+1+10,m_s3DSIvData, m_s3DSKey, (unsigned char *)pcSrcBuf,nLength-10-1-BROADCAST_LENGTH_BYTES);
		nLength = strlen(pcSrcBuf);
		delete []pcBuf;
		pcBuf = pcSrcBuf;
	}
	std::string sPacket = pcBuf;
	delete []pcBuf;

	sPacket = sPacket.substr(BROADCAST_LENGTH_BYTES,nLength-BROADCAST_LENGTH_BYTES);
	ParseBody(sPacket,GESS_FLD_SEPERATOR,GESS_VAL_SEPERATOR);
}

/******************************************************************************
函数描述:获取报文命令字即ApiName
返回值  :报文命令字即ApiName
创建者  :张伟
创建日期:2008.07.22
修改记录:
******************************************************************************/
const string&CBroadcastPacket::GetCmdID()
{
	std::string sApiName = (GESS_API_NAME);
	GetParameterVal(sApiName,m_sCmdID);
	return m_sCmdID;
}

//暂未用，获取流水
long CBroadcastPacket::GetSID()
{
	m_csSID.Lock();
	m_uiSID++;
	m_csSID.Unlock();
	return m_uiSID;
}

void CBroadcastPacket::SetEncryptPama(int nEnCryptWay,char * s3DSKey,char * s3DSIvData)
{
	m_nEnCryptWay=nEnCryptWay;
	strcpy(m_s3DSKey,s3DSKey);
	strcpy(m_s3DSIvData,s3DSIvData);
}