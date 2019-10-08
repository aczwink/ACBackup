//Local
#include "resources.hpp"
#include "StatusTracker.hpp"

//Event handlers
void StatusTrackerWebService::OnGETRequest(const Path& requestPath, const HTTPHeaders& requestHeaders, HTTPResponse& response)
{
	if(requestPath == String(u8"/data"))
	{
		CommonFileFormats::JsonValue json = CommonFileFormats::JsonValue::Array();

		this->statusTracker.AcquireProcesses();
		auto it = this->statusTracker.GetProcessesBegin();
		while(it != this->statusTracker.GetProcessesEnd())
		{
			json.Push((*it)->ToJSON());
			++it;
		}
		this->statusTracker.ReleaseProcesses();

		response.JSON(json);
	}
	else if(requestPath == String(u8"/"))
	{
		response.SetHeader(u8"Content-Type", u8"text/html; charset=utf-8");
		response.SetHeader(u8"Content-Length", String::Number((uint64)sizeof(rsrc_index)));
		response.WriteHeader(200);
		response.WriteData(rsrc_index, sizeof(rsrc_index));
	}
	else if(requestPath == String(u8"/ACJSWCL.js"))
	{
		response.SetHeader(u8"Content-Type", u8"text/javascript; charset=utf-8");
		response.SetHeader(u8"Content-Length", String::Number(ACJSWCL_js_len));
		response.WriteHeader(200);
		response.WriteData(ACJSWCL_js, ACJSWCL_js_len);
	}
	else if(requestPath == String(u8"/clean_light.css"))
	{
		response.SetHeader(u8"Content-Type", u8"text/css; charset=utf-8");
		response.SetHeader(u8"Content-Length", String::Number(clean_light_css_len));
		response.WriteHeader(200);
		response.WriteData(clean_light_css, clean_light_css_len);
	}
	else
	{
		response.WriteHeader(404);
	}
}