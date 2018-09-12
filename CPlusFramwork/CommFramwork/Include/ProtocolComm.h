#ifndef _PROTOCOLCOMM_H
#define _PROTOCOLCOMM_H

#include "Connector.h"
#include "Listener.h"
#include "Comm.h"
#include "Logger.h"
#include "strutils.h"
#include "osdepend.h"
#include <cassert>

using namespace std;
using namespace strutils;

//����״̬
typedef enum ConnectionState
{
	E_CONNECTED = 1,
	E_DISCONNECTED,
} E_CONN_STATE;

//Э��ͨѶ���������� 
class CProtocolProcess;
class CProtocolConnectPoint;
class COMM_CLASS CProtocolCommImp: public CProtocolComm
{

private:
	class CProtocolCommWorker: public CWorkThread
	{
	public:
		CProtocolCommWorker():m_pParent(0){}
		~CProtocolCommWorker(){}
		void Bind(CProtocolCommImp* p){m_pParent = p;}
		void Enque(PKEY_CONTEXT pKey)
		{
			m_condMutexWorker.Lock();
			PAIR_CLOSE pa;
			pa.first = time(0);
			pa.second = pKey;
			m_dqClose.push_back(pa);
			m_condMutexWorker.Unlock();
			m_condMutexWorker.Signal();
		}
	private:
		int ThreadEntry()
		{
			try
			{
				while(!m_bEndThread)
				{
					m_condMutexWorker.Lock();
					while (m_dqClose.empty() && !m_bEndThread)
						m_condMutexWorker.Wait();

					if (m_bEndThread)
					{
						try
						{
							for (deque<PAIR_CLOSE>::iterator it = m_dqClose.begin(); it != m_dqClose.end(); ++it)
							{
								m_pParent->Handle((*it).second);
							}
						}
						catch(...)
						{
							CRLog(E_ERROR,"%s","Unknown exception!");
						}
						m_condMutexWorker.Unlock();
						break;
					}

					PAIR_CLOSE pa = m_dqClose.front();
					m_dqClose.pop_front();
					m_condMutexWorker.Unlock();

					int nDiff = static_cast<int>(difftime(time(0), pa.first));
					if (abs(nDiff) < 4)
					{
						msleep(4-nDiff);
					}

					PKEY_CONTEXT pKey = pa.second;
					try
					{
						m_pParent->Handle(pKey);
					}
					catch(...)
					{
						CRLog(E_ERROR,"%s","Unknown exception!");
					}
				}

				CRLog(E_SYSINFO,"%s","ProtocolThread exit!");
				return 0;
			}
			catch(std::exception e)
			{
				CRLog(E_ERROR,"exception:%s!",e.what());
				return -1;
			}
			catch(...)
			{
				CRLog(E_ERROR,"%s","Unknown exception!");
				return -1;
			}
		}		
		int End()
		{
			m_condMutexWorker.Signal();
			Wait();
			return 0;
		}
	private:
		CProtocolCommImp*	m_pParent;

		typedef pair<time_t,PKEY_CONTEXT> PAIR_CLOSE;
		//���رն���
		std::deque<PAIR_CLOSE> m_dqClose;
		CCondMutex m_condMutexWorker;
	};

	friend class CProtocolCommNm;
	friend class CProtocolCommWorker;
public:
	CProtocolCommImp();
	virtual ~CProtocolCommImp();

	virtual int Init(CConfig * pCfg);
	virtual void Finish();
	virtual int Start();
	virtual void Stop();
	virtual void Bind(CConnectPointAsyn* pConnectPoint);

	//Ӧ�ò㱨���ϴ��ص����� ��Э�����̴������������ĺ����
	virtual int OnRecvPacket(CPacket & packet);
	
	//��������Э�����̴�����,������/�����̼߳�Э�����̴�����������ʱ������
	void ReqClose(CProtocolProcess* p);

	//����ͨ��SOCKET�������ݵĽӿں��� ��Э�鴦��������
	virtual int SendData(CProtocolProcess* p,const char* pBuf,int nSize);
	//����ͨ��SOCKET���յĽӿں���,��Aio����
	virtual int RecvData(PKEY_CONTEXT pKey);
	virtual int SendData(PKEY_CONTEXT pKey);

	//��¼�ɹ���ʧ�ܺ���
	virtual int OnLogin(const string& sLocalIp,int nLocalPort,const string& sPeerIp,int nPeerPort,int nFlag=0);
protected:
	virtual CAio* CreateAioInstance() = 0;
	//����Э�鴦����,�ɾ���ʵ����ʵ��
	virtual void CloseProcess(CProtocolProcess* pProcess) = 0;

	int RegisterAio(CProtocolProcess* p, int nEvt);
	PKEY_CONTEXT UnRegisterAio(CProtocolProcess* p);

	//��ΪTCPЭ�����ӵ���Ҫʵ�ֵĽӿ� ���ӹرպ���
	virtual int OnClose(const string& sLocalIp,int nLocalPort,const string& sPeerIp,int nPeerPort);

	//�շ��첽IO
	CAio* m_pAioReadWrite;
	//Э�����ӵ�
	CProtocolConnectPoint*	m_pConnectPoint;
private:
	//Э�鴦��������̺߳���,��Э�鴦��������̶߳������
	int Handle(PKEY_CONTEXT pKey);

	//Э�鴦��������̶߳���
	CProtocolCommWorker	m_oWorker;

	//��ע��socket��Э�鴦����/ͨѶ��������
	map<tsocket_t,PKEY_CONTEXT>	m_mapRegisted;
	CGessMutex	m_csComm;
};

//��Ϊ�ͻ��˵�Э��ͨѶ��������
//�ͻ����ڶ�socket���ӵ�����£���Ҫ·�ɷ�ʽΪ·�ɱ�·�ɣ���δ����
class COMM_CLASS CProtocolCommClient: public CProtocolCommImp
{
	friend class CConnector;
public:
	CProtocolCommClient();
	virtual ~CProtocolCommClient();

	virtual int Init(CConfig * pCfg);
	virtual void Finish();
	virtual int Start();
	virtual void Stop();
protected:
	//��Ϊ�ͻ�����Ҫʵ�ֵĽӿ� ���ӷ���˳ɹ�����
	virtual int OnConnect(tsocket_t sSocket,string sLocalIp,int nLocalPort,string sPeerIp,int nPeerPort,int nFlag=0);	
	
	//֪ͨConector����
	void NotifyConnect();
	//����״̬
	int GetConnState() const;

	//������Ϣ
	CConfig *	m_pCfg;	
private:
	//Ŀǰ����״̬
	int m_nConnState;

	//��Ϊ�ͻ��˵ĸ������ӵĶ��� Ŀǰδ���ǿͻ��˺ͷ����֧�ֶ�socket�������
	CConnector* m_pConnector;
};

///////////////////////////////////////////////////////////////////////////////

//��Ϊ����˵�Э��ͨѶ��������
class CProtocolConnectPointServer;
class COMM_CLASS CProtocolCommServer: public CProtocolCommImp
{
	friend  class CListener;
public:
	CProtocolCommServer();
	virtual ~CProtocolCommServer();
	
	virtual int Init(CConfig * pCfg);
	virtual void Finish();
	virtual int Start();
	virtual void Stop();

	//��Ϊ�������Ҫʵ�ֵĽӿ� ����Դ��ַ·����Ϣ����
	//�������Ҫ·�ɷ�ʽΪԴ��ַ·��
	virtual void On1stPacket(CProtocolProcess* p,const string& sRouteKey) = 0;

protected:
	//��Ϊ�������Ҫʵ�ֵĽӿ� ���������ܵ������Ӻ���
	virtual int OnAccept(tsocket_t sSocket,string sLocalIp,int nLocalPort,string sPeerIp,int nPeerPort);
	virtual CListener* CreateListener(CProtocolCommServer* p) = 0;
	virtual void DestroyListener(CListener* p) = 0;
	
	CConfig *	m_pCfg;
private:
	//��Ϊ����˵���������
	CListener* m_pListener;
};

//��װTCP�����ӿͻ��˵�ͨ�Ų�
class COMM_CLASS CTcpShortComm
{
public: 
	CTcpShortComm():m_sSocket(INVALID_SOCKET),m_nState(E_DISCONNECTED){}
	virtual ~CTcpShortComm(){}
public:	
	

	//��ʼ�� ��Դ�IP�Ͷ˿�
	int Init(CConfig *	pConfig);

	//���� 0�ɹ� -1ʧ��
	int Connect(const string& sIp, int nPort);

	//���� 0�ɹ� -1ʧ��
	int Connect();

	//����״̬
	bool IsConnected();

	//�Ͽ�
	void DisConnect();

	//���ͽ���������
	int SendData(const char* pcBuf, int nLen);

	//�������� ��ʱ����-1 �������ؽ��յ����ֽ���
	int RecvData(char* pcBuf,int& nLen,unsigned int uiTimeout);
private:
	std::string m_sIp;
	int m_nPort;
	tsocket_t m_sSocket;
	int m_nState;
	CGessMutex m_mutexConnector;
};

#endif