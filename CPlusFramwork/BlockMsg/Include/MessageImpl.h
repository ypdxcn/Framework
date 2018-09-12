#ifndef _MESSAGE_IMPL_H
#define _MESSAGE_IMPL_H

#include <string>
#include <map>
#include "BlockMsg.h"

class BLOCKMSG_CLASS CMessage
{
public:
	virtual int GetField(int nKey, std::string & sValue) const = 0;
	virtual int SetField(int nKey, const std::string & sValue) = 0;

	virtual int GetBinaryField(int nKey, std::string & sValue) const = 0;
	virtual int SetBinaryField(int nKey, const std::string & sValue) = 0;

	virtual int GetField(int nKey, unsigned short & usValue) const = 0;
	virtual int SetField(int nKey, unsigned short usValue) = 0;

	virtual int GetField(int nKey, unsigned int & uiValue) const = 0;
	virtual int SetField(int nKey, unsigned int uiValue) = 0;

	//virtual int GetField(int nKey, unsigned long & ulValue) const = 0;
	//virtual int SetField(int nKey, unsigned long ulValue) = 0;


	//virtual int GetField(int nKey, short & sValue) const = 0;
	//virtual int SetField(int nKey, short sValue) = 0;

	//virtual int GetField(int nKey, int & nValue) const = 0;
	//virtual int SetField(int nKey, int nValue) = 0;

	//virtual int GetField(int nKey, long & lValue) const = 0;
	//virtual int SetField(int nKey, long lValue) = 0;


	virtual int Copy(const CMessage & srcMsg, bool bReplace) = 0;

	virtual int Erase(int nKey) = 0;
	virtual void Clear() = 0;
	virtual size_t Size() const = 0; 
	virtual std::vector<int> GetKeys() const = 0;
	virtual std::string ToString() const = 0;
	virtual int FromString(const std::string& sMsg,char cParaSeperator = '&',char cValSeperator = '=') = 0;
};

class BLOCKMSG_CLASS CMessageImpl : public CMessage
{
public:
	CMessageImpl(void);
	CMessageImpl(const CMessageImpl & msg);
	~CMessageImpl(void);
	
	int GetField(int nKey, std::string & sValue) const;
	int SetField(int nKey, const std::string & sValue);

	int GetBinaryField(int nKey, std::string & sValue) const;
	int SetBinaryField(int nKey, const std::string & sValue);

	int GetField(int nKey, unsigned short & usValue) const;
	int SetField(int nKey, unsigned short usValue);

	int GetField(int nKey, unsigned int & uiValue) const;
	int SetField(int nKey, unsigned int uiValue);
	
	int SetField(int nKey, char cType, const std::string & sValue);
	int GetField(int nKey, char cType, std::string & sValue) const;

	int GetField(int nKey, unsigned long & ulValue) const;
	int SetField(int nKey, unsigned long ulValue);


	//int GetField(int nKey, short & sValue) const;
	//int SetField(int nKey, short sValue);

	//int GetField(int nKey, int & nValue) const;
	//int SetField(int nKey, int nValue);

	//int GetField(int nKey, long & lValue) const;
	//int SetField(int nKey, long lValue);

	int Copy(const CMessage & srcMsg, bool bReplace);

	int Erase(int nKey);
	void Clear();
	size_t Size() const; 
	std::vector<int> GetKeys() const;
	std::string ToString() const;
	int FromString(const std::string& sMsg,char cParaSeperator = '&',char cValSeperator = '=');
private:
	enum _enumFieldType
	{
		SHORT = 'n',
		LONG = 'l',
		STRING = 's',
		BINARY = 'b'
	};

	typedef struct _tagField
	{
		char cType;	// 'n': 16位整数 'l': 32位整数 't': 本地化文本 's': 字符串 'b': 二进制
		unsigned int ulValue;
		std::string sValue;
	} FIELD_ITEM;
	typedef std::map<int, FIELD_ITEM> FIELD_MAP;
	FIELD_MAP m_Fields;

};

#endif