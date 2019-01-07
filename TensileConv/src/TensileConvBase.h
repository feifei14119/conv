#pragma once

#include <limits>
#include <stdarg.h>
#include <vector>
#include <list>

#include "../common/ff_utils.h"
#include "AutoTuning.h"

#include "unistd.h"

namespace TensileConv {

/************************************************************************/
/* solution得分                                                         */
/************************************************************************/
typedef struct ScoreTypde
{
	double ElapsedTime;	//(s)
	double Flops;		//(Flops)
	double Performence;	//(%)
}T_Score;

/************************************************************************/
/* solution 控制 (so called generic solver)			                    */
/************************************************************************/
class ProblemCtrlBase;
class SolutionCtrlBase
{
public:
	SolutionCtrlBase(ProblemCtrlBase * problem)
	{
		repeatTime = 100;
		solutionScore.ElapsedTime = (std::numeric_limits<double>::max)();
		solutionScore.Performence = 0;

		cmdArgs = CmdArgs::GetCmdArgs();
		rtOcl = RuntimeOCL::GetInstance();
		stream = rtOcl->CreatCmdQueue(true);

		this->problem = problem;
		solutionParamSpace = new AutoTune::SearchSpace();
	}
	virtual ~SolutionCtrlBase() { delete stream; delete solutionParamSpace; }

	void RunSolution();

	std::string KernelName() { return kernelName; }
	dim3 GroupSize() { return group_sz; }
	dim3 GlobalSize() { return global_sz; }

protected:
	CmdArgs * cmdArgs;
	RuntimeOCL * rtOcl;
	CmdQueueOCL* stream;	// one command queue for all solution configs
	KernelOCL * kernel;
	cl_event profEvt;

	ProblemCtrlBase * problem;
	std::string solutionName;						// 配置名称
	AutoTune::SearchSpace *solutionParamSpace;		// 解决方案参数搜索空间

	std::string kernelName;
	dim3 group_sz;
	dim3 global_sz;

	int repeatTime;
	T_Score configScore;				// 当前配置的平均性能
	T_Score solutionScore;				// 全部配置的平均性能

	virtual E_ReturnState generateSolutionParamSpace() = 0;
	virtual E_ReturnState getKernelParam() {}
	virtual E_ReturnState generateKernel() = 0;
	virtual E_ReturnState prepareKernelArgs() = 0;
	virtual E_ReturnState launchKernel();
	virtual void getBackResult() = 0;
	virtual void releaseDevMem() = 0;
	virtual void getBestKernel() {}

	// 打印下标
	void printIndex(int *index, char* name, dim3 g_wk, dim3 l_wk);
};

/************************************************************************/
/* 问题句柄																*/
/************************************************************************/
class ProblemCtrlBase
{
public:
	ProblemCtrlBase()
	{
		cmdArgs = CmdArgs::GetCmdArgs();
		rtOcl = RuntimeOCL::GetInstance();

		problemParamSpace = new AutoTune::SearchSpace();
	}
	ProblemCtrlBase(std::string name)
	{
		problemName = name;

		cmdArgs = CmdArgs::GetCmdArgs();
		rtOcl = RuntimeOCL::GetInstance();

		problemParamSpace = new AutoTune::SearchSpace();
	}
	virtual ~ProblemCtrlBase() { delete problemParamSpace; }

	void RunAllProblem();
	SolutionCtrlBase * Solution() { return solution; }
	double Calculation() { return calculation; }
	double TheoryElapsedTime() { return theoryElapsedTime; }

	// TODO: dump/load input/output data

protected:
	CmdArgs * cmdArgs;
	RuntimeOCL * rtOcl;
	SolutionCtrlBase * solution;

	std::string problemName;
	AutoTune::SearchSpace *problemParamSpace;		// 问题参数搜索空间

	double calculation;					// 当前正在处理的问题配置的计算量
	double theoryElapsedTime;			// 当前正在处理的问题配置的理论执行时间

	virtual E_ReturnState initHostParam() = 0;
	virtual E_ReturnState runHostCompute() = 0;
	virtual E_ReturnState verifyDevCompute() = 0;
	virtual void releaseHostParam() = 0;
	virtual void caculateTheoryPerformance() {}
};
}

