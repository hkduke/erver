
#pragma once
//#include "SWindowsHead.h"
//#include "SMemoryLeakHead.h"
#include <string>
#include <vector>
#include "tinyxml2.h"
using namespace std;

namespace SDK
{

	class SXml
	{
	public:
		bool							LoadFile(const char *pPath);
		bool							LoadFileByString(const char *pStr);
		void							Save(const char *pPath=NULL);
		string							GetString();
	public:
		tinyxml2::XMLElement*			GetRootElement();
		tinyxml2::XMLElement*			GetElement(const char *pName);								
		tinyxml2::XMLElement*			GetElement(tinyxml2::XMLElement *pe,const char *pName);		
		vector<tinyxml2::XMLElement*>	GetChildren(tinyxml2::XMLElement *pe);
		const char*						GetName(tinyxml2::XMLElement *pe);
		tinyxml2::XMLElement*			CreateElement(tinyxml2::XMLElement *pParent,const char *pName);
		void							DeleteElement(tinyxml2::XMLElement *pParent,const char *pName);
		void							DeleteAllElement(tinyxml2::XMLElement *pParent);
	public:
		double							GetValue(tinyxml2::XMLElement *pe);
		const char*						GetValueString(tinyxml2::XMLElement *pe);
		void							SetValue(tinyxml2::XMLElement *pe,double d);
		void							SetValue(tinyxml2::XMLElement *pe,const char* p);
	public:
		bool							HaveAttrib(tinyxml2::XMLElement *pe,const char *pName);
		double							GetAttrib(tinyxml2::XMLElement *pe,const char *pName);
		const char*						GetAttribString(tinyxml2::XMLElement *pe,const char *pName);
		void							SetAttrib(tinyxml2::XMLElement *pe,const char *pName,double d);
		void							SetAttrib(tinyxml2::XMLElement *pe,const char *pName,const char *p);
		void							DeleteAttrib(tinyxml2::XMLElement *pe,const char *pName);
	private:
		tinyxml2::XMLDocument			m_doc;
		string							m_strFilePath;
	};
}
