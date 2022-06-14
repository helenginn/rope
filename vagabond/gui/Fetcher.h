// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifndef __vagabond__Fetcher__
#define __vagabond__Fetcher__

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#include <pthread.h>
#include <vagabond/utils/extra_curl_utils.h>
#endif


class Fetcher
{
public:
	Fetcher() {}
	virtual ~Fetcher()
	{

	}

	static void handleResult(void *ptr, void *data, int nbytes)
	{
		Fetcher *me = static_cast<Fetcher *>(ptr);

		char *chtmp = dataToChar(data, nbytes);
		std::string result = chtmp;

		me->processResult(result);
	}

	static void prepareResult(void *ptr, std::string result)
	{
		Fetcher *me = static_cast<Fetcher *>(ptr);
		me->_result = result;
		me->_process = true;
	}
	
	virtual void processResult(std::string result) = 0;
	virtual std::string toURL(std::string query) = 0;

	static char *dataToChar(void *data, int nbytes)
	{
		char *tmp = new char[nbytes + 1];
		memcpy(tmp, data, nbytes * sizeof(char));
		tmp[nbytes] = '\0';

		return tmp;
	}

	void load(std::string query)
	{
		std::string url = toURL(query);
		Fetcher *f = static_cast<Fetcher *>(this);
#ifndef __EMSCRIPTEN__
		ThreadStuff *ts = new ThreadStuff{url, &prepareResult, f};
		pthread_create(&_thread, NULL, pull_one_url, ts);
#else
		emscripten_async_wget_data(url.c_str(), f, handleResult, handleError);
#endif
	}

	virtual void handleError() = 0;

	static void handleError(void *me)
	{
		Fetcher *f = static_cast<Fetcher *>(me);
		f->handleError();
	}
protected:
	std::string _result;
	bool _process = false;

private:
	pthread_t _thread;
};

#endif
