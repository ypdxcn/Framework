#ifndef _BROADCAST_PACKET_H
#define _BROADCAST_PACKET_H

#include <string>
#include <vector>
#include <map>
#include "pthread.h"
#include "CommAp.h"
#include "PairPacket.h"

#define BROADCAST_LENGTH_BYTES		8

class COMMAP_CLASS CBroadcastPacket:public CPairPacket
{
public:
	CBroadcastPacket();
	CBroadcastPacket(const char* pcApiName);
	~CBroadcastPacket();

public:
	const string& GetCmdID();
	virtual std::string RouteKey()
	{
		//std::string sNodeType(""),sNodeID("");
		//GetParameterVal(GESS_NODE_TYPE,sNodeType);
		//GetParameterVal(GESS_NODE_ID,sNodeID);
		//return sNodeType + sNodeID;
		string sTsNodeID = "";
		GetParameterVal("Ts_NodeID",sTsNodeID);
		return sTsNodeID;
	}

	//�������
	virtual const char* Encode(unsigned int & uiLength,CPairPacket & packet){return 0;}
	virtual const char* Encode(unsigned int & uiLength);
	virtual void  Decode(const char * pData, unsigned int uiLength);
		
	void SetEncryptPama(int nEnCryptWay,char * s3DSKey,char * s3DSIvData);
private:
	std::string m_sCmdID;		//������ ��ApiName
	std::string m_sEncode;		//�������ַ���

	//�ӽ������
	//����ļ��ܷ�ʽ(Ҫ����ȥ�ı��ĵļ��ܷ���)��1�������ģ�2����ʹ�ù̶���Կ��3DS����
	int		 m_nEnCryptWay;
	//3DS���ܵĹ�ͬ��Կ
	char		 m_s3DSKey[1024];	
	//3DS���ܵĹ�ͬ����
	char		 m_s3DSIvData[1024];

	//��ȡ�������к�
	static long GetSID();

	static CGessMutex	m_csSID;
	static long m_uiSID;		//ά�������к�,����������ͨѶ�౨��
};
#endif