/******************************************************************************
��    Ȩ:��������������ϵͳ���޹�˾.
ģ������:GenericPacketHandler.cpp
������	:��ΰ
��������:2008.07.22
��    ��:1.0				
ģ������:��װ����TCP���Ľ��գ����ڻ���TCP��Ӧ�ò㱨�Ķ���ǧ���򻯣�������������
         ��ѡ�����������Ӧ
��Ҫ����:OnPacket(...) �麯���յ��������ĺ�ص�
		 GetPacketLength(...) ���ݱ��Ķ�������յ��Ĳ��ֱ���ͷ��������ձ��ĳ�
		 SendData(...) ���ͺ���
	     SendPacket(..) ���ͱ��Ľӿں���
�޸ļ�¼:
******************************************************************************/
#include <cassert>
#include "ProtocolProcess.h"
//#include "ProtocolComm.h"
#include "Logger.h"

//using namespace std;

//��ʼ��Ϊ4�ֽ�������ʶ����,���Ȱ�������ͷ
//PacketInfo CTcpHandler::m_PacketInfo = {4,0,ltInterger,true,0,hfNoUsed,0,tfNoUsed,0};
//bool CTcpHandler::m_blPacketInfoInited = false;

/******************************************************************************
��������:���ݱ��Ķ�������յ��Ĳ��ֱ���ͷ��������ձ��ĳ�
���ú���:�̺߳���
�������:char* pPacketData�ѽ��յ����ݻ���
����ֵ  :size_t �����ձ����ܳ���
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
#define MAX_CHARACTER_LEN	128
size_t CTcpHandler::GetPacketLength(const char* pPacketData)
{
	assert(pPacketData != 0);
	if (0 == pPacketData)
		return 0;

	size_t nLength = 0;
	if (m_PacketInfo.eLengthType == ltInterger)
	{//������ʶ����
		if (m_PacketInfo.nLengthBytes == 1)
		{
			nLength = *(pPacketData + m_PacketInfo.nLengthPos);
		}
		else if (m_PacketInfo.nLengthBytes == 2)
		{
			nLength = ntohs( *(reinterpret_cast<const unsigned short*>(pPacketData + m_PacketInfo.nLengthPos)));
		}
		else if (m_PacketInfo.nLengthBytes == 4)
		{
			nLength = ntohl( *(reinterpret_cast<const unsigned int*>(pPacketData + m_PacketInfo.nLengthPos)));
		}
		else
		{
			CRLog(E_CRITICAL, "�Ƿ��ĳ����ֶγߴ�: %d", m_PacketInfo.nLengthBytes);
			return 0;
		}
	}
	else if (m_PacketInfo.eLengthType == ltCharactersDec)
	{//10�����ַ�����ʶ����
		if (MAX_CHARACTER_LEN > m_PacketInfo.nLengthBytes)
		{
			char szLen[MAX_CHARACTER_LEN];
			memset(szLen,0x00,MAX_CHARACTER_LEN);
			memcpy(szLen, &pPacketData[m_PacketInfo.nLengthPos], m_PacketInfo.nLengthBytes);
			nLength = strtoul(szLen,0,10);
		}
		else
		{
			CRLog(E_CRITICAL, "�Ƿ��ĳ����ֶγߴ�: %d", m_PacketInfo.nLengthBytes);
			return 0;
		}
	}
	else if (m_PacketInfo.eLengthType == ltCharactersHex)
	{//16�����ַ�����ʶ����
		if (MAX_CHARACTER_LEN > m_PacketInfo.nLengthBytes)
		{
			char szLen[MAX_CHARACTER_LEN];
			memset(szLen,0x00,MAX_CHARACTER_LEN);
			memcpy(szLen, &pPacketData[m_PacketInfo.nLengthPos], m_PacketInfo.nLengthBytes);
			nLength = strtoul(szLen,0,16);
		}
		else
		{
			CRLog(E_CRITICAL, "�Ƿ��ĳ����ֶγߴ�: %d", m_PacketInfo.nLengthBytes);
			return 0;
		}
	}
	else
	{
		CRLog(E_CRITICAL, "�Ƿ��ĳ����ֶ�����: %d", m_PacketInfo.eLengthType);
		return 0;
	}
	
	//�����Ȳ���������ͷ,������ϱ���ͷ����
	if (!m_PacketInfo.blLenIncludeHeader)// && nLength!=0)
		nLength+=m_PacketInfo.nFixHeadLen;

	if ( nLength > MAX_PACKET_SIZE * 1024 * 4 )
	{   //ʧ��
		CRLog(E_ERROR,"Bad MSG PACK m_nLength = %d!", nLength );				
		return 0;
	}

	return nLength;
}

CTcpHandlerLong::CTcpHandlerLong()
:m_nLength(0)
,m_uiInTotal(0)
,m_uiInNowStat(0)
,m_uiInAveBytes(0)
{
	memset(&m_stRcvBuf,0x00, sizeof(DATA_BUF));
	for (int i = 0; i < eCommMaxStatic; i++)
	{
		m_uiLastStatics[i] = 0;;
		m_uiStatics[i] = 0;
	}
}

CTcpHandlerLong::~CTcpHandlerLong(void)
{
	if (0 != m_stRcvBuf.pBuf)
		delete []m_stRcvBuf.pBuf;
}

PDATA_BUF CTcpHandlerLong::GetRcvBufTcp()
{
	if (0 == m_stRcvBuf.pBuf)
	{
		m_stRcvBuf.pBuf = new char[MAX_PACKET_SIZE];
		m_stRcvBuf.uiLen = MAX_PACKET_SIZE;
		m_stRcvBuf.uiUsed = 0;
	}
	else if (m_stRcvBuf.uiLen == m_stRcvBuf.uiUsed)
	{
		
	}
	return &m_stRcvBuf;
}


/******************************************************************************
��������:socket�������������,�жϱ����Ƿ��������,��ͨ���ص������յ�����������
         �ϴ�
��������:GetPacketLength
�������:char* pBuf�ѽ��յ����ݻ���,nSize���泤��
����ֵ  :0��ȷ���� -1���մ���
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
int CTcpHandlerLong::OnRecvTcp(char* pBuf,int nSize)
{
	try
	{
		//ͳ��ƽ������
		//m_csStatics.Lock();
		//int nInterval = m_oBeginStatTm.IntervalToNow();
		//if (nInterval >= 5)
		//{
		//	m_uiInAveBytes = m_uiInNowStat / nInterval;
		//	m_oBeginStatTm.ToNow();
		//	m_uiInNowStat = 0;
		//}
		//m_uiInNowStat += nSize;
		//m_csStatics.Unlock();

		////ͳ���ۼ�����
		//UpdateStatics(eCommBytesRecvAll,nSize);

		//if (0 == m_stRcvBuf.pBuf)
		//{
		//	int nLen = max(MAX_PACKET_SIZE, nSize);
		//	m_stRcvBuf.pBuf = new char[nLen];
		//	m_stRcvBuf.uiLen = nLen;
		//}

		//if( m_nLength == 0 && nSize < m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
		//{
		//	CRLog(E_DEBUG,"��������δ���룬nSize = %d, m_nIndex = %d m_nLength = %d !", nSize, m_nIndex , m_nLength);	
		//	
		//	memcpy(m_stRcvBuf.pBuf,pBuf,nSize);
		//	m_stRcvBuf.uiUsed += nSize;
		//	return 0;
		//}

		////frame begin
		//if ( m_nLength == 0)
		//{
		//	if (0 == m_stRcvBuf.uiUsed)
		//	{
		//		m_nLength = GetPacketLength(pBuf);
		//	}
		//	else
		//	{
		//		memcpy(m_stRcvBuf.pBuf + m_stRcvBuf.uiUsed, pBuf, nSize);
		//		m_stRcvBuf.uiUsed += nSize;
		//		m_nLength = GetPacketLength(m_stRcvBuf.pBuf );
		//	}
		//	
		//	if ( m_nLength == 0 )
		//	{   //�����쳣Ϊ�Ұ����ʼ��
		//		//����֡ͷ������Ҫ��֡ͷͬ��...//
		//		CRLog(E_ERROR,"%s","�����쳣");
		//		UpdateStatics(eCommCountRecvErr);
		//		m_stRcvBuf.uiUsed = 0;
		//		return -1;
		//	}
		//}

		//if ( m_stRcvBuf.uiUsed <= m_nLength )
		//{
		//	memcpy(&m_szBuf[nLastIndex],pBuf,nSize);
		//	if ( m_nIndex == m_nLength )
		//	{   //�յ�������һ������ ֱ�Ӵ�������
		//		//����֡ͷ������Ҫ�ȼ��...//
		//		//����֡β���򰴹涨�㷨У���ദ����ȥ֡β��������...//
		//		UpdateStatics(eCommBytesRecv,m_nLength);
		//		if (0 > OnPacket(m_szBuf,m_nLength))
		//		{
		//			return -1;
		//		}
		//		m_nIndex = m_nLength = 0;
		//	}

		//	return 0;
		//}

		////�����Ƕ�һ���յ�����һ�����ĵĴ���
		////CRLog(E_DEBUG,"recv packet > 1: nSize = %d, m_nIndex = %d m_nLength = %d !", nSize, m_nIndex , m_nLength );
		//memcpy(&m_szBuf[nLastIndex],pBuf,m_nLength - nLastIndex);
		//if (0 > OnPacket(m_szBuf,m_nLength))
		//{
		//	return -1;
		//}
		//UpdateStatics(eCommBytesRecv,m_nLength);

		//pBuf += m_nLength - nLastIndex;
		//unsigned int nLeaveByte = nSize - (m_nLength - nLastIndex);
		//if( nLeaveByte < m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
		//{
		//	memcpy(m_szBuf,pBuf,nLeaveByte);
		//	m_nIndex = nLeaveByte;
		//	m_nLength = 0;
		//	return 0;
		//}

		////frame begin
		//unsigned int nLength = 0;
		//if ( nLeaveByte >= m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
		//{
		//	nLength = GetPacketLength(pBuf);
		//	if ( m_nLength == 0 )
		//	{   //�����쳣Ϊ�Ұ����ʼ��
		//		//����֡ͷ������Ҫ��֡ͷͬ��...//
		//		UpdateStatics(eCommCountRecvErr);
		//		m_nIndex = m_nLength = 0;
		//		return -1;
		//	}
		//}
		//
		//if ( nLeaveByte <= nLength )
		//{
		//	if ( nLeaveByte == nLength )
		//	{   //�յ�������һ������ ֱ�Ӵ�������
		//		//����֡ͷ������Ҫ�ȼ��...//
		//		//����֡β���򰴹涨�㷨У���ദ����ȥ֡β��������...//
		//		UpdateStatics(eCommBytesRecv,nLength);
		//		if (0 > OnPacket(pBuf,nLength))
		//		{
		//			return -1;
		//		}
		//		m_nIndex = m_nLength = 0;
		//	}
		//	else
		//	{
		//		memcpy(m_szBuf,pBuf,nLeaveByte);
		//		m_nIndex = nLeaveByte;
		//		m_nLength = nLength;
		//	}
		//	return 0;
		//}
		//
		//do
		//{
		//	//��ȷ��������
		//	//����֡ͷ������Ҫ�ȼ��...//
		//	//����֡β���򰴹涨�㷨У���ദ����ȥ֡β��������...//
		//	UpdateStatics(eCommBytesRecv,nLength);
		//	if (0 > OnPacket(pBuf,nLength))
		//	{
		//		return -1;
		//	}
		//	
		//	nLeaveByte -= nLength;
		//	pBuf += nLength;
		//	if ( nLeaveByte >=  m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
		//	{
		//		nLength = GetPacketLength(pBuf);
		//		if ( nLength == 0 )
		//		{   //�����쳣Ϊ�Ұ����ʼ��
		//			//����֡ͷ������Ҫ��֡ͷͬ��...//
		//			//IncreaseStatics(CommUpInErr);
		//			m_nIndex = m_nLength = 0;
		//			nLeaveByte = 0;
		//			return -1;
		//		}
		//		if ( nLeaveByte < nLength )
		//		{
		//			break;
		//		}
		//	}
		//	else
		//	{
		//		m_nLength = 0;
		//		break;
		//	}
		//} while ( nLeaveByte > 0 );

		//if ( nLeaveByte > 0 )
		//{
		//	memcpy( m_szBuf, pBuf, nLeaveByte );
		//	m_nIndex = nLeaveByte;
		//}
		//else
		//{
		//	m_nIndex = 0;
		//}
		return 0;		
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"%s","Unknown exception");
		return -1;
	}
}

/******************************************************************************
��������:socket�������������,�жϱ����Ƿ��������,��ͨ���ص������յ�����������
         �ϴ�,���漰�����Ѿ�����m_szBuf,m_nIndex
��������:GetPacketLength
����ֵ  :0��ȷ���� -1���մ���
������  :��ΰ
��������:2008.07.22
�޸ļ�¼:
******************************************************************************/
int CTcpHandlerLong::OnRecvTcp()
{
	try
	{
		//ͳ�ƽ���ƽ������ ������ʱÿ5�����ϼ��ͳ��һ��
		m_csStatics.Lock();
		int nInterval = m_oBeginStatTm.IntervalToNow();
		if (nInterval >= 5)
		{
			m_uiInAveBytes = m_uiInNowStat / nInterval;
			m_oBeginStatTm.ToNow();
			m_uiInNowStat = 0;
		}
		m_uiInNowStat += m_stRcvBuf.uiUsed;
		m_uiInTotal += m_stRcvBuf.uiUsed;
		m_csStatics.Unlock();

		//ͳ���ۼ�����
		UpdateStatics(eCommBytesRecvAll,m_stRcvBuf.uiUsed);

		if( m_nLength == 0 && m_stRcvBuf.uiUsed < m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
		{
			//CRLog(E_DEBUG,"��������δ���룬nSize = %d, m_nIndex = %d m_nLength = %d !", m_stRcvBuf.uiLen, m_nIndex , m_nLength);
			return 0;
		}

		//frame begin
		if ( m_nLength == 0)
		{
			m_nLength = GetPacketLength(m_stRcvBuf.pBuf);
			if ( m_nLength == 0 )
			{   //�����쳣Ϊ�Ұ����ʼ��
				//����֡ͷ������Ҫ��֡ͷͬ��...//
				CRLog(E_ERROR,"%s","�����쳣");
				UpdateStatics(eCommCountRecvErr);
				m_nLength = 0;
				m_stRcvBuf.uiUsed = 0;
				return -1;
			}
		}

		if ( m_stRcvBuf.uiUsed <= m_nLength )
		{
			int nRtn = 0;
			if ( m_stRcvBuf.uiUsed == m_nLength )
			{   //�յ�������һ������ ֱ�Ӵ�������
				//����֡ͷ������Ҫ�ȼ��...//
				//����֡β���򰴹涨�㷨У���ദ����ȥ֡β��������...//
				UpdateStatics(eCommBytesRecv,m_nLength);
				if (0 > OnPacket(m_stRcvBuf.pBuf,m_nLength))
				{
					return -1;
				}
				m_nLength = 0;
				m_stRcvBuf.uiUsed = 0;
			}
			else if (m_stRcvBuf.uiLen < m_nLength)
			{   //Ŀǰ����Ļ���������С�ڱ��ĳ���
				CRLog(E_DEBUG,"Ŀǰ����Ļ���������(%u)С�ڱ��ĳ���(%u)",m_stRcvBuf.uiLen,m_nLength);
				char* pBufTmp = m_stRcvBuf.pBuf;
				m_stRcvBuf.uiLen = m_nLength;
				m_stRcvBuf.pBuf = new char[m_stRcvBuf.uiLen];
				memcpy(m_stRcvBuf.pBuf, pBufTmp, m_stRcvBuf.uiUsed);
				delete []pBufTmp;
			}
			return nRtn;
		}

		//�����Ƕ�һ���յ�����һ�����ĵĴ���
		//CRLog(E_DEBUG,"recv packet > 1: nSize = %d, m_nIndex = %d m_nLength = %d !", nSize, m_nIndex , m_nLength );

		char* pszPos = m_stRcvBuf.pBuf;
		do
		{
			//��ȷ��������
			//����֡ͷ������Ҫ�ȼ��...//
			//����֡β���򰴹涨�㷨У���ദ����ȥ֡β��������...//
			UpdateStatics(eCommBytesRecv,m_nLength);
			if (0 > OnPacket(pszPos,m_nLength))
			{
				return -1;
			}
			pszPos += m_nLength;
			m_stRcvBuf.uiUsed -= m_nLength;
			if ( m_stRcvBuf.uiUsed >=  m_PacketInfo.nLengthPos + m_PacketInfo.nLengthBytes )
			{
				m_nLength = GetPacketLength(pszPos);
				if ( m_nLength == 0 )
				{   //�����쳣Ϊ�Ұ����ʼ��
					//����֡ͷ������Ҫ��֡ͷͬ��...//
					CRLog(E_ERROR,"%s","�����쳣");
					UpdateStatics(eCommCountRecvErr);
					m_nLength = 0;
					m_stRcvBuf.uiUsed = 0;
					return -1;
				}
				if ( m_stRcvBuf.uiUsed < m_nLength )
				{
					break;
				}
			}
			else
			{
				m_nLength = 0;
				break;
			}
		} while ( m_stRcvBuf.uiUsed > 0 );

		if ( m_stRcvBuf.uiUsed > 0 )
		{
			if (m_stRcvBuf.uiLen < m_nLength)
			{
				CRLog(E_DEBUG,"Ŀǰ����Ļ���������(%u)С�ڱ��ĳ���(%u)",m_stRcvBuf.uiLen,m_nLength);
				char* pBufTmp = m_stRcvBuf.pBuf;
				m_stRcvBuf.uiLen = m_nLength;
				m_stRcvBuf.pBuf = new char[m_stRcvBuf.uiLen];
				memcpy(m_stRcvBuf.pBuf, pBufTmp, m_stRcvBuf.uiUsed);
				delete []pBufTmp;
			}
			else
			{
				memcpy( m_stRcvBuf.pBuf, pszPos, m_stRcvBuf.uiUsed );
			}
		}
		else
		{
			m_stRcvBuf.uiUsed = 0;
		}
		return 0;
	}
	catch(std::exception e)
	{
		CRLog(E_CRITICAL,"exception:%s", e.what());
		return -1;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"%s","Unknown exception");
		return -1;
	}
}

void CTcpHandlerLong::UpdateStatics(eCommStaticType eType, int nCount)
{
	if (eCommMaxStatic == eType)
		return;

	m_uiStatics[eType] += nCount;

	//do something?

	m_uiLastStatics[eType] += nCount;
}

void CTcpHandlerLong::GetInFlowStatics(unsigned int & uiInTotalStat,unsigned int & uiInAveBytes)
{
	m_csStatics.Lock();
	int nInterval = m_oBeginStatTm.IntervalToNow();
	if (nInterval >= 5)
	{
		m_uiInAveBytes = m_uiInNowStat / nInterval;
		m_oBeginStatTm.ToNow();
		m_uiInNowStat = 0;
	}

	uiInTotalStat = m_uiInTotal;
	uiInAveBytes = m_uiInAveBytes;
	m_csStatics.Unlock();
}

CAppProcess::CAppProcess()
:m_pComm(0)
,m_sSocket(INVALID_SOCKET)
,m_uiOutBufPkts(0)
,m_uiOutBufBytes(0)
,m_uiOutBufPktsTotal(0)
,m_uiOutBufBytesTotal(0)
,m_uiOutBufCount(0)
,m_uiOutTotal(0)
,m_uiOutNowStat(0)
,m_uiOutAveBytes(0)
,m_pDataWait(0)
,m_nLenWait(0)
{
	for (int i = 0; i < eProcessMaxStatic; i++)
	{
		m_uiStatics[i] = 0;
	}

	m_oProcessTimer.Bind(this);
}

CAppProcess::~CAppProcess()
{
	if (0 != m_pDataWait)
	{
		delete []m_pDataWait;
	}
	for(deque<DATA_BUF>::iterator it = m_deqDataBuf.begin(); it != m_deqDataBuf.end(); ++it)
	{
		delete (*it).pBuf;
	}
}
	
	
//��Э��ͨѶ�������󶨵�Э�����̴�����
void CAppProcess::Bind(CProtocolComm* p)
{
	m_pComm = p;
}

//����/��ȡЭ�����̴�������Ӧ��socket
void CAppProcess::SocketID(tsocket_t sScoket)
{
	m_sSocket = sScoket;
}

tsocket_t  CAppProcess::SocketID()
{ 
	return m_sSocket;
}

void CAppProcess::SetPeerInfo(const string& sLocalIp,int nLocalPort,const string& sPeerIp,int nPeerPort)
{
	m_sLocalIp = sLocalIp;
	m_nLocalPort = nLocalPort;
	m_sPeerIp = sPeerIp;
	m_nPeerPort = nPeerPort;
}

void CAppProcess::GetPeerInfo(string& sLocalIp,int& nLocalPort,string& sPeerIp,int& nPeerPort)
{
	sLocalIp = m_sLocalIp;
	nLocalPort = m_nLocalPort;
	sPeerIp = m_sPeerIp;
	nPeerPort = m_nPeerPort;
}


int CAppProcess::SendPacket(CPacket & Packet)
{
	int nRtn = 0;
	unsigned int  nLen = 0;
	const char* pcBuf = Packet.Encode(nLen);
	if (0 == nLen)
		return 0;

	CMutexGuard oGard(m_csProcess);
	if (m_nLenWait > 0 || m_deqDataBuf.size() > 0)
	{
		DATA_BUF stBuf;
		stBuf.pBuf = new char[nLen];
		memcpy(stBuf.pBuf, pcBuf, nLen);
		stBuf.uiLen = nLen;
		stBuf.uiUsed = nLen;
		m_deqDataBuf.push_back(stBuf);

		m_uiOutBufBytes += nLen;
		m_uiOutBufPkts++;
		m_uiOutBufBytesTotal += nLen;
		m_uiOutBufPktsTotal++;

		nRtn = -2;
	}
	else
	{
		nRtn = SendData(pcBuf, nLen);
	}
	return nRtn;
}

//��������
int CAppProcess::SendData(const char* pBuf,int nSize)
{
	assert(0 != m_pComm);
	if (0 == m_pComm)
	{
		CRLog(E_ERROR,"%s","0 Comm");
		return -1;
	}

	CMutexGuard oGard(m_csProcess);
	int nInterval = m_oBeginStatTm.IntervalToNow();
	if (nInterval >= 5)
	{
		m_uiOutAveBytes = m_uiOutNowStat / nInterval;
		m_oBeginStatTm.ToNow();
		m_uiOutNowStat = 0;
	}

	int nRtn = m_pComm->SendData(this,pBuf,nSize);
	if (nRtn > 0)
	{
		m_uiOutNowStat += nRtn;
		m_uiOutTotal += nRtn;
	}

	if (0 < nRtn && nRtn < nSize)
	{
		m_nLenWait = nSize - nRtn;
		m_pDataWait = new char[m_nLenWait];
		memcpy(m_pDataWait, pBuf+nRtn, m_nLenWait);
		m_uiOutBufBytes += m_nLenWait;
		m_uiOutBufBytesTotal += m_nLenWait;
	}
	else if (-2 == nRtn)
	{
		DATA_BUF stBuf;
		stBuf.pBuf = new char[nSize];
		memcpy(stBuf.pBuf, pBuf, nSize);
		stBuf.uiLen = nSize;
		stBuf.uiUsed = nSize;
		m_deqDataBuf.push_back(stBuf);

		m_uiOutBufBytes += nSize;
		m_uiOutBufPkts ++;
		m_uiOutBufBytesTotal += nSize;
		m_uiOutBufPktsTotal++;
	}
	return nRtn;
}

int CAppProcess::OnSend(int nSize)
{
	if (m_uiOutBufCount % 10 == 0)
	{
		CRLog(E_DEBUG, "OnSend(%d):%u", m_sSocket, m_deqDataBuf.size());
	}

	int nRtn = 0;
	CMutexGuard oGard(m_csProcess);
	m_uiOutBufCount++;

	try
	{
		if (m_nLenWait > 0)
		{
			nRtn = m_pComm->SendData(this,m_pDataWait,m_nLenWait);
			if (nRtn <= 0)
				 return 0;

			if (nRtn < m_nLenWait)
			{
				m_nLenWait = m_nLenWait - nRtn;
				memcpy(m_pDataWait, m_pDataWait+nRtn, m_nLenWait);
				m_uiOutBufBytes -= nRtn;
				m_uiOutTotal += nRtn;
				return 0;
			}
			else
			{
				delete []m_pDataWait;
				m_pDataWait = 0;
				m_nLenWait = 0;
				m_uiOutBufBytes -= nRtn;
				m_uiOutTotal += nRtn;
			}
		}

		deque<DATA_BUF>::iterator it;
		for (it = m_deqDataBuf.begin(); it != m_deqDataBuf.end(); )
		{
			nRtn = m_pComm->SendData(this,(*it).pBuf,(*it).uiUsed);
			if (nRtn <= 0)
			{
				//CRLog(E_ERROR, "send err:%d", nRtn);
				return 0;
			}

			if (nRtn < (*it).uiUsed)
			{
				m_nLenWait = (*it).uiUsed - nRtn;
				m_pDataWait = new char[m_nLenWait];
				memcpy(m_pDataWait, (*it).pBuf + nRtn, m_nLenWait);
				delete [](*it).pBuf;
				it = m_deqDataBuf.erase(it);

				m_uiOutBufBytes -= nRtn;
				m_uiOutBufPkts--;
				m_uiOutTotal += nRtn;
				break;
			}
			else
			{
				delete [](*it).pBuf;
				it = m_deqDataBuf.erase(it);

				m_uiOutBufPkts--;
				m_uiOutBufBytes -= nRtn;
				m_uiOutTotal += nRtn;
			}
		}

		return 0;
	}
	catch(...)
	{
		CRLog(E_CRITICAL,"Unknown exception%s","");
		return 0;
	}
}

void CAppProcess::ReqClose()
{
	m_pComm->ReqClose(this);
}

//����Э��㱨��ͳ������
void CAppProcess::UpdateStatics(eProcessStaticType eType,int nCount)
{
	if (eProcessMaxStatic == eType)
		return;

	m_uiStatics[eType] += nCount;

	//do something?

	m_uiStatics[eType] += nCount;
}

//ȡЭ��㱨��ͳ������
void CAppProcess::GetProcessStatics(unsigned int	uiStatics[eProcessMaxStatic])
{
	for (int i = 0; i < eProcessMaxStatic; i++)
	{
		uiStatics[i] = m_uiStatics[i];
	}
}

//ȡͨ�ų���������
void CAppProcess::GetOutFlowStatics(unsigned int & uiOutTotalStat,unsigned int & uiOutAveBytes, unsigned int& uiOutBufBytes, unsigned int& uiOutBufPkts, unsigned int& uiOutBufBytesTotal, unsigned int& uiOutBufPktsTotal, unsigned int& uiOutBufCount)
{
	CMutexGuard oGard(m_csProcess);
	int nInterval = m_oBeginStatTm.IntervalToNow();
	if (nInterval >= 5)
	{
		m_uiOutAveBytes = m_uiOutNowStat / nInterval;
		m_oBeginStatTm.ToNow();
		m_uiOutNowStat = 0;
	}

	uiOutAveBytes = m_uiOutAveBytes;
	uiOutTotalStat = m_uiOutTotal;
	uiOutBufBytes = m_uiOutBufBytes;
	uiOutBufPkts = m_uiOutBufPkts;
	
	uiOutBufBytesTotal = m_uiOutBufBytesTotal;
	uiOutBufPktsTotal = m_uiOutBufPktsTotal;
	uiOutBufCount = m_uiOutBufCount;
}

int CAppProcess::CreateTimer(unsigned long& ulTmSpan)
{
	return CGessTimerMgrImp::Instance()->CreateTimer(&m_oProcessTimer,ulTmSpan,"PTimer");
}

void CAppProcess::DestroyTimer() 
{
	CGessTimerMgrImp::Instance()->DestroyTimer(&m_oProcessTimer,"PTimer");
}
	