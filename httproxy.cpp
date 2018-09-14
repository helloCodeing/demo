#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <unordered_map>
#include <queue>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <sys/types.h>

// network libraries
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// multi-thread libraries
#include <pthread.h>


#define DEBUG		1 // for printing debug info
#define CACHE_DEBUG	0 // for printing cache debug info
#define MULTI_THREAD	0 // enable multi-thread
#define LOG		1 // enable recording request info in log
#define LOG_CLEAR	1 // if make the log empty every run
#define CONNECT_ENABLE	1 // if handle HTTPS
#define TIMEOUT_ENABLE	0 // if set timeout

#define LOG_NAME	"./proxy.log" // name and path of logfile

#define PROXY_PORT	9090 // default listening port of proxy
#define MAX_CONN	500 // maximum connections to proxy
#define NAME_SIZE	4096 //4096
#define REQ_BUFF_SIZE	102400
#define RES_BUFF_SIZE	1024000 //10240000
#define RESIZE_SIZE	102400 // used for expand buffer size
#define MAX_NUM		10 // parameters used to set thread stack
#define CACHE_SIZE	1000 // maximum response number in the cache
#define DAY_SECONDS	86400
#define HTTPS_TIMEOUT	90 // timeout for HTTPS connection
#define HTTP_TIMEOUT	120 // timeout for HTTP


// lock for thread safety
pthread_mutex_t mutex;

// to store accepted client socket fd
std::queue <int>	client_conn_sfd_queue;
long long		request_id = 0;




// client's request information
class RequestInfo {
public:
    long long	request_id; // unique id of each request
    const char*	client_ip; // ip address of client

    RequestInfo () {}
    RequestInfo (int id, const char* ip) : request_id(id), client_ip(ip) {}
    ~RequestInfo () {}
};



// HttpRequest class
class HttpRequest {
public:
    std::vector <char>	data_buff; // buffer for storing request data
    std::string		request_line; // first line of request
    std::string		server_name; // requested server's name
    int			server_port; // requested server's port
    std::string		http_type; // HTTP/1.0 or HTTP/1.1
    std::string		method; // GET, POST or CONNECT
    int			headers_len; // length of request headers
    int			content_len; // length of request content (for POST)
    size_t			data_len; // length or the entire request


    HttpRequest () : headers_len(0), content_len(0), data_len(0)
    {
        data_buff.resize(REQ_BUFF_SIZE);
    }

    ~HttpRequest ()
    {

    }


    // get Etag method
    std::string get_Etag ()
    {
        std::string header(data_buff.data(), headers_len);
        std::size_t pos = header.find("ETag");

        if (pos!=std::string::npos)
        {
            std::size_t i = header.find("/r/n", pos);
            std::string etag = header.substr(pos, i-pos);
            return etag;
        }
        else
        {
            return NULL;
        }
    }


    // check if request has been received successfully
    bool headers_recv_finished ()
    {
        std::string str(data_buff.data());
        std::size_t pos = str.find("\r\n\r\n");
        headers_len = pos+4;
        data_len = headers_len;

        if (pos == std::string::npos)
        {
            // if no "\r\n\r\n"
            return false;
        }

        return true;
    }


    int get_content_len ()
    {
        std::string str(data_buff.data());
        std::size_t pos1 = str.find("Content-Length: ");
        if (pos1 == std::string::npos)
        {
            return -1;
        }
        str.erase(0, pos1);
        pos1 = str.find(": ");
        std::size_t pos2 = str.find("\r\n");
        if (pos2 == std::string::npos)
        {
            std::cerr << "\nSTRANGE: request headers with no ^M\n" << std::endl;
            return -1;
        }
        // used for converting string to int
        std::stringstream ss;
        ss << str.substr(pos1+2, pos2-pos1);
        ss >> content_len;
        std::cout << "Content-Length is: " << content_len << std::endl;
        data_len = headers_len + content_len;
        return 0;
    }



    int get_params ()
    {
        std::string request(data_buff.data());
        std::string first_line;

        // GET http://detectportal.firefox.com/success.txt HTTP/1.1
        std::size_t pos = request.find("\r\n");
        if (pos == std::string::npos)
        {
            std::cerr << "Illegal request format: no new line" << std::endl;
            std::cerr << "request is:\n\n" << request << std::endl;
            return -1;
        }

        first_line = request.substr(0, pos);
        request_line = first_line;

        // get method
        pos = first_line.find(" ");
        if (pos == std::string::npos)
        {
            std::cerr << "Illegal request format: no space before method" << std::endl;
            return -1;
        }
        method = first_line.substr(0, pos);
        // set port number according to method
        if (method == "CONNECT") // HTTPS
        {
            server_port = 443;
        }
        else
        {
            server_port = 80;
        }

        std::cout << "method: " << method << std::endl;
        // if method is not supported, return -1 and send 400 Bad Request
        if (method != "CONNECT" && method != "POST" && method != "GET")
        {
            return -1;
        }
        first_line.erase(0, pos+1);

        // get http request type
        pos = first_line.find(" ");
        if (pos == std::string::npos)
        {
            std::cerr << "Illegal request format: no space before HTTP type" << std::endl;
            return -1;
        }
        http_type = first_line.substr(pos+1);

        std::cout << "http_type: " << http_type << std::endl;
        first_line.erase(pos);

        // get server's name for GET and POST
        if (method != "CONNECT")
        {
            pos = first_line.find("://");
            if (pos == std::string::npos)
            {
                pos = first_line.find("/");
                if (pos == std::string::npos)
                { // google.com
                    server_name = first_line;
                }
                else  // google.com/search?...
                {
                    server_name = first_line.substr(0, pos);
                }
            }
            else
            {
                first_line.erase(0, pos+3);

                pos = first_line.find("/");
                if (pos == std::string::npos) // http://google.com
                {
                    server_name = first_line;
                }
                else
                { // http://google.com/search?...
                    server_name = first_line.substr(0, pos);
                }
            }
            // maybe a situation where there is:
            // http://vcm-xxx.vm.duke.edu:8080/rsvp/
            pos = server_name.find(":");
            if (pos != std::string::npos)
            {
                // update server_name and server_port
                std::string temp = server_name.substr(pos+1);
                server_name = server_name.substr(0, pos);
                std::stringstream ss;
                ss << temp;
                ss >> server_port;
            }
        }
        else
        { // get server name for CONNECT
            pos = first_line.find(":");
            if (pos == std::string::npos)
            {
                std::cout << "Illegal request format: no ':' for CONNECT" << std::endl;
                return -1;
            }
            server_name = first_line.substr(0, pos);
        }

        std::cout << "server_name: " << server_name << std::endl;
        if (http_type != "HTTP/1.1" && http_type != "HTTP/1.0")
        {
            // HTTP proxy could not handle other request type
            return -1;
        }
        return 0;
    }
};




// HttpResponse class
class HttpResponse 
{
public:
    std::vector <char>	data_buff; // buffer for storing response data
    std::string		response_line; // first line of response
    std::string		code; // 200, 400, 404, 502, 504...
    int			headers_len; // length of response headers
    int			content_len; // length of response content
    size_t			data_len; // length of the entire response

    // constructor and destructor
    HttpResponse () : headers_len(0), content_len(0), data_len(0) 
    {
        data_buff.resize(RES_BUFF_SIZE);
    }
    ~HttpResponse () 
    {

    }


    // check if headers have been received completely
    bool headers_recv_finished () 
    {
        std::string str(data_buff.data());
        std::size_t pos = str.find("\r\n\r\n");
        headers_len = pos+4;
        data_len = headers_len;
        if (pos == std::string::npos) 
        {
            // if no "\r\n\r\n"
            return false;
        }
        pos = str.find("\r\n");
        // response_line is the first line of response
        response_line = str.substr(0, pos);
        pos = response_line.find(" ");
        std::string temp = response_line.substr(pos+1);
        pos = temp.find(" ");
        code = temp.substr(0, pos);
        std::cout << "response status: " << code << std::endl;
        return true;
    }



    // check if server's response is garbage
    bool validate () 
    {
        // check HTTP/1.1 or HTTP/1.0
        if (data_buff[0] == 'H' && data_buff[1] == 'T' &&
            data_buff[2] == 'T' && data_buff[3] == 'P' &&
            data_buff[4] == '/' && data_buff[5] == '1' &&
            data_buff[6] == '.' && (data_buff[7] == '1' || data_buff[7] == '0')) 
        {
            return true;
        }
        return false;
    }


    // method to check if content indicating by Transfer-Encoding has
    // been received completely
    bool coded_content_recv_finished (std::size_t current_index) 
    {
        std::cout << "decoding..." << std::endl;
        // use "0\r\n\r\n" as the sign
        for (std::size_t i = current_index; i < data_len; ++i) 
        {
            if (i > 3 && data_buff[i] == '\n' &&
                data_buff[i-1] == '\r' && data_buff[i-2] == '\n' &&
                data_buff[i-3] == '\r' && data_buff[i-4] == '0') 
            {
                std::cout << "done decoding..." << std::endl;
                return true;
            }
        }
        return false;
    }


    // get response's content length
    int get_content_len () 
    {
        std::string str(data_buff.data());
        std::size_t pos1 = str.find("Content-Length: ");
        if (pos1 == std::string::npos) 
        {
            return -1;
        }
        str.erase(0, pos1);
        pos1 = str.find(": ");
        std::size_t pos2 = str.find("\r\n");
        if (pos2 == std::string::npos) 
        {
            std::cerr << "\nSTRANGE: response headers with no ^M\n" << std::endl;
            return -1;
        }
        std::stringstream ss;
        ss << str.substr(pos1+2, pos2-pos1);
        ss >> content_len;
        // get total length of response
        data_len = headers_len + content_len;
        std::cout << "Content-Length is: " << content_len << std::endl;
        if (content_len == 0) 
        {
            return -1;
        }
        return 0;
    }


    // check if there is Transfer-Encoding field
    bool check_encoding () 
    {
        std::string str(data_buff.data());
        std::size_t pos = str.find("Transfer-Encoding: ");
        if (pos == std::string::npos) 
        {
            return false;
        }
        return true;
    }


    // get Age field in response header
    int get_age () 
    {
        std::string header(data_buff.data(), headers_len);
        std::size_t pos = header.find("\r\nAge:");
        if (pos!=std::string::npos) 
        {
            std::size_t i = header.find("\r\n",pos+2);
            std::string age = header.substr(pos+7, i-pos-7);
            long res = 0;
            std::stringstream ss;
            ss << age;
            ss >> res;
            return res;
        }
        return 0;
    }


    // perform revalidation of response
    void revalidate()
    {
        std::string header(data_buff.data(), headers_len);
        std::size_t pos = header.find("\r\nAge:");
        if (pos != std::string::npos) 
        {
            std::size_t i = header.find("\r\n", pos+4);
        }
    }


    // get Etag method, currently used to verify revalidation
    int get_Etag () 
    {
        std::string header(data_buff.data(),headers_len);
        std::size_t pos = header.find("ETag");
        if (pos != std::string::npos) 
        {
            //std::size_t i = header.find("/r/n",pos);
            //std::string etag = header.substr(pos,i-pos);
            return 1;
        }
        else
            return 0;
    }
    int get_Etag(std::string &str)
    {
        std::string header(data_buff.data(), headers_len);
        std::size_t pos = header.find("\r\nETag:");
        std::cout << "FindETag: " << header << std::endl;
        std::cout << "FindETag: " << headers_len << header << std::endl;
        if (pos != std::string::npos) 
        {
            std::size_t i = header.find("\r\n", pos+4);
            str += header.substr(pos+8, i-pos-8);
#if CACHE_DEBUG
            std::cout << "!!FindETag" << header.substr(pos+8, i-pos-8) << std::endl;
#endif
            return 1;
        }
        else 
        {
            return 0;
        }
    }
};




// CacheNode class
class CacheNode 
{ // use linkedlist to implement LRU
public:
    std::string   key; // first line of request
    long          time; // expiration time
    long		life;
    HttpResponse	value; // response
    CacheNode*    prev;
    CacheNode*    next;

    CacheNode (std::string& k, int t, long l, HttpResponse& v) :
        key(k), time(t), life(l), value(v), prev(NULL), next(NULL) {}

    ~CacheNode () {}
};



// proxy LRU cache class, linked list is used to perform efficient
// LRU replacement policy
class LRUCache 
{
public:
    int           byte; // linked list size
    int           size; // maximum of cache-list size
    CacheNode*    head;
    CacheNode*    tail;
    std::unordered_map <std::string, CacheNode*> map; // use to perform quick lookup

    LRUCache (int size) : size(size), head(NULL), tail(NULL) {}

    ~LRUCache () 
    {
        CacheNode* temp = head;
        while (temp != NULL) 
        {
            head = head->next;
            delete temp;
            temp = head;
        }
    }

    // revalidate cache value
    void revalidate (HttpRequest& key, long reval_time) 
    {
        std::unordered_map <std::string, CacheNode*>::iterator it = map.find(key.request_line);
        // if response is found in the cache
        if (it != map.end()) 
        {
            time_t rawtime;
            time(&rawtime);
            int current = time(&rawtime);
            CacheNode* node = it->second;
            node->time = reval_time;
            remove(node);
            set_head(node);
#if CACHE_DEBUG
            std::cout << "RevalidateCache" << std::endl;
#endif
        }
    }


    // try to get response from cache
    time_t get (HttpRequest &key, HttpResponse &val) 
    {
        std::unordered_map <std::string, CacheNode*>::iterator it = map.find(key.request_line);
        if (it != map.end()) 
        { // if response found in the cache
            time_t rawtime; // current time
            time(&rawtime);
            //int current = time(&rawtime);
            CacheNode* node = it->second;
            // check if response has expired
            if (node->time + node->life > rawtime) 
            { // not expired
                remove(node);
                set_head(node);
                val = node->value;
#if CACHE_DEBUG
                std::cout << "cache: get in cache!" << std::endl;
#endif
                return 0; // successfully get response
            }
            else 
            { // response expired
#if CACHE_DEBUG
                std::cout << "response expired at: " << rawtime << " , current time is:"
                    << node->time << std::endl;
#endif
                if (node->value.get_Etag()) 
                {
                    //long i =-(node->time+node->life);
                    //std::cout<<"!needreval:"<<i<<std::endl;
                    return -(node->time+node->life); // revalidation check
                }
                else 
                {
                    remove(node);
                    map.erase(it);
                    //time_t temp = node->life + node->time;
                    //struct tm* time_info = gmtime(&temp);
                    return node->life + node->time; // expired
                }
            }
        }
        else 
        { // cache doesn't have response
            return -1;
        }
    }


    // get cache size in bytes
    int get_size () 
    {
        return byte;
    }


    // get number of response stored
    int count () 
    {
        return map.size();
    }


    // store response to proxy cache
    void set (HttpRequest& key, long time, long life, HttpResponse& value,std::ofstream& log) 
    {
        std::unordered_map <std::string, CacheNode*>::iterator it1 = map.find(key.request_line);
        if (it1 != map.end()) 
        { // if already stored, update
            CacheNode* node = it1->second;
            node->value = value;
            remove(node);
            set_head(node); // add as head as it is newly accessed
        }
        else 
        { // if not already in the cache
#if CACHE_DEBUG
            std::cout << "not set" << std::endl;
#endif
            CacheNode* new_node = new CacheNode(key.request_line, time, life, value);
            if (map.size() >=(size_t)size) 
            { // check if cache is full
                std::unordered_map <std::string, CacheNode*>::iterator it2 = map.find(tail->key);
                byte -= tail->value.data_len;

                std::string temp = tail->key;
                std::size_t pos = temp.find("http://");
                temp = temp.substr(pos+7);
                pos = temp.find(" HTTP");
                temp = temp.substr(0, pos);
                log << "(no-id): NOTE evicted " << temp << " from cache" << std::endl;

                remove(tail); // replace the tail of linked list
                //#################################################################################################
                map.erase(it2);
            }
            // add the new response as head
            set_head(new_node);
            byte += value.data_len;
            map.insert(std::pair <std::string, CacheNode*> (key.request_line, new_node));
        }
    }


    // remove node from cache linked list
    void remove (CacheNode* node) 
    {
        if (node->prev != NULL) 
        {
            node->prev->next = node->next;
        }
        else
        {
            head = node->next;
        }
        if (node->next != NULL) 
        {
            node->next->prev = node->prev;
        }
        else
        {
            tail = node->prev;
        }
        //delete node;
    }


    // add new response to the head of cache linked list
    void set_head (CacheNode* node) 
    {
        node->next = head;
        node->prev = NULL;
        if (head != NULL) 
        {
            head->prev = node;
        }
        head = node;
        if (tail == NULL) 
        {
            tail = head;
        }
    }
};




// NOTE: this is a global variable used as proxy's cache
LRUCache proxy_cache(CACHE_SIZE);

// functions for caching, convert string to int/long
long to_int (std::string& str) 
{
    long res = 0;
    std::stringstream ss;
    ss << str;
    ss >> res;

    return res;
}
std::string to_string(int i)
{
    std::string ans;
    std::stringstream ss;
    if (i < 10) 
    {
        int n = 0;
        ss << n;
    }
    ss << i;
    ss >> ans;
    return ans;
}


// parse string to time with specific format
long to_time (std::string& str) 
{
    time_t rawtime;
    time(&rawtime);
    struct tm* timeinfo = localtime(&rawtime);
    std::size_t pos = str.find(", ");
    if (pos==std::string::npos) 
    {
        return -1;  //wrong type of time
    }
    std::string month[12] ={"Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"};
    int i = 0;
    std::string mon(str.substr(pos+5, 3));
    timeinfo->tm_sec = 1;
    timeinfo->tm_min = 54;
    timeinfo->tm_hour = 15;
    for(i = 0; i < 12; ++i)
    {
        if(mon.compare(month[i]) == 0)
        {
            timeinfo->tm_mon = i;
            break;
        }
    }
    if (i > 11) 
    {
        return -1; //wrong time;
    }

    std::string mday(str.substr(pos+2, 2));
    timeinfo->tm_mday = to_int(mday);
    std::string year (str.substr(pos+9, 4));
    timeinfo->tm_year = to_int(year) - 1900;
    std::string hour(str.substr(pos+14, 2));
    timeinfo->tm_hour = to_int(hour);
    std::string min(str.substr(pos+17, 2));
    timeinfo->tm_min = to_int(min);
    std::string sec(str.substr(pos+20, 2));
    timeinfo->tm_sec = to_int(sec);

    return mktime(timeinfo);
}



// test if the cached response can be revalidate, if can, modify the request head
int revalidate(HttpRequest &req,HttpResponse &res, long time)
{
    std::string etag;
    if (res.get_Etag(etag)) 
    {
        std::string str("If-Modified-Since: ");
        struct tm *timeinfo = gmtime(&time);
        std::string month[12] ={"Jan","Feb","Mar","Apr","May","Jun",
            "Jul","Aug","Sep","Oct","Nov","Dec"};
        std::string weekday[7] ={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        str+= weekday[timeinfo->tm_wday];
        str+=", ";
        str+=to_string(timeinfo->tm_mday);
        str+=" ";
        str+=month[timeinfo->tm_mon];
        str+=" ";
        str+=to_string(timeinfo->tm_year+100);
        str+=" ";
        str+=to_string(timeinfo->tm_hour);
        str+=":";
        str+=to_string(timeinfo->tm_min);
        str+=":";
        str+=to_string(timeinfo->tm_sec);
        str+=" ";
        str+="GMT\r\n";
        str+="If-None-Match: ";
        str+=etag;
        str+="\r\n\r\n";
        for(size_t i = 0; i<str.size();i++){
            req.data_buff.push_back(str[i]);
        }
        req.headers_len += str.size() - 2;
        req.data_len += str.size() - 2;

#if CACHE_DEBUG
        std::cout << "size" << req.data_buff.size() << std::endl;
        std::cout << "myrevalid\n" << asctime(timeinfo) << "etag:"
            << etag<<"\nstr:" << str << str.size() << "\nbuff:"
            << req.data_buff.data() << "!myrevalid\n";
#endif
        return 1;
    }
#if CACHE_DEBUG
    std::cout << "myrevalid:noEtag" << std::endl;
#endif
    return 0;
}



// function to check if response need to be cached
time_t need_cache (HttpResponse& res, long delay) 
{
    std::string header(res.data_buff.data(),res.headers_len);
    std::size_t pos = header.find("200 OK");
    if (pos == std::string::npos) 
    {
        //we only need to cache 200 ok
#if CACHE_DEBUG
        std::cout << "Not200OK" << std::endl;
#endif
        return -1; // not 200 OK
    }

    pos = header.find("304 Not Modified");
    if (pos != std::string::npos) 
    {
        //if contains 304, don't care, this seems the same as the former one
#if CACHE_DEBUG
        std::cout << "Find304" << std::endl;
#endif
        return -2;
    }

    pos = header.find("Cache-Control");
    // if cache-control, use it
    if (pos != std::string::npos) 
    {
#if CACHE_DEBUG
        std::cout << "findCache-Control" << std::endl;
#endif
        std::size_t i = header.find("\r\n", pos);
        std::string control = header.substr(pos, i-pos);
        // if no-store
        pos = control.find("no-store");
        if (pos != std::string::npos) {
#if CACHE_DEBUG
            std::cout << "controlfind:no-store" << std::endl;
#endif
            return -3; // no-store
        }
        // if no-cache
        pos = control.find("no-cache");
        if (pos != std::string::npos) {
#if CACHE_DEBUG
            std::cout << "controlfind:no-cache" << std::endl;
#endif
            return -4; // no-cache
        }
        // find max-age
        pos = control.find("max-age=");
        if (pos != std::string::npos) {
            std::string time = control.substr(pos+8);
            if ((to_int(time)-delay) > 0) {
#if CACHE_DEBUG
                std::cout << "controlfind:max-age" << std::endl;
#endif
                return to_int(time);
            }
            else {
#if CACHE_DEBUG
                std::cout << "controlfind:need-re" << std::endl;
#endif
                return 0; // need revalidation
            }
        }
        // about no-cache, I think it is similar to If-No-Match
        // will be adopted by browser and should be checked in server
    }

    // check if response has expired
    pos = header.find("Expires");
    if (pos != std::string::npos) 
    {
        std::size_t i = header.find("\r\n", pos);
        std::string control = header.substr(pos+9, i-pos);
        if ((to_time(control) - delay) > 0) {
#if CACHE_DEBUG
            std::cout << "timeExpires:" << control << std::endl;
#endif
            return to_time(control);
        }
        else {
#if CACHE_DEBUG
            std::cout << "timeExpires:need-re" << std::endl;
#endif
            return 0; //need revalidation
        }
    }
    if (res.get_Etag()) 
    {
#if CACHE_DEBUG
        std::cout << "getEtag:need-re" << std::endl;
#endif
        return 0; //need revalidation
    }
#if CACHE_DEBUG
    std::cout << "!noflag" << std::endl;
#endif
    return -5; //no flag, don't cache
}





/* GET CLIENT'S REQUEST ***************************************************************************/
int get_request (int client_conn_sfd, HttpRequest& request) 
{
    std::size_t len = 0;
    std::size_t i = 0;
    std::size_t received_bytes = 0;

    // continue receive headers until "\r\n\r\n" is seen
    while (1) 
    {
        len = recv(client_conn_sfd, &request.data_buff.data()[i], REQ_BUFF_SIZE, 0);
        received_bytes += len;

        if (request.headers_recv_finished() == true) 
        {
            // done receiving headers
            break;
        }
        else 
        {
            // if no enough space and no "\r\n\r\n", resize and continue
            if (received_bytes == request.data_buff.size()) 
            {
                try 
                {
                    request.data_buff.resize(received_bytes + RESIZE_SIZE);
                    i = received_bytes;
                }
                catch (std::bad_alloc& ba) 
                {
                    std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
                    return -1;
                }
            }
            // if 0 is received, the connection is closed
            else if (len == 0) 
            {
                if (received_bytes != 0) 
                {
                    break;
                }
                else 
                { // no request received, close connection and exit
                    perror("no request");
                    return -1;
                }
            }
            else if (len < 0) 
            { // error
                perror("request recv");
                return -1;
            }
            else 
            {
                std::cout << "\n\nbuff isn't filled up (req no ^M)\n" << std::endl;
                i = received_bytes;
            }
        }
    }

    // 400, Bad Request
    if (request.get_params() < 0) {
        perror("get request parameters error");
        return -1;
    }

    if (request.method == "POST") 
    {
        // manage to count the rest content
        std::cout << "\nrequest:\n" << request.data_buff.data() << std::endl;
        if (request.get_content_len() < 0) 
        { // no Content-Length field
            std::cerr << "\n\nPOST with no Content-Length\n" << std::endl;
            return -1; // 400 Bad Request
        }

        std::cout << "\nrequest.content_len: " << request.content_len << std::endl;
        std::cout << "\ncurrent content len: " << received_bytes - request.headers_len << std::endl;
        int rest_content_len = request.content_len - (received_bytes - request.headers_len);
        std::cout << "\nrest_content_len: " << rest_content_len << std::endl;

        i = received_bytes;
        if ((size_t)rest_content_len > request.data_buff.size()) 
        {
            // no enough space, resize
            try 
            {
                request.data_buff.resize(rest_content_len + request.headers_len + request.content_len);
            }
            catch (std::bad_alloc& ba) 
            {
                std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
                return -1;
            }
        }
        // receive rest contents
        while (rest_content_len != 0) 
        {
            len = recv(client_conn_sfd, &request.data_buff.data()[i], REQ_BUFF_SIZE, 0);
            received_bytes += len;
            rest_content_len = rest_content_len - len;

            if (rest_content_len == 0) 
            {
                // finish receiving request content
                break;
            }
            if (len == 0) 
            {
                // connection closed
                break;
            }
            else if (len < 0) 
            {
                return -1;
            }

            i = received_bytes;
        }// end while(1)
    }// end if

    std::cout << "\nrequest received:\n" << request.data_buff.data();
    std::cout << "\nRequest received bytes: " << received_bytes << std::endl;
    std::cout << "\n\n";
    return 0;
}
/**************************************************************************************************/




// function to form response to handle error conditions like 400, 404, 502...
void proxy_response (int client_conn_sfd, std::string code,std::string reason, std::string info) 
{
    std::string response("HTTP/1.1 ");

    response += code; // HTTP/1.1 404
    response += " ";
    response += reason; // HTTP/1.1 404 Not Found
    response += "\r\nContent-Length: ";
    std::stringstream ss;
    std::string temp;
    ss << info.length();
    ss >> temp;
    response += temp;
    response += "\r\nContent-Type: text/plain\r\n\r\n";
    response += info;
    response += "\n";

    // send error info back to the client
    // set MSG_NOSIGNAL to prevent to handle SIGPIPE locally
    int len = send(client_conn_sfd, response.c_str(),
                   response.length(), MSG_NOSIGNAL);

    std::cout << "ERROR: " << response << "LEN: " << len << std::endl;
    return;
}





/* HANDLE CONNECT REQUEST *************************************************************************/
// build a socket connection: client <=> proxy <=> server, move data back and forth
int handle_https (int client_conn_sfd, HttpRequest& request,
                  RequestInfo request_info, std::ofstream& log) 
{
    int stat;
    int len;
    fd_set read_fds;
    std::string ready_msg("200 OK");
    // create connection between proxy and the server
    sockaddr_in server_addr;
    struct hostent* server_info;
    struct timeval wait_time;

    server_info = gethostbyname(request.server_name.c_str());
    if (server_info == NULL) 
    { // 404, Not Found
        std::cerr << "\nCannot find server: " << request.server_name << std::endl;
        std::string info("Cannot find server: ");
        info += request.server_name;
        proxy_response(client_conn_sfd, "404", "Not Found", info);
        log << request_info.request_id << ": Responding \"" << request.http_type
            << " 404 Not Found\"" << std::endl; 
        return -2;
    }
    // create TCP socket for connecting to the server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(request.server_port);
    server_addr.sin_addr = *((struct in_addr*)server_info->h_addr);
    memcpy(&server_addr.sin_addr, server_info->h_addr_list[0],
           server_info->h_length);
    int server_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sfd < 0) 
    {
        // proxy's internal issue
        perror("Failed to create socket connecting to the server");
        return -2;
    }
   // proxy connect to the server
    stat = connect(server_sfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (stat < 0) 
    { // 504
        perror("Cannot connect to the server");
        std::string info("Server does not response on time...");
        info += request.server_name;
        proxy_response(client_conn_sfd, "504", "Gateway Timeout", info);
        log << request_info.request_id << ": Responding " << "\"" << request.http_type
            << " 504 Gateway Timeout" << "\"" << std::endl;
        return -2;
    }

    // inform client the connection is ready
    // set MSG_NOSIGNAL to prevent to handle SIGPIPE locally
    len = send(client_conn_sfd, ready_msg.c_str(),
               ready_msg.length(), MSG_NOSIGNAL);

    // continute to check if there is any data to send
    while (1) 
    {
        std::vector <char> data_buff(RES_BUFF_SIZE);

        FD_ZERO(&read_fds);
        FD_SET(client_conn_sfd, &read_fds);
        FD_SET(server_sfd, &read_fds);
        stat = select(FD_SETSIZE, &read_fds, NULL, NULL, NULL);

        // check if client is ready to send data
        if (FD_ISSET(client_conn_sfd, &read_fds)) 
        {
            len = recv(client_conn_sfd, &data_buff.data()[0], RES_BUFF_SIZE, 0);
            if (len < 0) 
            {
                perror("Failed to received from client");
                return -1;
            }
            else if (len == 0) 
            { // conenction is closed, exit
                std::cout << "HTTPS Connection closed..." << std::endl;
                return -1;
            }
            // set MSG_NOSIGNAL to prevent to handle SIGPIPE locally
            len = send(server_sfd, data_buff.data(), len, MSG_NOSIGNAL);
            if (len < 0) 
            {
                perror("Failed to send the server");
                return -1;
            }
        }
        // check if server is ready to response data
        else if (FD_ISSET(server_sfd, &read_fds)) 
        {
            len = recv(server_sfd, &data_buff.data()[0], RES_BUFF_SIZE, 0);
            if (len < 0) 
            { // 502 Bad Gateway
                perror("Failed to received from server");
                return -1;
            }
            else if (len == 0) 
            {
                std::cout << "HTTPS Connection closed..." << std::endl;
                return -1;
            }

            // set MSG_NOSIGNAL to prevent to handle SIGPIPE locally
            len = send(client_conn_sfd, data_buff.data(), len, MSG_NOSIGNAL);
            if (len < 0) 
            {
                perror("Failed to send the client");
                return -1;
            }
        }
        data_buff.clear();
    }
    return 0;
}


// get server's response
int get_response (int server_sfd, HttpResponse& response) 
{
    std::size_t len = 0;
    std::size_t received_bytes = 0;
    std::size_t i = 0;

    // receive headers and a part of the contents
    while (1) 
    {
        std::cout << "recving response headers..." << std::endl;
        len = recv(server_sfd, &response.data_buff.data()[i], RES_BUFF_SIZE, 0);
        received_bytes += len;

        if (response.headers_recv_finished() == true) 
        {
            // done receiving response headers
            break;
        }
        else 
        {
            // if no enough space and no "\r\n\r\n", resize and continue
            if (received_bytes == response.data_buff.size()) 
            {
                try 
                {
                    response.data_buff.resize(received_bytes + RESIZE_SIZE);
                    i = received_bytes;
                }
                // try catch bad alloc when doing memory allocation
                catch (std::bad_alloc& ba) 
                {
                    std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
                    return -1;
                }
            }
            // if 0 is received, the connection is closed
            else if (len == 0) 
            {
                break;
            }
            else if (len < 0) 
            {
                return -1;
            }
            else 
            {
                std::cout << "\n\nbuff isn't filled up (res no ^M)\n" << std::endl;
                i = received_bytes;
            }
        }
    }

    // validate if the response is garbage
    if (response.validate() == false) 
    {
        return -1; // garbage received, response with 502
    }
    // manage to count the rest content
    std::cout << "\nresponse:\n" << response.data_buff.data() << std::endl;
    if (response.get_content_len() < 0 && response.check_encoding() == false) 
    {
        // no Content-Length and Transfer-Encoding
        return 0;
    }


    // receive rest content, use Content-Length field to decide when to finish
    if (response.content_len > 0) 
    {
        std::cout << "\nreceived_bytes: " << received_bytes << std::endl;
        std::cout << "\nresponse.headers_len: " << response.headers_len << std::endl;
        std::cout << "\nresponse.content_len: " << response.content_len << std::endl;
        std::cout << "\ncurrent content len: "
            << received_bytes - response.headers_len << std::endl;
        int rest_content_len = response.content_len -
            (received_bytes - response.headers_len);
        std::cout << "\nrest_content_len: " << rest_content_len << std::endl;

        i = received_bytes;
        if ((size_t)rest_content_len > response.data_buff.size()) 
        {
            // no enough space, resize, try to catch memory issue
            try 
            {
                response.data_buff.resize(rest_content_len + response.headers_len + response.content_len);
            }
            catch (std::bad_alloc& ba) 
            {
                std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
                return -1; 
            }
        }
        // receive rest contents
        while (rest_content_len != 0) 
        {
            std::cout << "recving response content..." << std::endl;
            len = recv(server_sfd, &response.data_buff.data()[i], RES_BUFF_SIZE, 0);
            received_bytes += len;
            rest_content_len = rest_content_len - len;
            if (rest_content_len == 0) 
            {
                // finish receiving response content
                break;
            }
            if (len == 0) 
            {
                // connection closed
                break;
            }
            else if (len < 0) 
            {
                // connection timeout or broken unexpectedly
                return -1;
            }
            i = received_bytes;
        }
    }


    // receive rest content, use Transfer-Encoding field to decide when to finish
    else if (response.check_encoding() == true &&
             response.coded_content_recv_finished(response.headers_len) == false) 
    {
        i = received_bytes;
        // receive the rest of contents
        while (1) 
        {
            std::cout << "recving response chunked..." << std::endl;
            len = recv(server_sfd, &response.data_buff.data()[i], RES_BUFF_SIZE, 0);
            received_bytes += len;
            response.data_len = received_bytes;
            // calculate response's content length
            response.content_len = response.data_len - response.headers_len;

            if (response.coded_content_recv_finished(i) == true) 
            {
                // done receiving response contents
                break;
            }
            else 
            {
                // if no enough space and no "0\r\n\r\n", resize and continue
                if (received_bytes == response.data_buff.size()) 
                {
                    // try to add relatively small amount of data first
                    try 
                    {
                        response.data_buff.resize(received_bytes + RESIZE_SIZE);
                        i = received_bytes;
                    }
                    catch (std::bad_alloc& ba) 
                    {
                        std::cerr << "bad_alloc caught: " << ba.what() << std::endl;
                        return -1; 
                    }
                }
                // if 0 is received, the connection is closed
                else if (len == 0) 
                {
                    break;
                }
                // failed to receive, 502 probably
                else if (len < 0) 
                {
                    return -1;
                }
                else 
                {
                    std::cout << "\n\nbuff isn't filled up (chunked)\n" << std::endl;
                    std::cout << "len in this loop: " << len << std::endl;
                    std::cout << "data_buff:\n" << response.data_buff.data() << std::endl;
                    i = received_bytes;
                }
            }
        }
    }
    else 
    {
        return 0;
    }
    std::cout << "\nresponse received:\n" << response.data_buff.data();
    std::cout << "\nResponse received bytes: " << received_bytes << std::endl;
    // successfully received response
    return 0;
}



/* FORWAR CLIENT'S REQUEST TO THE SERVER AND GET RESPONSE *****************************************/
// forward client's request and get server's response
int forward_request_and_get_response (int client_conn_sfd, HttpRequest& request,
                                      HttpResponse& response, RequestInfo& request_info,
                                      std::ofstream& log) 
{
    sockaddr_in server_addr;
    struct hostent* server_info;
    int stat = 0;
    int len = 0;
    struct timeval wait_time;

    // create TCP socket to connect to the server
    server_info = gethostbyname(request.server_name.c_str());
    if (server_info == NULL) 
    { // 404
        std::cerr << "\nCannot find server: " << request.server_name << std::endl;
        std::string info("Cannot find server: ");
        info += request.server_name;
        proxy_response(client_conn_sfd, "404", "Not Found", info);
        log << request_info.request_id << ": " << "Responding \""
            << request.http_type << "\" 404 Not Found" << std::endl;
        return -2;
    }
    // create TCP socket for connecting to the server
    server_addr.sin_family = AF_INET;
    std::cout << "server_port: " << request.server_port << std::endl;
    server_addr.sin_port = htons(request.server_port);
    server_addr.sin_addr = *((struct in_addr*)server_info->h_addr);
    memcpy(&server_addr.sin_addr, server_info->h_addr_list[0], server_info->h_length);
    int server_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sfd < 0) 
    {
        perror("Failed to create socket connecting to the server");
        return -1;
    }

#if TIMEOUT_ENABLE
    wait_time.tv_sec = HTTP_TIMEOUT;  // set 120 Secs timeout for each connection
    setsockopt(server_sfd, SOL_SOCKET, SO_RCVTIMEO|SO_REUSEADDR,
               (struct timeval*)&wait_time, sizeof(wait_time));
#endif

    // proxy connect to the server
    stat = connect(server_sfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (stat < 0) 
    { // 504 Gateway Timeout
        perror("Cannot connect to the server");
        std::string info("Server does not response on time...");
        info += request.server_name;
        // send 504 response to the client
        proxy_response(client_conn_sfd, "504", "Gateway Timeout", info);
        log << request_info.request_id << ": Responding " << "\"" << request.http_type
            << " 504 Bad Timeout" << "\"" << std::endl;
        return -2;
    }

    // set MSG_NOSIGNAL to prevent to handle SIGPIPE locally
    len = send(server_sfd, request.data_buff.data(), request.data_len, MSG_NOSIGNAL);

    log << request_info.request_id << ": Requesting \"" << request.request_line
        << "\" from " << request.server_name << std::endl;

    std::cout << "\nrequest sent:\n" << request.data_buff.data();
    std::cout << "\nRequest sent bytes: " << len << std::endl;
    std::cout << "\nrequest.headers_len: " << request.headers_len << std::endl;
    std::cout << "\nrequest.content_len: " << request.content_len << std::endl;
    /* GET SEVER'S RESPONSE ***************************************************************************/
    stat = get_response(server_sfd, response);
    close(server_sfd);
    if (stat < 0) 
    { // 502 Bad Gateway
        return -1;
    }
    log << request_info.request_id << ": Received \"" << response.response_line
        << "\" from " << request.server_name << std::endl;
    return EXIT_SUCCESS;
}




// return server's response back to the client
int return_response (int client_conn_sfd, HttpResponse& response) 
{
    // set MSG_NOSIGNAL to handle SIGPIPE locally
    int len = send(client_conn_sfd, response.data_buff.data(),
                   response.headers_len + response.content_len, MSG_NOSIGNAL);
    if (len <= 0) 
    { // error
        return -1;
    }
    std::cout << "\nresponse returned:\n" << response.data_buff.data();
    std::cout << "\nResponse sent bytes: " << len << std::endl;
    return EXIT_SUCCESS;
}




// each thread's function, which handles one request
void* handle_request (void* arg) 
{
    HttpRequest request;
    HttpResponse response;
    HttpResponse cached_response;
    int stat;
    long request_time;
    long response_time;
    long response_delay; // time to add response_delay
    time_t time_stat;
    time_t raw_time;
    time_t curr_time;
    struct tm* ptm;
    std::ofstream log(LOG_NAME, std::ios::app);
    // get request info for printing log
    RequestInfo request_info = *((RequestInfo*)arg);


    std::cout << "client id: " << request_info.request_id << std::endl;
    std::cout << "client ip: " << request_info.client_ip << std::endl;

    // use mutex to make thread safe
    pthread_mutex_lock(&mutex);
    // each thread get a socket fd from the queue and handle the request
    int client_conn_sfd = client_conn_sfd_queue.front();
    client_conn_sfd_queue.pop();
    request_info.request_id = request_id;
    ++request_id;
    pthread_mutex_unlock(&mutex);

    std::cout << "\nNEW THREAD, fd: " << client_conn_sfd << std::endl;

    // check time period for request
    request_time = time(&raw_time);

    // get request from the client
    stat = get_request(client_conn_sfd, request);

    response_time = time(&raw_time);
    response_delay = response_time - request_time;

    // if failed to get any request, exit this thread
    if (stat == -1) { // HTTP 400, bad request
        std::string info("Invalid request, incorrect format...");
        // form andt send response to the client
        proxy_response(client_conn_sfd, "400", "Bad Request", info);
        std::cout << "\nclient_conn_sfd: " << client_conn_sfd
            << " closed -1\n" << std::endl;
        close(client_conn_sfd);
        perror("Failed to recv request\n");
#if MULTI_THREAD
        pthread_exit(0);
#else
        return NULL;
#endif
    }

    time(&curr_time);
    ptm = gmtime(&curr_time);
    // print received request info to log
    log << request_info.request_id << ": " << "\""
        << request.request_line << "\""
        << " from " << request_info.client_ip
        << " @ " << asctime(ptm);


#if CONNECT_ENABLE
    // handle CONNECT, i.e. communication via HTTPS
    if (request.method == "CONNECT") 
    {
        stat = handle_https(client_conn_sfd, request, request_info, log);
        // tunnle is closed
        if (stat != -2) 
        {
            log << request_info.request_id << ": Tunnle closed" << std::endl;
        }
    }
#else
    if (0) {}
#endif


    // handle GET and POST via HTTP
    else 
    {
        // if HTTP method is GET, try find response in cache first
        if (request.method == "GET") 
        {
            std::string header(request.data_buff.data(), request.headers_len);
            std::size_t pos = header.find("If-None-Match");

            if (pos == std::string::npos) 
            {
                pthread_mutex_lock(&mutex);
                // try to get response from proxy cache
                try 
                {
                    time_stat = proxy_cache.get(request, cached_response);
                    pthread_mutex_unlock(&mutex);
                }
                catch (std::exception& e) 
                {
                    std::cerr << "Exception caught: " << e.what() << std::endl;
                    std::cerr << "close socket and exit thread" << std::endl;
                    pthread_mutex_unlock(&mutex);
                    close(client_conn_sfd);
#if MULTI_THREAD
                    pthread_exit(0);
#else
                    return NULL;
#endif
                }

                //##################################################################################################
                if (time_stat == -1) 
                { // stat == -1 -> not in cache
                    log << request_info.request_id << ": not in cache" << std::endl;
                }
                if (time_stat > 0) 
                { // stat > 0 -> expired
                    struct tm* time_info = gmtime(&time_stat);
                    log << request_info.request_id << ": in cache, but expired at "
                        << asctime(time_info);
                }
                if (time_stat < -1) 
                { // stat < -1 -> need revalidation
                    log << request_info.request_id << ": in cache, requires validation" << std::endl;
                    int reval = revalidate(request,cached_response,-(time_stat)); //check if res has ETag
                    if(reval)
                    { //has ETag, can revalidate
                        stat = forward_request_and_get_response(client_conn_sfd, request,
                                                                response, request_info, log);
                        if(response.response_line.compare("HTTP/1.1 304 Not Modified")==0)
                        {
                            //get 304,refresh cache and send cached response back;
                            stat = return_response(client_conn_sfd, cached_response);
                            std::cout<<"revalidate304"<<std::endl;
                            if (stat == -1) 
                            {

                                std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                                    << " closed2\n" << std::endl;
                                perror("Failed to recv response");
                                close(client_conn_sfd);
                                proxy_cache.revalidate(request,request_time - response.get_age());
#if MULTI_THREAD
                                pthread_exit(0);
#else
                                return NULL;
#endif
                            }
                        }
                        else
                        { //no 304, return response
                            std::cout<<"revalidateresponse"<<std::endl;
                            stat = return_response(client_conn_sfd, response);
                            if (stat == -1) 
                            {
                                std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                                    << " closed2\n" << std::endl;
                                perror("Failed to recv response");
                                close(client_conn_sfd);
#if MULTI_THREAD
                                pthread_exit(0);
#else
                                return NULL;
#endif
                            }
                        }
                    }
                    //if no ETag, cannot revalidate, go back to common method
                }

                if (time_stat == 0) 
                { // here, stat == 0 -> valid
                    log << request_info.request_id << ": in cache, valid" << std::endl;
#if CACHE_DEBUG
                    std::cout << "cached_response:\n" << cached_response.data_buff.data() << std::endl;
#endif
                    stat = return_response(client_conn_sfd, cached_response);
                    if (stat == -1) 
                    {
                        std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                            << " closed2\n" << std::endl;
                        perror("Failed to recv response");
                        close(client_conn_sfd);
#if MULTI_THREAD
                        pthread_exit(0);
#else
                        return NULL;
#endif
                    }
                    std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                        << " closed4\n" << std::endl;
                    close(client_conn_sfd);
#if MULTI_THREAD
                    pthread_exit(0);
#else
                    return NULL;
#endif
                }
            }
        }


        // forward received request to the server and get its response
        stat = forward_request_and_get_response(client_conn_sfd, request,
                                                response, request_info, log);
        // if failed to get any request, exit this thread
        if (stat == -1) 
        { // HTTP 502, Bad Gateway
            std::string info("Connection timeout, invalid response...");
            info += request.server_name;
            // response the client with 502
            proxy_response(client_conn_sfd, "502", "Bad Gateway", info);
            log << request_info.request_id << ": Responding \""
                << request.http_type << "\" 502 Bad Gateway" << std::endl;
            std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                << " closed -2\n" << std::endl;
            perror("Failed to recv response");
            close(client_conn_sfd);
#if MULTI_THREAD
            pthread_exit(0);
#else
            return NULL;
#endif
        }
        // already responsed 400 or 404
        else if (stat == -2) 
        {
            close(client_conn_sfd);
#if MULTI_THREAD
            pthread_exit(0);
#else
            return NULL;
#endif
        }


        // CACHE RESPONSE ==================================================================================
        // need_cache give the time if the response should be cached
        // response_delay + response.get_age() is the life time
        time_stat = need_cache(response, response_delay + response.get_age());
        if (time_stat >= 0) 
        {
            pthread_mutex_lock(&mutex);
            // add response to cache
            try 
            {
                proxy_cache.set(request, request_time - response.get_age(),
                                time_stat, response, log);
                pthread_mutex_unlock(&mutex);
            }
            catch (std::exception& e) 
            {
                std::cerr << "Exception caught: " << e.what() << std::endl;
                std::cerr << "close socket and exit thread" << std::endl;
                pthread_mutex_unlock(&mutex);
                close(client_conn_sfd);
#if MULTI_THREAD
                pthread_exit(0);
#else
                return NULL;
#endif
            }

            if (time_stat == 0) 
            {
                log << request_info.request_id << ": cached, but requires re-validation"<< std::endl;
            }
            else 
            {
                time_t temp = time_stat + request_time - response.get_age();
                struct tm* time_info = gmtime(&temp);
                log << request_info.request_id << ": cached, expires at "<< asctime(time_info);
            }
        }
        else if (time_stat == -2) 
        {
            log << request_info.request_id
                << ": not cacheable because 304 Not Modified is received"
                << std::endl;
        }
        else if (time_stat == -3) {
            log << request_info.request_id
                << ": not cacheable because header indicates no-store" << std::endl;
        }
        else if (time_stat == -4) 
        {
            log << request_info.request_id
                << ": not cacheable because header indicates no-cache" << std::endl;
        }
        //==================================================================================================


        // return response back to the client
        stat = return_response(client_conn_sfd, response);
        if (stat == -1) 
        { // HTTP 502, Bad Gateway
            std::string info("Connection with client is closed...");
            info += request.server_name;
            // response the client with 502
            proxy_response(client_conn_sfd, "502", "Bad Gateway", info);
            log << request_info.request_id << ": Responding \""
                << request.http_type << "502 Bad Gateway\"" << std::endl;
            std::cout << "\nclient_conn_sfd: " << client_conn_sfd
                << " closed -3\n" << std::endl;
            perror("Failed to return response");
            close(client_conn_sfd);
#if MULTI_THREAD
            pthread_exit(0);
#else
            return NULL;
#endif
        }
        log << request_info.request_id << ": Responding \"" << response.response_line << "\"" << std::endl; 
    }

    std::cout << "\nclient_conn_sfd: " << client_conn_sfd << " closed\n" << std::endl;
    close(client_conn_sfd);
#if MULTI_THREAD
    pthread_exit(0);
#else
    return NULL;
#endif
}




/* MAIN FUNCTION **********************************************************************************/
int main (int argc, char** argv) 
{
    int proxy_sfd;
    int stat;
    //char proxy_hostname[NAME_SIZE];
    int proxy_port = PROXY_PORT;
    struct sockaddr_in proxy_addr_info;
    //struct hostent* proxy_info;

    // ignore SIGPIPE to handle tons of connections
    signal(SIGPIPE, SIG_IGN);

#if LOG_CLEAR
    std::ofstream log;
    // clear log on every run
    log.open(LOG_NAME);
    log.close();
#endif

    std::cout << "    Welcome to use our HTTP proxy\n"
        << "    the default listening port is 9090\n"
        << "    you can press Ctrl+C to exit\n"
        << "    proxy is working...\n" << std::endl;


    proxy_addr_info.sin_family = AF_INET;
    proxy_addr_info.sin_port = htons(proxy_port);
    proxy_addr_info.sin_addr.s_addr = INADDR_ANY;
    proxy_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_sfd < 0) 
    {
        perror("Cannot create socket");
    }
    int yes = 1;
    stat = setsockopt(proxy_sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
    stat = bind(proxy_sfd, (struct sockaddr*)&proxy_addr_info, sizeof(proxy_addr_info));
    if (stat < 0) 
    {
        perror ("Failed binding");
    }  
    stat = listen(proxy_sfd, MAX_CONN);
    if (stat < 0) 
    {
        perror("Failed listening");
    }
    request_id = 0;  
    // continue to 
    while (1) 
    {
        try 
        {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_conn_sfd = accept(proxy_sfd, (struct sockaddr*)&client_addr, &addr_len);
            if (client_conn_sfd == -1) {
                perror("Cannot accept connection");
                continue;
            }
            std::cout << "\n\nconnection from:\n" << inet_ntoa(client_addr.sin_addr) << std::endl;

            // HANDLED BY EACH THREAD==========================================================================
            std::cout << "locking..." << std::endl;
            pthread_mutex_lock(&mutex);
            client_conn_sfd_queue.push(client_conn_sfd);
            pthread_mutex_unlock(&mutex);

            RequestInfo request_info(request_id, inet_ntoa(client_addr.sin_addr));

#if MULTI_THREAD
            pthread_attr_t thread_attr;
            pthread_attr_init(&thread_attr);
            pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
            pthread_attr_setstacksize(&thread_attr, RES_BUFF_SIZE * MAX_NUM);
            pthread_t thread_id;
            pthread_create(&thread_id, &thread_attr, &handle_request, &request_info);
            pthread_detach(thread_id);
#else
            handle_request(&request_info);
#endif
            //=================================================================================================
            // in case there are too many requests cumulated
            if (request_id == INT_MAX) 
            {
                request_id = 0;
            }
        }
        catch (std::exception& e) 
        {
            std::cerr << "Exception caught in main thread: " << e.what() << std::endl;
            std::cerr << "close all connections and exit proxy process..." << std::endl;
            close(proxy_sfd);
            exit(0);
        }
    }
    close(proxy_sfd);

    return EXIT_SUCCESS;
}
