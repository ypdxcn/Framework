#ifndef _SELECTOR_H
#define _SELECTOR_H

#include "Comm.h"
#include "WorkThread.h"
#include "Singleton.h"

#define CONCUREENT_PROCESSOR	2
#define AIO_MAX_FD FD_SETSIZE

using namespace std;

class CSelectorWorker;
class COMM_CLASS CSelector:public CWorkThread,public CAio
{
	friend class CSelectorWorker;
protected:
	CSelector();
	virtual ~CSelector();
public:
	virtual int Init(CConfig * pCfg);
	virtual void Finish();
	virtual int Start();
	virtual void Stop();

	//��ֻ�ṩ���¼�ע��/ע��
	int Register(REG_INF& stRefInf);
	void UnRegister(REG_INF& stRefInf);
	void UnRegister(tsocket_t sSocket);

	//������Ϣ���
	string ToString();
protected:
	CConfig *	m_pCfg;

	//�����̳߳���
	int m_nNumber;
	
	
	//�Ѿ�������
	vector< deque<REG_INF> > m_vqReady;
	//�����̳߳�
	vector< CSelectorWorker* > m_vWorker;
	//������
	vector< CGessMutex* > m_vcsQueue;
	//��ǰ�����߳�����
	int m_nPos;

	//����ļ���������
	int m_nMaxfds;
	//��ǰ�ļ���������
	int m_nFdsRead;
	int m_nFdsWrite;
	int m_nWaitTimeOut;

	//��ע��read socket������
	std::map<tsocket_t,REG_INF> m_mapRegisterRead;
	std::map<tsocket_t,REG_INF> m_mapRegisterWrite;
	CCondMutex m_condRegister;

	//�����źŵ�read socket
	tsocket_t m_sSocketReadSig;
	CGessMutex m_csReadSig;
	tsocket_t m_sSocketFireSig;

protected:
	
	virtual int Handle(int nIndex) = 0;
	virtual void OnActive(REG_INF& stRegInf) = 0;
	virtual int UdpSocket(tsocket_t& s,int& nUdpPort) = 0;
	int Push(REG_INF& stRegInf);

	int ThreadEntry();
	int End();

    void Lock(int nIndex);
	void Unlock(int nIndex);
	void NotifyQueue();

	
private:
	//���select����
	void Signal();
	//����signal��read udp socket port
	int m_nPortSig;
};

class  CSelectorWorker:public CWorkThread
{
public:
	CSelectorWorker(CSelector* p,int nIndex):m_pScheduler(p),m_nIndex(nIndex){}
	virtual ~CSelectorWorker(){}

	int Start()
	{
		BeginThread();
		return 0;
	}
	void Stop()
	{
		EndThread();
	}
	void Signal()
	{
		m_condMutexWorker.Lock();
		m_condMutexWorker.Signal();
		m_condMutexWorker.Unlock();
	}
private:
	int m_nIndex;
	CSelector* m_pScheduler;
	CCondMutex m_condMutexWorker;
	int ThreadEntry()
	{
		try
		{
			m_condMutexWorker.Lock();
			while(!m_bEndThread)
			{
				m_condMutexWorker.Wait();
				if (m_bEndThread)
					break;

				try
				{
					if (0 != m_pScheduler)
					{
						//Handlerʵ�ֺ��������Ӵ���ʱҪע���ֹ����
						m_pScheduler->Handle(m_nIndex);
					}
				}
				catch(std::exception e)
				{
					CRLog(E_ERROR,"exception:%s!Thread  no %d",e.what(),m_nIndex);
					return -1;
				}
				catch(...)
				{
					CRLog(E_ERROR,"Unknown exception!Thread  no %d",m_nIndex);
					return -1;
				}

			}
			m_condMutexWorker.Unlock();

			CRLog(E_SYSINFO,"Selector worker thread exit no %d",m_nIndex);
			return 0;		
		}
		catch(...)
		{
			m_condMutexWorker.Unlock();
			CRLog(E_ERROR,"Unknown exception exit!Thread  no %d",m_nIndex);
			return -1;
		}
	}
	int End()
	{
		CRLog(E_SYSINFO,"Signal Selector worker thread %d to exit",m_nIndex);
		Signal();

		CRLog(E_SYSINFO,"Selector worker thread %d wait end",m_nIndex);
		Wait();
		return 0;
	}
};

class COMM_CLASS CSelectorListen:public CSelector,public CSingleton< CSelectorListen > 
{
	friend class CSingleton< CSelectorListen >;
public:
	void Finish();

protected:
	CSelectorListen(){}
	virtual ~CSelectorListen(){}

	int Handle(int nIndex);
	void OnActive(REG_INF& stRegInf);
	int UdpSocket(tsocket_t& s,int& nUdpPort);
private:	
};

class COMM_CLASS CSelectorIo:public CSelector,public CSingleton< CSelectorIo > 
{
	friend class CSingleton< CSelectorIo >;
public:
	void Finish();
protected:
	CSelectorIo(){}
	virtual ~CSelectorIo(){}

	int Handle(int nIndex);
	void OnActive(REG_INF& stRegInf);
	int UdpSocket(tsocket_t& s,int& nUdpPort);
private:	
};

//class COMM_CLASS CSelectorConnector:public CSelector,public CSingleton< CSelectorConnector > 
//{
//	friend class CSingleton< CSelectorConnector >;
//public:
//	void Finish();
//
//protected:
//	CSelectorConnector(){}
//	virtual ~CSelectorConnector(){}
//
//	int Handle(int nIndex);
//	void OnActive(tsocket_t sSocket, void* p);
//	int UdpSocket(tsocket_t& s,int& nUdpPort);
//private:	
//	//�������߳�״̬�Ƿ���Ҫ������
//	bool IsNetManaged(string& sKeyName);
//};
#endif