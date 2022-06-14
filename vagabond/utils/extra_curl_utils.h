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

#ifndef __vagabond__extra_curl_utils__
#define __vagabond__extra_curl_utils__

#include <string>
#include <vector>
#include <sstream>

inline static size_t curlWriteCallback(void *contents, size_t size, 
                                size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

struct ThreadStuff
{
	std::vector<std::string> links;
	void(*callback)(void *ptr, std::string) = nullptr;
	void *ptr = nullptr;
	std::string post;
	
	ThreadStuff(std::string l, void(*cb)(void *, std::string), void *p)
	{
		if (l.length())
		{
			links.push_back(l);
		}
		callback = cb;
		ptr = p;
	}
};

inline static void *pull_one_url(void *stuff) 
{
	ThreadStuff *ts = (static_cast<ThreadStuff *>(stuff));

	for (std::string &link : ts->links)
	{
		CURL *curl;
		std::string seq;

		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &seq);

		if (ts->post.length())
		{
			struct curl_slist *hs=NULL;
			hs = curl_slist_append(hs, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);

			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ts->post.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, ts->post.length());
		}

		curl_easy_perform(curl); /* ignores error */

		void *myptr = ts->ptr;
		if (myptr == nullptr)
		{
			myptr = &link;
		}
		(*ts->callback)(myptr, seq);

		curl_easy_cleanup(curl);
	}
	delete ts;

	return NULL;
}


#endif
