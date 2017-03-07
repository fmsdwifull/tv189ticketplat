#include <iostream>
#include "tinyxml.h"
#include "tinystr.h"
#include <string>
using namespace std;
/*
<?xml version="1.0" encoding="UTF-8"?>
<phonebook>
    <!--one item behalfs one contacted person.-->
    <item>
    	<name>miaomaio</name>
	<addr>Shaanxi Xi'an</addr>
	<tel>13759911917</tel>
	<email>miaomiao@home.com</email>
    </item>
    <item>
    	<name>gougou</name>
	<addr>Liaoning Shenyang</addr>
	<tel>15840330481</tel>
	<email>gougou@home.com</email>
    </item>
    <!--more contacted persons.-->
</phonebook>


TiXmlDeclarationָ�ľ���<?xml version="1.0" encoding="UTF-8"?>��
TiXmlCommentָ�ľ���<!--one item behalfs one contacted person.-->�� <!--more contacted persons.-->��
TiXmlDocumentָ�ľ�������xml�ĵ���
TiXmlElementָ�ľ���<phonebook>��<item>��<name>��<addr>�ȵ���Щ�ڵ㣬
TiXmlTextָ�ľ��ǡ�gougou������15840330481����Щ����<item>��</item>��<name>��</name>��<addr>��</addr>֮����ı����֣�
TiXmlAttributeָ�ľ���<?xml version="1.0" encoding="UTF-8"?>�ڵ���version��encoding��
����֮�����TiXmlUnknown��
*/

/*

*/

int main()
{
	//����һ��XML���ĵ�����
	const char* filepath = "report.xml";
	TiXmlDocument doc(filepath);
	bool loadOk = doc.LoadFile();	
	if (!loadOk) {	
		printf( "Could not load test file \n");
		return -1;
	}
	
	//��ø�Ԫ��
	TiXmlElement* root = doc.RootElement();
	
	//�����Ԫ������ ����phonebook
    cout << "RootElement value=" << root->Value() << endl; 
	
	
	printf("------------------------------------\n\n");
	// ��������ÿ���ڵ�
	for( TiXmlNode*  item = root->FirstChild( "item" );	item;
		item = item->NextSibling( "item" ) ) // nextsibling ��һ���ֵ�
	{
		// read name.
		TiXmlNode* child = item->FirstChild();
		const char* name = child->ToElement()->GetText();
		if (name) {
			printf("name:%s\n",name);
		} else {
			printf("name:\n");
		}

		// read address.
		child = item->IterateChildren(child);
		const char* addr = child->ToElement()->GetText();
		if (addr) {
			printf("addr:%s\n",addr);
		} else {
			printf("addr:\n");
		}

		// read telephone no.
		child = item->IterateChildren(child);
		const char* tel = child->ToElement()->GetText();
     	if (tel) {
			printf("tel:%s\n",tel);
		} else {
			printf("tel:\n");
		}

		// read e-mail.
		child = item->IterateChildren(child);
		const char* email = child->ToElement()->GetText();
		if(email) {
			printf("email:%s\n",email);
		} else {
			printf("email:\n");
		}
		
		printf("-------\n");
	
	}
	printf("---------------------------------------------\n");
	



	TiXmlElement *FirstPerson = root->FirstChildElement();
	cout << "FirstPerson value=" << FirstPerson->Value() << endl;
	
	TiXmlElement *NameElement = FirstPerson->FirstChildElement();  
    TiXmlElement *AddrElement = NameElement->NextSiblingElement();  
    TiXmlAttribute *IDAttribute = FirstPerson->FirstAttribute(); //��ȡID����

	
	printf("xxxxxxxxxxxxxxxxxxxx--0\n");
	cout << NameElement->FirstChild()->Value() << endl;  
    cout << AddrElement->FirstChild()->Value() << endl;  
    //cout << IDAttribute->Value() << endl; 
	
	
	// xml�������ļ� ��д�ļ�
	printf("xxxxxxxxxxxxxxxxxxxx--1\n");
	TiXmlElement* writeRoot = doc.RootElement();
	TiXmlNode*    newNode = new TiXmlElement("item");
    const TiXmlNode* name4NewNode = new TiXmlElement("name");
	newNode->InsertEndChild(*name4NewNode)->InsertEndChild(TiXmlText("pipi"));
	
	const TiXmlNode* addr4NewNode = new TiXmlElement("addr");
	newNode->InsertEndChild(*addr4NewNode)->InsertEndChild(TiXmlText("Shaanxi Xianyang"));

	const TiXmlNode* tel4NewNode = new TiXmlElement("tel");
	newNode->InsertEndChild(*tel4NewNode)->InsertEndChild(TiXmlText("02937310627"));
	
	const TiXmlNode* email4NewNode = new TiXmlElement("email");
	newNode->InsertEndChild(*email4NewNode)->InsertEndChild(TiXmlText("pipi@home.com"));
	
	writeRoot->InsertEndChild(*newNode);

	printf("xxxxxxxxxxxxxxxxxxxx--2\n");

	doc.SaveFile();	//



}
