#!/usr/bin/python3
import requests
import os
import time
import re
from bs4 import BeautifulSoup

user_agent = 'Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/54.0.2840.87 Safari/537.36'
headers = { 'Accept':'text/css,*/*;q=0.1',
'Accept-Encoding':'gzip, deflate, sdch, br',
'Accept-Language':'zh-CN,zh;q=0.8',
'Cache-Control':'max-age=0',
'Connection':'keep-alive',
'User-Agent' : user_agent ,
}

# url = 'http://www.27270.com/beautiful/dongmantupian/2016/170070.html'
# url='http://www.27270.com/beautiful/dongmantupian/'
# url = 'http://www.27270.com/'
#url='https://www.2222zv.com/html/artdata/?id=33434'
urls=[]
num = 10
def get_url(url):
    pos = url.rfind('/')
    if pos == -1:
        return
    url = url[:pos]
    return url +'/'

def get_response(url):
    res = requests.get(url,headers=headers)
    res.encoding=res.apparent_encoding
    return BeautifulSoup(res.text,'html.parser')

def get_context(url):
    res = requests.get(url,headers=headers)
    res.encoding=res.apparent_encoding
    return res.text

def data_name():
    data=now = time.strftime("%Y-%m-%d-%H_%M_%S",time.localtime(time.time()))
    return data

def down_img(url,alt):
    img = requests.get(url,headers=headers)
    img_name = data_name() + alt + '.jpg'
    time.sleep(0.5)
    with open(img_name,'wb') as f:
        f.write(img.content)
        f.close()

def get_img(url):
    soup = get_response(url)
    img_url = soup.find('img').get("src")
    img_alt = soup.find('img').get('alt')
    down_img(img_url,img_alt)

def next_img(url):
    soup = get_response(url)
    nexts = soup.find_all('ul',class_='articleV4Page l')
    for i in nexts:
        href = i.find_all('a')
        for txt in href:
            ss = txt.get('href')
            if type(ss) != str:
                continue
            if ss in urls:
                continue
            urls.append(ss)
            if ss.endswith(".html"):
                print("*"*50)
                rl = get_url(url)
                rl = rl + ss
                get_img(rl)

def dongman_img(url):
    soup = get_response(url)
    url_list = soup.find_all('ul',class_='pic_list')
    for i in url_list:
        href = i.find_all('a')
        for txt in href:
            ss = txt.get('href')
            if ss.endswith(".html"):
                print("*"*30)
                print(ss)
                print("*"*30)
                get_img(ss)
                next_img(ss)

def home_url(url):
    soup = get_response(url)
    nen_list = soup.find_all('li',class_='NenuLi')
    print(len(nen_list))
    time.sleep(2)

    for url_list in nen_list:
        url_list = url_list.find_all('div',class_='ShowNav')
        for urls in url_list:
            print(urls)
        print('*'*50)
        print(len(url_list))

def meinv_url(url):
    soup = get_response(url)
    first_img_url = soup.find_all('div')
    print(first_img_url)
    # lists = soup.find_all('div',class_='cont')
    # print(lists)
    # print(len(lists))
    # for i in lists:
        # print(ss)


def main():
    # home_url(url)
    # dongman_img(url)
    # meinv_url(url)
    print(get_context(url))


if __name__ == "__main__":
    main()
