import requests
import re
import json
from urllib.parse import urlencode
from bs4 import BeautifulSoup

lyric_url = 'https://c.y.qq.com/lyric/fcgi-bin/fcg_query_lyric.fcg?'
music_url = 'https://u.y.qq.com/cgi-bin/musicu.fcg?'

header = {
    "authority":       "c.y.qq.com",
    "referer": "https://y.qq.com/",
    "user-agent":      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.81 Safari/537.36",
}

music_header = {
    "authority":       "u.y.qq.com",
    "referer": "https://y.qq.com/portal/player.html",
    "user-agent":      "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.81 Safari/537.36",
}

def get_lyric_params(music_id):
    params = {
        "nobase64": "1",
        "musicid": music_id,
        "callback": "jsonp1",
        "g_tk": "5381",
        "jsonpCallback": "jsonp1",
        "loginUin": "0",
        "hostUin": "0",
        "format": "jsonp",
        "inCharset": "utf8",
        "outCharset": "utf-8",
        "notice": "0",
        "platform": "yqq",
        "needNewCode": "0",
    }
    return params

def get_music_data():
    data = {
        "req":{
            "module":"CDN.SrfCdnDispatchServer",
            "method":"GetCdnDispatch",
            "param":{
                "guid":"7335234349",
                "calltype":0,
                "userip":""
            }
        },

        "req_0":{
            "module":"vkey.GetVkeyServer",
            "method":"CgiGetVkey",
            "param":{
                "guid":"7335234349",
                "songmid":["0032ZOkm0LBgHW"],
                "songtype":[0],
                "uin":"0",
                "loginflag":1,
                "platform":"20"
            }
        },
        "comm":{
            "uin":0,
            "format":"json",
            "ct":20,
            "cv":0
        }
    }
    return data

def get_music_params():
    params = {
        "callback": "getplaysongvkey9350453401709924",
        "g_tk": "5381",
        "jsonpCallback": "getplaysongvkey9350453401709924",
        "loginUin": "0",
        "hostUin": "0",
        "format": "jsonp",
        "inCharset": "utf8",
        "outCharset": "utf-8",
        "notice": "0",
        "platform": "yqq",
        "needNewCode": "0",
        "data":'data: {"req":{"module":"CDN.SrfCdnDispatchServer","method":"GetCdnDispatch","param":{"guid":"7335234349","calltype":0,"userip":""}},"req_0":{"module":"vkey.GetVkeyServer","method":"CgiGetVkey","param":{"guid":"7335234349","songmid":["0032ZOkm0LBgHW"],"songtype":[0],"uin":"0","loginflag":1,"platform":"20"}},"comm":{"uin":0,"format":"json","ct":20,"cv":0}}',
    }
    return params


def get_lyric(html):
    res = json.loads(html.text.lstrip('jsonp1(').rstrip(')'))
    key = 'lyric'
    if key in res:
        lyric = json.loads(html.text.lstrip('jsonp1(').rstrip(')'))['lyric']
        dr1 = re.compile(r'&#\d.;',re.S)
        dr2 = re.compile(r'\[\d+\]',re.S)
        dd = dr1.sub(r'',lyric)
        dd = dr2.sub(r'\n',dd).replace('\n\n','\n')
        print(dd)

def get_html(url,header,params):
    url = url + urlencode(params)
    html = requests.get(url,headers=header)
    html.encoding=html.apparent_encoding
    return html

def get_lyric_request(url,music_id):
    params = get_lyric_params(music_id)
    html = get_html(url,header,params)
    get_lyric(html)

def get_music_resquest(url):
    params = get_music_params()
    html = get_html(url,music_header,params)
    print(html.text)



def main():
    # lyric_id = "216514336"
    # get_lyric_request(lyric_url,lyric_id)
    get_music_resquest(music_url)

if __name__ == "__main__":
   main()
