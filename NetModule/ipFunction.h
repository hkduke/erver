/*----------------- timeFunction.h
*
* Copyright (C): 2017  GaoXin Network
* Author       : luwei
* Version      : V1.0
* Date         : 2017-09-22 17:23:16
*--------------------------------------------------------------
*
*------------------------------------------------------------*/
#include <vector>
#include <string>

#pragma once

//--- 通过域名获取IP
extern bool	GetIpByDomainName(char *szHost, std::vector<std::string>& vecIp);