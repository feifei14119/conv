
#include <string.h>
#include <iostream>

#include "ff_log.h"
#include "ff_cmd_args.h"

namespace feifei 
{
	CmdArgs * CmdArgs::pCmdArgs = nullptr;

	CmdArgs::CmdArgs(int argc, char *argv[])
	{
		argsNum = 0;
		argsMap = new std::map<E_ArgId, T_CmdArg*>();
		pCmdArgs = this;
		initCmdArgs();

		paserCmdArgs(argc, argv);
	}
	CmdArgs::CmdArgs()
	{
		argsNum = 0;
		argsMap = new std::map<E_ArgId, T_CmdArg*>();
		pCmdArgs = this;
		initCmdArgs();

		paserCmdArgs(0, nullptr);
	}
	CmdArgs * CmdArgs::GetCmdArgs()
	{
		if (pCmdArgs == nullptr)
			pCmdArgs = new CmdArgs();
		
		return pCmdArgs;
	}

	void * CmdArgs::GetOneArg(E_ArgId id)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;
		it = argsMap->find(id);

		if (it == argsMap->end())
		{
			return nullptr;
		}

		return getOneArgValue(it->second);
	}

	/*
	* NEED TO DO!
	* 添加默认参数和命令
	*/
	void CmdArgs::initCmdArgs()
	{
		addOneArg(CMD_ARG_HELP, E_DataType::String, "help", '\0', "help", "help infomation");
		addOneArg(CMD_ARG_DEVICE, E_DataType::Int, "0", 'd', "device", "specify a device");
		addOneArg(CMD_ARG_EVINFO, E_DataType::Int, "0", '\0', "evinfo", "get environment info");
		addOneArg(CMD_ARG_W, E_DataType::Int, "14", 'w', "width", "specify input tensor width");
		addOneArg(CMD_ARG_H, E_DataType::Int, "14", 'h', "height", "specify input tensor height");
		addOneArg(CMD_ARG_C, E_DataType::Int, "1024", 'c', "chanin", "specify input channel");
		addOneArg(CMD_ARG_K, E_DataType::Int, "64", 'k', "chanout", "specify output channel");
		addOneArg(CMD_ARG_N, E_DataType::Int, "1", 'n', "batch", "specify batch size");
		addOneArg(CMD_ARG_UV, E_DataType::Int, "1", 'u', "stride", "specify stride size");
		addOneArg(CMD_ARG_BIAS, E_DataType::Int, "0", 'b', "bias", "specify if use fusion bias opt");
		addOneArg(CMD_ARG_RELU, E_DataType::Int, "0", 'r', "relu", "specify if use fusion relu opt");
		addOneArg(CMD_ARG_LOOP, E_DataType::Int, "10", 'i', "loop", "specify loop times");
		addOneArg(CMD_ARG_SEARCH, E_DataType::Int, "1", '\0', "search", "specify search methord: 0=brute_search; 1=genetic_search");
	}
	void CmdArgs::addOneArg(E_ArgId id, E_DataType dType, std::string defaultVal, char sName, std::string lName, std::string tHelp)
	{
		T_CmdArg *arg = new T_CmdArg;

		arg->id = id;
		arg->type = dType;
		arg->value = defaultVal;
		arg->shortName = sName;
		arg->longName = lName;
		arg->helpText = tHelp;
		setOneArgValue(arg, defaultVal);

		argsMap->insert(std::pair<E_ArgId, T_CmdArg*>(id, arg));
		argsNum++;
	}

	/*
	 * 解析所有命令行参数
	 */
	void CmdArgs::paserCmdArgs(int argc, char *argv[])
	{
		if (argc == 0)
		{
			executePath = std::string(".") + DIR_SPT;
			return;
		}

		// use first arg to get current path
		size_t p;
		p = std::string(argv[0]).rfind(DIR_SPT);

		if (p == std::string::npos)
		{
			executePath = std::string(".") + DIR_SPT;
		}
		else
		{
			executePath = std::string(argv[0]).substr(0, p);
		}

		// other args
		for (int i = 1; i < argc; i++)
		{
			if ((std::string(argv[i]) == "--help") || (std::string(argv[i]) == "-h"))
			{
				helpText();
				exit(0);
			}
			else if ((argv[i][0] == '-')&&(argv[i][1] == '-'))
			{
				if (argv[i + 1][0] != '-')
					setOneArg(std::string(&argv[i][2]), std::string(argv[i + 1]));
			}
			else if (argv[i][0] == '-')
			{
				if(argv[i + 1][0]!='-')
					setOneArg(argv[i][1], std::string(argv[i + 1]));
			}
		}
	}
	void CmdArgs::helpText()
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->shortName != '\0')
				printf("-%c", it->second->shortName);
			else
				printf("  ");

			if (it->second->longName != "")
				printf(", --%s", it->second->longName.c_str());

			switch (it->second->type)
			{
			case E_DataType::Int:printf("(%d)", it->second->iValue); break;
			case E_DataType::Float:printf("(%.2f)", it->second->fValue); break;
			case E_DataType::String:printf("(%s)", it->second->sValue.c_str()); break;
			default: break;
			}

			if (it->second->helpText != "")
				printf(": %s", it->second->helpText.c_str());

			printf(".\n");
		}
	}

	/*
	* 根据命令行找到一个对应参数 
	*/
	E_ReturnState CmdArgs::setOneArg(char sName, std::string value)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->shortName == sName)
				break;
		}

		if (it == argsMap->end())
		{
			FATAL("no such param -%c.", sName);
		}

		return setOneArgValue(it->second, value);
	}
	E_ReturnState CmdArgs::setOneArg(std::string lName, std::string value)
	{
		std::map<E_ArgId, T_CmdArg *>::iterator it;

		for (it = argsMap->begin(); it != argsMap->end(); it++)
		{
			if (it->second->longName == lName)
				break;
		}

		if (it == argsMap->end())
		{
			FATAL("no such param --%s.", lName.c_str());
		}

		return setOneArgValue(it->second, value);
	}
	
	/*
	* 根据命令行,设置/获取一个参数的值
	* 参数类型在初始化阶段设置
	*/
	E_ReturnState CmdArgs::setOneArgValue(T_CmdArg * arg, std::string value)
	{
		switch (arg->type)
		{
		case E_DataType::Int: 
			arg->iValue = atoi(value.c_str()); 
			break;
		case E_DataType::Float:
			arg->fValue = atof(value.c_str());
			break;
		case E_DataType::String:
			arg->sValue = value;
			break;
		default: 
			break;
		}
		return E_ReturnState::SUCCESS;
	}
	void * CmdArgs::getOneArgValue(T_CmdArg * arg)
	{
		switch (arg->type)
		{
		case E_DataType::Int:		return &(arg->iValue);
		case E_DataType::Float:		return &(arg->fValue);
		case E_DataType::String:	return &(arg->sValue);
		default:					return nullptr;
		}
	}
}
