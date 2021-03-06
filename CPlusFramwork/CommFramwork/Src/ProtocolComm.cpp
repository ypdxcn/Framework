#include "ProtocolConnectPoint.h"
#include "ProtocolComm.h"

CProtocolCommImp::CProtocolCommImp()
:m_pAioReadWrite(0)
,m_pConnectPoint(0)
{
}

CProtocolCommImp::~CProtocolCommImp()
{
}

int CProtocolCommImp::Init(CConfig * pCfg)
{
	//绑定协议流程处理器清理线程
	m_oWorker.Bind(this);

	//创建读写AIO实例
	m_pAioReadWrite = CreateAioInstance();
	if (0 == m_pAioReadWrite)
	{
		CRLog(E_CRITICAL,"%s", "No mem");
		return -1;
	}
	m_pAioReadWrite->Init(pCfg);
	return 0;
}

void CProtocolCommImp::Finish()
{
	if (0 != m_pAioReadWrite)
		m_pAioReadWrite->Finish();
}

int CProtocolCommImp::Start()
{
	//启动socket收发的Aio实例
	if (0 != m_pAioReadWrite)
		m_pAioReadWrite->Start();
	
	//启动协议流程处理器清理线程
	m_oWorker.BeginThread();
	return 0;
}

void CProtocolCommImp::Stop()
{
	map<tsocket_t,PKEY_CONTEXT>::iterator it;
	m_csComm.Lock();	
	for (it = m_mapRegisted.begin(); it != m_mapRegisted.end(); ++it)
	{
		m_pAioReadWrite->UnRegister((*it).first);
		m_pAioReadWrite->FreeKeyContex((*it).second);
		closesocket((*it).first);
		if ((*it).second != 0)
			CloseProcess((*it).second->pProcess);
	}
	m_mapRegisted.clear();
	m_csComm.Unlock();

	//停止socket收发的Aio实例
	if (0 != m_pAioReadWrite)
	{
		CRLog(E_SYSINFO,"%s","Stop ReadWriteAio");
		m_pAioReadWrite->Stop();
	}

	m_oWorker.EndThread();
}

//注册协议流程处理器/协议通讯处理器对
int CProtocolCommImp::RegisterAio(CProtocolProcess* p, int nEvt)
{	
	PKEY_CONTEXT lpKeyContext = m_pAioReadWrite->GetKeyContex();
	if (0 == lpKeyContext )
	{
		CRLog(E_CRITICAL,"%s", "No mem");
		return -1;
	}
	lpKeyContext->pProcess = p;
	lpKeyContext->pProtocolComm = this;
	tsocket_t sSocket = p->SocketID();
	lpKeyContext->sSocket = sSocket;
	REG_INF stRegInf;
	stRegInf.nEvt = nEvt;
	stRegInf.sSocket = sSocket;
	stRegInf.pInf = lpKeyContext;
	int nRtn = m_pAioReadWrite->Register(stRegInf);
	if (0 != nRtn)
	{
		string sLocalIp,sPeerIp;
		int nLocalPort,nPeerPort;
		p->GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
		CRLog(E_ERROR, "Register %d fail!%s:%d-%s:%d",sSocket,sLocalIp.c_str(),nLocalPort,sPeerIp.c_str(),nPeerPort);
		m_pAioReadWrite->FreeKeyContex(lpKeyContext);
		return -1;
	}
	m_csComm.Lock();
	m_mapRegisted[sSocket] = lpKeyContext;
	m_csComm.Unlock();
	return 0;
}

//注销协议流程处理器/协议通讯处理器对
PKEY_CONTEXT CProtocolCommImp::UnRegisterAio(CProtocolProcess* p)
{
	if (0 == p)
		return 0;

	PKEY_CONTEXT pKey = 0;
	map<tsocket_t,PKEY_CONTEXT>::iterator it;
	m_csComm.Lock();
	//不能使用p->SocketID() find,p可能已被清理
	for (it = m_mapRegisted.begin(); it != m_mapRegisted.end(); )
	{
		if (p == (*it).second->pProcess)
		{
			pKey = (*it).second;
			m_mapRegisted.erase(it++);
			break;
		}
		else
		{
			++it;
		}
	}
	m_csComm.Unlock();
	return pKey;
}

//真正通过SOCKET发送的接口函数
int CProtocolCommImp::SendData(CProtocolProcess* pProcess,const char* pBuf,int nSize)
{
	try
	{
		tsocket_t sSocket = pProcess->SocketID();

		//协议流程处理器负责同步
		int nSndLen = send(sSocket,pBuf,nSize,0);

		if (nSize == nSndLen)
		{
			//
		}
		else if (0 < nSndLen)
		{
			CRLog(E_ERROR,"socket:%d:%s",sSocket,"not all data sended");
			// add by liuwei 2012-8-15 begin
			m_csComm.Lock();
			map<tsocket_t,PKEY_CONTEXT>::iterator it = m_mapRegisted.find(sSocket);
			if (it != m_mapRegisted.end())
			{
				REG_INF stRegInf;
				stRegInf.nEvt = E_EVT_WRITE;
				stRegInf.sSocket = sSocket;
				stRegInf.pInf = (*it).second;
				m_pAioReadWrite->Register(stRegInf);
			}
			m_csComm.Unlock();
			// add by liuwei 2012-8-15 end
		}
		else
		{
			if (EWOULDBLOCK != GET_LAST_SOCK_ERROR())
			{
                // 增加日志输出错误码, Jerry Lee, 2011-8-1
                // begin
                CRLog(E_ERROR,"socket:%d:sended err: %d:error code: %d",sSocket,nSndLen, GET_LAST_SOCK_ERROR());

				ReqClose(pProcess);
			}
			else
			{
				m_csComm.Lock();
				map<tsocket_t,PKEY_CONTEXT>::iterator it = m_mapRegisted.find(sSocket);
				if (it != m_mapRegisted.end())
				{
					REG_INF stRegInf;
					stRegInf.nEvt = E_EVT_WRITE;
					stRegInf.sSocket = sSocket;
					stRegInf.pInf = (*it).second;
					m_pAioReadWrite->Register(stRegInf);
				}
				m_csComm.Unlock();
				nSndLen = -2;
			}
		}
		return nSndLen;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception");
		return -1;
	}
}

//真正通过SOCKET接收的接口函数
int CProtocolCommImp::RecvData(PKEY_CONTEXT pKey)
{
	if (0 == pKey)
		return -1;

	CProtocolProcess* pProcess = pKey->pProcess;
	if (0 == pProcess)
		return -1;
	
	bool blContinue = true;
	do
	{
		PDATA_BUF pRcvBuf = pProcess->GetRcvBuf();
		int nRecv = recv(pKey->sSocket,pRcvBuf->pBuf + pRcvBuf->uiUsed,pRcvBuf->uiLen - pRcvBuf->uiUsed,0);
		if (SOCKET_ERROR == nRecv || nRecv < 0)
		{
			//CRLog(E_SYSINFO,"%s","Recv -1");
			//break;
			//EAGAIN
			if (EWOULDBLOCK != GET_LAST_SOCK_ERROR())
			{
                // 增加日志输出错误码, 2011-8-1
                // begin
                CRLog(E_ERROR,"Recv error: %d", GET_LAST_SOCK_ERROR());
                // end
				blContinue = false;
				ReqClose(pProcess);				
			}
			break;
		}
		else if (0 == nRecv)
		{
			blContinue = false;
			ReqClose(pProcess);
			break;
		}
		
		pRcvBuf->uiUsed += nRecv;	

		pProcess->OnRecv();
        
	} while(true);

	if (blContinue)
	{
		m_csComm.Lock();
		map<tsocket_t,PKEY_CONTEXT>::iterator it = m_mapRegisted.find(pKey->sSocket);
		if (it != m_mapRegisted.end())
		{
			REG_INF stRegInf;
			stRegInf.nEvt = E_EVT_READ;
			stRegInf.sSocket = pKey->sSocket;
			stRegInf.pInf = pKey;
			m_pAioReadWrite->Register(stRegInf);
		}
		m_csComm.Unlock();
	}
	return 0;
}

int CProtocolCommImp::SendData(PKEY_CONTEXT pKey)
{
	if (0 == pKey)
		return -1;

	CProtocolProcess* pProcess = pKey->pProcess;
	if (0 == pProcess)
		return -1;

	return pProcess->OnSend(0);
}

//作为TCP协议连接点需要实现的接口 连接关闭后处理
int CProtocolCommImp::OnClose(const string& sLocalIp,int nLocalPort,const string& sPeerIp,int nPeerPort)
{
	return m_pConnectPoint->OnClose(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
}

//登录成功或失败后处理 nFlag=0成功 1失败
int CProtocolCommImp::OnLogin(const string& sLocalIp,int nLocalPort,const string& sPeerIp,int nPeerPort,int nFlag)
{
	return m_pConnectPoint->OnLogin(sLocalIp,nLocalPort,sPeerIp,nPeerPort,nFlag);
}

//应用层报文上传回调函数 供协议流程处理器解析报文后调用
int CProtocolCommImp::OnRecvPacket(CPacket & packet)
{
	assert(0 != m_pConnectPoint);
	if (0 == m_pConnectPoint)
		return -1;

	return m_pConnectPoint->OnRecvPacket(packet);
}

//异步协议连接点绑定
void CProtocolCommImp::Bind(CConnectPointAsyn* pConnectPoint)
{
	m_pConnectPoint = dynamic_cast<CProtocolConnectPoint*>(pConnectPoint);

	assert(0 != m_pConnectPoint);
	if (0 == m_pConnectPoint)
	{
		CRLog(E_ERROR,"%s", "cast CProtocolConnectPoint null");
	}
}

//请求清理协议流程处理器,供发送/接收线程及协议流程处理器自身定时器调用
void CProtocolCommImp::ReqClose(CProtocolProcess* p)
{
	PKEY_CONTEXT pKey = UnRegisterAio(p);
	if (0 != pKey)
		m_oWorker.Enque(pKey);
}

//协议处理器清除线程函数,由协议处理器清除线程对象调用
int CProtocolCommImp::Handle(PKEY_CONTEXT pKey)
{
	try
	{	
		if (0 == pKey)
			return 0;

		m_pAioReadWrite->UnRegister(pKey->sSocket);	
		closesocket(pKey->sSocket);

		string sLocalIp,sPeerIp;
		int nLocalPort,nPeerPort;
		pKey->pProcess->GetPeerInfo(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
		
		CloseProcess(pKey->pProcess);
		OnClose(sLocalIp,nLocalPort,sPeerIp,nPeerPort);

		m_pAioReadWrite->FreeKeyContex(pKey);
		return 0;
	}
	catch(...)
	{
		CRLog(E_ERROR,"%s","Unknown exception!");
		return -1;
	}
}

CProtocolCommClient::CProtocolCommClient():
m_pCfg(0)
,m_nConnState(E_DISCONNECTED)
,m_pConnector(0)
{}

CProtocolCommClient::~CProtocolCommClient()
{
	if (GetStage() != CAction::esFinish)
		Finish();
}

int CProtocolCommClient::Init(CConfig * pConfig)
{
	if (!StateCheck(esInit))
		return -1;

	m_pCfg = pConfig;
	m_pConnector = new CConnector(this);
	if (0 == m_pConnector)
	{
		CRLog(E_CRITICAL,"%s", "No mem");
		return -1;
	}
	m_pConnector->Init(m_pCfg);

	return CProtocolCommImp::Init(pConfig);
}

void CProtocolCommClient::Finish()
{
	if (!StateCheck(CAction::esFinish))
		return;

	assert(0 !=  m_pConnector);
	if (0 == m_pConnector)
		return;

	m_pConnector->Finish();
	delete m_pConnector;
	m_pConnector = 0;

	CProtocolCommImp::Finish();
}

int CProtocolCommClient::Start()
{
	if (!StateCheck(esStart))
		return -1;
	
	assert(0 !=  m_pConnector);
	if (0 == m_pConnector)
		return -1;

	m_pConnector->Start();

	//通知连接线程进行连接
	m_pConnector->NotifyConnect();

	return CProtocolCommImp::Start();
}

void CProtocolCommClient::Stop()
{
	if (!StateCheck(esStop))
		return;
	
	assert(0 !=  m_pConnector);
	if (0 == m_pConnector)
		return;

	CRLog(E_SYSINFO,"%s","Stop Connetor");
	m_pConnector->Stop();

	CProtocolCommImp::Stop();
}

//用于断连后通知进行连接
void CProtocolCommClient::NotifyConnect()
{
	m_pConnector->NotifyConnect();
}

int CProtocolCommClient::OnConnect(tsocket_t sSocket,string sLocalIp,int nLocalPort,string sPeerIp,int nPeerPort,int nFlag)
{
	if (0 == nFlag)
	{
		m_nConnState = E_CONNECTED;
	}
	else
	{
		m_nConnState = E_DISCONNECTED;
	}

	CProtocolConnectPointClient* pCpCln = dynamic_cast<CProtocolConnectPointClient*>(m_pConnectPoint);
	assert(0 != pCpCln);
	if (0 == pCpCln)
	{
		CRLog(E_ERROR,"%s", "cast CProtocolConnectPointClient null");
		return -1;
	}

	return pCpCln->OnConnect(sLocalIp,nLocalPort,sPeerIp,nPeerPort,nFlag);
}

int CProtocolCommClient::GetConnState() const
{
	return m_nConnState;
}

/////////////////////////////////////////
CProtocolCommServer::CProtocolCommServer()
:CProtocolCommImp()
,m_pCfg(0)
,m_pListener(0)
{
}

CProtocolCommServer::~CProtocolCommServer()
{
	if (GetStage() != CAction::esFinish)
		Finish();
}



int CProtocolCommServer::Init(CConfig * pConfig)
{
	if (!StateCheck(esInit))
		return -1;
	
	assert(0 != pConfig);
	if (0 == pConfig)
		return -1;

	m_pCfg = pConfig;
	m_pListener = CreateListener(this);
	if (m_pListener == 0)
	{
		CRLog(E_CRITICAL,"%s", "No mem");
		return -1;
	}
	m_pListener->Init(m_pCfg);
	
	return CProtocolCommImp::Init(pConfig);
}


void CProtocolCommServer::Finish()
{
	if (!StateCheck(CAction::esFinish))
		return;

	assert(0 !=  m_pListener);
	if (0 ==  m_pListener)
		return;

	m_pListener->Finish();
	DestroyListener(m_pListener);
	m_pListener = 0;	

	CProtocolCommImp::Finish();
}


int CProtocolCommServer::Start()
{
	if (!StateCheck(esStart))
		return -1;
	
	assert(0 !=  m_pListener);
	if (0 ==  m_pListener)
		return -1;

	if (0 != m_pListener->Start())
		return -1;

	return CProtocolCommImp::Start();
}

void CProtocolCommServer::Stop()
{
	if (!StateCheck(esStop))
		return;
	
	assert(0 !=  m_pListener);
	if (0 ==  m_pListener)
		return;

	CRLog(E_SYSINFO,"%s","Stop Listner");
	m_pListener->Stop();

	CProtocolCommImp::Stop();
}

//
int CProtocolCommServer::OnAccept(tsocket_t sSocket,string sLocalIp,int nLocalPort,string sPeerIp,int nPeerPort)
{
	CProtocolConnectPointServer* pCpServer = dynamic_cast<CProtocolConnectPointServer*>(m_pConnectPoint);
	assert(0 != pCpServer);
	if (0 == pCpServer)
	{
		CRLog(E_ERROR,"%s", "cast CProtocolConnectPointServer null");
		return -1;
	}

	return pCpServer->OnAccept(sLocalIp,nLocalPort,sPeerIp,nPeerPort);
}

//封装TCP短连接客户端的通信层
//初始化 获对待IP和端口
int CTcpShortComm::Init(CConfig *	pConfig)
{
	assert(pConfig != 0);
	if (pConfig == 0)
		return -1;
	
	std::string sPort("");
	pConfig->GetProperty("port",sPort);
	m_nPort = FromString<int>(sPort);
	pConfig->GetProperty("ip",m_sIp);

	return 0;
}

//连接 0成功 -1失败
int CTcpShortComm::Connect(const string& sIp, int nPort)
{
	m_sSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_sSocket)
	{
		CRLog(E_SYSINFO,"%s","Error at Connect socket()\n");
		return -1;
	}

	sockaddr_in addrServer; 
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr(sIp.c_str());
	addrServer.sin_port = htons(nPort);
	if (SOCKET_ERROR == connect( m_sSocket, (struct sockaddr*) &addrServer, sizeof(addrServer) ))
	{
		closesocket(m_sSocket);
		CRLog(E_SYSINFO,"连[%s:%d]失败",sIp.c_str(), nPort);
		return -1;
	}

	m_nState = E_CONNECTED;
	CRLog(E_SYSINFO, "与[%s:%d]的连接成功", sIp.c_str(), nPort);

	return 0;
}

//连接 0成功 -1失败
int CTcpShortComm::Connect()
{
	m_sSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == m_sSocket)
	{
		CRLog(E_DEBUG,"%s","Error at Connect socket()\n");
		return -1;
	}

	sockaddr_in addrServer; 
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.s_addr = inet_addr(m_sIp.c_str());
	addrServer.sin_port = htons(m_nPort);
	if (SOCKET_ERROR == connect( m_sSocket, (struct sockaddr*) &addrServer, sizeof(addrServer) ))
	{
		closesocket(m_sSocket);
		CRLog(E_DEBUG,"连[%s:%d]失败",m_sIp.c_str(), m_nPort);
		return -1;
	}

	m_nState = E_CONNECTED;
	CRLog(E_DEBUG, "与[%s:%d]的连接成功", m_sIp.c_str(), m_nPort);

	return 0;
}

//连接状态
bool CTcpShortComm::IsConnected(){return m_nState == E_CONNECTED;}

//断开
void CTcpShortComm::DisConnect()
{
	closesocket(m_sSocket);
	m_nState = E_DISCONNECTED;
}

//发送解码后的数据
int CTcpShortComm::SendData(const char* pcBuf, int nLen)
{
	int nSendLeft = nLen;
	int nSendLen = 0;
	m_mutexConnector.Lock();
	do
	{
		nSendLen = send(m_sSocket,pcBuf,nSendLeft,0);
		nSendLeft -= nSendLen;
	} while (nSendLen > 0 && nSendLeft > 0);
	m_mutexConnector.Unlock();

	if (nSendLen < 0)
		return -1;

	return 0;
}

//接收数据 超时返回-1 正常返回接收到的字节数
int CTcpShortComm::RecvData(char* pcBuf,int& nLen,unsigned int uiTimeout)
{
	fd_set  fdRead; 
	struct  timeval timeout;
	timeout.tv_sec = uiTimeout;
	timeout.tv_usec = 0;

	int nRtn = 0;
	m_mutexConnector.Lock();

	//int nRead = -1;
	//int nLenLeft = nLen;
	//do
	//{
	//	int nReadThis = recv(m_sSocket, pcBuf, nLenLeft, 0);
	//	if (nReadThis == 0)
	//	{//对端关闭socket
	//		m_nState = E_DISCONNECTED;
	//		m_mutexConnector.Unlock();

	//		return nRead;
	//	}
	//	else if (nReadThis == SOCKET_ERROR || nReadThis < 0)
	//	{//错误
	//		m_nState = E_DISCONNECTED;
	//		m_mutexConnector.Unlock();
	//		return nRead;
	//	}
	//	nRead += nReadThis;
	//	pcBuf += nReadThis;
	//	nLenLeft -= nReadThis;
	//} while (true);
	FD_ZERO(&fdRead); 
	FD_SET(m_sSocket, &fdRead); 
	tsocket_t maxFd = m_sSocket;
	int nSelect = select(static_cast<int>(maxFd+1), &fdRead, 0, 0, &timeout); 
	if (nSelect == 0)
	{//超时
		m_mutexConnector.Unlock();
		return -1;
	}
	else if (nSelect == SOCKET_ERROR || nSelect < 0)
	{//出错
		m_mutexConnector.Unlock();
		return -1;
	}

	int nRead = recv(m_sSocket, pcBuf, nLen, 0);
	if (nRead == 0)
	{//对端关闭socket
		m_nState = E_DISCONNECTED;
		m_mutexConnector.Unlock();
		return -1;
	}
	else if (nRead == SOCKET_ERROR || nRead < 0)
	{//错误
		m_nState = E_DISCONNECTED;
		m_mutexConnector.Unlock();
		return -1;
	}

	m_mutexConnector.Unlock();
	return nRead;
}