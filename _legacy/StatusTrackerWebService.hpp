#pragma once
//Local
#include "ProcessStatus.hpp"

class StatusTrackerWebService : public HTTPServer
{
protected:
	//Event handlers
	void OnGETRequest(const Path& requestPath, const HTTPHeaders& requestHeaders, HTTPResponse& response) override;
};