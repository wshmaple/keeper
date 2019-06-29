#ifndef INCLUDE_HPP_CLIB_FUN
#define INCLUDE_HPP_CLIB_FUN

#include <string>
#include <iostream>
#include <memory>

namespace clib
{
    namespace fun
    {
#define IS_SPACE(c)  (c == ' ' || c == '\t' || c == '\r' || c == '\n')
		inline std::string trim(const std::string& str)
		{
			if (str.empty())
			{
				return std::string();
			}

			size_t start = 0;
            size_t end = str.size();
            size_t i = 0;

			while (IS_SPACE(str[i])) i++;
			start = i;

			if (end > start)
			{
				for (size_t j = end - 1; j > start; j--)
				{
					if (str[j] == '\0')
					{
						continue;
					}
					else if (!IS_SPACE(str[j]))
					{
						end = j + 1;
						break;
					}
				}
			}

			return std::string(str.c_str() + start, end - start);
		}

        inline void xstr(char* dest, size_t dest_size, size_t len, char *src)
        {
            memset(dest, '\0', dest_size);
            if(len > 3 && len < 252 && len < strlen(src) && len < 256)
            {
                size_t x = len - 3;
                size_t s = 0;
                size_t i = 0;

                for(i = 0; i < x; i++)
                {
                    if(*(src + i) > 127)
                        s = (s == 0 ? 1 : 0);
                    *(dest + i) = *(src + i);
                }
                *(dest + i) = (s ? * (src + i) : '.');
                *(dest + i + 1) = '.';
                *(dest + i + 2) = '.';
                *(dest + i + 3) = '\0';
            }
            else
            {
#ifdef _WIN32
                strcpy_s(dest, dest_size, src);
#else
                strncpy(dest, src, dest_size - 1);
#endif
            }
        }
        inline char* str2hex(char *des, int des_size, void *src, int src_size , int upper, int space, int enter)
        {
            int i, k, j = 0;
            int v, rv;
            char *des_c = des;
            char *src_c = (char*)src;

            if(des_c == NULL)
            {
                return des;
            }

            for(i = 0; i < src_size && j < des_size - 4; i++)
            {
                rv = src_c[i];
                if(rv < 0)
                    rv = 256 + rv;
                for(k = 1; k >= 0; k--)
                {
                    v = rv % 16;
                    if(k)
                        v = rv / 16;
                    if(v <= 9)
                        des_c[j++] = v + '0';
                    else
                        des_c[j++] = v + (upper ? 'A' : 'a') - 10;
                }
                if(space)
                    des_c[j++] = ' ';
                if(enter && (i + 1) % 10 == 0)
                    des_c[j++] = '\n';
            }
            des_c[j++] = '\0';

            return des_c;
        }
        inline std::string str2hex(std::string& des, const std::string& src, int upper = 'A', int space = 1, int enter = 1)
        {
            size_t i, k;
            int v, rv;

            des.clear();
            if (src.empty())
            {
                return des;
            }

            for (i = 0; i < src.size(); i++)
            {
                rv = src[i];
                if(rv < 0)
                    rv = 256 + rv;
                for(k = 1; k >= 0; k--)
                {
                    v = rv % 16;
                    if(k)
                        v = rv / 16;
                    if(v <= 9)
                        des.push_back(v + '0');
                    else
                        des.push_back(v + (upper ? 'A' : 'a') - 10);
                }
                if(space)
                    des.push_back(' ');
                if(enter && (i + 1) % 10 == 0)
                    des.push_back('\n');
            }
            des.push_back('\0');

            return des;
        }
        inline void split(const std::string& s, const std::string& tag, std::vector<std::string>* ret)
        {
			if (s.empty())
			{
				return;
			}

            size_t last = 0;
            size_t tlen = tag.size();
            size_t index = s.find(tag, last);
            while (index != std::string::npos)
            {
                ret->push_back(s.substr(last, index - last));
                last = index + tlen;
                index = s.find(tag, last);
            }
            if (index - last > 0)
            {
                ret->push_back(s.substr(last, index - last));
            }
        }
        inline void get_line(const std::string& s, std::vector<std::string>* ret)
        {
            if(s.find("\r\n") != std::string::npos)
            {
                split(s, std::string("\r\n"), ret);
            }
            else
            {
                split(s, std::string("\n"), ret);
            }
        }
        inline bool find_line(const std::string& s, const char* keyword, std::string* result = NULL)
        {
            std::vector<std::string> list;
            get_line(s, &list);

			result->clear();
            for(std::vector<std::string>::const_iterator iter = list.begin();
                    iter != list.end();
                    ++iter)
            {
                if((*iter).find(keyword) != std::string::npos)
                {
                    if(result != NULL)
                    {
                        *result = trim(*iter);
                    }
                    return true;
                    break;
                }
            }
            return false;
        }
        inline std::string print_r(const std::vector<std::string>& list, int pr = true)
        {
            int num = 0;
			std::string p;
            for(std::vector<std::string>::const_iterator iter = list.begin();
                    iter != list.end();
                    ++iter)
            {
				if (pr)
				{
					printf("---[%d]: %s\n", num++, (*iter).empty() ? "(empty)" : (*iter).c_str());
				}
				p += (*iter);
            }
            return p;
        }
        inline std::string real_string(std::string& s)
        {
            if(s.find_first_of('\\') != std::string::npos)
            {
                std::string res;
                for(size_t i = 0; i < s.size(); ++i)
                {
                    if(s[i] == '\\')
                    {
                        res.append("\\\\");
                    }
					else if (s[i] == '"')
					{
						res.append("\\\"");
					}
                    else
                    {
                        res.push_back(s[i]);
                    }
                }
                return res;
            }
            else
            {
                return s;
            }
        }
		inline bool have_str(const std::string& str, const char* key, size_t start_pos, bool eq_pos)
		{
			size_t pos = str.find(key);
			if ((eq_pos && pos == start_pos) || (!eq_pos && pos >= start_pos))
			{
				return true;
			}
			return false;
		}
		inline bool have(const std::vector<std::string>& list, const char* key, int start_pos, bool eq_pos)
		{
			for (std::vector<std::string>::const_iterator iter = list.begin();
				iter != list.end();
				++iter)
			{
				if (have_str(*iter, key, start_pos, eq_pos))
				{
					return true;
					break;
				}
			}
			return false;
		}
        inline bool val(const std::vector<std::string>& list, const std::string& key, std::string* result)
        {
            size_t klen = key.size();
            for(std::vector<std::string>::const_iterator iter = list.begin();
                    iter != list.end();
                    ++iter)
            {
                size_t pos = (*iter).find(key);
                if(pos == std::string::npos)
                {
                    continue;
                }
                else if(pos == 0)
                {
                    size_t vlen = (*iter).size();
                    result->assign((*iter).c_str() + klen, vlen - klen);
                    return true;
                    break;
                }
            }
            return false;
        }
        inline int val_int(std::string& str)
        {
            int re = 0;
			const char* s = str.c_str();

            bool start = false;
			for (size_t i = 0; i < str.size(); ++i)
            {
				if (*(s + i) >= '0' && *(s + i) <= '9')
                {
                    start = true;
					re = re * 10 + (*(s + i) - '0');
                }
                else if(start)
                {
                    break;
                }
            }
            return re;
        }
        inline bool find_arg(int argc, char* argv[], const char* key, char* val = NULL, size_t n = 0)
        {
            bool result = false;
            if(val != NULL && n > 0)
            {
                memset(val, '\0', n);
            }
            for(int i = 0; i < argc; ++i)
            {
                if(((*argv[i] == '-') || (*argv[i] == '/')) &&
                        (_stricmp(argv[i] + 1, key) == 0))
                {
                    result = true;
                }
                else if(result)
                {
                    if((*argv[i] != '-') && (*argv[i] != '/') && val != NULL && n > 0)
                    {
                        memcpy(val, argv[i], n);
                    }
                    break;
                }
            }
            return result;
        }
        inline bool find_arg(const char* line, const char* key)
        {
            std::string cmd(line);
            size_t pos;
            char tmp[260];
            size_t len;

            tmp[0] = '-';
            tmp[1] = '\0';
            strcat_s(tmp, sizeof(tmp) - 1, key);
            len = strlen(tmp);
            pos = cmd.find(tmp);
            if(pos != std::string::npos &&
				(pos == 0 || cmd[pos - 1] == ' ' || cmd[pos - 1] == '-') &&
                ((pos + len) == cmd.size() || cmd[pos + len] == ' '))
            {
				return true;
            }

            tmp[0] = '/';
            tmp[1] = '\0';
            strcat_s(tmp, sizeof(tmp) - 1, key);
            len = strlen(tmp);
            pos = cmd.find(tmp);
            if(pos != std::string::npos &&
                (pos == 0 || cmd[pos - 1] == ' ' || cmd[pos - 1] == '/') &&
                ((pos + len) == cmd.size() || cmd[pos + len] == ' '))
            {
				return true;
            }

            return false;
        }
    }
	
}

#endif