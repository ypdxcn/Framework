#ifndef _LISTENER_H
#define _LISTENER_H

#include "Comm.h"
#include "WorkThread.h"
#include "osdepend.h"

class CProtocolCommServer;
class COMM_CLASS CListener:virtual public CAction
{
	//friend class CProtocolCommServer;
public: 
	CListener(CProtocolCommServer *p);
	virtual ~CListener();

	virtual int Init(CConfig *	pConfig);
	virtual void Finish();
	virtual int Start();
	virtual void Stop();	
protected:
	virtual CAio* CreateAioListen() = 0;

	CProtocolCommServer * m_pCommServer;		//Э��ͨ�Ŵ�����(�����)
	CAio *m_pAioListen;							//

	 //����IP�Ͷ˿�
	std::string m_sIp;
	int m_nPort;
	int m_nWaitConnNum;

	tsocket_t m_sListen;
private:
	//ACL���
	int						m_nAclAvtive;
	map<string, string>		m_mapAclIp;
	int						m_nExceptPortStatic;
public: 
	virtual int OnAccept(tsocket_t sSocket,const string& sIp,int nPort);
	virtual int OnAccept();
	inline tsocket_t SocketID(){return m_sListen;}
	inline int ListenPort(){return m_nPort;}
};
#endif