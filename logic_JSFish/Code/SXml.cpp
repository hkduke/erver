#include "SXml.h"
//#include "SDebug.h"
using namespace SDK;
using namespace tinyxml2;


bool					SXml::LoadFile(const char *pPath)
{
	if(pPath==NULL)
	{
		//SHOW("SXml::LoadFile ��������:pPath==NULL");
		return false;
	}

	m_strFilePath=pPath;
	return m_doc.LoadFile(pPath)==XML_NO_ERROR;
}
bool					SXml::LoadFileByString(const char *pStr)
{
	if(pStr==NULL)
	{
		//SHOW("SXml::LoadFileByString ��������:pStr==NULL");
		return false;
	}

	return m_doc.Parse(pStr)==XML_NO_ERROR;
}
void					SXml::Save(const char *pPath/*=NULL*/)
{
	if(pPath==NULL && m_strFilePath.empty()==true)
	{
		//SHOW("SXml::Save ��������:pPath==NULL length:%d ",m_strFilePath.length());
		return;
	}

	if(pPath)
		m_doc.SaveFile(pPath);
	else
		m_doc.SaveFile(m_strFilePath.c_str());
}
string					SXml::GetString()
{
	XMLPrinter pt;
	m_doc.Accept(&pt);
	return string(pt.CStr());
}
XMLElement*				SXml::GetRootElement()
{
	XMLElement *pe=m_doc.RootElement();
	if(pe==NULL)
	{
		pe=m_doc.NewElement("root");
		m_doc.InsertEndChild(pe);
	}
	return pe;
}
XMLElement*				SXml::GetElement(const char *pName)
{
	if(pName==NULL)
	{
		//SHOW("SXml::GetElement ��������:pName==NULL");
		return NULL;
	}

	XMLElement *pe=GetRootElement();
	if(pe==NULL)
		return NULL;
	if(strcmp(GetName(pe),pName)==0)
		return pe;

	return GetElement(pe,pName);
}
XMLElement*				SXml::GetElement(XMLElement *pe,const char *pName)
{
	if(pName==NULL)
	{
		//SHOW("SXml::GetElement ��������:pName==NULL");
		return NULL;
	}

	vector<XMLElement*> v;
	v.push_back(pe);
	vector<XMLElement*> vChildren;

	while(true)
	{
		for(unsigned int i=0;i<v.size();++i)
		{
			vector<XMLElement*> vTemp=GetChildren(v[i]);
			vChildren.insert(vChildren.end(),vTemp.begin(),vTemp.end());
		}
		v=vChildren;
		if(vChildren.size()==0)
			return NULL;
		for(unsigned int i=0;i<vChildren.size();++i)
		{
			if(strcmp(GetName(vChildren[i]),pName)==0)
				return vChildren[i];
		}
		vChildren.clear();
	}
}
vector<XMLElement*>		SXml::GetChildren(XMLElement *pe)
{
		vector<XMLElement*> v;
	if(pe==NULL)
	{
		//SHOW("SXml::GetChildren ��������:pe==NULL");
		return v;
	}
	XMLElement *pTemp=pe->FirstChildElement();
	while(pTemp)
	{
		v.push_back(pTemp);
		pTemp=pTemp->NextSiblingElement();
	}

	return v;
}
const char*				SXml::GetName(XMLElement *pe)
{
	if(pe==NULL)
	{
		//SHOW("SXml::GetName ��������:pe==NULL");
		return "";
	}

	return pe->Value();
}
XMLElement*				SXml::CreateElement(XMLElement *pParent,const char *pName)
{
	if (pParent==NULL || pName==NULL)
	{
		//SHOW("SXml::CreateElement ��������, pParent=%p,pName=%p",pParent,pName);
		return NULL;
	}

	XMLElement *pe=m_doc.NewElement(pName);
	if (pe)
		pParent->InsertEndChild(pe);
	return pe;
}
void					SXml::DeleteElement(tinyxml2::XMLElement *pParent,const char *pName)
{
	if (pParent==NULL || pName==NULL)
	{
		//SHOW("SXml::DeleteElement ��������, pParent=%p,pName=%p",pParent,pName);
		return ;
	}	
		XMLElement *pe=GetElement(pParent,pName);
	if (pe)
		pParent->DeleteChild(pe);
}
void					SXml::DeleteAllElement(tinyxml2::XMLElement *pParent)
{
	if (pParent==NULL  )
	{
		//SHOW("SXml::DeleteAllElement ��������, pParent=%p",pParent);
		return ;
	}

	pParent->DeleteChildren();
}
double					SXml::GetValue(XMLElement *pe)
{
	if (pe==NULL  )
	{
		//SHOW("SXml::DeleteAllElement ��������, pe=%p",pe);
		return 0.0;
	}

	return atof(GetValueString(pe));
}
const char*				SXml::GetValueString(XMLElement *pe)
{
	if (pe==NULL  )
	{
		//SHOW("SXml::GetValueString ��������, pe=%p",pe);
		return NULL;
	}
	return pe->GetText();
}
void					SXml::SetValue(XMLElement *pe,double d)
{	
	if (pe==NULL  )
	{
		//SHOW("SXml::SetValue ��������, pe=%p",pe);
		return ;
	}

	pe->SetText(d);
}
void					SXml::SetValue(XMLElement *pe,const char* p)
{	
	if (pe==NULL || p==NULL )
	{
		//SHOW("SXml::SetValue ��������, pe=%p ,p=%p",pe,p);
		return ;
	}
	pe->SetText(p);
}
bool					SXml::HaveAttrib(XMLElement *pe,const char *pName)
{
	if (pe==NULL || pName==NULL )
	{
		//SHOW("SXml::HaveAttrib ��������, pe=%p ,pName=%p",pe,pName);
		return false ;
	}

	return pe->Attribute(pName)!=NULL;
}
double					SXml::GetAttrib(XMLElement *pe,const char *pName)
{
	if (pe==NULL || pName==NULL )
	{
		//SHOW("SXml::GetAttrib ��������, pe=%p ,pName=%p",pe,pName);
		return 0.0 ;
	}
	double temp=0;
	pe->QueryDoubleAttribute(pName,&temp);
	return temp;
}
const char*				SXml::GetAttribString(XMLElement *pe,const char *pName)
{
	if (pe==NULL || pName==NULL )
	{
		//SHOW("SXml::GetAttribString ��������, pe=%p ,pName=%p",pe,pName);
		return "" ;
	}
	return pe->Attribute(pName);
}
void					SXml::SetAttrib(XMLElement *pe,const char *pName,double d)
{
	
	if (pe==NULL || pName==NULL )
	{
		//SHOW("SXml::SetAttrib_d ��������, pe=%p ,pName=%p",pe,pName);
		return  ;
	}

	char szBuf[100]={};
	//sprintf_s(szBuf,100,"%f",d);
	snprintf(szBuf, 100, "%f", d);
	SetAttrib(pe,pName,szBuf);
}
void					SXml::SetAttrib(XMLElement *pe,const char *pName,const char *p)
{	

	if (pe==NULL || pName==NULL  || p==NULL)
	{
		//SHOW("SXml::SetAttrib_P ��������, pe=%p ,pName=%p ,p=%p",pe,pName,p);
		return  ;
	}

	pe->SetAttribute(pName,p);
}
void					SXml::DeleteAttrib(tinyxml2::XMLElement *pe,const char *pName)
{	
	if (pe==NULL || pName==NULL  )
	{
		//SHOW("SXml::DeleteAttrib ��������, pe=%p ,pName=%p ",pe,pName);
		return  ;
	}
	pe->DeleteAttribute(pName);
}