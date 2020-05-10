/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACBackup.
 *
 * ACBackup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACBackup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACBackup.  If not, see <http://www.gnu.org/licenses/>.
 */
//Class header
#include "StatusTrackerWebService.hpp"
//Local
#include "webresources.hpp"
#include "WebStatusTracker.hpp"

//Constructor
StatusTrackerWebService::StatusTrackerWebService(WebStatusTracker& statusTracker, uint16 port) : statusTracker(statusTracker),
																							  HTTPServer(IPv4Address::GetAnyHostAddress(), port)
{
}

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
		    (*it)->MeasureSpeedSample();
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