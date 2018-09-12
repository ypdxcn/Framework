#ifndef _TCP_SHORT_CONNECT_POINT_H
#define _TCP_SHORT_CONNECT_POINT_H

#include "ProtocolComm.h"
#include "Comm.h"
#include <cassert>

using namespace std;


//TCP�����ӷ�װ��ͬ�����ӵ�
template<typename PROCESS>
class CTcpShortCpCli: public CConnectPointSync
{
public:
	CTcpShortCpCli(void):m_Process(0),m_pCfg(0){}
	virtual ~CTcpShortCpCli(void){}
public:
	int Init(CConfig* pConfig)
	{
		if (0 != m_oComm.Init(pConfig))
			return -1;

		m_Process = new PROCESS;
		if (0 == m_Process)
		{
			return -1;
		}

		m_Process->Init(m_pCfg);
		m_Process->Bind(&m_oComm);
		return 0;
	}

	void Finish(void)
	{
		m_oComm.DisConnect();
		delete m_Process;
		m_Process = 0;
	}

	//�����ṩ���շ�������Ҫ�ӿں���
	int SendPacket(CPacket &sndPacket,CPacket &rcvPacket,unsigned int uiTimeout = 10)
	{
		if (0 != m_oComm.Connect())
			return -1;

		if (0 == m_Process)
			return -1;

		int nRtn = m_Process->SendPacket(sndPacket,rcvPacket,uiTimeout);
		m_oComm.DisConnect();
		return nRtn;
	}
private:
	PROCESS*	m_Process;	
	CConfig *	m_pCfg;
	CTcpShortComm	m_oComm;
};

#endif