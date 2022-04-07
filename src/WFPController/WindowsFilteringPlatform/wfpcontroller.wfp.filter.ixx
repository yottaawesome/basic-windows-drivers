module;

#include <Windows.h>
#include <fwpmu.h>

export module wfpcontroller.wfp.filter;

export namespace WFPController::WFP
{
	class Filter
	{
		public:
			virtual void Remove();
	};
}