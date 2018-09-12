#ifndef _NM_CONSTANT_H_
#define _NM_CONSTANT_H_

#include <string>
using namespace std;

namespace NmConst
{
	const int gc_nQuolityGood				= 0;				//��������Good
	const int gc_nQuolityBad				= 1;				//��������Bad
	const int gc_nQuolityUncertain			= 2;				//��������Uncertain

	const string gc_nItemTypeDigital		= "0";				//�����ͼ����(ֵ��ʱ����ɢ�仯)
	const string gc_nItemTypeAnalog			= "1";				//ģ���ͼ����(ֵ��ʱ�������仯)
	const string gc_nItemTypeConfig			= "2";				//������(ֵ����ʱ��仯)

	const int gc_nValueTypeInt				= 0;				//int��ֵ����
	const int gc_nValueTypeDbl				= 1;				//double��ֵ����
	const int gc_nValueTypeStr				= 2;				//string��ֵ����

	const int gc_nEvtSimple					= 0;				//���¼�
	const int gc_nEvtTrack					= 1;				//track�¼�

	const int gc_nAlmGrade0					= 0;				//����
	const int gc_nAlmGrade1					= 1;				//һ��澯
	const int gc_nAlmGrade2					= 2;				//��Ҫת��

	const int gc_nAlmNtfNew					= 0;				//�����澯
	const int gc_nAlmNtfEnd					= 1;				//�澯����
	const int gc_nAlmNtfChg					= 2;				//�澯ת��
}

#endif