
#ifndef AFX_ENCODE_H__BASE643DES_40D9_86DC_F4D25F73E58E__INCLUDED_
#define AFX_ENCODE_H__BASE643DES_40D9_86DC_F4D25F73E58E__INCLUDED_





class UTIL_CLASS Base643Des
{
public:
	Base643Des();

public:
	~Base643Des();
	
	///����ָ��������������ַ���
	static string BuildRand(int nLen);

	// ����ָ����ȡ�ļ�����ȡ3DES ����Key����Կ����
	///fileName �ļ���
	///pIvData ����
	///pKeyData ��Կ
	///outBuffer ����֮������
	static int ReadFileKey(const char *fileName,char *pIvData,char *pKeyData,char *psrcData);

	// ����ָ���ļ���д3DES ����Key����Կ����
	///fileName �ļ���
	///pIvData ����
	///pKeyData ��Կ
	///outBuffer ����֮������
	static int WriteFileKey(const char *fileName,char *pIvData,char *pKeyData,char *psrcData);

	// ����ָ����ȡ�ļ�����ȡ3DES ����Key����Կ���� ���ö�����ķ�ʽ���ļ����ܾ�дģʽ��
	///fileName �ļ���
	///pIvData ����
	///pKeyData ��Կ
	///outBuffer ����֮������
	static int ReadFileKey_DENYWR(const char *fileName,char *pIvData,char *pKeyData,char *psrcData);

	// ����ָ���ļ���д3DES ����Key����Կ���ģ����ļ���ռ��ʽ���ļ����ܾ���дģʽ��
	///fileName �ļ���
	///pIvData ����
	///pKeyData ��Կ
	///outBuffer ����֮������
	static int WriteFileKey_DENYRW(const char *fileName,char *pIvData,char *pKeyData,char *psrcData);

	// ����ָ���ַ�����key��3DES����
	///pszSrc ����
	///pdesIv ��������
	///pdesKey ������Կ
	///outBuffer ����֮������
	static int Des3Encrypt(char *pszSrc,char *pChDesIV,char *desKey,unsigned char *outBuffer);

	// ����ָ�������ġ�key��3DES����
	///pszSrc ����
	///pdesIv ��������
	///pdesKey ������Կ
	///outBuffer ����֮������
	static int Des3UnEncrypt(char *pszSrc,char *pChDesIV,char *desKey,unsigned char *outBuffer,int uiLen);

	//����ָ�������ַ�����key ѡ����3DES���ܺ����Base64����
	///pBufPwd ����֮������
	///pBufSrc ����
	///pdesIv  ��������
	///pdesKey ������Կ
	static int Des3Base64Enc(char *pBufPwd,char *pBufSrc,char *pdesIv,char *pdesKey);

	//����ָ��3DES�ַ�����������key,�Ƚ��н������н���
	///pBufSrcData ����֮������
	///pBufEncoded ����
	///pdesIv ��������
	///pdesKey ������Կ	
	static int Base64Dec3Des(unsigned char *pBufSrcData,char *pBufEncoded,char *pdesIv,char *pdesKey);

	//static int enBASE64(char *buf, int n, char *outbuf);

	//static int unBase64(char *buf, int n, char *outbuf);

	//private:
	//	static int	ENC64(int ch);
	//	static int	DEC64(char c);

	//�Ӽ����ļ�sFileName�ж�ȡ�����ܺ���ֶ�sStr(string)(ʹ�ö�����ʽ)
	//�������-1��������ļ�ʧ�ܣ��������-2��������ܳ���
	static int GetStrFromCodeFile(string sFileName,string & sStr);
	//���ֶ�sStr��char*�����ܺ�д������ļ�sFileName(ʹ�ö�ռ�ļ�ģʽ)
	//�������-1������д���ļ�ʧ�ܣ��������-2��������ܳ���
	static int SetStrToCodeFile(string sFileName,char * sStr);
};

#endif
